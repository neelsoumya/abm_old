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
 * file simView.h                                                       *
 * Declarations for abstract SimView class -                            * 
 *   derived classes must define render()                               * 
 * Derived from wxGLCanvas; needed in order to use use openGL rendering *
 * commands in wxWindows window frame                                   *
 ************************************************************************/

#ifndef SIMVIEW_H
#define SIMVIEW_H

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <vector>
#include "color.h"

class SimView : public wxGLCanvas
{
  public:
    //--------------------------- CREATORS --------------------------------- 
    SimView(wxWindow *parent, int x, int y, int w, int h);
    // copy constructor not used
    virtual ~SimView() {};		// no dynamic memory allocated

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used
    virtual void render(wxPaintDC& dc) = 0;	// specific to derived classes

    //----------------------- EVENT HANDLERS ------------------------------- 
    void OnSize(wxSizeEvent& e);
    void OnPaint(wxPaintEvent& e);
    void OnEraseBackground() {};	// prevent unwanted erases

  protected:
    int width, height;			// of window
    vector<Color> mol_palette;		// colors to use for molecules
    vector<Color> cell_palette;		// ... and cells (by index)

    // required for event handling
    DECLARE_EVENT_TABLE()

    // not used
    SimView(const SimView &rsv);
    SimView operator = (const SimView &rsv);
};

#endif

