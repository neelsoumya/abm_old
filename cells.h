
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
 * file cells.h                                                         *
 * Declarations for Cells class                                         * 
 * Attributes and behavior for all cells                                *
 ************************************************************************/

#ifndef CELLS_H
#define CELLS_H

#include <vector>
#include <string>
#include <fstream>
#include "cell.h"		// for access to getTypeIndex
#include "array3D.h"
#include "simPoint.h"

class CellType;

class Cells {
  public:
    //--------------------------- CREATORS --------------------------------- 
    Cells(); 	
    // copy constructor not used
    ~Cells();	

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    // model definition         
    void setGeometry(int xsize, int ysize, int zsize, int gridsize);
    void makeEmpty();		// remove cells, but not cell types
				// (for reinitialization)
    void addCellType(CellType *pct) {cell_type_list.push_back(pct);};

    // model initialization 

    // add list of cells with attributes from file
    void initFromFile(ifstream &infile);

    // add 2D square array of cells
    void addSheet(const string type_name, double zpos);

    // add 2D hexagonal array of dense-pack cells
    void addHexSheet(const string type_name, double zpos);
    void addHexMix(const string type1, const string type2, 
		    double perc, double zpos);

    // add cells in regular grid of specified size
    void addGrid(const string type_name, int size);
    void addMixedGrid(const string type1_name, const string type2_name,
		    int size);
    void addGrid2D(const string type_name, int size, double zpos);

    // add specified number of cells to random locations
    void addRandomly(const string type_name, int numCells);
    void addRandomly2D(const string type_name, int numCells, double z);

    // add one new cell to new_cell_list; need mergeNew to move to cell_list
    void addCell(const string& typeName, SimPoint pos, bool birth = 1);	
    void addCell(int typeIndex, SimPoint pos, bool birth = 1);

    // final cleanup before running simulations
    void initialize() { mergeNew(); };

    // running simulation
    void update(double deltaT);
    //--------------------------- ACCESSORS --------------------------------
    int getNumCellTypes() const {return cell_type_list.size();};
    int getNumCells() const {return cell_list.size();};

    const CellType *getCellType(const string& type_name) const;
    const CellType *getCellType(int i) const {return cell_type_list[i];};
    int getCellTypeIndex(const string& type_name) const;

    // find and return one cell within distance d of pc    
    Cell * getTarget(Cell *pc, double d);

    // determine whether there is a cell of tupe typeID within distance d of pc
    bool checkNeighbors(Cell *pc, double d, int typeID);

    // find all cells in patches surrounding pc, return in clist
    void getNeighbors(Cell *pc, vector<Cell*>& clist);
    // get whole cell list
    const vector<Cell *> &getCellList() const {return cell_list;};

    // ---------------------- OUTPUT ROUTINES -----------------------------
    void writeDefinition(ofstream &outfile);
    void writeData(ofstream &outfile);

  private:
    int m_xsize, m_ysize, m_zsize;		// number of grid cells/dim
    int m_gridsize;				// size in microns of one grid
    int m_xrange, m_yrange, m_zrange;		// size of sim space in microns

    vector<CellType*> cell_type_list;

    vector<Cell*> cell_list;

    vector<Cell*> new_cell_list;

    Array3D< vector<Cell*> > m_patches;		// list of cells by grid

    // private member functions used to clean up cell lists
    void mergeNew();	// to be used when safe after new cells added
			// currently called by tissue's update 
    void removeFromPatch(int xi, int yi, int zi, Cell *pc);	
			// removes specified cell from patch given by indices
    void removeDead();  // removes dead cells from cell_list            

    // move cells according to velocities calculated during update -
    int testOpenBC(SimPoint &pos);
    void bounceBC(SimPoint &pos, SimPoint& vel);
    void wrapBC(SimPoint &pos);
    SimPoint getDistVector(Cell *from, Cell *to);
    SimPoint sumNeighContr(Cell *pc, double radius);
    void moveCells(double deltaT);

    // figure out largest cell size for determining grid size
    int getLargestRadius();

    int getIndex(double p) { return (int) p/m_gridsize; }
//    int getIndex(double p) {
//      int i = 0;
//      int upper = m_gridsize;
//      while (p > upper) {i++; upper+=m_gridsize;}
//      return i;
//    }
   
    // not used
    Cells(const Cells &c);		// copy constructor should not be used
    Cells operator = (const Cells &c);    // assignment should not be used

};

#endif

