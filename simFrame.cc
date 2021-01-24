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
 * file simFrame.cc                                                     * 
 * Routines for SimFrame class                                          *    
 * Top-level class for displays associated with sim; manages            * 
 * all controls, uses View classes to display model                     * 
 ************************************************************************/

#include "simFrame.h"
#include <iostream> 
#include <cassert> 
#include "simView.h"
#include "historyView.h"
#include "simView3D.h"
#include "tissue.h"
#include "history.h"
#include "fileDef.h"
#include "fileInit.h"
#include "dataDialog.h"
#include "util.h"

using namespace std;

/************************************************************************ 
 * Event Table - sets up proper associations to respond to buttons      * 
 ************************************************************************/
BEGIN_EVENT_TABLE(SimFrame,wxFrame)
  EVT_BUTTON(ID_LOADDEF, SimFrame::onLoadDef)
  EVT_BUTTON(ID_LOADINIT, SimFrame::onLoadInit)
  EVT_BUTTON(ID_RUN, SimFrame::onRun)
  EVT_IDLE(SimFrame::onIdle)
  EVT_BUTTON(ID_HISTORY, SimFrame::onWriteHistory)
  EVT_BUTTON(ID_DETAIL, SimFrame::onWriteDetail)
  EVT_BUTTON(ID_MOLTYPE, SimFrame::onMolType)
  EVT_BUTTON(ID_CELLTYPE, SimFrame::onCellType)
  EVT_BUTTON(ID_GEOMETRY, SimFrame::onGeometry)
  EVT_BUTTON(ID_SETSEED, SimFrame::onSetSeed)
  EVT_MENU(Menu_Settings_Duration,	SimFrame::onSetDur)
  EVT_MENU(Menu_Settings_Timestep,	SimFrame::onSetStep)
  EVT_MENU(Menu_Settings_View,		SimFrame::onSetView)
  EVT_MENU(Menu_Settings_Update,	SimFrame::onSetUpdate)
END_EVENT_TABLE()

/************************************************************************ 
 * SimFrame()                                                           *
 *   Constructor                                                        *
 *                                                                      *
 * Parameters                                                           *
 *   wxString title: 	 	displayed at top of main window         *
 *   wxSize size:               size of main window                     *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
SimFrame::SimFrame(wxString title, wxSize size)
    : wxFrame((wxFrame*)NULL, -1, title, wxDefaultPosition, size),
      m_status(BLANK), m_tissuep(0), m_timestep(1), m_duration(100000),
      m_notebookp(0), m_viewp(0), m_viewInterval(100), m_dialog(0),
      m_historyp(0), 
      loaddef_button(0), loadinit_button(0), run_button(0),
      hist_button(0), moltype_button(0), celltype_button(0), geometry_button(0),
      moltype_text(0), celltype_text(0), geometry_text(0) 
{
    wxString info;			// used to get data for text controls

    CreateStatusBar(2);

    // create tissue model - sets seed & default model
    m_tissuep = new Tissue();

    // create notebook
    m_notebookp = new wxNotebook(this, ID_NOTEBOOK);
    wxPanel *panel;		// use to add pages to notebook

    //---------------Model Definition page-------------------------

    panel = new wxPanel(m_notebookp);

    // define buttons
    loaddef_button = new wxButton(panel, ID_LOADDEF, "Define From File",
		wxPoint(10,10), wxDefaultSize);

    // display basic current model info; add buttons to redefine
    moltype_button = new wxButton(panel, ID_MOLTYPE, "Molecule Types", 
		wxPoint(10, 100), wxSize(150, 25));
    info.Printf(_("%d"), m_tissuep->getNumMolTypes());
    moltype_text = new wxTextCtrl(panel, -1, info,
	wxPoint(200, 100), wxSize(25, 25), wxTE_READONLY); 
    celltype_button = new wxButton(panel, ID_CELLTYPE, "Cell Types", 
		wxPoint(10, 200), wxSize(150, 25));
    info.Printf(_("%d"), m_tissuep->getNumCellTypes());
    celltype_text = new wxTextCtrl(panel, -1, info,
	wxPoint(200, 200), wxSize(25, 25), wxTE_READONLY); 

// to save model definition
//    m_tissuep->writeDefinition("checkinit.cfg");

    m_notebookp->AddPage(panel, "Model Definition", TRUE);

    //-------------Model Initialization page-----------------------

    panel = new wxPanel(m_notebookp);

    // define buttons
    seed_button = new wxButton(panel, ID_SETSEED, 
	"Set Seed", wxPoint(10,10), wxDefaultSize );

    loadinit_button = new wxButton(panel, ID_LOADINIT, 
	"Initialize From File", wxPoint(100,10), wxDefaultSize );

    geometry_button = new wxButton(panel, ID_GEOMETRY, "Geometry", 
		wxPoint(10, 100), wxSize(150, 25));
    getGeometryInfo(info);
    geometry_text = new wxTextCtrl(panel, -1, info,  
	wxPoint(200, 90), wxSize(200, 45), wxTE_READONLY | wxTE_MULTILINE); 
// to save model initialization
//    m_tissuep->writeData("checkinit.dat");

    m_notebookp->AddPage(panel, "Model Initialization", TRUE);

    //-------------------- Run page -------------------------------

    panel = new wxPanel(m_notebookp);

    // define buttons
    run_button = new wxButton(panel, ID_RUN, "Run", wxPoint(10,10), 
		wxDefaultSize );
    hist_button = new wxButton(panel, ID_HISTORY, "Write History", 
		wxPoint(160,10), wxDefaultSize );
    detail_button = new wxButton(panel, ID_DETAIL, "Write Snapshot", 
		wxPoint(310,10), wxDefaultSize );

    m_notebookp->AddPage(panel, "Run", TRUE);

    //---------------------  menu  setup --------------------------------
    wxMenu *settingsMenu = new wxMenu;
    settingsMenu->Append(Menu_Settings_Duration, "D&uration\tAlt-D", 
			"Set sim duration");
    settingsMenu->Append(Menu_Settings_Timestep, "T&imestep\tAlt-T", 
			"Set sim timestep");
    settingsMenu->Append(Menu_Settings_View, "V&iew\tAlt-V", 
			"Set sim timestep");
    settingsMenu->Append(Menu_Settings_Update, "Display U&pdate rate\tAlt-U", 
			"Set display update rate");

    wxMenuBar* menuBar = new wxMenuBar( wxMB_DOCKABLE );
    menuBar->Append(settingsMenu, "Settings");
    SetMenuBar(menuBar);

    m_notebookp->SetSelection(0);
}

/************************************************************************ 
 * onLoadDef                                                            *
 *   Event handler for Define From File button                          *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onLoadDef(wxEvent& event)
{
  if (m_status)		// should only define from file if status is BLANK (0)
  {
    // prompt to proceed by deleting Tissue and starting over, or cancel
    wxMessageDialog message(this, 
      "Redefining model will discard existing model.  Continue?",
      "Warning", wxOK|wxCANCEL|wxICON_EXCLAMATION);
    if (message.ShowModal() == wxID_OK)
    {
      delete(m_tissuep); 
      m_tissuep = new Tissue();
      m_status = BLANK;
      delete(m_historyp); m_historyp = 0;
      delete(m_viewp); m_viewp = 0;
    }
    else         
      return;  
  }

  wxFileDialog dialog(this, "Load Model Definition", "", "", "*.def", wxOPEN);
  if (dialog.ShowModal() == wxID_OK)
  {
    FileDef parser;
    parser.defineFromFile(m_tissuep, (const char*)dialog.GetPath());
    m_status = DEFINED;

    // update info on Model Definition page
    wxString info;			// used to get data for text controls
    info.Printf(_("%d"), m_tissuep->getNumMolTypes());
    moltype_text->Clear(); moltype_text->AppendText(info);
    info.Printf(_("%d"), m_tissuep->getNumCellTypes());
    celltype_text->Clear(); celltype_text->AppendText(info);
  }
}

/************************************************************************ 
 * onMolType                                                            *
 *   Event handler for Molecule Types button                            *
 *   Launches routines/displays to handle definition of molecule types  *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onMolType(wxEvent& event)
{
  cout << "Interactive molecule type definitions not implemented yet!" << endl;
}

/************************************************************************ 
 * onCellType                                                           *
 *   Event handler for Cell Types button                                *
 *   Launches routines/displays to handle definition of cell types      *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onCellType(wxEvent& event)
{
  cout << "Interactive cell type definitions not implemented yet!" << endl;
}

/************************************************************************ 
 * getGeometryInfo                                                      *
 *   Gets geometry description from Tissue object, assembles a string   *
 *   suitable for display on Model Definition page                      *
 *                                                                      *
 * Parameters                                                           *
 *   wxString& str: 	string to display                              	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::getGeometryInfo(wxString& str)
{
  str.Printf(_("%dx%dx%d microns\n%d microns/grid edge"), 
	m_tissuep->getXSize(), m_tissuep->getYSize(),
	m_tissuep->getZSize(), m_tissuep->getGridSize() );
}

/************************************************************************ 
 * onGeometry                                                           *
 *   Event handler for Geometry button                                  *
 *   Launches routines/displays to handle definition of tissue geometry *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onGeometry(wxEvent& event)
{
  cout << "Interactive geometry definition not implemented yet!" << endl;
}

/************************************************************************ 
 * onLoadInit                                                           *
 *   Event handler for Initialize From File button                      *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onLoadInit(wxEvent& event)
{
  if (m_status == BLANK)	
  {
    // if Tissue hasn't been defined, initializing from file won't work 
    wxMessageDialog message(this, 
      "Model must be defined before initializing.",
      "Warning", wxOK||wxICON_EXCLAMATION);
    message.ShowModal();
    return;  
  }

  if (m_status > DEFINED)		// reinitialization
  { // just warn - option to cancel given below
    wxMessageDialog message(this, 
      "Reinitializing",		
      "Warning", wxOK||wxICON_EXCLAMATION);
    message.ShowModal();

    // note that file initializations include geometry, and Tissue::setGeometry
    // always removes existing cells and molecules

    // reset simtime
    m_tissuep->setTime(0);

    // delete existing history; reinitialize it below           
    delete(m_historyp); 

    // have to delete view in case it's a history view
    delete(m_viewp); m_viewp = 0;
  }

  wxFileDialog dialog(this, "Load Model Initialization", "", "", "*.init", 
	wxOPEN);
  if (dialog.ShowModal() == wxID_OK)
  {
    FileInit parser;
    parser.initFromFile(m_tissuep, (const char*)dialog.GetPath());
    m_status = INITIALIZED;
    setupHistory();

    // update info on Model Initialization page
    wxString info;			// used to get data for text controls
    getGeometryInfo(info); 
    geometry_text->Clear(); geometry_text->AppendText(info);

    // also set up text display of current data
    if (!m_dialog)
    {
      m_dialog = new DataDialog(this);
      m_dialog->Show(TRUE);
    }
    m_dialog->initialize(*m_tissuep);

    update();
  }
}

/************************************************************************ 
 * onSetSeed()                                                          *
 *   Event handler for Set Seed button                                  *
 *   Brings up dialog box to set RNG seed                               *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event:		event that triggers this call;          *
 *				not really used			        *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onSetSeed(wxEvent & event)
{
// GetNumberFromUser does something weird, & spin control doesn't work
// anyway - just use text dialog

  wxString tmp1; 
  long value;

  tmp1.Printf(_("%ld"), m_tissuep->getSeed());
  wxString tmp = wxGetTextFromUser("Enter seed", "Set seed", tmp1, this);
  if ( wxSscanf(tmp, _T("%ld"), &value) == 1 )
    if ( value >= 0 && value < 2000000000 )
      m_tissuep->setSeed(value);
    else 
    {
      wxMessageDialog message(this, 
        "Invalid value; seed not changed",
        "Warning", wxOK||wxICON_EXCLAMATION);
      message.ShowModal();
    }
}


/************************************************************************ 
 * onSetStep()                                                          *
 *   Event handler for Set Timestep button                              *
 *   Brings up dialog box to set simulation timestep                    *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event:		event that triggers this call;          *
 *				not really used			        *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onSetStep(wxEvent & event)
{
  // save current timestep
  double old = m_timestep;

  // set up view choices                                            
  const wxString choices[] = { "1 second", "10 seconds", "1 minute", "1 hour" }; 
  wxSingleChoiceDialog choice(this, "", "Select Timestep", 4, choices);
  if (choice.ShowModal() == wxID_OK) 
  {
    switch (choice.GetSelection())
    {
      // units are in seconds
      case 0:	// 1 second      
	m_timestep = 1;
	break;
      case 1:	// 10 seconds
	m_timestep = 10;
	break;
      case 2:	// 1 minute      
	m_timestep = 60;
	break;
      case 3:	// 1 hour      
	m_timestep = 3600;
	break;
    }
  }

  // make sure timestep not too big for spatial resolution:
  int size = m_tissuep->getGridSize();
  if (size)
  {
    double constraint = size*size / m_tissuep->getMaxDiffRate();
    if ( m_timestep > constraint )
    {
      m_timestep = old;
      wxLogError("timestep too big for gridsize; not changed");
    }
  }
}

/************************************************************************ 
 * onSetDur()                                                           *
 *   Event handler for Set Duration					*
 *   Brings up dialog box to set simulation timestep                    *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event:	event that triggers this call;          *
 *				not really used			        *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onSetDur(wxEvent & event)
{
  wxString tmp1; 
  long value;

  tmp1.Printf(_("%ld"), m_duration);
  wxString tmp = wxGetTextFromUser("Enter duration", "Set duration", 
	tmp1, this);
  if ( wxSscanf(tmp, _T("%ld"), &value) == 1 )
    if ( value >= 0 && value <= 1000000 )
      m_duration = value;        
    else 
    {
      wxMessageDialog message(this, 
        "Invalid value; duration not changed",
        "Warning", wxOK||wxICON_EXCLAMATION);
      message.ShowModal();
    }
}

/************************************************************************ 
 * onSetUpdate()                                                        *
 *   Event handler for Set Display Update Rate				*
 *   Brings up dialog box to set display update interval                *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event:	event that triggers this call; 		        *
 *				not really used			        *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onSetUpdate(wxEvent & event)
{
  wxString tmp1; 
  int value;

  tmp1.Printf(_("%d"), m_viewInterval);
  wxString tmp = wxGetTextFromUser("Enter update interval (in #timesteps)", 
	"Set update rate", tmp1, this);
  if ( wxSscanf(tmp, _T("%d"), &value) == 1 )
    if ( value >= 0 && value < 1000000 )
      m_viewInterval = value;        
    else 
    {
      wxMessageDialog message(this, 
        "Invalid value; update rate not changed",
        "Warning", wxOK||wxICON_EXCLAMATION);
      message.ShowModal();
    }
}
/************************************************************************ 
 * onSetView()                                                          *
 *   Event handler for Set View button                                  *
 *                                                                      *
 * Parameters                                                           *
 *   wxCommandEvent event:	event that triggers this call;          *
 *				not really used			        *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onSetView(wxCommandEvent & event)
{
  // set up view choices                                            
  const wxString choices[] = { "history view", "3D view", "none" };
  wxSingleChoiceDialog choice(this, "", "Select view", 3, choices);
  if (choice.ShowModal() == wxID_OK) 
  {
    wxSize size = GetClientSize();
    switch (choice.GetSelection())
    {
      case 0:	// history view
        delete m_viewp;
        m_viewp = new HistoryView(m_notebookp->GetPage(2), 50, 50, 
	      size.GetWidth()-100, size.GetHeight()-100, m_historyp);
	SetStatusText("", 1);
        break;
      case 1:	// 3D view
        delete m_viewp;
        m_viewp = new SimView3D(m_notebookp->GetPage(2), 50, 50, 
	      size.GetWidth()-100, size.GetHeight()-100, m_tissuep);
	SetStatusText("", 1);
        break;
      case 2:	// no view
        delete m_viewp;
        m_viewp = 0;
	SetStatusText("", 1);
        break;
    }	// end switch on view choice
  }	// end if choice made
}

/************************************************************************ 
 * onRun() and toggleRun                                                *
 *   onRun is actual Event handler for Run button                       *
 *   toggleRun does actual work; separated from onRun so it could be    *
 *   called from within the application.                                *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 		event that triggers this call;          *
 *				not really used			        *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onRun(wxEvent& event)
{
  toggleRun();
}

void SimFrame::toggleRun()
{
  assert(m_tissuep);
  switch (m_status)
  {
    case INITIALIZED: 	
      // fall through to actually start sim running
    case STOPPED:	
      m_status = RUNNING;
      run_button->SetLabel("Stop");
      break; 
    case RUNNING:
      m_status = STOPPED;
      run_button->SetLabel("Run");
      update(); 		// display should reflect last changes to sim 
      break;
    default:
      wxLogError("Model has not been initialized; nothing to run");
  }
}

/************************************************************************ 
 * onIdle                                                               *
 *   Actual simulation loop, but interruptable                          *
 *                                                                      *
 * Parameters                                                           *
 *   wxIdleEvent event: 	event that triggers this call		*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onIdle(wxIdleEvent& event)
{
  if (m_status == RUNNING)
  {
    int t = int(m_tissuep->getTime());
    if ( t < m_duration)
    {
      m_tissuep->update(m_timestep);        // t now t+deltaT
      m_historyp->sample(*m_tissuep);
      int n = int((t+m_timestep)/m_timestep);	// number of steps taken
      if ( ( n % m_viewInterval ) == 0)
        update();
      event.RequestMore();
    }
    else
      toggleRun();	// stop automatically at specified duration
  }
}

/************************************************************************ 
 * onWriteHistory                                                       *
 *   Event handler for WriteHistory button                              *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onWriteHistory(wxEvent& event)
{
  wxFileDialog dialog(this, "Save History", "", "", "*.hist", 
	wxSAVE|wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK)
  {
    char buf[400];
    sprintf(buf, "%s", (const char*)dialog.GetPath());
    ofstream outfile(buf); 
    if (!outfile)
      error("Error:  could not open file", buf);

    outfile << *m_historyp;  
    outfile.close();  
  }
}

/************************************************************************ 
 * onWriteDetail                                                        *
 *   Event handler for WriteDetail button                               *
 *                                                                      *
 * Parameters                                                           *
 *   wxEvent event: 	event that triggers this call; not really used	*
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::onWriteDetail(wxEvent& event)
{
  wxFileDialog dialog(this, "Save Snapshot", "", "", "*.detail", 
	wxSAVE|wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK)
  {
    char buf[400];
    sprintf(buf, "%s", (const char*)dialog.GetPath());
    ofstream outfile(buf); 
    if (!outfile)
      error("Error:  could not open file", buf);

    outfile.close();  
    m_tissuep->writeData(buf);
  }
}
/************************************************************************ 
 * update()                                                             *
 *   Updates display(s)                                                 *
 *                                                                      *
 * Parameters - none                                                    *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::update()
{
  // update time display
  char buf[200];
  sprintf(buf, "Elapsed time = %.1f seconds", m_tissuep->getTime());
  SetStatusText(buf, 0);

  // update view & msg about what view this is
  if (m_viewp)
    m_viewp->Refresh(false);

  // update text display of cell counts & concentration data
  m_dialog->update(*m_historyp);
}

/************************************************************************ 
 * setupHistory()                                                       *
 *   Creates History object to record data during run and takes first   *
 *   sample - should only be called after Tissue initialized and        *
 *   before running.                                                    *
 *                                                                      *
 * Parameters - none                                                    *
 *				                		        *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimFrame::setupHistory()
{
    assert(m_tissuep);
    m_historyp = new History(*m_tissuep, 0);
    m_historyp->sample(*m_tissuep);
}

