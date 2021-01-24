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
 * file historyView.h                                                   *
 * Declarations for HistoryView class                                   * 
 * Shows graph of population levels and average concentrations          * 
 * Derived from abstract class SimView                                  *
 ************************************************************************/

#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include "simView.h"

class History;

class HistoryView : public SimView   
{
  public:
    //--------------------------- CREATORS ---------------------------------- 
    HistoryView(wxWindow *parent, int x, int y, int w, int h, 
	const History *hist);	
    // copy constructor not used
    // ~HistoryView()			// use default destructor

    //------------------------- MANIPULATORS ------------------------------- 
    // assignment not used
    void render(wxPaintDC& dc);

  private:
    const History *history;
    bool m_first;

    int m_border;		// width of area outside graph axes

    // private functions
    void drawAxes(wxDC &dc, double maxTime, int maxCells, double maxConc);

    // not used
    HistoryView(const HistoryView &rhv);
    HistoryView operator = (const HistoryView &rhv);
};

#endif

