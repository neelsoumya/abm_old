
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
 * file history.cc                                                      *
 * Definitions for History class                                        * 
 * Holds aggregate data for duration of simulation                      *
 ************************************************************************/

#include "history.h"
#include <iostream>
#include "tissue.h"
#include "cellType.h"
#include "util.h"

using namespace std;

/************************************************************************
 * History()                                                            *
 *   Constructor                                                        * 
 *                                                                      *
 * Parameters:                                                          *
 *   const Tissue &tr:	Tissue object this history object is tracking   *
 *   char *attr_file:	optional input file for cell-attr pairs to track*
 *                                                                      * 
 * Returns:  nothing                                                    *
 ************************************************************************/
History::History(const Tissue &tr, char *attr_file) 
	: num_mol_types(tr.getNumMolTypes()), 
	  num_cell_types(tr.getNumCellTypes()), 
	  max_conc(0), max_cells(0)
{
  conc_histories.resize(num_mol_types);
  for (int i=0; i<num_mol_types; i++)
    mol_names.push_back(tr.getMolecule(i).getName());
  cell_histories.resize(num_cell_types);
  for (int i=0; i<num_cell_types; i++)
    cell_names.push_back(tr.getCellType(i)->getName());
  totals.resize(num_cell_types);
  for (unsigned int i=0; i<totals.size(); i++)
    totals[i].resize(tr.getCellType(i)->getNumAttributes());

  if (attr_file)
    ReadAttrFile(attr_file, tr);
}

/************************************************************************
 * ReadAttrFile()                                                       *
 *   Reads file containing list of cell type names and attribute names  * 
 * that identify particular attributes to include in history data.      * 
 * Creates appropriate data structures for tracking that data.          * 
 *                                                                      * 
 * Parameters:                                                          *
 *   char *filename:	input file                                      *
 *   const Tissue &tr:	Tissue object this history object is tracking   *
 *                                                                      * 
 * Returns:  nothing                                                    *
 ************************************************************************/
void History::ReadAttrFile(char *filename, const Tissue& tr)   
{
  // open file
  ifstream infile(filename);
  if (!infile)
    error("History:  could not open file", filename);

  char cellname[20], attrname[20];          	// for input strings 
  char name[50];				// name to store
  while (!infile.eof())
  {
    // read names, get indices and store in pair list
    infile >> cellname >> attrname;
    sprintf(name, "%s:%s", cellname, attrname);
    int cellid = tr.getCellTypeIndex(cellname);
    int attrid = tr.getCellType(cellname)->getAttributeIndex(attrname);
    attr_pairs.push_back(CellAttrPair(cellid, attrid, name));
    infile >> ws;	// skip whitespace
  }

  attr_histories.resize(attr_pairs.size());
  infile.close();
}

/************************************************************************
 * sample()                                                             *
 *   Appends current data from Tissues object to history arrays         * 
 *   Keeps track of largest concentration & cell count                  * 
 *                                                                      * 
 * Parameters:                                                          *
 *   const Tissue &t:	Tissue object                                   *
 *                                                                      * 
 * Returns:  nothing                                                    *
 ************************************************************************/
void History::sample(const Tissue& t)
{
  try { times.push_back(t.getTime()); }
  catch(std::bad_alloc&) 
    { cerr << "not enough memory for history" << endl; abort(); }

  for (int i=0; i<num_mol_types; i++)
  {
    double conc = t.getAvgConc(i);
    try { conc_histories[i].push_back(conc); }
    catch(std::bad_alloc&) 
      { cerr << "not enough memory for history" << endl; abort(); }
    if (conc > max_conc)
      max_conc = conc;
  }

  // just allocate space here; updateCellStats will get values
  for (int i=0; i<num_cell_types; i++)
  {
    try { cell_histories[i].push_back(0); }
    catch(std::bad_alloc&) 
      { cerr << "not enough memory for history" << endl; abort(); }
  }
  
  updateCellStats(t);

  // save select attribute data 
  for (unsigned int i=0; i<attr_pairs.size(); i++)
  {
    double value = totals[attr_pairs[i].celltypeid][attr_pairs[i].attrid];
    try { attr_histories[i].push_back(value); }
    catch(std::bad_alloc&)
      { cerr << "not enough memory for history" << endl; abort(); }
  }
}

/************************************************************************ 
 * updateCellStats()                                                    *
 * Clears previous data, then goes through Cell list and tallies      	* 
 * counts and attribute data by type.                                   * 
 *                                                                      * 
 * Parameters:                                                          *
 *   const Tissue &tr:  access to Cells data                            *
 *                                                                      * 
 * Returns:  nothing                                        		* 
 ***********************************************************************/
void History::updateCellStats(const Tissue& tr)
{
  vector<Cell *> list = tr.getCellList(); 
  int timeindex = times.size()-1;

  // first reset attribute values to 0
  for (int i=0; i<num_cell_types; i++)        
     for (unsigned int j=0; j<totals[i].size(); j++)
       totals[i][j] = 0;
                                    
  // now update values
  for (unsigned int i=0; i<list.size(); i++)       
  {
     int index = list[i]->getTypeIndex();
     (cell_histories[index][timeindex])++;
     for (unsigned int j=0; j<totals[index].size(); j++)
       totals[index][j] += list[i]->getValue(j);
  }

  // update max count
  for (int i=0; i<num_cell_types; i++)       
    if (cell_histories[i][timeindex] > max_cells)
      max_cells = cell_histories[i][timeindex];
}

ostream& operator<<(ostream& s, const History& h)
{   
  // write labels for data columns
  s << "#time\t";
  for (int j=0; j<h.num_cell_types; j++)
    s << h.cell_names[j] << "\t";
  for (int j=0; j<h.num_mol_types; j++)
    s << h.mol_names[j] << "\t";
  for (unsigned int j=0; j<h.attr_pairs.size(); j++)
    s << h.attr_pairs[j].name << "\t";
  s << endl;

  // now actual data
  for (unsigned int i=0; i<h.times.size(); i++)
  {
    s << h.times[i] << "\t";
    for (int j=0; j<h.num_cell_types; j++)
      s << h.cell_histories[j][i] << "\t";
    for (int j=0; j<h.num_mol_types; j++)
      s << h.conc_histories[j][i] << "\t";
    for (unsigned int j=0; j<h.attr_pairs.size(); j++)
      s << h.attr_histories[j][i] << "\t";
    s << endl;
  }
  return s;
}

