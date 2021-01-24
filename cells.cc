
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
 * file cells.cc                                                        * 
 * Routines for Cells class                                             *    
 * Storage and behavior for all cells                                   * 
 ************************************************************************/

#include "cells.h"
#include <vector>
#include <algorithm>								
#include <functional>							
#include <iostream>		// for cout, cerr
#include <fstream>		// for file I/O  
#include <cmath>		// for abs
#include "cellType.h"
#include "cell.h"
#include "simPoint.h"
#include "random.h"
#include "util.h"

using namespace std;

/************************************************************************ 
 * Cells()                                  				*
 *   Constructor - sets up empty cell lists                             *
 *									*
 * Parameters          			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
Cells::Cells() : m_xrange(0), m_yrange(0), m_zrange(0)
{
}

/************************************************************************ 
 * ~Cells()                                  				*
 *   Destructor - removes Cell and CellType objects from lists     	*
 *									*
 * Parameters          			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
Cells::~Cells() 
{
  unsigned int i;
  for(i=0; i<cell_type_list.size(); i++)
    delete cell_type_list[i];
  for(i=0; i<cell_list.size(); i++)
    delete cell_list[i];
  for(i=0; i<new_cell_list.size(); i++)
    delete new_cell_list[i];
}

/************************************************************************
 * setGeometry()                                                        *
 *   Changes geometry definition; in particular, creates lists of Cell  *
 *   pointers by grid cell location.                                    *
 *                                                                      *
 * Parameters                                                           *
 *   int xrange, yrange, zrange:   size of sim space in microns         *
 *   int gridsize:   		   size of each grid 'patch'            *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void Cells::setGeometry(int xrange, int yrange, int zrange, int gridsize)
{
  assert(xrange>0); assert(yrange>0); assert(zrange>0);

  m_xrange = xrange; m_yrange = yrange; m_zrange = zrange;
  m_gridsize = gridsize;

  if (!m_gridsize)		// pretend system 'well-mixed' - no real space
  {
    m_xsize = m_ysize = m_zsize = 1;
  }
  else		// need to make sure geometry reasonable; set up patches      
  {
    // sim space should be divisible by gridsize in each dimension - check
    if ( (xrange % gridsize) || (yrange % gridsize) || (zrange % gridsize) )
      error("Cells::setGeometry:  dimensions should be divisible by patch size",
	      m_gridsize);

    m_xsize = xrange/m_gridsize; 
    m_ysize = yrange/m_gridsize;
    m_zsize = zrange/m_gridsize;

    // if there are fewer than 3 grid cells in the x or y direction
    // (but not all three), getNeighbors routine below will create duplicates
    if ( (m_xsize<3)&&(m_ysize>=3) || (m_xsize>=3)&&(m_ysize<3) )
      error("Cells::getNeighbors not equipped to handle specified geometry");

    // set up cell lists by grid cell
    try {
      m_patches.resize(m_xsize, m_ysize, m_zsize);
    }
    catch(std::bad_alloc&) {
      cerr << "Cells::setGeometry:  not enough memory for cell lists by patch"
         << endl;
      abort();
    }
  }
}

/************************************************************************ 
 * getLargestRadius()                       				*
 *   Determine radius of largest cell type                              *
 *									*
 * Parameters          			 				*
 *									*
 * Returns - largest radius as an int 					*
 ************************************************************************/
int Cells::getLargestRadius()
{
  int max = 0;
  int radius;
  for (unsigned int i=0; i<cell_type_list.size(); i++)
  {
    radius = int(cell_type_list[i]->getRadius());
    if (radius>max)
      max = radius;
  }
  return max;
}

/************************************************************************ 
 * makeEmpty()                              				*
 *   Removes all cells from lists - for reinitialization                *
 *   Does not affect cell type definitions              	 	*
 *									*
 * Parameters          			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::makeEmpty()
{
  cell_list.resize(0,0);
  new_cell_list.resize(0,0);
}

/************************************************************************ 
 * initFromFile()                           				*
 *   Reads a list of cells - with attribute values - from an already-	*
 *   open file.                                                         *
 *									*
 * Parameters          			 				*
 *   ifstream infile:		file with cell data			*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::initFromFile(ifstream &infile)
{
  // get number of cells
  int count, index;
  char buff[20];
  infile >> count;

  for (int i=0; i<count; i++)
  {
    // read cell type; get index
    infile >> buff;	// 'type'
    infile >> index;
    CellType *pct = cell_type_list[index];

    // call Cell constructor to get rest of info and create Cell;
    // Cell constructor will need number of attributes
    Cell *c;
    try { c = new Cell(infile, index, pct->getNumAttributes()); }
    catch(std::bad_alloc&) {
        cerr << "not enough memory to make new cell" << endl;
        abort();
    }
    
    // add to temporary list; this list will be merged with cell_list
    // for the class later
    try { new_cell_list.push_back(c); }
    catch(std::bad_alloc&) {
      cerr << "not enough memory to add new cell to list" << endl;
      abort();
    }
  }	// end for each cell
}
 
/************************************************************************ 
 * addCell()                                				*
 *   Constructs a new cell of specified type at a specified 		*
 *   location and inserts cell into temporary new cell list.            *
 *   Internal Cell parameters are initialized in a type-specific manner *
 *   Two versions - differ in first parameter only (first calls second) *
 *									*
 * Parameters          			 				*
 *   CellType *pct OR const string& type_name OR int index:		*
				identifies type of cell to add		*
 *   SimPoint pos:		location of the center of the cell	*
 *   bool birth:		is this a new daughter cell?		*
 *				as opposed to a migrating, possibly     *
 *				more mature cell			*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addCell(const string& type_name, SimPoint pos, 
		bool birth /* = 1 */)
{
  int index = getCellTypeIndex(type_name);
  if (index < 0) 
    error("Cells::addCell:  can't find cell type", type_name);

  addCell(index, pos, birth);
}

void Cells::addCell(int index, SimPoint pos, 
	bool birth /* = 1 */)
{
  assert (index >= 0);
  assert (int(cell_type_list.size()) > index);

  // make sure position is within bounds - apply periodic b.c.
  wrapBC(pos);

  Cell *c;

  try { c = new Cell(index, pos); }
  catch(std::bad_alloc&) {
    cerr << "not enough memory to make new cell" << endl;
    abort();
  }

  CellType *pct = cell_type_list[index];
  if (birth)
    pct->initializeCell(c);
  else
    pct->randomizeCell(c);

  // add to temporary list; this list will be merged with cell_list
  // for the class later
  try { new_cell_list.push_back(c); }
  catch(std::bad_alloc&) {
    cerr << "not enough memory to add new cell to list" << endl;
    abort();
  }
}

/************************************************************************ 
 * mergeNew()                             				*
 *   Moves cells from new_cell_list to cell_list; set new_cell_list     *
 *   back to empty.  Should be called after, not during, update loop    *
 *   on cells.					 			*
 *									*
 * Parameters - none   			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::mergeNew()
{
  // move cells to 'real' list
  cell_list.insert(cell_list.end(), 
		   new_cell_list.begin(), new_cell_list.end());

  if (m_gridsize)
  {
    // for each cell in new list, add entry in appropriate patch list
    Cell *pc;
    SimPoint pos;
    int xindex, yindex, zindex;
    for (unsigned int i=0; i<new_cell_list.size(); i++)
    {
      pc = new_cell_list[i];
      pos = pc->getPosition();
      xindex = getIndex(pos.getX());
      yindex = getIndex(pos.getY());
      zindex = getIndex(pos.getZ());
      m_patches.at(xindex, yindex, zindex).push_back(pc);
    }
  }

  // empty new_list for next use 
  new_cell_list.clear();
}

/************************************************************************ 
 * removeFromPatch()                        				*
 *   Removes specified cell pointer from patch at specified indices.    *
 *									*
 * Parameters - none   			 				*
 *   int xi, yi, zi:	specifies grid cell cell is listed in		*
 *   Cell *pc:		specifies cell pointer to be removed		*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::removeFromPatch(int xi, int yi, int zi, Cell *pc)
{
  vector<Cell*>& rcl = m_patches.at(xi,yi,zi);
  vector<Cell*>::iterator p = find(rcl.begin(), rcl.end(), pc);
  rcl.erase(p);            
}

/************************************************************************ 
 * removeDead()                             				*
 *   Removes dead cells from cell_list.  Call before starting any new   *
 *   loops through cells that might try to access 'dead' cells		*
 *									*
 * Parameters - none   			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::removeDead()
{
  Cell *pc;
  SimPoint pos;
  int xindex, yindex, zindex;

  for (unsigned int i=0; i<cell_list.size(); )
  {
    if (!cell_list[i]->isAlive())
    {
      if (m_gridsize) 
      { // find and remove pointer to this cell from patch list
      	pc = cell_list[i];
      	pos = pc->getPosition();
      	xindex = getIndex(pos.getX());
      	yindex = getIndex(pos.getY());
      	zindex = getIndex(pos.getZ());
      	removeFromPatch(xindex, yindex, zindex, pc);
      }

      // delete actual cell, and delete pointer from master list
      delete(cell_list[i]);			
      cell_list[i] = cell_list[cell_list.size()-1];
      cell_list.pop_back();
    }
    else
      i++;
  }
}

/************************************************************************ 
 * getCellType()                            				*
 *   Finds a cell type by name, returns pointer 	                *
 *									*
 * Parameters          			 				*
 *   string type_name;   	identifies cell type being modified     *
 *									*
 * Returns - pointer to CellType object 				*
 ************************************************************************/
const CellType *Cells::getCellType(const string& type_name) const
{
  for (unsigned int i=0; i<cell_type_list.size(); i++)
  {
    CellType *pct = cell_type_list[i];
    if (pct->isMatch(type_name))   
      return pct;
  }

  // if we got this far, we didn't find it
  return 0;
}

/************************************************************************ 
 * getCellTypeIndex()                       				*
 *   Finds a cell type by name, returns its array index		        *
 *									*
 * Parameters          			 				*
 *   string type_name;   	identifies cell type being modified     *
 *									*
 * Returns - index of CellType object in arrays				*
 ************************************************************************/
int Cells::getCellTypeIndex(const string& type_name) const
{
  for (unsigned int i=0; i<cell_type_list.size(); i++)
  {
    CellType *pct = cell_type_list[i];
    if (pct->isMatch(type_name))   
      return i;
  }

  // if we got this far, we didn't find it
  return -1;
}

/************************************************************************ 
 * addSheet()                               				*
 *   Adds enough cells to fill one xy plane of 3D grid in square 	*
 *   pattern								*
 *									*
 * Parameters          			 				*
 *   string type_name		name of cell type we're adding		*
 *   double zpos:     		specifies location of plane in 3D grid  *
 *				position in microns, not grid spaces	*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addSheet(const string type_name, double zpos)
{
  assert( (zpos >= 0) && (zpos < m_zrange) );

  const CellType *t = getCellType(type_name);
  if (!t)
    error("Cells::addSheet: can't find cell type", type_name);

  double radius = t->getRadius();
  double diameter = 2*radius;

  for (double x=radius; x<(m_xrange); x+=diameter)
    for (double y=radius; y<(m_yrange); y+=diameter)
      addCell(type_name, SimPoint(x,y,zpos), false);

  mergeNew();
}

/************************************************************************ 
 * addHexSheet()                               				*
 *   Adds enough cells to fill one xy plane of 3D grid in hexagonal     *
 *   pattern.                      					*
 *   Should be approximately nxm cells, where m=floor(yrange/2R), and	*
 *   n=floor((xrange-.268R)/2.732R)					*
 *									*
 * Parameters          			 				*
 *   string type_name		name of cell type we're adding		*
 *   double zpos:     		specifies location of plane in 3D grid  *
 *				position in microns, not grid spaces	*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addHexSheet(const string type_name, double zpos)
{
  assert( (zpos >= 0) && (zpos < m_zrange) );

  const CellType *t = getCellType(type_name);
  if (!t)
    error("Cells::addHexSheet: can't find cell type", type_name);

  // spacing in one dimension is just 2R as before;
  // in other dimension, first cells are offset by R every other row, and
  // spacing between rows is R*(1+sin30)/cos30, or 1.732R
  double radius = t->getRadius();
  double diameter = 2*radius;
  double hspace = 1.732*radius;

  bool odd = true;
  for (double x=radius; x<(m_xrange); x+=hspace)
    if (odd)
    {
      for (double y=radius; y<(m_yrange); y+=diameter)
        addCell(type_name, SimPoint(x,y,zpos), false);
      odd = false;
    }
    else
    {
      for (double y=2*radius; y<(m_yrange); y+=diameter)
        addCell(type_name, SimPoint(x,y,zpos), false);
      odd = true;
    }

  mergeNew();
}

/************************************************************************ 
 * addHexMix()                                 				*
 *   Adds enough cells to fill one xy plane of 3D grid in hexagonal     *
 *   pattern.  Similar to addHexSheet, but this routine creates the 	*
 *   grid with two cell types, using the specified percentages of each. *
 *   Should be approximately nxm cells, where m=floor(yrange/2R), and	*
 *   n=floor((xrange-.268R)/2.732R)					*
 *									*
 * Parameters          			 				*
 *   string type1, type2	names of cell types we're adding	*
 *   double perc1,       	percentage of type 1 to use		*
 *   double zpos:     		specifies location of plane in 3D grid  *
 *				position in microns, not grid spaces	*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addHexMix(const string type1, const string type2, 
		double perc, double zpos)
{
  assert( (zpos >= 0) && (zpos < m_zrange) );

  const CellType *t1 = getCellType(type1);
  if (!t1)
    error("Cells::addHexMix: can't find cell type", type1);
  const CellType *t2 = getCellType(type2);
  if (!t2)
    error("Cells::addHexMix: can't find cell type", type2);

  // spacing in one dimension is just 2R as before;
  // in other dimension, first cells are offset by R every other row, and
  // spacing between rows is R*(1+sin30)/cos30, or 1.732R
  double radius = t1->getRadius();
  if (t2->getRadius() != radius)
    error("Cells::addHexMix: cell types must have same radius");
  double diameter = 2*radius;
  double hspace = 1.732*radius;

  bool odd = true;
  for (double x=radius; x<(m_xrange); x+=hspace)
    if (odd)
    {
      for (double y=radius; y<(m_yrange); y+=diameter)
	if (RandK::randk() < perc)
          addCell(type1, SimPoint(x,y,zpos), false);
        else
          addCell(type2, SimPoint(x,y,zpos), false);
      odd = false;
    }
    else
    {
      for (double y=2*radius; y<(m_yrange); y+=diameter)
	if (RandK::randk() < perc)
          addCell(type1, SimPoint(x,y,zpos), false);
        else
          addCell(type2, SimPoint(x,y,zpos), false);
      odd = true;
    }

  mergeNew();
}

/************************************************************************ 
 * addGrid()                               				*
 *   Adds cells in an x-y grid pattern (first approximation at alveoli)	*
 *									*
 * Parameters          			 				*
 *   string type_name		name of cell type we're adding		*
     int size:			size of grid spaces in microns		*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addGrid(const string type_name, int size)
{
  const CellType *t = getCellType(type_name);
  if (!t)
    error("Cells::addGrid: can't find cell type", type_name);

  double radius = t->getRadius();
  double diameter = 2*radius;
  for (double x=radius; x<(m_xrange); x+=diameter)
    for (double y=radius; y<(m_yrange); y+=diameter)
      for (double z=radius; z<(m_zrange); z+=diameter)
        if ( ( int(x-radius)%size==0) || ( int(y-radius)%size==0) )
          addCell(type_name, SimPoint(x,y,z), false);

  mergeNew();
}

/************************************************************************ 
 * addMixedGrid()                              				*
 *   Adds cells in an x-y grid pattern (first approximation at alveoli)	*
 *   'Corner' cells are a different type than the rest.			*
 *									*
 * Parameters          			 				*
 *   string type1_name		name of most-used cell type		*
 *   string type2_name		name of 'corner' cell type       	*
 *   int size:			size of grid spaces in microns		*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addMixedGrid(const string type1_name, const string type2_name, 
		int size)
{
  const CellType *t1 = getCellType(type1_name);
  if (!t1)
    error("Cells::addGrid: can't find cell type", type1_name);
  const CellType *t2 = getCellType(type2_name);
  if (!t2)
    error("Cells::addGrid: can't find cell type", type2_name);

  double radius = t1->getRadius();
  if (t2->getRadius() != radius)
	  error("Cells:addMixedGrid:  cell radii differ");

  double diameter = 2*radius;
  for (double x=radius; x<(m_xrange); x+=diameter)
    for (double y=radius; y<(m_yrange); y+=diameter)
      for (double z=radius; z<(m_zrange); z+=diameter)
        if ( ( int(x-radius)%size==0 ) && ( int(y-radius)%size==0 ) &&
			( int(z-radius)%size==0 ) )
          addCell(type2_name, SimPoint(x,y,z), false);
  	else if ( ( int(x-radius)%size==0) || ( int(y-radius)%size==0) )
          addCell(type1_name, SimPoint(x,y,z), false);

  mergeNew();
}

/************************************************************************ 
 * addGrid2D()                               				*
 *   Adds cells in an x-y grid pattern (first approximation at alveoli)	*
 *   with specified z position                                         	*
 *									*
 * Parameters          			 				*
 *   string type_name		name of cell type we're adding		*
 *   int size:			size of grid spaces in microns		*
 *   double zpos:		z coordinate for all cell positions	*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addGrid2D(const string type_name, int size, double zpos)
{
  const CellType *t = getCellType(type_name);
  if (!t)
    error("Cells::addGrid: can't find cell type", type_name);

  double radius = t->getRadius();
  double diameter = 2*radius;
  for (double x=radius; x<(m_xrange); x+=diameter)
    for (double y=radius; y<(m_yrange); y+=diameter)
        if ( ( int(x-radius)%size==0) || ( int(y-radius)%size==0) )
          addCell(type_name, SimPoint(x,y,zpos), false);

  mergeNew();
}

/************************************************************************ 
 * addRandomly()                            				*
 *   Adds the specified number of cells to random locations within      *
 *   the allocated space          		 		   	*
 *   Random number generator should be initialized before calling this  *
 *									*
 * Parameters          			 				*
 *   string type_name:		identifies type of cell to add		*
 *   int number:     		number of cells to add                  *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addRandomly(const string type_name, int number)
{
  assert(m_xrange+m_yrange+m_zrange);

  const CellType *t = getCellType(type_name);
  if (!t)
    error("Cells::addRandomly: can't find cell type", type_name);

  for (int i=0; i<number; i++)
  {
    // choose position within tissue boundaries
    double x = m_xrange*RandK::randk();
    double y = m_yrange*RandK::randk();
    double z = m_zrange*RandK::randk();
    addCell(type_name, SimPoint(x,y,z), false);
  }

  mergeNew();
}

/************************************************************************ 
 * addRandomly2D()                          				*
 *   Adds the specified number of cells to random locations at the      *
 *   "bottom" of the allocated space - all have the same z position.	*
 *   Random number generator should be initialized before calling this  *
 *									*
 * Parameters          			 				*
 *   string type_name:		identifies type of cell to add		*
 *   int number:     		number of cells to add                  *
 *   double z:			z coordinate to use for all cells       *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::addRandomly2D(const string type_name, int number, double z)
{
  assert(m_xrange+m_yrange+m_zrange);

  const CellType *t = getCellType(type_name);
  if (!t)
    error("Cells::addRandomly2D: can't find cell type", type_name);

  for (int i=0; i<number; i++)
  {
    // choose position within tissue boundaries
    double x = m_xrange*RandK::randk();
    double y = m_yrange*RandK::randk();

    addCell(type_name, SimPoint(x,y,z), false);
  }

  mergeNew();
}
		
/************************************************************************ 
 * getTarget                                  				*
 *   Returns pointer to a single cell within a specified distance of    *
 *   the cell passed in by first getting the list of all cells in   	*
 *   27 surrounding patches, then repeatedly choosing a cell at random  *
 *   from the list until it finds one that is less than the specified   *
 *   distance.  The point of the random selection is to avoid biasing   *
 *   the search by the order in which getNeighbors checks neighboring   *
 *   patches.                                                           *
 *   Returns null if no such cell found within a reasonable #tries.	*
 *									*
 * Parameters          			 				*
 *   Cell *pc:      		defines patch to search        		*
 *   double d;			max dist to target cell           	*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
Cell * Cells::getTarget(Cell *pc, double d)
{
  if (d > m_gridsize)
    cout << "Cells::getTarget warning - search radius larger than gridsize" 
	    << endl;

  vector<Cell*> clist;
  getNeighbors(pc, clist);

  int index = 0;			// actual index into list
  unsigned int i = 0; 		// track #times through loop
  bool found = false;
  while ( !found && i<clist.size() )
  {
    // pick a random index, test that cell 
    index = int( RandK::randk()*clist.size() );
    if ( clist[index]->isAlive() && (clist[index] != pc) )
    {
      // test distance 
      SimPoint dv = getDistVector(clist[index], pc);
      double mag = dv.dist(SimPoint(0,0,0));
      if (mag <= d)
        found = true;
    }
    i++;
  }

  if (!found)
    return NULL;
  else
  {
    return clist[index];
  }
}

/************************************************************************ 
 * checkNeighbors                             				*
 *   Returns a boolean value indicating whether there is a cell of the  *
 *   specified type within a specified distance of the cell passed in.  *
 *   Similar to getTarget in that it uses getNeighbors to get a list of *
 *   all cells in the 27 surrounding patches.  But since this routine   *
 *   is not selecting one of those cells, it just searches the list     *
 *   sequentially.
 *									*
 * Parameters          			 				*
 *   Cell *pc:      		defines patch to search        		*
 *   double d;			max dist to target cell           	*
 *   int typeID:		index of desired cell type       	*
 *									*
 * Returns - boolean; was appropriate cell found?			*
 ************************************************************************/
bool Cells::checkNeighbors(Cell *pc, double d, int typeID)
{
  if (d > m_gridsize)
    cout << "Cells::checkNeighbors warning - search radius > gridsize" 
	    << endl;

  vector<Cell*> clist;
  getNeighbors(pc, clist);

  unsigned int i = 0; 			// index to cell in list    
  bool found = false;
  while ( !found && i<clist.size() )
  {
    Cell *pt = clist[i];
    if ( pt->isAlive() && (pt != pc) && (pt->getTypeIndex() == typeID) )
    {
      // test distance 
      SimPoint dv = getDistVector(clist[i], pc);
      double mag = dv.dist(SimPoint(0,0,0));
      if (mag <= d)
        found = true;
    }
    i++;
  }

  return found;
}

/************************************************************************ 
 * getNeighbors                               				*
 *   Assembles a list of cells within the 27 grid cells that surround   *
 *   the location of the cell passed in.  Calling routine must create   *
 *   and empty the vector before calling.				*
 *   This routines assumes periodic boundary conditions.		*
 *   It also assumes that there are currently no 'dead' cells in lists  *
 *   (called by moveCells, which is preceded by removeDead).            *
 *   									*
 *   Assumes sim volume at least 3x3x1 -checked in SetGeometry		*
 *									*
 * Parameters          			 				*
 *   Cell *pc;                                               		*
 *   vector<Cell *> clist;                                   		*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::getNeighbors(Cell *pc, vector<Cell *>& clist)
{
  // if there are fewer than 3 grid cells in each direction, all cells
  // are neighbors
  if ( (m_xsize <= 3) && (m_ysize <= 3) && (m_zsize <= 3) )
    clist.insert(clist.end(), cell_list.begin(), cell_list.end());
  
  else
  {
    // identify patch pc is in
    SimPoint pos = pc->getPosition();
    int xindex = getIndex(pos.getX());
    int yindex = getIndex(pos.getY());
    int zindex = getIndex(pos.getZ());
    int ii, jj, kk;			// indices of neighboring patches
  
    // go through all the neighboring patches, allowing for wraparound
    for (int i=xindex-1; i<xindex+2; i++)
    {
      ii = i;
      if (ii<0) ii=m_xsize-1;
      else if (ii>=m_xsize) ii=0; 
      for (int j=yindex-1; j<yindex+2; j++)
      {
        jj = j;
        if (jj<0) jj=m_ysize-1;
        else if (jj>=m_ysize) jj=0; 

        // now check z-dimension - most likely to be single or double layer
        if (m_zsize <= 3)
          // just check the number of layers that exist
          for (int k=0; k<m_zsize; k++)
          {
            vector<Cell*>& rcl = m_patches.at(ii, jj, k);
            clist.insert(clist.end(), rcl.begin(), rcl.end());
          }	// end for k
        else
          // just check neighboring layers
          for (int k=zindex-1; k<zindex+2; k++)
          {
	    kk = k;
            if (kk<0) kk=m_zsize-1;
            else if (kk>=m_zsize) kk=0; 
            vector<Cell*>& rcl = m_patches.at(ii, jj, kk);
	    clist.insert(clist.end(), rcl.begin(), rcl.end());
          }	// end for kk
      }	// end for jj
    }	// end for ii
  }	// end if we actually have enough patches to check

  // now remove the original cell passed in
  vector<Cell*>::iterator p = find(clist.begin(), clist.end(), pc);
  clist.erase(p);            
}
  
/************************************************************************ 
 * testOpenBC                                 				*
 *   Check position against tissue boundaries, return 1 if cell is out  *
 *   of bounds								*
 *									*
 * Parameters          			 				*
 *   SimPoint& pos:		position in microns                     *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
int Cells::testOpenBC(SimPoint& pos)
{
  if ((pos.getX() < 0) || (pos.getX() >= m_xrange) || 
      (pos.getY() < 0) || (pos.getY() >= m_yrange) || 
      (pos.getZ() < 0) || (pos.getZ() >= m_zrange))
    return 1;
  else
    return 0;
}

/************************************************************************ 
 * bounceBC                                   				*
 *   Implements reflective boundary conditions - if cell is out of      *
 *   bounds, resets position and velocity as if cell had bounced	*
 *									*
 * Parameters          			 				*
 *   SimPoint& pos:		new position in microns                 *
 *   SimPoint& vel:		actually position change in microns     *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::bounceBC(SimPoint& pos, SimPoint& vel)
{
  if (pos.getX() < 0)
  {
    pos.setX(-pos.getX());
    vel.setX(-vel.getX());
  }
  else if (pos.getX() >= m_xrange)
  {
    pos.setX(m_xrange - (pos.getX() - m_xrange));
    vel.setX(-vel.getX());
  }
  if (pos.getY() < 0)
  {
    pos.setY(-pos.getY());
    vel.setY(-vel.getY());
  }
  else if (pos.getY() >= m_yrange)
  {
    pos.setY(m_yrange - (pos.getY() - m_yrange));
    vel.setY(-vel.getY());
  }
  if (pos.getZ() < 0)
  {
    pos.setZ(-pos.getZ());
    vel.setZ(-vel.getZ());
  }
  else if (pos.getZ() >= m_zrange)
  {
    pos.setZ(m_zrange - (pos.getZ() - m_zrange));
    vel.setZ(-vel.getZ());
  }
}

/************************************************************************ 
 * wrapBC                                   				*
 *   Implements periodic boundary conditions - if cell is out of        *
 *   bounds, resets position as if cell left one side and entered the   *
 *   other								*
 *									*
 * Parameters          			 				*
 *   SimPoint& pos:		new position in microns                 *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::wrapBC(SimPoint& pos)
{
  // unlikely - but cells may wrap more than once!
  while (pos.getX() < 0)
    pos.setX(m_xrange+pos.getX());
  while (pos.getX() >= m_xrange)
    pos.setX(pos.getX() - m_xrange);
  while (pos.getY() < 0)
    pos.setY(m_yrange+pos.getY());
  while (pos.getY() >= m_yrange)
    pos.setY(pos.getY() - m_yrange);
  while (pos.getZ() < 0)
    pos.setZ(m_zrange+pos.getZ());
  while (pos.getZ() >= m_zrange)
    pos.setZ(pos.getZ() - m_zrange);
}

/************************************************************************ 
 * getDistVector()                            				*
 *   Calculates distance - as a vector - between centers of two cells.  *
 *   This routine assumes periodic boundary conditions.			*
 *									*
 * Parameters          			 				*
 *   Cell *from, *to:		pointers to cells in question         	*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
SimPoint Cells::getDistVector(Cell *from, Cell *to)
{
  if (from == to)
    return SimPoint(0,0,0);

  double xdist, ydist, zdist;
  SimPoint frompos = from->getPosition();
  SimPoint topos = to->getPosition();

  xdist = topos.getX() - frompos.getX();
  if ( fabs(fabs(xdist) - m_xrange) < fabs(xdist) )
    if (xdist<0) xdist += m_xrange;
    else xdist -=m_xrange;
  ydist = topos.getY() - frompos.getY();
  if ( fabs(fabs(ydist) - m_yrange) < fabs(ydist) )
    if (ydist<0) ydist += m_yrange;
    else ydist -= m_yrange;
  zdist = topos.getZ() - frompos.getZ();
  if ( fabs(fabs(zdist) - m_zrange) < fabs(zdist) )
    if (zdist<0) zdist += m_zrange;
    else zdist -= m_zrange;

  return SimPoint(xdist, ydist, zdist);
}

/************************************************************************ 
 * sumNeighContr()                            				*
 *   Sums the forces of each neighbor on the cell passed in; returns    *
 *   the total velocity contribution.					*
 *									*
 * Parameters          			 				*
 *   Cell *pc;			affected cell				*
 *   double radius;		cell radius  	 			*
 *									*
 * Returns - net velocity contribution		*
 ************************************************************************/
SimPoint Cells::sumNeighContr(Cell *pc, double radius)
{
  SimPoint Vnet;

  // get all potential neighbors
  vector<Cell *> clist;
  getNeighbors(pc, clist);

  // calculate force on pc from each neighbor
  for (unsigned int j=0; j<clist.size(); j++)
  {
    // start with distance between cell centers d
    SimPoint d = getDistVector(clist[j], pc);
    double mag = d.dist(SimPoint(0,0,0));
    if (mag!=0)	// shouldn't be 0, but could happen
    {
      // normalize direction vector; get ratio of distance to cell sizes
      SimPoint dir = d * (1.0/mag);
      CellType *pct2 = cell_type_list[clist[j]->getTypeIndex()];
      double r = mag / (radius + pct2->getRadius());

      // if cells are overlapping, add repulsive contribution - form was chosen 
      // heuristically to cancel the velocity of a cell moving directly at the 
      // neighbor at 2 microns/min (.03/sec) just when the cells touch, and to 
      // push it away more strongly as they overlap more
      if (r<1)
        Vnet += (dir * 0.03 *(2-r));
    }
  }	// end for each neighboring grid cell

  return Vnet;
}

/************************************************************************ 
 * moveCells(deltaT)                          				*
 *									*
 * Parameters          			 				*
 *   double deltaT: 		size of timestep in seconds             *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::moveCells(double deltaT)
{
  for (unsigned int i=0; i<cell_list.size(); i++)
  {
    Cell *pc = cell_list[i];                               
    CellType *pct = cell_type_list[pc->getTypeIndex()];

    if (pct->getSpeed()) 	// is this a mobile cell?            
    {
      // sum velocity contributions to the cell
      // 1) due to cell's own movement 
      SimPoint Vnet = pc->getDirection() * pct->getSpeed();

      // 2) due to forces from neighboring cells
      Vnet += sumNeighContr(pc, pct->getRadius());

      pc->setVelocity(Vnet);	
    }	// end if cell is moving
  }	// end of outer cell loop through all cells

  int oldxi, newxi, oldyi, newyi, oldzi, newzi;
  SimPoint oldpos, pos;

  // now go back through and actually move cells, checking boundaries
  for (unsigned int i=0; i<cell_list.size(); i++)
  {
    Cell *pc = cell_list[i];                               
    CellType *pct = cell_type_list[pc->getTypeIndex()];

    if (pct->getSpeed()) 	// is this a mobile cell?            
    {
      oldpos = pc->getPosition();
      pos = oldpos + pc->getVelocity()*deltaT;

      // open boundaries; cells just disappear
//    if (testOpenBC(pos)) removeCell(c1); 
	  // make sure CellType::count is updated!

      // reflective; cells bounce off 'walls'
//    bounceBC(pos, vel);
      // now update actual Cell values (may be putting same value back in)
//    pc->setPosition(pos);
//    pc->setVelocity(vel);

      // periodic - wrap around
      wrapBC(pos);
      pc->setPosition(pos);	

      // update grid pointers to cell if new position not in same grid
      oldxi = getIndex(oldpos.getX());
      oldyi = getIndex(oldpos.getY());
      oldzi = getIndex(oldpos.getZ());
      newxi = getIndex(pos.getX());
      newyi = getIndex(pos.getY());
      newzi = getIndex(pos.getZ());
      if ( (newxi != oldxi) || (newyi != oldyi) || (newzi != oldzi) )
      {
        removeFromPatch(oldxi, oldyi, oldzi, pc);
	m_patches.at(newxi, newyi, newzi).push_back(pc);
      }

    }	// end if cell is moving
  }	// end of outer cell loop through all cells

// take 'dead' (disappeared) cells out of list - only for open b.c.
//	removeDead();
}


/************************************************************************ 
 * update()                                 				*
 *   Handles internal processing and actions by each cell during one    *
 *   timestep.  Cell sensing of the local environment, since it's       * 
 *   governed by molecular binding, is handled at a higher level, and   *
 *   is assumed to have already been done by the time this function is  *
 *   called.								*
 *									*
 * Parameters          			 				*
 *   double deltaT: 		size of timestep in seconds             *
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::update(double deltaT)
{
  // randomize cell list order to minimize order effects                  
  if (cell_list.size() > 1)
    shuffle(cell_list);

  // do sensing and processing for all cells, one at a time
  // Sensing updates internal variables in response to
  // current conditions.  Processing checks for cell death, division, 
  // secretion, etc.; internal velocity parameters may be affected, but cell 
  // doesn't move until later.
  for (unsigned int i=0; i<cell_list.size(); i++)	
  {
    Cell *pc = cell_list[i];                                

    CellType *pct = cell_type_list[pc->getTypeIndex()];
    pct->update(pc, deltaT);
  }

  // remove dead cells 
  removeDead();

  // move cells - after done messing with molecular environment
  if (m_gridsize)
    moveCells(deltaT);

  // make sure cells added during this update step 
  // are put into 'real' cell list for processing next time
  mergeNew();
}

/************************************************************************ 
 * writeDefinition()                       				*
 *   writes parameters for each cell type to already-open file          *
 *									*
 * Parameters -   			 				*
 *   ofstream outfile:		where to write data to			*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::writeDefinition(ofstream &outfile)
{
  if (cell_type_list.size())
    outfile << "num_cell_types:  " << cell_type_list.size() << endl << endl;
  for (unsigned int i=0; i<cell_type_list.size(); i++)
    outfile << *cell_type_list[i];
}

/************************************************************************ 
 * writeData()                            				*
 *   writes number of cells and list of their positions to              *
 *   already-open file							*
 *									*
 * Parameters -   			 				*
 *   ofstream outfile:		where to write data to			*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void Cells::writeData(ofstream &outfile)
{
  outfile << "cell_detail:  " << cell_list.size() << endl;
  for (unsigned int i=0; i<cell_list.size(); i++)
    outfile << *cell_list[i] << endl;
}

