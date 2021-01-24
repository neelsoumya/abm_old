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
 * file color.h                                                         *
 * Declarations for Color class                                         * 
 ***********************************************************************/

#ifndef COLOR_H
#define COLOR_H

#include <cassert>
#include <iostream>
using namespace std;

class Color
{
  public:
    //--------------------------- CREATORS --------------------------------- 
    Color(int red, int green, int blue, int alpha=255) :
	m_red(red/255.0), m_green(green/255.0), m_blue(blue/255.0), 
	m_alpha(alpha/255.0) 
	{assert(m_red>=0); assert(m_blue>=0); assert(m_green>=0); 
	 assert(m_alpha>=0);
	 assert(m_red<=1); assert(m_blue<=1); assert(m_green<=1);
	 assert(m_alpha<=1);
	};
    Color(float red, float green, float blue, float alpha=1.0) :
	m_red(red), m_green(green), m_blue(blue), 
	m_alpha(alpha) 
	{assert(m_red>=0); assert(m_blue>=0); assert(m_green>=0); 
	 assert(m_alpha>=0);
	 assert(m_red<=1); assert(m_blue<=1); assert(m_green<=1);
	 assert(m_alpha<=1);
	};
    Color(const Color& rhs) : 
	m_red(rhs.m_red), m_green(rhs.m_green), m_blue(rhs.m_blue),
	m_alpha(rhs.m_alpha) {};
    // ~Color()		use default destructor

    //------------------------- MANIPULATORS -------------------------------
    Color &operator = (const Color &rc)
    { m_red=rc.m_red; m_green=rc.m_green; m_blue=rc.m_blue;
      m_alpha=rc.m_alpha; return *this; }	

    const Color operator*(float sf) const
    { assert (sf>=0); 	//  OK for scale factor to exceed 1
      return Color(sf*m_red, sf*m_green, sf*m_blue, m_alpha); };
    // scale only RGB values, leave alpha alone

    //--------------------------- ACCESSORS --------------------------------
    const float * getfv() const { return &m_red; };

    unsigned char red() const {return int(255*m_red);};
    unsigned char green() const {return int(255*m_green);};
    unsigned char blue() const {return int(255*m_blue);};
    
  private:
    float m_red, m_green, m_blue, m_alpha;


  friend ostream& operator<<(ostream & s, const Color &rc);
};

inline ostream& operator<<(ostream& s, const Color & rc)
{
  s << "(" << rc.m_red << ", " << rc.m_green << ", " 
	   << rc.m_blue << ", " << rc.m_alpha << ")";
  return s;
}

#endif

