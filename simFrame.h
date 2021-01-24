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
 * file simFrame.h                                                      *
 * Declarations for SimFrame class                                      * 
 * Attributes and behavior for top-level frame of simtissue program.    *
 * Includes status bar, buttons, and views of the tissue model.         *
 ************************************************************************/

#ifndef SIMFRAME_H
#define SIMFRAME_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/glcanvas.h>

class Tissue;
class SimView;
class History;
class DataDialog;

// want all identifiers above range of built-in wxWindows constants
#define ID_0 (wxID_HIGHEST+1)

// upper level identifiers
#define ID_NOTEBOOK (ID_0+100)

// button identifiers
#define ID_LOADDEF (ID_0)
#define ID_LOADINIT (ID_0+1)
#define ID_RUN (ID_0+2)
#define ID_HISTORY (ID_0+3)
#define ID_DETAIL (ID_0+4)
#define ID_MOLTYPE (ID_0+5)
#define ID_CELLTYPE (ID_0+6)
#define ID_GEOMETRY (ID_0+7)
#define ID_SETSEED (ID_0+8)

// menu identifiers
#define Menu_Settings_Duration (ID_0+10)
#define Menu_Settings_Timestep (ID_0+11)
#define Menu_Settings_View (ID_0+12)
#define Menu_Settings_Update (ID_0+13)

class SimFrame : public wxFrame
{
  public:
    //--------------------------- CREATORS --------------------------------- 
    SimFrame(wxString title, wxSize size);	
    // copy constructor not used
    // ~SimFrame()			// use default destructor

    //----------------------- EVENT HANDLERS ------------------------------- 
    void onLoadDef(wxEvent & event);
    void onSetSeed(wxEvent & event);
    void onLoadInit(wxEvent & event);
    void onSetView(wxCommandEvent & event);
    void onRun(wxEvent & event);
    void onIdle(wxIdleEvent & event);
    void onWriteHistory(wxEvent & event);
    void onWriteDetail(wxEvent & event);
    void onMolType(wxEvent & event);
    void onCellType(wxEvent & event);
    void onGeometry(wxEvent & event);
    void onSetStep(wxEvent & event);
    void onSetDur(wxEvent & event);
    void onSetUpdate(wxEvent & event);

    // helper for onRun
    void toggleRun();

  private:

    enum State { BLANK, DEFINED, INITIALIZED, RUNNING, STOPPED };
    State m_status;			// current state of simulation

    Tissue *m_tissuep;			// model

    double m_timestep;			// time elapsed in one sim loop
    long m_duration;			// amount of time to run sim in sec

    wxNotebook *m_notebookp;		// holds tabbed pages
    SimView* m_viewp;			// main tissue display
    int m_viewInterval;			// how often to update displays
					// (in number of sim timesteps)

    DataDialog *m_dialog;		// text display of current data

    History *m_historyp;		// holds aggregate sim data

    // buttons
    wxButton* loaddef_button;		
    wxButton* loadinit_button;		
    wxButton* view_button;		
    wxButton* run_button;		
    wxButton* hist_button;		
    wxButton* detail_button;		
    wxButton* moltype_button;		
    wxButton* celltype_button;		
    wxButton* geometry_button;		
    wxButton* step_button;		
    wxButton* dur_button;		
    wxButton* seed_button;		

    // text controls
    wxTextCtrl* moltype_text;
    wxTextCtrl* celltype_text;
    wxTextCtrl* geometry_text;

    // private functions to manage simulation
    void getGeometryInfo(wxString& str);
    void setupHistory();		// creates & initializes History object
    void update();			// directs various display updates

    // required for event handling
    DECLARE_EVENT_TABLE()

    // not used
    SimFrame(const SimFrame &rsf);
    SimFrame operator = (const SimFrame &rsf);
};

#endif
