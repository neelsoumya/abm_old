
/************************************************************************
 * 									*
 * Copyright (C) 2004  Christina Warrender				*
 * 									*
 * This file is part of CyCells.					*
 *									*
 * CyCells is free software; you can redistribute it and/or modify it 	*
 * under the terms of the GNU General Public License as published by	*
 * the Free Software Foundation; either version 2 of the License, or	*
 * (at your option) any later version.					*
 *									*
 * CyCells is distributed in the hope that it will be useful,		*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	*
 * GNU General Public License for more details.				*
 *									*
 * You should have received a copy of the GNU General Public License	*
 * along with CyCells; if not, write to the Free Software Foundation, 	*
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA	*
 *									*
 ************************************************************************/
/************************************************************************
 * file molecule.cc                                                     *
 * Routines for Molecule class                                          *
 * Storage and behavior for all molecules of one type                   *
 ************************************************************************/

#include "molecule.h"
#include <iostream>
#include <string>
#include "simPoint.h"
#include "random.h"
#include "array3D.h"

using namespace std;

// static geometry info - accessed many times; quicker to set/calculate once
int Molecule::sm_xsize;
int Molecule::sm_ysize;
int Molecule::sm_zsize;
int Molecule::sm_gridsize;
int Molecule::sm_size;
int Molecule::sm_gridsq;
double Molecule::sm_invNavVol;

/************************************************************************ 
 * setGeometry()                                                        *
 *   Static routine to set geometry info mentioned above                *
 *                                                                      *
 * Parameters                                                           *
 *   int xrange, yrange, zrange:  size in microns in each dimension	*
 *   int gridsize:		length of grid cell edge in microns     *
 *                              0 means just use one grid cell		*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::setGeometry(int xrange, int yrange, int zrange, int gridsize)
{
  assert(xrange>0); assert(yrange>0); assert(zrange>0); assert(gridsize>=0);

  sm_gridsize = gridsize; 

  if (gridsize)		// could still be single grid cell
  {
    sm_xsize = xrange/gridsize; 
    sm_ysize = yrange/gridsize; 
    sm_zsize = zrange/gridsize;
    sm_size = sm_xsize*sm_ysize*sm_zsize;
    sm_gridsq = gridsize*gridsize;

    // precalculate 1 / (Nav*gridVol) for changeConc:
    // gridsize specified in microns - volume of one grid cell in ml is
    //	m_gridsize^3 * 1E-12 
    double temp = 6.022E11 * sm_gridsize*sm_gridsize*sm_gridsize;
    sm_invNavVol = 1 / temp;
    assert(sm_invNavVol>0);
  }
  else		// molecular concentration homeogeneous
  {
    sm_xsize = sm_ysize = sm_zsize = sm_size = 1;

    // still need 1 / (Nav*gridVol) for changeConc:
    // volume of sim space in ml is
    // xrange*yrange*zrange* 1E-12 
    double temp = 6.022E11 * xrange * yrange * zrange;
    sm_invNavVol = 1 / temp;
    assert(sm_invNavVol>0);
  }

  cout << "set Molecule geometry:  " << sm_xsize << "x" << sm_ysize
       << "x" << sm_zsize << "=" << sm_size << endl; 
}

/************************************************************************ 
 * Molecule()                                                           *
 *   Default constructor                                                *
 *                                                                      *
 * Parameters                                                           *
 *   const string& title:       name for this molecule                  *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
Molecule::Molecule(const string& title) : m_name(title), 
	m_diffusionRate(0), m_decayRate(0) 
{
  initialize();
}

/************************************************************************ 
 * Molecule()                                                           *
 *   Constructor; sets name, diffusion rate and decay rate              *
 *                                                                      *
 * Parameters                                                           *
 *   const string& title;       name for this molecule                  *
 *   double diff:               diffusion rate (microns^2/sec)          *
 *   double decay:               decay rate (%/sec)  		        *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
Molecule::Molecule(const string& title, double diff, double decay) 
	: m_name(title), m_diffusionRate(diff), m_decayRate(decay) 
{
  assert(diff>=0);
  initialize();
}

/************************************************************************ 
 * initialize()                                                         *
 *   Allocates memory for concentration arrays and assigns all values   *
 *   to 0.  Use setUniformConc to set all values to some non-zero       *
 *   concentration.                                                     *
 *   Allocates enough memory for sim volume + one layer of guard grid   *
 *   cells for each edge                                                *
 *                                                                      *
 * Parameters                                                           *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::initialize()
{
  assert(sm_size);

  try {
    m_concentration.resize(sm_xsize+2, sm_ysize+2, sm_zsize+2);
    m_deltaConc.resize(sm_xsize+2, sm_ysize+2, sm_zsize+2);   // for updates
  }
  catch(std::bad_alloc&) {
    cerr << "not enough memory to set up molecular concentration data" 
	 << endl;
    abort();
  }

  m_concentration.setAll(0);
  m_deltaConc.setAll(0);
}

/************************************************************************ 
 * setUniformConc()                                                     *
 *   sets concentration in all grid cells to values passed in           *
 *   with optional addition of Gaussian noise                           *
 *                                                                      *
 * Parameters                                                           *
 *   Conc amount:               moles/ml                                *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::setUniformConc(Conc amount, double stddev /* = 0 */)
{
  assert(amount>=0);

  Conc c;

  if (stddev)
  {
    for (int i=1; i<=sm_xsize; i++)
      for (int j=1; j<=sm_ysize; j++)
        for (int k=1; k<=sm_zsize; k++)
	{
	  while ( (c = sampleGaussian(amount,stddev)) < 0 )
	    cout << "Molecule::setUniformConc warning:  " 
		 << "sample gave negative concentration" << endl;
	  m_concentration.at(i,j,k) = c;
	}
    // set all guard layers
    setGuards();
    setGuardCorners();
  }
  else	// setAll automatically sets guard layers as well as 'real' values
    m_concentration.setAll(amount);
}

/************************************************************************ 
 * initFromFile()                                                       *
 *   reads concentrations for all grid cells from already-open file     *
 *                                                                      *
 * Parameters                                                           *
 *   ifstream infile:		file containing concentration data      *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::initFromFile(ifstream &infile)
{
  for (int i=1; i<=sm_xsize; i++)
    for (int j=1; j<=sm_ysize; j++)
      for (int k=1; k<=sm_zsize; k++)
	infile >> m_concentration.at(i,j,k);

  // set all guard layers
  setGuards();
  setGuardCorners();
}

/************************************************************************ 
 * changeConc()                                                         *
 *   Adds or subtracts a specified number of molecules at a specified   *
 *   location           						*
 *   - for secretion or binding by cells                                *
 *                                                                      *
 * Parameters                                                           *
 *   double amount:   	        #of molecules to add/subtract        	* 
 *   const SimPoint &p:         grid location                       	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::changeConc(double amount, const SimPoint &p)
{
  // find indices of grid cell to change - nearest grid point to p
  int xi=1, yi=1, zi=1;		// default - only one grid cell
  if (sm_gridsize)
  {
    assert(p.getX()>=0); assert(p.getX()<sm_gridsize*sm_xsize);
    assert(p.getY()>=0); assert(p.getY()<sm_gridsize*sm_ysize);
    assert(p.getZ()>=0); assert(p.getZ()<sm_gridsize*sm_zsize);
    xi = int(p.getX()/sm_gridsize+1);
    yi = int(p.getY()/sm_gridsize+1);
    zi = int(p.getZ()/sm_gridsize+1);
  }

  // amount passed in should be #molecules:
  // convert to moles/ml based on volume of grid cell and add to conc
  // want amount/(N_AV*volume) - denominator precalculated in setGeometry
  // and inverted
  Conc change = amount * sm_invNavVol;
  m_concentration.at(xi, yi, zi) += change;
  assert(m_concentration.at(xi, yi, zi) >= 0);

  // fix guard layers
  if (sm_gridsize)
    setSpecificGuards(xi, yi, zi);

// the interpolation tried below causes problems if we're subtracting
// molecules rather than adding them - stick to simpler scheme for now
//  if (sm_size == 1)	// no interpolation necessary 
//  {
//    m_concentration.at(1,1,1) += change;
//    assert(m_concentration.at(1,1,1) >= 0);
//  }
//  else
//  {
    // distribute change among 8 nearest neighbor grid points
    // find 'fractional indices' that interpolation routine uses
//    double fix = p.getX()/sm_gridsize + 0.5;
//    double fiy = p.getY()/sm_gridsize + 0.5;
//    double fiz = p.getZ()/sm_gridsize + 0.5;

    // indices for lower-bound stored value
//    int xi = int(fix); int yi = int(fiy); int zi = int(fiz);
  
    // interpolation parameters in each dimension
//    double fx = fix - xi; double fy = fiy - yi; double fz = fiz - zi;
  
    // Calculate the change to each of the neighboring grid cells  
//    if (sm_zsize == 1)		// need only 2D interpolation
//    {
//      addConc(xi, yi, zi, (1-fx)*(1-fy)*change);
//      addConc(xi+1, yi, zi, fx*(1-fy)*change);
//      addConc(xi+1, yi+1, zi, fx*fy*change);
//      addConc(xi, yi+1, zi, (1-fx)*fy*change);
//    }
//    else
//    {
//      addConc(xi, yi, zi, (1-fx)*(1-fy)*(1-fz)*change);
//      addConc(xi+1, yi, zi, fx*(1-fy)*(1-fz)*change);
//      addConc(xi+1, yi+1, zi, fx*fy*(1-fz)*change);
//      addConc(xi, yi+1, zi, (1-fx)*fy*(1-fz)*change);
//      addConc(xi, yi, zi+1, (1-fx)*(1-fy)*fz*change);
//      addConc(xi+1, yi, zi+1, fx*(1-fy)*fz*change);
//      addConc(xi+1, yi+1, zi+1, fx*fy*fz*change);
//      addConc(xi, yi+1, zi+1, (1-fx)*fy*fz*change);
//    }
//  }	// end else need to interpolate
}

/************************************************************************ 
 * addConc                                                              *
 *    checks whether (i,j,k) is a guard cell, changes corresponding     *
 * 'real' cell if so							*
 *  Then updates all relevant guard cells                               *
 *                                                                      *
 * Parameters                                                           *
 *   int i, j, k: 	location of grid cell to change              	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::addConc(int i, int j, int k, Conc change)
{
  // get current amount, add change (which may be negative), 
  // verify that result is >=0
  Conc c = m_concentration.at(i,j,k) + change;
  assert(c >= 0);

  // check whether this is a guard layer, and update corresponding
  // center grid cell
  int ti, tj, tk;		// target cell to change
  if (i == 0) ti = sm_xsize;
  else if (i == sm_xsize+1) ti = 1;
  else ti = i;
  if (j == 0) tj = sm_ysize;
  else if (j == sm_ysize+1) tj = 1;
  else tj = j;
  if (k == 0) tk = sm_zsize;
  else if (k == sm_zsize+1) tk = 1;
  else tk = k;

  m_concentration.at(ti, tj, tk) = c;
 
  // now set guard layers to reflect change
  setSpecificGuards(ti, tj, tk);
}

/************************************************************************ 
 * decay()                                                              *
 *   Calculates changes in molecular concentration due to exponential	*
 *   decay only - no diffusion						*
 *   Periodic boundary conditions.                                      *
 *                                                                      *
 * Parameters                                                           *
 *   double deltaT:              duration of time step in seconds	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::decay(double deltaT)
{ 
  int i,j,k;
  Conc current;

  double decay_factor = m_decayRate*deltaT;
  assert (decay_factor < 1);

  // update all grid cells - including guard layers 
  for (i=0; i<=sm_xsize+1; i++)
    for (j=0; j<=sm_ysize+1; j++)
      for (k=0; k<=sm_zsize+1; k++)
      {
	current = m_concentration.at(i,j,k);
        m_concentration.at(i,j,k) = current - decay_factor*current;
        assert(m_concentration.at(i,j,k)>=0);
      }
}

/************************************************************************ 
 * explicitDecayDiff2D()                                                *
 *   Calculates changes in molecular concentration due to exponential	*
 *   decay and diffusion 						*
 *   This is an explicit method for solving diffusion equation -        *
 *   assumes calling routine has chosen time step appropriately         *
 *   Periodic boundary conditions.                                      *
 *   This version assumes zsize == 1                                    *
 *                                                                      *
 * Parameters                                                           *
 *   double deltaT:              duration of time step in seconds	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::explicitDecayDiff2D(double deltaT)
{ 
  assert(sm_zsize=1);

  int i,j;
  Conc sum, current;

  // premultiply constants, instead of in loop
  // if deltaT is always the same, can do this in routines that set
  // diffusion & decay rates, but for now, assume it may vary
  double decay_factor = m_decayRate*deltaT;
  assert (decay_factor < 1);
  double diff_factor = m_diffusionRate*deltaT/sm_gridsq;

  // calculate all the changes before applying any to avoid
  // changing values before using them
  for (i=1; i<=sm_xsize; i++)
    for (j=1; j<=sm_ysize; j++)
      {
	current = m_concentration.at(i,j,1);
        m_deltaConc.at(i,j,1) = -decay_factor*current;
	sum = 0;
	sum += m_concentration.at(i-1,j,1) - current;
	sum += m_concentration.at(i+1,j,1) - current;
	sum += m_concentration.at(i,j-1,1) - current;
	sum += m_concentration.at(i,j+1,1) - current;
        m_deltaConc.at(i,j,1) += diff_factor*sum;
      }

  // now apply changes 
  for (i=1; i<=sm_xsize; i++)
    for (j=1; j<=sm_ysize; j++)
      {
        m_concentration.at(i,j,1) = 
		m_concentration.at(i,j,1) + m_deltaConc.at(i,j,1);
        assert(m_concentration.at(i,j,1)>=0);
      }

  // update guard layers 
  setGuards();
  setGuardCorners();
}

/************************************************************************ 
 * explicitDecayDiff3D()                                                *
 *   Calculates changes in molecular concentration due to exponential	*
 *   decay and diffusion 						*
 *   This is an explicit method for solving diffusion equation -        *
 *   assumes calling routine has chosen time step appropriately         *
 *   Periodic boundary conditions.                                      *
 *                                                                      *
 * Parameters                                                           *
 *   double deltaT:              duration of time step in seconds	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::explicitDecayDiff3D(double deltaT)
{ 
  // premultiply constants, instead of in loop
  // if deltaT is always the same, can do this in routines that set
  // diffusion & decay rates, but for now, assume it may vary
  double decay_factor = m_decayRate*deltaT;
  assert (decay_factor < 1);
  double diff_factor = m_diffusionRate*deltaT/sm_gridsq;

  // calculate all the changes before applying any to avoid
  // changing values before using them
  int i,j,k;
  Conc sum, current;
  for (i=1; i<=sm_xsize; i++)
    for (j=1; j<=sm_ysize; j++)
      for (k=1; k<=sm_zsize; k++)
      {
	current = m_concentration.at(i,j,k);
        m_deltaConc.at(i,j,k) = -decay_factor*current;
	sum = 0;
	sum += m_concentration.at(i-1,j,k) - current;
	sum += m_concentration.at(i+1,j,k) - current;
	sum += m_concentration.at(i,j-1,k) - current;
	sum += m_concentration.at(i,j+1,k) - current;
	sum += m_concentration.at(i,j,k-1) - current;
	sum += m_concentration.at(i,j,k+1) - current;
        m_deltaConc.at(i,j,k) += diff_factor*sum;
      }

  // now apply changes 
  for (i=1; i<=sm_xsize; i++)
    for (j=1; j<=sm_ysize; j++)
      for (k=1; k<=sm_zsize; k++)
      {
        m_concentration.at(i,j,k) = 
		m_concentration.at(i,j,k) + m_deltaConc.at(i,j,k);
        assert(m_concentration.at(i,j,k)>=0);
      }

  // update guard layers
  setGuards();
  setGuardCorners();  
}

/************************************************************************ 
 * setGuards()                                                          *
 *   Copies concentration data from each 'edge' of array to the guard   *
 *   layer on the opposite 'edge'.  This routine does not set values    *
 *   in the array 'corners' (such as at (0,0,0).                        *
 *                                                                      *
 * Parameters                                                           *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::setGuards()           
{
  int i,j,k;
  for (j=1; j<=sm_ysize; j++)
    for (k=1; k<=sm_zsize; k++)
    {
      m_concentration.at(0,j,k) = m_concentration.at(sm_xsize,j,k);
      m_concentration.at(sm_xsize+1,j,k) = m_concentration.at(1,j,k);
    }
  for (i=1; i<=sm_xsize; i++)
    for (k=1; k<=sm_zsize; k++)
    {
      m_concentration.at(i,0,k) = m_concentration.at(i,sm_ysize,k);
      m_concentration.at(i,sm_ysize+1,k) = m_concentration.at(i,1,k);
    }
  for (i=1; i<=sm_xsize; i++)
    for (j=1; j<=sm_ysize; j++)
    {
      m_concentration.at(i,j,0) = m_concentration.at(i,j,sm_zsize);
      m_concentration.at(i,j,sm_zsize+1) = m_concentration.at(i,j,1);
    }
}

/************************************************************************ 
 * setGuardCorners()                                                    *
 *   Copies concentration data to the corner guard layers missed by     *
 *   setGuards.                                                         *
 *                                                                      *
 * Parameters                                                           *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::setGuardCorners()           
{
 // four edges in x direction
 for (int i=1; i<=sm_xsize; i++)
 {
   m_concentration.at(i,0,0) = m_concentration.at(i,sm_ysize,sm_zsize);
   m_concentration.at(i,0,sm_zsize+1) = m_concentration.at(i,sm_ysize,1);
   m_concentration.at(i,sm_ysize+1,0) = m_concentration.at(i,1,sm_zsize);
   m_concentration.at(i,sm_ysize+1,sm_zsize+1) = m_concentration.at(i,1,1);
 }

 // four edges in y direction
 for (int j=1; j<=sm_ysize; j++)
 {
   m_concentration.at(0,j,0) = m_concentration.at(sm_xsize,j,sm_zsize);
   m_concentration.at(0,j,sm_zsize+1) = m_concentration.at(sm_xsize,j,1);
   m_concentration.at(sm_xsize+1,j,0) = m_concentration.at(1,j,sm_zsize);
   m_concentration.at(sm_xsize+1,j,sm_zsize+1) = m_concentration.at(1,j,1);
 }

 // four edges in z direction
 for (int k=1; k<=sm_zsize; k++)
 {
   m_concentration.at(0,0,k) = m_concentration.at(sm_xsize,sm_ysize,1);
   m_concentration.at(0,sm_ysize+1,k) = m_concentration.at(sm_xsize,1,k);
   m_concentration.at(sm_xsize+1,0,k) = m_concentration.at(1,sm_ysize,1);
   m_concentration.at(sm_xsize+1,sm_ysize+1,k) = m_concentration.at(1,1,k);
 }

 // eight corners
 m_concentration.at(0,0,0) = m_concentration.at(sm_xsize,sm_ysize,sm_zsize);
 m_concentration.at(0,0,sm_zsize+1) = m_concentration.at(sm_xsize,sm_ysize,1);
 m_concentration.at(0,sm_ysize+1,0) = m_concentration.at(sm_xsize,1,sm_zsize);
 m_concentration.at(0,sm_ysize+1,sm_zsize+1)=m_concentration.at(sm_xsize,1,1);
 m_concentration.at(sm_xsize+1,0,0) = m_concentration.at(1,sm_ysize,sm_zsize);
 m_concentration.at(sm_xsize+1,0,sm_zsize+1)=m_concentration.at(1,sm_ysize,1);
 m_concentration.at(sm_xsize+1,sm_ysize+1,0)=m_concentration.at(1,1,sm_zsize);
 m_concentration.at(sm_xsize+1,sm_ysize+1,sm_zsize+1)=m_concentration.at(1,1,1);
}

/************************************************************************ 
 * setSpecificGuards()                                                  *
 *   Copies concentration data from a newly-changed array location      *
 *   to the appropriate guard layer locations (up to 7)                 *
 *                                                                      *
 * Parameters                                                           *
 *   int i,j,k:		Indices of changed concentration                *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::setSpecificGuards(int i, int j, int k)           
{
  assert(i>=1 && i<=sm_xsize && j>=1 && j<=sm_ysize && k>=1 && k<=sm_zsize);

  Conc original = m_concentration.at(i,j,k);

  // there must be a better way!
  if (i == 1) {
    m_concentration.at(sm_xsize+1, j, k) = original;      		// i
    if (j == 1) {
      m_concentration.at(sm_xsize+1, sm_ysize+1, k) = original; 	// i&j 
      if (k == 1) {
        m_concentration.at(i, sm_ysize+1, sm_zsize+1) = original; 	// j&k
        m_concentration.at(sm_xsize+1,sm_ysize+1,sm_zsize+1) = original; // all
	}	// end inner if k==1
      if (k == sm_zsize) {
        m_concentration.at(i, sm_ysize+1, 0) = original; 	// j&k
        m_concentration.at(sm_xsize+1,sm_ysize+1,0) = original; // all
	}	// end inner if k==zsize
      }		// end if j==1
    if (j == sm_ysize) {
      m_concentration.at(sm_xsize+1, 0, k) = original; 			// i&j 
      if (k == 1) {
        m_concentration.at(i, 0, sm_zsize+1) = original; 		// j&k
        m_concentration.at(sm_xsize+1, 0, sm_zsize+1) = original; 	// all
	}	// end inner if k==1
      if (k == sm_zsize) {
        m_concentration.at(i, 0, 0) = original; 		// j&k
        m_concentration.at(sm_xsize+1, 0, 0) = original; 	// all
	}	// end inner if k==zsize
      }		// end if j==ysize
    if (k == 1) 
      m_concentration.at(sm_xsize+1, j, sm_zsize+1) = original; 	// i&k
    if (k == sm_zsize) 
      m_concentration.at(sm_xsize+1, j, 0) = original; 	// i&k
    }	// end if i==1
  if (i == sm_xsize) {
    m_concentration.at(0, j, k) = original;      			// i
    if (j == 1) {
      m_concentration.at(0, sm_ysize+1, k) = original; 			// i&j 
      if (k == 1) {
        m_concentration.at(i, sm_ysize+1, sm_zsize+1) = original; 	// j&k
        m_concentration.at(0, sm_ysize+1, sm_zsize+1) = original; 	// all
	}	// end inner if k==1
      if (k == sm_zsize) {
        m_concentration.at(i, sm_ysize+1, 0) = original; 	// j&k
        m_concentration.at(0, sm_ysize+1, 0) = original; 	// all
	}	// end inner if k==zsize
      }		// end if j==1
    if (j == sm_ysize) {
      m_concentration.at(0, 0, k) = original;	 			// i&j 
      if (k == 1) {
        m_concentration.at(i, 0, sm_zsize+1) = original; 		// j&k
        m_concentration.at(0, 0, sm_zsize+1) = original; 		// all
	}	// end inner if k==1
      if (k == sm_zsize) {
        m_concentration.at(i, 0, 0) = original; 		// j&k
        m_concentration.at(0, 0, 0) = original; 		// all
	}	// end inner if k==zsize
      }		// end if j==ysize
    if (k == 1) 		// i still xsize, but j has middle value
      m_concentration.at(0, j, sm_zsize+1) = original; 			// i&k
    if (k == sm_zsize) 
      m_concentration.at(0, j, 0) = original; 		// i&k
    }	// end if i==xsize
  if (j == 1)  { 		// but i may have a middle value
    m_concentration.at(i, sm_ysize+1, k) = original;   			// j
    if (k == 1) 
      m_concentration.at(i, sm_ysize+1, sm_zsize+1) = original;		// j&k
    if (k == sm_zsize) 
      m_concentration.at(i, sm_ysize+1, 0) = original; 		// j&k
    }		// end if j==1
  if (j == sm_ysize) {	
    m_concentration.at(i, 0, k) = original;       		// j
    if (k == 1) 
      m_concentration.at(i, 0, sm_zsize+1) = original;		// j&k
    if (k == sm_zsize) 
      m_concentration.at(i, 0, 0) = original; 			// j&k
    }		// end if j==ysize
  if (k == 1) 		
    m_concentration.at(i, j, sm_zsize+1) = original;  		// k
  if (k == sm_zsize) 	
    m_concentration.at(i, j, 0) = original;  			// k
}

/************************************************************************ 
 * update()                                                             *
 *   Eventually designed to implement all changes to molecules during   *
 *   one timestep:  molecular diffusion, decay, (maybe reactions later) *
 *   This assumes an explicit method for solving the diffusion         *
 *   equation and runs calculation multiple times at the appropriate    *
 *   time steps if necessary.                                           *
 *                                                                      *
 * Parameters                                                           *
 *   double deltaT:              duration of time step in seconds	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::update(double deltaT)
{
  if ( !m_diffusionRate||sm_size==1 ) 	// don't need diffusion
    if (!m_decayRate)
      return;			// nothing to update
    else
      decay(deltaT);		// decay only
  else
  { // diffusion
    // check time step against diffusion rate, choose appropriate number
    // of iterations for explicit method
    // stability requirement:  deltaT <= (deltaX)^2/(2*numdim*D)
    int i, num_time_steps;

    if (sm_zsize==1)	// essentially 2D
    {
      num_time_steps = int(4*m_diffusionRate*deltaT/sm_gridsq) + 1;
      for (i=0; i<num_time_steps; i++)
        explicitDecayDiff2D(deltaT/num_time_steps);
    }
    else		// full 3D
    {
      num_time_steps = int(6*m_diffusionRate*deltaT/sm_gridsq) + 1;
      for (i=0; i<num_time_steps; i++)
        explicitDecayDiff3D(deltaT/num_time_steps);
    }
  }
}

/************************************************************************ 
 * getConc()                                                            *
 *   Returns the molecular concentration at a specific location         *
 *   This version uses the simplest 'interpolation' - concentration 	*
 *   is uniform within each grid cell, so we just find the grid cell    *
 *   that contains the specified location (the nearest known point)     *
 *                                                                      *
 * Parameters                                                           *
 *   const SimPoint &p:         grid location                       	*
 *                                                                      *
 * Returns - concentration                                              *
 ************************************************************************/
Molecule::Conc Molecule::getConc(const SimPoint &p) const
{
  // find indices of grid cell to change - nearest grid point to p
  int xi=1, yi=1, zi=1;		// default - only one grid cell
  if (sm_gridsize)
  {
    assert(p.getX()>=0); assert(p.getX()<sm_gridsize*sm_xsize);
    assert(p.getY()>=0); assert(p.getY()<sm_gridsize*sm_ysize);
    assert(p.getZ()>=0); assert(p.getZ()<sm_gridsize*sm_zsize);
    xi = int(p.getX()/sm_gridsize+1);
    yi = int(p.getY()/sm_gridsize+1);
    zi = int(p.getZ()/sm_gridsize+1);
  }

  return m_concentration.at( xi, yi, zi );
}

/************************************************************************ 
 * getInterpConc()                                                      *
 *   Returns the molecular concentration at a specific location.        *
 *   Relies on linear interpolation function in Array3D.                *
 *   This version not const because it needs to set guard layers.       *
 *                                                                      *
 * Parameters                                                           *
 *   const SimPoint &p:         grid location                       	*
 *                                                                      *
 * Returns - concentration                                              *
 ************************************************************************/
Molecule::Conc Molecule::getInterpConc(const SimPoint &p) const
{
  // Concentrations used in calculating gradients may actually
  // be up to half a grid cell outside of normal sim boundaries
  // interpolation valid for points from -gridsize/2 to range+gridsize/2
  // (we're assuming concentrations stored represent values at the centers
  // of the indexed grid cells, but interpolation uses those points as
  // 'corners')
  double halfgrid = 0.5*sm_gridsize;
  assert(p.getX()>=-halfgrid); assert(p.getX()<sm_gridsize*sm_xsize+halfgrid);
  assert(p.getY()>=-halfgrid); assert(p.getY()<sm_gridsize*sm_ysize+halfgrid);
  assert(p.getZ()>=-halfgrid); assert(p.getZ()<sm_gridsize*sm_zsize+halfgrid);

  // if only 1 grid cell, no reason to interpolate
  if (sm_size == 1)
    return m_concentration.at(1, 1, 1);

  // find 'fractional indices' that interpolation routine uses
  // given that indices of (-halfgrid, -halfgrid, -halfgrid) are (0,0,0)
  double fix = p.getX()/sm_gridsize + 0.5;
  double fiy = p.getY()/sm_gridsize + 0.5;
  double fiz = p.getZ()/sm_gridsize + 0.5;

  return m_concentration.interpolate(fix, fiy, fiz);
}

/************************************************************************ 
 * getAvgConc()                                                         *
 *   Calculates average concentration over all indices                  *
 *                                                                      *
 * Parameters - none                                                    *
 *                                                                      *
 * Returns - average concentration for tissue (moles/ml)                *
 ************************************************************************/
Molecule::Conc Molecule::getAvgConc() const
{
  Molecule::Conc total=0.0;
  Molecule::Conc average;

  for (int i=1; i<=sm_xsize; i++)
    for (int j=1; j<=sm_ysize; j++)
      for (int k=1; k<=sm_zsize; k++)
        total += m_concentration.at(i,j,k);

  average = total/sm_size;		

  return average;
}

/************************************************************************ 
 * getNumMolecules()                                                    *
 *   Returns number of molecules within the grid cell containing the    *
 *   specified location                                                 *
 *                                                                      *
 * Parameters -                                                         *
 *   double volume:		volume in milliliters                   *
 *   const SimPoint &pos:	center point of volume                  *
 *                                                                      *
 * Returns - number of molecules (int)                                  * 
 ************************************************************************/
int Molecule::getNumMolecules(double volume, const SimPoint &pos) const
{
  // determine concentration at specified point;
  // convert moles/ml to number of molecules
  // multiply by N_AV and volume specified
  return int( 6.022E23 * getConc(pos) * volume );
}

/************************************************************************ 
 * getGradient()                                                        *
 *   Returns the concentration gradient at a specific location          *
 *   This version samples the concentration at a specified distance r   *
 *   in each direction (6) from the specified point.			*
 *                                                                      *
 * Parameters                                                           *
 *   const SimPoint &p:         grid location                       	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
SimPoint Molecule::getGradient(const SimPoint &pos, double r) const
{
  // if there is only 1 grid cell, gradient always 0
  if (sm_size == 1) 
    return (SimPoint(0,0,0));

  assert(pos.getX()>=0); assert(pos.getX()<sm_gridsize*sm_xsize);
  assert(pos.getY()>=0); assert(pos.getY()<sm_gridsize*sm_ysize);
  assert(pos.getZ()>=0); assert(pos.getZ()<sm_gridsize*sm_zsize);
  assert(r>0); assert(r<=sm_gridsize/2.0);

  Conc x = getInterpConc( pos + SimPoint(r,0,0) ) 
	- getInterpConc( pos + SimPoint(-r,0,0) );
  Conc y = getInterpConc( pos + SimPoint(0,r,0) ) 
	- getInterpConc( pos + SimPoint(0,-r,0) );
  Conc z = 0;
  if (sm_zsize > 1)
    z = getInterpConc( pos + SimPoint(0,0,r) )
	- getInterpConc( pos + SimPoint(0,0,-r) );

  double scale = 1.0 / (2.0*r);
  return (SimPoint(x,y,z)*scale);
}

/************************************************************************ 
 * printConc()                                                          *
 *   Prints list of 3D indices and associated concentrations            *
 *                                                                      *
 * Parameters - none                                                    *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::printConc() const
{
  for (int i=1; i<=sm_xsize; i++)
    for (int j=1; j<=sm_ysize; j++)
      for (int k=1; k<=sm_zsize; k++)
        cout << i << "\t" << j << "\t" << k << "\t" 
	     << m_concentration.at(i,j,k) << endl;
}

/************************************************************************ 
 * writeDefinition()                                                    *
 *   Writes parameters for this molecule type to already-open file      *
 *                                                                      *
 * Parameters -                                                         *
 *   ofstream outfile:		where to write to                       *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::writeDefinition(ofstream &outfile) const
{
    outfile << "molecule_type " << m_name << "{" << endl;
    outfile << "diffusion_rate " << m_diffusionRate << endl;
    outfile << "decay_rate " << m_decayRate << endl << "}" << endl;
}

/************************************************************************ 
 * writeData()                                                          *
 *   Writes current concentrations to already-open file			*
 *                                                                      *
 * Parameters -                                                         *
 *   ofstream outfile:		where to write to                       *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Molecule::writeData(ofstream &outfile) const
{
  outfile << "molecule_detail: " << m_name << endl;

  for (int i=1; i<=sm_xsize; i++)
    for (int j=1; j<=sm_ysize; j++)
      for (int k=1; k<=sm_zsize; k++)
        outfile << m_concentration.at(i,j,k) << "\t";
  outfile << endl;
}
    
