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
 * file dataDialog.cc                                                   *
 * Definitions for DataDialog class                                     * 
 * text summary of current cell counts and average molecular 		*
 * concentrations     							*
 ***********************************************************************/

#include "dataDialog.h"
#include "tissue.h"
#include "history.h"
#include "cellType.h"
#include "cell.h"

DataDialog::DataDialog(wxWindow *parent) : 
	wxDialog(parent, -1, wxString("Data Summary")), 
	cells_text(0), mols_text(0)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  // setup separate window w/text ctrls for cell/molecule data
  mols_text = new wxTextCtrl(this, -1, "Mol Type\tMoles/ml",
      wxDefaultPosition, wxSize(200,100), wxTE_MULTILINE|wxTE_READONLY);
  cells_text = new wxTextCtrl(this, -1, "Cell Type\tnumber",
      wxDefaultPosition, wxSize(200,100), wxTE_MULTILINE|wxTE_READONLY);  
  attr_text = new wxTextCtrl(this, -1, "Cell Attributes\ttotals",
      wxDefaultPosition, wxSize(200,200), wxTE_MULTILINE|wxTE_READONLY);  

  sizer->Add(mols_text, 1, wxEXPAND | wxALL, 5);
  sizer->Add(cells_text, 1, wxEXPAND | wxALL, 5);
  sizer->Add(attr_text, 1, wxEXPAND | wxALL, 5);

  SetAutoLayout(TRUE);
  SetSizer(sizer);

  sizer->SetSizeHints(this);
  sizer->Fit(this);
}

void DataDialog::initialize(const Tissue &tr)
{
  // empty vectors in case this is a redefinition
  m_cellNames.clear();
  m_attrNames.clear();
  m_molNames.clear();

  // get cell/attr/molecule type names for future reference
  m_numMolTypes = tr.getNumMolTypes();
  for (int i=0; i<m_numMolTypes; i++)
    m_molNames.push_back(wxString(tr.getMolecule(i).getName().c_str()));
  m_numCellTypes = tr.getNumCellTypes();
  for (int i=0; i<m_numCellTypes; i++)
  {
    const CellType *pct = tr.getCellType(i);
    m_cellNames.push_back(wxString(pct->getName().c_str()));
    for (int j=0; j<pct->getNumAttributes(); j++)
      m_attrNames.push_back(wxString(pct->getAttributeName(j).c_str()));
  }
}

void DataDialog::update(const History &hr)
{
  // clear current text; write over it
  mols_text->Clear();
  cells_text->Clear();
  attr_text->Clear();

  wxString info;

  // molecules 
  info.Printf(_("Mol Type\tMoles/ml\n"));
  mols_text->AppendText(info);
  for (int i=0; i<m_numMolTypes; i++)
  {
    mols_text->AppendText(m_molNames[i]);
    info.Printf(_("\t%.1e\n"), hr.getCurrentConc(i));
    mols_text->AppendText(info);
  }

  // cells header
  info.Printf(_("Cell Type\tnumber\n"));
  cells_text->AppendText(info);

  // cell attribute header
  info.Printf(_("Cell Type:Attribute\t\ttotal value\n"));
  attr_text->AppendText(info);

  int jj=0;
  for (int i=0; i<m_numCellTypes; i++)
  {
    cells_text->AppendText(m_cellNames[i]);
    info.Printf(_("\t%d\n"), hr.getCurrentCount(i));
    cells_text->AppendText(info);

    const vector<double>& totals = hr.getTotals(i);
    for (unsigned int j=0; j<totals.size(); j++)
    {
      attr_text->AppendText(m_cellNames[i]);
      attr_text->AppendText(":");
      attr_text->AppendText(m_attrNames[jj]);
      info.Printf(_("\t\t\t%g\n"), totals[j]);
      attr_text->AppendText(info);
      jj++;
    }
  }
}

