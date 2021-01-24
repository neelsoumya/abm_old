
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
 * file sense.cc                                                        *
 * Definitions for constructors and calculate() member functions of     *
 * various Sense classes.                                               *
 * Each concrete class represents a different function object, which is *
 * used to update one variable out of a list (corresponding to a single *
 * cell's internal variables) and may also update other parameters.     *
 ************************************************************************/

#include <cmath>      
#include "sense.h"
#include "cell.h"
#include "cells.h"
#include "molecule.h"
#include "random.h"

/************************************************************************
 * class SensePhag                                                      *
 *   Implementation of phagocytosis, using Cells::getTarget to pick a 	*
 * random cell within the appropriate distance (may not be able to find *
 * one).  First, phagocyte has to be able to bind target - capability   *
 * represented by some internal attribute of the phagocyte, which must  *
 * be larger than a specified threshold.  If it is, and if the returned *
 * cell is of the appropriate type, SensePhag will remove the 'eaten'   *
 * target cell and update a phagocyte attribute representing internal   *
 * target load.								*
 ************************************************************************/
SensePhag::SensePhag(int pattr, int targettype, double dist, int Rattr,
		double thr, Cells *cells) :
	m_pattr(pattr), m_targetType(targettype), m_dist(dist), 
	m_Rattr(Rattr), m_thr(thr), m_cells(cells)
{
  assert(m_pattr >= 0);
  assert(m_targetType >= 0);
  assert(m_dist >= 0);
  assert(m_Rattr >= 0);
  assert(m_thr >= 0);
  assert(cells);
}

void SensePhag::calculate(Cell *cell, double deltaT)
{ 
  if (cell->getValue(m_Rattr)>m_thr)
    if ( Cell *pc = m_cells->getTarget(cell, m_dist) )
      if ( pc->getTypeIndex() == m_targetType) 
    {
      pc->die();
      cell->setValue(m_pattr, cell->getValue(m_pattr)+1);
    }	    
}

/************************************************************************
 * class SenseCognate                                                   *
 *   Implementation of cell-cell sensing, using  Cells::getTarget to 	*
 *   determine whether there is a cell of the appropriate type within	*
 *   the appropriate distance.	Sets an internal flag variable 		*
 *   reflecting search result.						*
 ************************************************************************/
SenseCognate::SenseCognate(int pattr, int targettype, double dist, 
		Cells *cells) :
	m_pattr(pattr), m_targetType(targettype), m_dist(dist), m_cells(cells)
{
  assert(m_pattr >= 0);
  assert(m_targetType >= 0);
  assert(m_dist >= 0);
  assert(cells);
}

void SenseCognate::calculate(Cell *cell, double deltaT)
{ 
    if ( m_cells->checkNeighbors(cell, m_dist, m_targetType ) )
      cell->setValue(m_pattr, 1);
    else
      cell->setValue(m_pattr, 0);
}

/************************************************************************
 * class SenseCopyConc                                                  *
 *   In this case, the molecular concentration at a particular point is *
 * copied directly into the appropriate variable; no 'binding' takes    *
 * place and the molecular concentration is not changed.                *
 ************************************************************************/
SenseCopyConc::SenseCopyConc(int index, Molecule *field) :
	m_index(index), m_field(field)
{
  assert(m_index >= 0);
  assert(field);
}

void SenseCopyConc::calculate(Cell *cell, double deltaT)
{  
  double conc = m_field->getConc(cell->getPosition());
  cell->setValue(m_index, conc);
}

/************************************************************************
 * class SenseBindRev                                                   *
 *   This class carries out the basic receptor-ligand binding described *
 * by equation 2-8 in Lauffenberger's book                              *
 * Change in bound receptors = deltaT * (k_f*L_free*R_free - k_r*C)	*
 * Assumes total #receptors/cell does not change (no trafficking)	*
 * Here, total receptors is a constant passed in; it could also be 	*
 * another indexed Cell variable					*
 * Assumes monovalent ligand and receptor				*
 * Assumes reversible binding, no competition				*
 * Assumes conservation of the total receptors and ligand		*
 * The equation is used for well-mixed, large-number systems;		*
 * I'm intending to apply it to one cell at a time, using the 		*
 * concentration at that cell's location, assuming that it adequately 	*
 * represents the average over the cell's 'sensing volume'		*
 * The ligand conservation assumption above means I'm assuming that 	*
 * this reaction is independent of the diffusion and decay of ligand 	*
 * that is handled elsewhere and leads to:				*
 * change in number of molecules = -(change in bound receptors)		*
 * The indexed variable contained in Cell is assumed to represent the	*
 * number of bound receptors for the ligand modeled by the Molecule 	*
 ************************************************************************/
SenseBindRev::SenseBindRev(int index, Molecule *field, double kf, 
			   double kr, double r) :
   	m_index(index), m_field(field), m_kf(kf), m_kr(kr), m_r(r) 
{
  assert(m_index >= 0);
  assert(field);
  assert(m_kf>=0);
  assert(m_kr>=0);
  assert(m_r>0);
}

void SenseBindRev::calculate(Cell *cell, double deltaT)
{  
  // get local ligand concentration - will be in moles/ml
  double ligand = m_field->getConc(cell->getPosition());

  // get current number of bound receptors 
  double bound = cell->getValue(m_index);	
  assert(bound<=m_r);

  // determine how much #bound receptors changes
  double deltaBound = deltaT * (m_kf*(m_r-bound)*ligand - m_kr*bound);

  // Change both #bound receptors and molecular concentration
  cell->setValue(m_index, bound+deltaBound);
  m_field->changeConc(-deltaBound, cell->getPosition());
}

/************************************************************************
 * class SenseConsume                                                   *
 *   This class treats sensing as simple internalization of ligand at   *
 * a rate which is a saturating function of the ligand concentration.   *
 * This saturating function corresponds to the rate at which ligand is  *
 * changed when the cell comes to a quasi-steady state in which numbers *
 * of bound and free receptors are constant.                            *
 * The amount consumed in time deltaT is subtracted from the Molecule's *
 * concentration at the cell's location; the amount consumed in 1 sec   *
 * is stored in the Cell's internal variable.				*
 * Make sure grid volume is not smaller than the volume containing      *
 * the amount of molecule a cell could consume in one timestep!		*
 ************************************************************************/
SenseConsume::SenseConsume(int index, Molecule *field, double maxRate, 
			   double halfSat) :
   	m_index(index), m_field(field), m_maxRate(maxRate), m_halfSat(halfSat)
{
  assert(m_index >= 0);
  assert(field);
  assert(m_maxRate>=0);
  assert(m_halfSat>0);
}

void SenseConsume::calculate(Cell *cell, double deltaT)
{
  // get local ligand concentration - will be in moles/ml
  double conc = m_field->getConc(cell->getPosition());

  // determine how many molecules the cell actually consumes per second
  double rate = (m_maxRate * conc / (m_halfSat + conc));
  assert(rate>=0);

  double amount = rate * deltaT;
  if (amount)	
  {
    // Set Cell internal value - this should represent 'current' 'signal';
    // i.e. not cumulative in deltaT
    cell->setValue(m_index, rate);

    // subtract from molecular concentration - this does depend on deltaT
    m_field->changeConc(-amount, cell->getPosition());
  }
  else
    cell->setValue(m_index, 0);

}

/************************************************************************
 * class SenseConsumeIndiv                                              *
 *   Same as SenseConsume, but allows for the possibility that the      *
 * maximum consumption rate is different for each cell.                 *
 * Make sure grid volume is not smaller than the volume containing      *
 * the amount of molecule a cell could consume in one timestep!		*
 ************************************************************************/
SenseConsumeIndiv::SenseConsumeIndiv(int index, Molecule *field, int rate_index,
			   double halfSat) :
   	m_index(index), m_field(field), m_rateIndex(rate_index), 
		m_halfSat(halfSat)
{
  assert(m_index >= 0);
  assert(field);
  assert(m_rateIndex>=0);
  assert(m_halfSat>0);
}

void SenseConsumeIndiv::calculate(Cell *cell, double deltaT)
{
  // get local ligand concentration - will be in moles/ml
  double conc = m_field->getConc(cell->getPosition());

  // determine how many molecules the cell actually consumes per second
  double maxRate = cell->getValue(m_rateIndex);
  assert(maxRate>=0);
  double rate = (maxRate * conc / (m_halfSat + conc));
  assert(rate>=0);

  if (double amount = rate * deltaT)	// don't bother if amount = 0
  {
    // Set Cell internal value - this should represent 'current' 'signal';
    // i.e. not cumulative in deltaT
    cell->setValue(m_index, rate);

    // subtract from molecular concentration - this does depend on deltaT
    m_field->changeConc(-amount, cell->getPosition());
  }
}
