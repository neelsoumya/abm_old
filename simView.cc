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
 * file simView.cc                                                      * 
 * Routines for SimView class                                           *    
 * Generic tissue display - derived classes must define render()        * 
 ************************************************************************/
                                                                        
#include "simView.h"
#include <GL/gl.h>  

/************************************************************************ 
 * Event Table                                                          * 
 ************************************************************************/
BEGIN_EVENT_TABLE(SimView,wxGLCanvas)
  EVT_PAINT(SimView::OnPaint)
  EVT_SIZE(SimView::OnSize)
  EVT_ERASE_BACKGROUND(SimView::OnEraseBackground)
END_EVENT_TABLE()

/************************************************************************ 
 * SimView()                                                            *
 *   Constructor                                                        *
 *   Sets up (hard-coded) color palettes                                *
 *                                                                      *
 * Parameters                                                           *
 *   wxWindow *parent:          window containing this view             *
 *   int x, y, w, h:  		position (x,y) and size (w,h) of view   *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
SimView::SimView(wxWindow *parent, int x, int y, int w, int h)
	: wxGLCanvas(parent, -1, wxPoint(x, y), wxSize(w, h)), 
	  width(w), height(h)
{
  int alpha = 90;		// used for blending molecule colors

  // molecule colors
  mol_palette.push_back(Color(128,128,128,alpha));
  mol_palette.push_back(Color(128,128,0,alpha));
  mol_palette.push_back(Color(0,128,128,alpha));
  mol_palette.push_back(Color(128,0,128,alpha));
  mol_palette.push_back(Color(0,0,128,alpha));
  mol_palette.push_back(Color(0,128,0,alpha));
  mol_palette.push_back(Color(128,0,0,alpha));

  // cell colors
  cell_palette.push_back(Color(255,0,0));
  cell_palette.push_back(Color(0,255,0));
  cell_palette.push_back(Color(0,0,255));
  cell_palette.push_back(Color(255,255,0));
  cell_palette.push_back(Color(0,255,255));
  cell_palette.push_back(Color(255,0,255));
  cell_palette.push_back(Color(255,255,255));
}

/************************************************************************ 
 * OnSize()                                                             *
 *   Event handler for resizing window	                                *
 *                                                                      *
 * Parameters                                                           *
 *   wxSizeEvent& event:	size event				*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimView::OnSize(wxSizeEvent &e)
{
  GetClientSize(&width, &height);
}

/************************************************************************ 
 * OnPaint()                                                            *
 *   Event handler for (re)drawing view                                 *
 *                                                                      *
 * Parameters                                                           *
 *   wxWindow *parent:          window containing this view             *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimView::OnPaint(wxPaintEvent &e)
{
  wxPaintDC dc(this);		// required to create this in OnPaint handler

  // SetCurrent and SwapBuffers moved to derived classes that use OpenGL;
  // one derived class uses a different method of drawing, so don't want
  // OpenGL stuff here

//  SetCurrent();		// directs OpenGL commands to this window
  render(dc);			// my OpenGL code for this view
//  SwapBuffers();		// show above changes
}

