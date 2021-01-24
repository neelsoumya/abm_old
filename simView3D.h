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
 * file simView3D.h                                                     *
 * Declarations for SimView3D class                                     * 
 * Derived from abstract class SimView                                  *
 ************************************************************************/

#ifndef SIMVIEW3D_H
#define SIMVIEW3D_H

#include "simView.h"
#include <GL/gl.h>

class Tissue;

class SimView3D : public SimView   
{
  public:
    //--------------------------- CREATORS --------------------------------- 
    SimView3D(wxWindow *parent, int x, int y, int w, int h, const Tissue *t);	
    // copy constructor not used
    // ~SimView3D()			// use default destructor

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used
    void render(wxPaintDC& dc);

  private:
    const Tissue *m_tissuep;		// pointer to model
    bool m_first;			// flag for first call to render

    // local copies of model info needed for drawing
    int m_xsize, m_ysize, m_zsize;
//    int m_gridsize;				// sizes in microns
//    int m_xnum, m_ynum, m_znum;		// # of grid cells
    double *m_cellSizes;			// radii in microns

    GLUquadricObj *quadratic;     

    // private functions
    void renderCells();
    void drawCell();
    void renderMolecules();
    void drawGrid();

    // not used
    SimView3D(const SimView3D &rsv);
    SimView3D operator = (const SimView3D &rsv);
};

#endif

