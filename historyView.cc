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
 * file historyView.cc                                                  * 
 * render() routine for HistoryView                                     *    
 * Graphs cell populations and molecular concentrations over time       * 
 * This version plots with wx/GTK instead of openGL - can use text      * 
 ************************************************************************/
                                                                        
#include "historyView.h"
#include <GL/gl.h>  
#include "history.h"

/************************************************************************ 
 * HistoryView()                                                        *
 *   Constructor                                                        *
 *                                                                      *
 * Parameters                                                           *
 *   wxWindow *parent:          window containing this view             *
 *   int x, y, w, h:  		position (x,y) and size (w,h) of view   *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
HistoryView::HistoryView(wxWindow *parent, int x, int y, int w, int h, 
	const History *hist) 
	: SimView(parent, x, y, w, h), history(hist), m_first(true)
{
  m_border = 100;
}

/************************************************************************ 
 * render()                                                             *
 *   wx/GTK code to draw history of sim                                 *
 *                                                                      *
 * Parameters -                                                         *
 *   wxPaintDC& dc:	device context for writing text to window       *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void HistoryView::render(wxPaintDC &dc)
{
  // setup for text                 
  dc.SetMapMode(wxMM_TEXT);
  dc.SetUserScale(1.0, 1.0);
  dc.SetLogicalOrigin(0, 0);
  // dc.SetAxisOrientation( );
  dc.SetBackgroundMode(wxSOLID);
  dc.SetTextForeground(*wxBLACK);
  dc.SetTextBackground(*wxWHITE);
  dc.SetFont( wxFont(10, wxMODERN, wxNORMAL, wxNORMAL, FALSE) );

  dc.Clear();

  // get number of samples taken and max value for each axis
  const vector<double>& times = history->getTimes();
  int numsamples = times.size();
  double maxTime = times[numsamples-1];
  int maxCells = history->getMaxCount();
  double maxConc = history->getMaxConc();

  // set pen for drawing axes, will change color below for data
  wxPen pen( wxPen( "black", 0, wxSOLID ) );
  dc.SetPen( pen );
  drawAxes(dc, maxTime, maxCells, maxConc);

  // don't bother plotting unless there are at least two data points
  if (numsamples > 1)
  {
    // get cell and molecule data
    History::CellHistory counts;
    History::ConcHistory concs;

    // draw actual data
    int t0 = m_border;
    int tmax = width-m_border;
    double tscale = (tmax-t0)/maxTime;
    int y0 = height-m_border;
    int ymax = m_border;

    // cell counts first - scale according to max count so far
    if (maxCells == 0)
    {
      if (int last = history->getNumCellTypes())  
      {
        // just draw one line (last color) along x axis
        pen.SetColour(cell_palette[last].red(), cell_palette[last].green(),
		cell_palette[last].blue());
        dc.SetPen( pen );
	dc.DrawLine(t0, y0, tmax, y0);
      }
    }
    else
    {
      double yscale = (y0-ymax)/double(maxCells);

      // each loop handles drawing one cell type
      for (int i=0; i<history->getNumCellTypes(); i++)
      {
        counts = history->getCounts(i);
        // set color according to palette 
        pen.SetColour(cell_palette[i].red(), cell_palette[i].green(),
		cell_palette[i].blue());
        dc.SetPen( pen );
        for (int j=0; j<numsamples-1; j++)
	  dc.DrawLine( int(tscale*times[j]+t0), y0-int(yscale*counts[j]),
		int(tscale*times[j+1] + t0), y0 - int(yscale*counts[j+1]) );
      }	// end for loop
    }	// end if maxCells not 0

    // rescale for molecular concentrations
    if (maxConc == 0)
    {
      if (int last = history->getNumMolTypes())  
      {
        // just draw one line (last color) along x axis
        pen.SetColour(mol_palette[last].red(), mol_palette[last].green(),
		mol_palette[last].blue());
        dc.SetPen( pen );
	dc.DrawLine(t0, y0, tmax, y0);
      }
    }
    else
    {
      double yscale = (y0-ymax)/maxConc;

      // each loop handles drawing one molecule type
      for (int i=0; i<history->getNumMolTypes(); i++)
      {
        concs = history->getConc(i);
        // set color according to palette 
        pen.SetColour(mol_palette[i].red(), mol_palette[i].green(),
		mol_palette[i].blue());
        dc.SetPen( pen );
        for (int j=0; j<numsamples-1; j++)
	  dc.DrawLine( int(tscale*times[j] + t0), y0 - int(yscale*concs[j]),
		int(tscale*times[j+1] + t0), y0 - int(yscale*concs[j+1]) );
      }	// end for loop
    }   // end if maxConc not 0
  }	// end if more than one sample

}

/************************************************************************ 
 * drawAxes()                                                           *
 *   Sets up xaxis (time) and 2 yaxes (one for cells and one for 	*
 *   molecules)    							*
 *                                                                      *
 * Parameters -                                                         *
 *   wxDC &dc:		device context for drawing                      *
 *   double maxTime:	max value on y axis         	                *
 *   int maxCells:	max value on left y axis          	        *
 *   double maxConc:    max value on right y axis	                *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void HistoryView::drawAxes(wxDC &dc, double maxTime, 
				int maxCells, double maxConc)
{
  // draw lines for axes
  dc.DrawLine( m_border, m_border, m_border, height-m_border );
  dc.DrawLine( m_border, height-m_border, width-m_border, height-m_border);
  dc.DrawLine( width-m_border, height-m_border, width-m_border, m_border );

  // draw labels
  wxString label;
  long len, ht, des;		// length, height, descent of text string

  label.Printf("time (seconds)");
  dc.GetTextExtent(label, &len, &ht, &des);
  dc.DrawText(label, (width-len)/2 , height-m_border/2);

  label.Printf("# cells");
  dc.GetTextExtent(label, &len, &ht, &des);
  dc.DrawRotatedText(label, ht, (height+len)/2, 90);

  label.Printf("concentration (Moles/ml)");
  dc.GetTextExtent(label, &len, &ht, &des);
  dc.DrawRotatedText(label, width-2*ht, (height+len)/2, 90);

  // x axis (time)
  label.Printf("%.1f", maxTime);
  dc.DrawText(label, width-m_border, height-4*m_border/5);

  // right y axis (# cells)
  label.Printf("%d", maxCells);
  dc.DrawText(label, m_border/2, m_border);

  // left y axis (concentration)
  label.Printf("%.1e", maxConc);
  dc.DrawText(label, width-m_border+2, m_border);

}

