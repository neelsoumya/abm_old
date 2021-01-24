
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
 * file tissue.h                                                        *
 * Declarations for Tissue class                                        * 
 * Top-level model class for CyCells program                            *
 ************************************************************************/

#ifndef TISSUE_H
#define TISSUE_H

#include <vector>
#include <string>
#include "cells.h"
#include "cellType.h"
#include "molecule.h"
#include "array3D.h"

class Tissue {
  public:
    //--------------------------- CREATORS --------------------------------- 
    Tissue(); 			// default constructor; creates empty model
				// seed initialized randomly from clock & pid
    // copy constructor not used
    ~Tissue();			

    //------------------------- MANIPULATORS ------------------------------- 
    // assignment not used

    // model definition routines
    void addMolType(Molecule *pm) {mol_types.push_back(MolDef(pm));};
    void addCellType(CellType *pct) {cells->addCellType(pct);};

    // model initialization routines
    void setGeometry();
    void setGeometry(int xrange, int yrange, int zrange, 
		    int molgridsize, int cellgridsize);
    void setSeed(long new_seed);
    void setTime(double time) {simtime = time;};
    void setMolReset(string molname, double interval, Molecule::Conc conc,
	double sd);

    // running simulation
    void update(double deltaT);		// run sim for one timestep

    //--------------------------- ACCESSORS --------------------------------
    // routine to get RNG seed                             
    long getSeed() const {return m_seed;};

    // routines to get model space dimensions in microns:
    int getXSize() const {return m_xrange;};
    int getYSize() const {return m_yrange;};
    int getZSize() const {return m_zrange;};
    int getGridSize() const {return m_molres;};
    bool withinBounds(int dim, double value);

    // routines to get number of molecule and cell types and number of cells:
    int getNumMolTypes() const {return mol_types.size();};
    int getNumCellTypes() const {return cells->getNumCellTypes();};
    int getNumCells() const {return cells->getNumCells();};

    // returns time in seconds
    double getTime() const {return simtime;};	

    // return molecule concentration for molecule type of index i:
    Molecule::Conc getAvgConc(int i) const 
      {return mol_types[i].typeptr->getAvgConc();};
    const Array3D<Molecule::Conc>& getConc(int i) const
	{return mol_types[i].typeptr->getConc();};

    // determine largest diffusion rate - for sanity checks on spatial vs.
    // temporal discretization
    double getMaxDiffRate() const {
      double rate, max = 0;
      for (unsigned int i=0; i<mol_types.size(); i++)
        max = ( (rate = getMolecule(i).getDiffRate()) > max ? rate : max);
      return max;
    }

    // get pointers/refs to various components 
    Molecule *getMolecule(string name); 
    const Molecule &getMolecule(int i) const 
	{assert(i>=0); assert(i<(int)mol_types.size()); 
		return *(mol_types[i].typeptr);}; 
    Cells *getCellsPtr() {return cells;};
    const vector<Cell *> &getCellList() const {return cells->getCellList();};
    const CellType *getCellType(const string& name) const
	{return cells->getCellType(name);};
    const CellType *getCellType(int i) const 
	{return cells->getCellType(i);};
    int getCellTypeIndex(const string& name) const
	{return cells->getCellTypeIndex(name);};

    //---------------------- OUTPUT ROUTINES -----------------------------
    void writeDefinition(char *filename);	// model parameters 
    void writeData(char *filename); 	// actual data for model at some time

  private:
    string description;		// comments for this particular model

    //geometry info
    int m_xrange, m_yrange, m_zrange;	// tissue dimensions in microns
    int m_molres;	// molecular resolution; size of grid 'cell' in microns
    int m_cellres;	// cell resolution; size of patch of cells in microns

    struct MolDef {
      Molecule *typeptr;
      double reset_interval;		// how often to reset concentration
      Molecule::Conc reset_value;	// concentration to use
      double reset_sd;			// standard deviation
      double next_reset;		// sim time of next reset
      MolDef(Molecule *mp) : typeptr(mp), 
        reset_interval(numeric_limits<double>::max()), 
	reset_value(0), reset_sd(0), 
	next_reset(numeric_limits<double>::max()) {};
    };

    vector<MolDef> mol_types;

    Cells *cells;		// pointer to Cells object, which contains
				// list of cell types & list of individual cells

    long m_seed;		// for random number generator

    double simtime;		// elapsed time in sim

    // not used
    Tissue(const Tissue &t);	// copy constructor should not be used
    Tissue operator = (const Tissue &t);    // assignment should not be used	
};

#endif

