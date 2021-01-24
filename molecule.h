
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
 * file molecule.h                                                      *
 * Declarations for Molecule class                                      *
 * Attributes and behavior for all molecules of one type                *
 ************************************************************************/

#ifndef MOLECULE_H
#define MOLECULE_H

#include <string>			// for molecule name
#include <fstream>			// for file I/O
#include <cassert>
#include "array3D.h"
class SimPoint;        

using namespace std;

class Molecule {
  public:
    typedef double Conc;

    //--------------------------- CREATORS --------------------------------- 
    explicit Molecule(const string& title);
    Molecule(const string& title, double diff, double decay);
    // copy constructor not used
    // ~Molecule();				// use default destructor

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    // set number and size of grid cells used by all Molecule objects
    // MUST CALL initialize() for each Molecule object after calling setGeometry
    static void setGeometry(int xrange, int yrange, int zrange, int gridsize);

    void initialize();		// allocates memory for concentration arrays

    // set diffusion and decay parameters
    void setDiffRate(double rate) {assert(rate>=0); m_diffusionRate = rate;};
    void setDecayRate(double rate) {m_decayRate = rate;};

    // setting initial concentrations - measured in moles/ml
    void setUniformConc(Conc amount, double stddev=0);	
    void initFromFile(ifstream &infile);

    // add or subtract some number of molecules at specified location - 
    // for secretion or binding by cells
    void changeConc(double amount, const SimPoint &p);  

    void update(double deltaT);

    //--------------------------- ACCESSORS --------------------------------
    bool isMatch(const string& type_name) const
    	{return (type_name == m_name);};
    const string& getName() const {return m_name;};
    double getDiffRate() const {return m_diffusionRate;};
    double getDecayRate() const {return m_decayRate;};

    // getting concentrations - measured in moles/ml
    Conc getConc(const SimPoint &pos) const;	// from nearest grid point   
    Conc getInterpConc(const SimPoint &pos) const;	// using 8 grid points
    Conc getAvgConc() const;
    SimPoint getGradient(const SimPoint &pos, double r) const;

    // note that indices to real data from this 3D array should start at 1
    const Array3D<Conc>& getConc() const {return m_concentration;};

    // get number of molecules in specified volume centered on specified point
    // (volume in ml)                                                          
    int getNumMolecules(double volume, const SimPoint &pos) const;

    // I/O stuff
    void printConc() const;
    void writeDefinition(ofstream &outfile) const;
    void writeData(ofstream &outfile) const;

  private:
    // geometry info that applies to all molecules
    static int sm_xsize;
    static int sm_ysize;
    static int sm_zsize;

    static int sm_gridsize;
    static int sm_size;
    static int sm_gridsq;		// used in diffusion calculations
    static double sm_invNavVol;		// used in changeConc             

    string m_name;
    double m_diffusionRate;		// microns^2/sec
    double m_decayRate;			// /sec

    // concentrations measured in moles/ml
    Array3D<Conc> m_concentration;	// unordered list of grid spaces
    Array3D<Conc> m_deltaConc;		// grid space list used for updates

    // private functions - explicit solution of diffusion equation, w/decay
    void decay(double deltaT);
    void explicitDecayDiff2D(double deltaT);	
    void explicitDecayDiff3D(double deltaT);	

    // add (possibly negative) concentration to grid cell at (i,j,k);
    // checks whether (i,j,k) is a guard cell, changes corresponding 'real'
    // cell if so
    void addConc(int i, int j, int k, Conc change);	

    // set various guard cells
    void setGuards();	
    void setGuardCorners();	
    void setSpecificGuards(int i, int j, int k);	

    // not used
    Molecule(const Molecule &m);
    Molecule operator = (const Molecule &m); 
};

#endif

