
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
 * file cell.h                                                          *
 * Declarations for Cell class                                          * 
 * Attributes for one cell and routines to change/access them           *
 ***********************************************************************/

#ifndef CELL_H
#define CELL_H

#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include "simPoint.h"		// need header for SimPoint objects
using namespace std;

class Cell {	
  public:
    //--------------------------- CREATORS --------------------------------- 
    Cell(int index, const SimPoint& position) :
	m_typeIndex(index), m_pos(position), m_velocity(SimPoint(0,0,0)), 
        m_direction(SimPoint(0,0,0)), m_alive(true) 
	{assert(index >= 0);};	
    Cell(ifstream &infile, int index, int numAttr) :
	m_typeIndex(index), m_alive(true)
      { assert(index >= 0);
  	infile >> m_pos >> m_velocity >> m_direction;
        setNumAttributes(numAttr);
        for (int i=0; i<numAttr; i++) infile >> m_internals[i];
      };	

    // copy constructor not used
    // ~Cell();			// use default destructor

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    // set type, position, 'velocity' (actually change in position), 
    // internal values
    void setTypeIndex(int index) {assert(index>=0); m_typeIndex = index;};
    void setPosition(const SimPoint& p) {m_pos = p;};
    void setVelocity(const SimPoint& v) {m_velocity = v;};
    void setDirection(const SimPoint& v) {m_direction = v;};
    void setNumAttributes(int num) {m_internals.assign(num, 0);};
    void setValue(int index, double value) 
	{assert(index>=0); assert(index<int(m_internals.size())); 
	 m_internals[index]=value;};		
    void die() {m_alive = false;};

    //--------------------------- ACCESSORS --------------------------------
    int getTypeIndex() const {return m_typeIndex;};
    bool isType(int i) const {return (m_typeIndex==i);};
    const SimPoint& getPosition() const {return m_pos;};
    const SimPoint& getVelocity() const {return m_velocity;};
    const SimPoint& getDirection() const {return m_direction;};
    bool isAlive() const {return m_alive;};
    double getValue(int index) const 
	{assert(index>=0); assert(index<int(m_internals.size())); 
	 return m_internals[index];};
    const vector<double>& getInternals() const {return m_internals;};

  private:
    int m_typeIndex;		// identifies which type of cell this is
    SimPoint m_pos;		// 3D location within space, in microns
    SimPoint m_velocity;	// velocity vector; microns/sec in each dir.
    SimPoint m_direction;	// cell's chosen heading; different from 
				// normalized velocity if other forces act
    bool m_alive;		// is this cell alive?
				// for efficient list management, may need to 
				// leave cell in list even when dead

    vector<double> m_internals;		// cell attributes 

    // not used    
    Cell(const Cell &c);		// copy constructor should not be used
    Cell operator = (const Cell &c);    // assignment should not be used

};

inline ostream& operator<<(ostream& s, const Cell& c)
{
  s << "type " << c.getTypeIndex();
  s << " " << c.getPosition() << " " << c.getVelocity() << " "
	  << c.getDirection() << " ";
  vector<double> internals = c.getInternals();
  for (unsigned int i=0; i<internals.size(); i++)
    s << internals[i] << " ";
  return s;
}

#endif

