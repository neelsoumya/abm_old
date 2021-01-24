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
 * file app.h                                                           *
 * Declarations for App class                                           * 
 * This class is required in wxWindows programs; OnInit() functions     *
 * as main().                                                            *
 *----------------------------------------------------------------------*
 * class App                                                            *
 *    Public member functions:                                          *
 *      OnInit()                                                        *
 ************************************************************************/

#include <wx/wx.h>

class SimFrame;      
  
class App : public wxApp
{
  public:
    virtual bool OnInit();
  private:
    SimFrame* frame;
};

