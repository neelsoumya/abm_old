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
 * file app.cc                                                          * 
 * Routines for App class                                               *    
 * Sets up windows and controls for simulation.                         * 
 ************************************************************************/

#include "app.h"
#include "simFrame.h"

IMPLEMENT_APP(App)

/************************************************************************ 
 * App()                                                                *
 *   Constructor - base class constructor used                          *
 ************************************************************************/

/************************************************************************ 
 * OnInit()                                                             *
 *   The main procedure for this wxWindows program                      *
 *                                                                      *
 * Parameters - none                                                    *
 *                                                                      *
 * Returns - boolean status indicator                                   *
 ************************************************************************/

bool App::OnInit()
{
  frame = new SimFrame( "Sim Window", wxSize(600,600));
  frame->SetSizeHints(600,600);	// min frame size      
  SetTopWindow(frame);
  frame->Center();
  frame->Show(true);
  return true;
}

