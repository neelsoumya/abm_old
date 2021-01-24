
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
 * file tissue.cc                                                       * 
 * Routines for Tissue class                                            *    
 * Top-level model class for CyCells program                            *
 ************************************************************************/
									
#include "tissue.h"
#include <string>
#include <iostream>		// for cout, cerr
#include <fstream>		// for writeDefinition & writeData
#include <time.h>		// next three for seeding RNG
#include <sys/types.h>
#include <unistd.h>
#include "cells.h"
#include "molecule.h"
#include "random.h"
#include "util.h"

using namespace std;

#define INITREVNO 4	// if init file format changes, need to make
			// corresponding changes to writeData
			
/************************************************************************ 
 * Tissue()                                 				*
 *   Constructor                                                        *
 *   Randomly initializes seed (although it may be overwritten later)   *
 *   Sets default geometry (which may be overridden) and starts with    *
 *   no Molecules and empty Cells list           	         	*
 *									*
 * Parameters - none   			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
Tissue::Tissue(void) : description(), m_xrange(0), m_yrange(0), m_zrange(0),
	m_molres(0), m_cellres(0), cells(0), simtime(0)
{
  // idea for seed from Perl book
  pid_t pid = getpid();
  m_seed = time(NULL) ^ (pid + (pid << 15) );
cout << "constructor seed = " << m_seed << endl;
  RandK::randk(-labs(m_seed));	// get random number generator ready
				// may be re-seeded later if desired

  // create Cells object to handle cell type definitions
  cells = new Cells();  	

  // default geometry (sets those of Cells and Molecule also)
  setGeometry();
}

/************************************************************************ 
 * ~Tissue()                                 				*
 *   Destructor                                                         *
 *   Deletes objects pointed to by mol_types list and cells           	*
 *									*
 * Parameters - none   			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
Tissue::~Tissue()
{
  for(unsigned int i=0; i<mol_types.size(); i++)
    delete mol_types[i].typeptr;
  delete cells;
}

/************************************************************************ 
 * setGeometry()                            				*
 *   Sets default geometry definition					*
 *									*
 * Parameters -        			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Tissue::setGeometry()
{
  setGeometry(1000, 1000, 1000, 0, 0);
}

/************************************************************************ 
 * setGeometry()                            				*
 *   Changes geometry definition; ensures that all internal refs to     *
 *   geometry data are consistent                                       *
 *									*
 * Parameters -        			 				*
 *   int xrange, yrange, zrange:  size of Tissue modelled 		*
 *                              - number of microns/direction		*
 *   int molgridsize:          	size of Molecule spatial mesh		*
 *   int cellgridsize:          size of Cells patch            		*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Tissue::setGeometry(int xrange, int yrange, int zrange, 
		int molgridsize, int cellgridsize)
{
  // set up Tissue values
  m_xrange = xrange; m_yrange = yrange; m_zrange = zrange;
  m_molres = molgridsize;
  m_cellres = cellgridsize;

  // set up Molecule static values and (re)initialize concentration arrays - 
  // this will wipe out any existing data (reasonable, since geometry should
  // be set before setting concentrations
  Molecule::setGeometry(xrange, yrange, zrange, molgridsize);
  for (unsigned int i=0; i<mol_types.size(); i++)
    mol_types[i].typeptr->initialize();

  // set up Cell values and make sure cell list is empty - same reason
  // as above; any existing cells might be outside new geometry boundaries
  cells->makeEmpty();
  cells->setGeometry(m_xrange, m_yrange, m_zrange, cellgridsize);
}

/************************************************************************ 
 * withinBounds()                           				*
 *   Tests whether specified coordinate lies within the model geometry  *
 *									*
 * Parameters -        			 				*
 *   int dim:		which coordinate to test			*
 *   double value:	actual x, y, or z value 			*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
bool Tissue::withinBounds(int dim, double value)
{
  if (value < 0)
    return false;

  switch (dim)
  {
    case 0:	// x coord
      if (value > m_xrange)
	return false;
      else
	return true;
    case 1:	// y coord
      if (value > m_yrange)
	return false;
      else
	return true;
    case 2:	// z coord
      if (value > m_zrange)
	return false;
      else
	return true;
    default:	// invalid coord
      return false;
  }
}

/************************************************************************ 
 * setSeed()                                				*
 *   Changes seed to passed-in value and reinitalizes RNG               *
 *									*
 * Parameters -        			 				*
 *   long seed:			new seed 			        *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Tissue::setSeed(long new_seed)
{
  m_seed = new_seed;
cout << "seed = " << m_seed << endl;
  RandK::randk(-labs(m_seed));		// get random number generator ready
}

/************************************************************************ 
 * getMolecule()                            				*
 *   Finds a Molecule object by its name                                *
 *									*
 * Parameters          			 				*
 *   string name:	name of molecule described by Molecule object	*
 *									*
 * Returns                         					*
 *   pointer to Molecule object						*
 ************************************************************************/
Molecule *Tissue::getMolecule(string name) 
{

  for (unsigned int i=0; i<mol_types.size(); i++)
    if (mol_types[i].typeptr->isMatch(name))
      return mol_types[i].typeptr;

  // if we made it here, molecule wasn't found
  return 0;
}

/************************************************************************ 
 * setMolReset()                            				*
 *   Sets parameters for periodically resetting molecular concentration *
 *									*
 * Parameters          			 				*
 *   string name:	name of molecule type                        	*
 *   double interval:   amount of time between resets           	*
 *   Molecule::Conc conc: 	value to reset concentration to     	*
 *   double sd:		specifies spatial variance in concentration   	*
 *									*
 * Returns                         					*
 *   nothing                      			*
 ************************************************************************/
void Tissue::setMolReset(string name, double interval, Molecule::Conc conc,
		        double sd)
{
  bool found = false;
  for (unsigned int i=0; i<mol_types.size(); i++)
    if (mol_types[i].typeptr->isMatch(name)) 
    {
      struct Tissue::MolDef &md = mol_types[i];
      md.reset_interval = interval; 
      md.reset_value = conc; 
      md.reset_sd = sd; 
      md.next_reset = interval; 
      found = true;
      break;
    }

  if (!found)
    error("Molecule::setMolReset error - can't find molecule type", name);
}

/************************************************************************
 * update()                                                             *
 *   Updates model for one timestep.  Relies largely on update routines *
 *   for individual components of the model.                            *
 *                                                                      *
 * Parameters -                                                         *
 *   double deltaT:	time for one sim loop (in seconds)              *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Tissue::update(double deltaT)
{
  // changes due strictly to molecular interactions/mechanisms
  // right now, this refers to diffuson and decay (or possibly secretion
  // by cells not explicitly modeled - negative decay); may need to 
  // add reactions later
  for (unsigned int i=0; i<mol_types.size(); i++)
    if (simtime >= mol_types[i].next_reset)
    {
      mol_types[i].typeptr->setUniformConc(mol_types[i].reset_value,
		      mol_types[i].reset_sd);
      mol_types[i].next_reset += mol_types[i].reset_interval;
    }
    else
      mol_types[i].typeptr->update(deltaT);

  // cell actions
  cells->update(deltaT);

  // adjust sim time
  simtime += deltaT;
}

/************************************************************************
 * writeDefinition()                                                    *
 *   Writes molecule and cell type definitions to file                  *
 *                                                                      *
 * Parameters                                                           *
 *   char *filename:	where to write info                             *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Tissue::writeDefinition(char *filename)
{
  // open file
  ofstream outfile(filename);	
  if (!outfile)
    error("Tissue::writeDefinition:  could not open file", filename);

//  outfile << description << endl;             

  // write molecule types:
  if (mol_types.size())
    outfile << "num_molecule_types:  " << mol_types.size() << endl << endl;
  for (unsigned int i=0; i<mol_types.size(); i++)
  {
    mol_types[i].typeptr->writeDefinition(outfile);
    outfile << endl;
  }

  // write cell data
  cells->writeDefinition(outfile);
}

/************************************************************************
 * writeData()                                                          *
 *   Writes all model data for a particular timestep to file            *
 *   (molecule concentrations, cell positions)                          *
 *                                                                      *
 * Parameters                                                           *
 *   char *filename:	where to write info                             *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Tissue::writeData(char *filename)
{
  // open file
  ofstream outfile(filename);	
  if (!outfile)
    error("Tissue::writeData:  could not open file", filename);

  // start with InitFormat specification so this can be used to initialize
  // a new sim 
  outfile << "#InitFormat " << INITREVNO << endl << endl;

//  outfile << description << endl;             

  // write geometry
  outfile << "geometry" << endl;
  outfile << m_xrange << "x" << m_yrange << "x" << m_zrange 
	  << " microns;  mol_res: " << m_molres << "  cell_res: " 
	  << m_cellres << endl << endl;

  // write time and RNG info
  outfile << "timestamp:  " << simtime << endl << endl;
  outfile << "rnginfo:  ";
  RandK::writeToFile(outfile);

  // write molecule data:
  for (unsigned int i=0; i<mol_types.size(); i++)
  {
    mol_types[i].typeptr->writeData(outfile);
    outfile << endl;
  }

  // write cell data
  cells->writeData(outfile);
}

