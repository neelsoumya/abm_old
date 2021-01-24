
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
 *  file tallyActions.h                                                 *
 *  Declarations for Singleton class that tallies #uses of Action       *
 *  classes ActionDie, ActionDivide, and ActionAdmit                    *
 ************************************************************************/

#ifndef TALLY_H
#define TALLY_H

#include <string>
#include <cassert>
#include <vector>
#include <iostream>

using namespace std;

class TallyActions {
  public:
    static TallyActions* getInstance();

    ~TallyActions();

    // the number returned by register should be used in update calls
    int addName(const string aname);
    void update(int id) {
	    assert(id>=0); assert(id<=int(m_tallies.size())); 
	    m_tallies[id]++; };
    int getTally(int id) {
	    assert(id>=0); assert(id<=int(m_tallies.size())); 
	    return m_tallies[id]; };

  protected:
    TallyActions();

  private:

    static TallyActions* m_instance;
    vector<string> m_names;
    vector<int> m_tallies;

    // not used
    TallyActions(const TallyActions &tar);
    TallyActions operator = (const TallyActions &tar);

  friend ostream& operator<<(ostream& sr, const TallyActions& tar);
};

inline ostream& operator<<(ostream& sr, const TallyActions& tar)
{
  for (unsigned int i=0; i<tar.m_names.size(); i++)
    sr << tar.m_names[i] << "\t" << tar.m_tallies[i] << endl;
  return sr;
}


#endif


