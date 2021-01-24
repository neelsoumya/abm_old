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
 * file dataDialog.h                                                    *
 * Declarations for DataDialog class                                    * 
 * text summary of current cell counts and average molecular 		*
 * concentrations     							*
 ***********************************************************************/

#ifndef DATADIALOG_H
#define DATADIALOG_H

#include <wx/wx.h>
#include <vector>
#include <string>
class Tissue;
class History;

using namespace std;

class DataDialog : public wxDialog
{
  public:
    DataDialog(wxWindow *parent);

    void initialize(const Tissue &tr);
    void update(const History &hr);

  private:
    wxTextCtrl *cells_text;
    wxTextCtrl *attr_text;
    wxTextCtrl *mols_text;

    int m_numCellTypes, m_numMolTypes;
    vector<wxString> m_cellNames;
    vector<wxString> m_attrNames;
    vector<wxString> m_molNames;
};

#endif

