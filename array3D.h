
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
 * file array3D.h                                                       *
 * Declarations for Array3D	                                 	* 
 * three-dimensional array                                              *
 ***********************************************************************/

#ifndef ARRAY3D_H
#define ARRAY3D_H

#include <cassert>

template<class T>
class Array3D {	
  public:
    //--------------------------- CREATORS --------------------------------- 
    Array3D() : m_i(0), m_j(0), m_k(0), m_n(0), m_data(0) {};	
    Array3D(int i, int j, int k) : m_i(i), m_j(j), m_k(k), m_n(i*j*k), m_data(0)
	{ assert(m_n); m_data = new T[m_n]; };	
    // copy constructor not used
    ~Array3D() {delete [] m_data;};		

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    void resize(int i, int j, int k);
  
    void setAll(const T &value)
    { for (int i=0; i<m_n; i++) m_data[i] = value; }

    //--------------------------- ACCESSORS --------------------------------
    // get sizes 
    int xsize() const {return m_i;};
    int ysize() const {return m_j;};
    int zsize() const {return m_k;};
    int size() const {return m_n;};

    // 3D access
    const T& at(int i, int j, int k) const
      { assert(validIndices(i,j,k)); return m_data[getIndex(i,j,k)]; }
    T& at(int i, int j, int k) 
      { assert(validIndices(i,j,k)); return m_data[getIndex(i,j,k)]; }
	 
    // 1D access
    const T& operator[](int i) const 
      { assert(validIndex(i)); return m_data[i]; };
    T& operator[](int i) { assert(validIndex(i)); return m_data[i]; };

    // linear interpolation 
    // only works if T is something that CAN be interpolated!
    // (really designed for doubles)
    // takes 'fractional indices' indicating point in index space for 
    // which values should be calculated
    const T interpolate(double fix, double fiy, double fiz) const;

  private:
    int m_i, m_j, m_k;			// size of array in each dimension
    int m_n;				// total size    
    T * m_data;				// standard array proved faster
					// than vector; Molecule's concentration
					// access is key

    // private functions
    bool validIndex(int i) const { return i>=0 && i<m_n; };
    bool validIndices(int i, int j, int k) const
	{ return i>=0 && i<m_i && j>=0 && j<m_j && k>=0 && k<m_k; };

    // convert 3D indices to single index
    int getIndex(int i, int j, int k) const
    	{ return i*(m_j*m_k) + j*m_k + k; };

    // not used
    Array3D(const Array3D &p);	
    Array3D& operator = (const Array3D &p);
};


// template member functions have to be in header - 
// compiler doesn't handle export

template<class T> void Array3D<T>::resize(int i, int j, int k)
{ 
  m_i=i; m_j=j; m_k=k; 
  m_n=i*j*k; assert(m_n);
  delete [] m_data; 
  m_data = new T[m_n]; 
};

template<class T>
const T Array3D<T>::interpolate(double fix, double fiy, double fiz) const
{
  assert( validIndices( int(fix), int(fiy), int(fiz) ) ); 

  // indices for lower-bound stored value
  int xi = int(fix); int yi = int(fiy); int zi = int(fiz);

  // interpolation parameters in each dimension
  double fx = fix - xi; double fy = fiy - yi; double fz = fiz - zi;

  // interpolate - using 8 known values surrounding unknown value
  T value = (1-fx)*(1-fy)*(1-fz)*m_data[getIndex(xi,yi,zi)];
  value += fx*(1-fy)*(1-fz)*m_data[getIndex(xi+1,yi,zi)];
  value += fx*fy*(1-fz)*m_data[getIndex(xi+1,yi+1,zi)];
  value += (1-fx)*fy*(1-fz)*m_data[getIndex(xi,yi+1,zi)];
  value += (1-fx)*(1-fy)*fz*m_data[getIndex(xi,yi,zi+1)];
  value += fx*(1-fy)*fz*m_data[getIndex(xi+1,yi,zi+1)];
  value += fx*fy*fz*m_data[getIndex(xi+1,yi+1,zi+1)];
  value += (1-fx)*fy*fz*m_data[getIndex(xi,yi+1,zi+1)];
                           
  return value;
}

#endif

