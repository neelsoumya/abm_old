
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
 * file simPoint.h                                                      *
 * Declarations for SimPoint class                                      * 
 * Attributes and operations for a 3-D point or vector                  *
 ***********************************************************************/

#ifndef POINT_H
#define POINT_H

#include <iostream> 
#include <cmath> 
using namespace std;

class SimPoint {	
  public:
    //--------------------------- CREATORS --------------------------------- 
    SimPoint() : m_x(0), m_y(0), m_z(0) {};	
    SimPoint(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {};	
    SimPoint(const SimPoint &p) : m_x(p.m_x), m_y(p.m_y), m_z(p.m_z) {};	
    // ~SimPoint();			// use default destructor

    //------------------------- MANIPULATORS -------------------------------
    SimPoint& operator = (const SimPoint &p) 
	{m_x=p.m_x; m_y=p.m_y; m_z=p.m_z; return *this;};       
    SimPoint& operator += (const SimPoint &p)
	{m_x+=p.m_x; m_y+=p.m_y; m_z+=p.m_z; return *this;};
    SimPoint& operator *= (const double value)
	{
	m_x*=value; m_y*=value; m_z*=value; return *this;};
    void setX(double value) {m_x=value;};
    void setY(double value) {m_y=value;};
    void setZ(double value) {m_z=value;};
 
    //--------------------------- ACCESSORS --------------------------------
    double getX() const {return m_x;};
    double getY() const {return m_y;};
    double getZ() const {return m_z;};
    double dist(const SimPoint &p) const 
      { return sqrt( (m_x-p.m_x)*(m_x-p.m_x) + 
		     (m_y-p.m_y)*(m_y-p.m_y) + 
		     (m_z-p.m_z)*(m_z-p.m_z) ); };

  private:
    double m_x, m_y, m_z;

  friend bool operator==(const SimPoint& lhs, const SimPoint& rhs );
  friend bool operator>(const SimPoint& lhs, const SimPoint& rhs);
  friend bool operator>=(const SimPoint& lhs, const SimPoint& rhs);
  friend bool operator<(const SimPoint& lhs, const SimPoint& rhs);
  friend bool operator<=(const SimPoint& lhs, const SimPoint& rhs);
  friend ostream& operator<<(ostream& s, const SimPoint& p);
  friend istream& operator>>(istream& s, SimPoint& p);
};

// note that some pairs of points will fail all comparison tests
// test for equality
inline bool operator == (const SimPoint &lhs, const SimPoint &rhs)
{return (lhs.m_x==rhs.m_x && lhs.m_y==rhs.m_y && lhs.m_z==rhs.m_z);}       

// test for first point being STRICTLY greater than second - used for
// bounds checking
inline bool operator > (const SimPoint &lhs, const SimPoint &rhs)
{return (lhs.m_x>rhs.m_x && lhs.m_y>rhs.m_y && lhs.m_z>rhs.m_z);}    

// test for each coordinate of first point being greater than or equal to
// those of the second second - used for bounds checking
inline bool operator >= (const SimPoint &lhs, const SimPoint &rhs)
{return (lhs.m_x>=rhs.m_x && lhs.m_y>=rhs.m_y && lhs.m_z>=rhs.m_z);}    

// test for first point being STRICTLY less than second - used for 
// bounds checking
inline bool operator < (const SimPoint &lhs, const SimPoint &rhs) 
{return (lhs.m_x<rhs.m_x && lhs.m_y<rhs.m_y && lhs.m_z<rhs.m_z);}       

// test for each coordinate of first point being less than or equal to
// those of the second second - used for bounds checking
inline bool operator <= (const SimPoint &lhs, const SimPoint &rhs) 
{return (lhs.m_x<=rhs.m_x && lhs.m_y<=rhs.m_y && lhs.m_z<=rhs.m_z);}       

// output
inline ostream& operator<<(ostream& s, const SimPoint& p)
{
  s << " (" << p.m_x << ", " << p.m_y << ", " << p.m_z << ") ";
  return s;
}

// read from input
inline istream& operator>>(istream& s, SimPoint& p)
{
  char ch;
  double x, y, z;

  s >> ch;
  if (ch != '(')
    s.clear(ios_base::failbit);
  s >> x;
  s >> ch;	
  if (ch != ',')
    s.clear(ios_base::failbit);
  s >> y;
  s >> ch;		
  if (ch != ',')
    s.clear(ios_base::failbit);
  s >> z; 
  s >> ch;	
  if (ch != ')')
    s.clear(ios_base::failbit);

  if (s) {p.m_x = x; p.m_y=y; p.m_z=z;};
  return s;
}

  inline const SimPoint operator + (const SimPoint& lhs, const SimPoint& rhs)
	{ return SimPoint(lhs) += rhs; }
  inline const SimPoint operator * (const SimPoint& lhs, const double value)
	{ return SimPoint(lhs) *= value; }

#endif

