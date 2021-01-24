
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
 * file history.h                                                       *
 * Declarations for History class                                       * 
 * Holds aggregate model data for duration of simulation                *
 ***********************************************************************/

#ifndef HISTORY_H
#define HISTORY_H

#include <vector>
#include <iostream>
using namespace std;

class Tissue;

class History {	
  public:
    typedef vector<int> CellHistory;
    typedef vector<double> ConcHistory;
    typedef vector<double> AttrList;
    typedef vector<double> AttrHistory;

    //--------------------------- CREATORS --------------------------------- 
    History(const Tissue &tr, char *attr_file);	
    // copy constructor not used
    // ~History();			// use default destructor
 
    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    void sample(const Tissue& t);

    //--------------------------- ACCESSORS --------------------------------
    const int getNumMolTypes() const {return num_mol_types;};
    const int getNumCellTypes() const {return num_cell_types;};
    const vector<double>& getTimes() const {return times;};
    const ConcHistory& getConc(int i) const {return conc_histories[i];};
    double getCurrentConc(int i) const 
      {return conc_histories[i][times.size()-1];};
    double getMaxConc() const {return max_conc;};
    const CellHistory& getCounts(int i) const {return cell_histories[i];};
    int getCurrentCount(int i) const 
      {return cell_histories[i][times.size()-1];};
    int getMaxCount() const {return max_cells;};
    const vector<double>& getTotals(int ti) const {return totals[ti];};

  private:
    // summary info - total cell#s and average molecular concentrations
    vector<double> times;
    vector<ConcHistory> conc_histories;
    vector<CellHistory> cell_histories;
    vector<string> cell_names;
    vector<string> mol_names;

    // cell attribute data by type - only keep current timestep, not history
    vector<AttrList> totals;

    // will want to track only select cell attribute values
    struct CellAttrPair
    {
      int celltypeid;
      int attrid;
      string name;
      CellAttrPair(int cid, int aid, string s) : 
	      celltypeid(cid), attrid(aid), name(s) {};
    };
    vector<CellAttrPair> attr_pairs;
    vector<AttrHistory> attr_histories;	

    int num_mol_types;
    int num_cell_types;
    double max_conc;
    int max_cells;

    // private function to update cell attribute stats
    void updateCellStats(const Tissue &tr);

    // private function to read which cell attributes should be tracked
    void ReadAttrFile(char *attr_file, const Tissue &tr);

    // not used
    History(const History &h);    // copy constructor should not be used
    History operator = (const History &h);    // assignment should not be used

  friend ostream& operator<<(ostream& s, const History& h);
};

#endif

