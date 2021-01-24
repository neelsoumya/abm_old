
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
 * file cellType.cc                                                     *
 * Definitions for CellType                                             * 
 * Attributes and behavior for all cells of one type                    *
 ************************************************************************/

#include "cellType.h"
#include <string>	
#include "cell.h"
#include "sense.h"
#include "process.h"
#include "condition.h"
#include "action.h"
#include "random.h"

/************************************************************************ 
 * CellType()                                                           *
 *   Constructor - sets default values except for name                  *
 *                                                                      *
 * Parameters                                                           *
 *   string type_name:          for user reference			*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
CellType::CellType(string type_name) :
	m_name(type_name), m_radius(5), m_speed(0)
{
}

/************************************************************************ 
 * CellType()                                                           *
 *   Constructor                                                        *
 *                                                                      *
 * Parameters                                                           *
 *   string type_name:          for user reference			*
 *   double cell_radius:		 radius of each cell, in microns	*
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
CellType::CellType(string type_name, double cell_radius) :
	m_name(type_name),  m_radius(cell_radius)
{
}

/************************************************************************ 
 * ~CellType()                                                          *
 *   Destructor                                                         *
 *   Removes objects pointed to by sensor and action lists              *
 *                                                                      *
 * Parameters                                                           *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
CellType::~CellType()
{
  for(unsigned int i=0; i<sensors.size(); i++)
    delete sensors[i];
  for(unsigned int i=0; i<unconditionals.size(); i++)
    delete unconditionals[i];
}

/************************************************************************
 * getAttributeIndex()                                                  *
 *   Returns array index of named attribute                             *
 *                                                                      *
 * Parameters                                                           *
 *   string attrName:   name of attribute we're trying to find          *
 *                                                                      *
 * Returns - integer index                                              *
 ************************************************************************/
int CellType::getAttributeIndex(const string& attrName) const
{
  for (unsigned int i=0; i<attributes.size(); i++)
    if (attributes[i].m_name == attrName)
      return i;
  
  return -1;
}

/************************************************************************
 * isMatch()                                                            *
 *   Tests whether this object is that named in the parameter           *
 *                                                                      *
 * Parameters                                                           *
 *   string type_name:  name of cell type we're trying to find          *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
bool CellType::isMatch(const string& type_name) const
{
  if (type_name == m_name)
    return true;
  
  return false;
}

/************************************************************************ 
 * initializeCell()                                                     *
 *   Does type-specific initialization for a new cell using value       *
 *   ranges associated with initFlag			                *
 *                                                                      *
 * Parameters                                                           *
 *   Cell *pc:		specific cell affected                          *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void CellType::initializeCell(Cell *pc)
{
  // allocate memory for internal variables
  pc->setNumAttributes(attributes.size());

  // set real values according to CellType's initial values
  for (unsigned int i=0; i<attributes.size(); i++)
    switch (attributes[i].m_initFlag)
    {
      case FIXED:
        pc->setValue(i, attributes[i].m_init1);
	break;
      case UNIFORM:	// init1 is min, init2 is max
        pc->setValue(i, (attributes[i].m_init2 - 
		attributes[i].m_init1)*RandK::randk() + attributes[i].m_init1);
	break;
      case GAUSSIAN:	// init1 is mean, init2 is std. dev.
        pc->setValue(i, sampleGaussian(attributes[i].m_init1,
		   	attributes[i].m_init2));
   	break;
      case LOGNORMAL:	// init1 is mean, init2 is std. dev. of Gaussian
        pc->setValue(i, exp(sampleGaussian(attributes[i].m_init1, 
			attributes[i].m_init2)));
   	break;
    }

  // if this is a mobile cell, set initial orientation randomly
  if (m_speed)
  {
    double x = 2*RandK::randk() - 1;
    double y = 2*RandK::randk() - 1;
    double z = 2*RandK::randk() - 1;
    double sf = 1 / sqrt( x*x + y*y + z*z );
    pc->setDirection(SimPoint(x*sf, y*sf, z*sf));
  }
}

/************************************************************************ 
 * randomizeCell()                                                      *
 *   Does type-specific initialization for an existing cell just 	*
 *   entering the simulated compartment, as opposed to a new daughter	*
 *   cell; uses value ranges associated with randFlag rather than 	*
 *   initFlag				 		                *
 *                                                                      *
 * Parameters                                                           *
 *   Cell *pc:		specific cell affected                          *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void CellType::randomizeCell(Cell *pc)
{
  // allocate memory for internal variables
  pc->setNumAttributes(attributes.size());

  // set real values according to CellType's parameters for random values
  for (unsigned int i=0; i<attributes.size(); i++)
    switch (attributes[i].m_randFlag)
    {
      case FIXED:
        pc->setValue(i, attributes[i].m_rand1);
	break;
      case UNIFORM:	// rand1 is min, rand2 is max
        pc->setValue(i, (attributes[i].m_rand2 - 
		attributes[i].m_rand1)*RandK::randk() + attributes[i].m_rand1);
	break;
      case GAUSSIAN:	// rand1 is mean, rand2 is std. dev.
        pc->setValue(i, sampleGaussian(attributes[i].m_rand1,
		   attributes[i].m_rand2));
   	break;
      case LOGNORMAL:	// rand1 is mean, rand2 is std. dev. of Gaussian
        pc->setValue(i, exp(sampleGaussian(attributes[i].m_rand1, 
			attributes[i].m_rand2)));
   	break;
    }

  // if this is a mobile cell, set initial orientation randomly
  if (m_speed)
  {
    double x = 2*RandK::randk() - 1;
    double y = 2*RandK::randk() - 1;
    double z = 2*RandK::randk() - 1;
    double sf = 1 / sqrt( x*x + y*y + z*z );
    pc->setDirection(SimPoint(x*sf, y*sf, z*sf));
  }
}

/************************************************************************ 
 * sense()                                                              *
 *   Update one Cell's internal variables according to CellType's       *
 *   sensors list; not currently used - replaced by update.             *
 *                                                                      *
 * Parameters                                                           *
 *   Cell *pc:		specific cell affected                          *
 *   double deltaT:	elapsed time for this activity                  *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void CellType::sense(Cell *pc, double deltaT)
{
  // for each Sense function in list, invoke Sense::calculate
  // which updates the value for appropriate internal variable for Cell *pc
  // calculate may change Molecule values also
  for(unsigned int i=0; i<sensors.size(); i++)
    sensors[i]->calculate(pc, deltaT);
}
   
/************************************************************************ 
 * process()                                                            *
 *   Carries out all of the internal processing defined for 		*
 *   this cell type with one cell; not currently used - replaced by     *
 *   update.                                                            *
 *                                                                      *
 * Parameters                                                           *
 *   Cell *c:		specific cell affected                          *
 *   double deltaT:	elapsed time for this activity                  *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void CellType::process(Cell *pc, double deltaT)
{
  // for each Process function in list, invoke Process::update
  for(unsigned int i=0; i<processes.size(); i++)
    processes[i]->update(pc, deltaT);
}
   
/************************************************************************ 
 * act()                                                                *
 *   Carries out all of the Actions defined for this cell type with one *
 *   cell; not currently used - replaced by update.                     *
 *                                                                      *
 * Parameters                                                           *
 *   Cell *c:		specific cell affected                          *
 *   double deltaT:	elapsed time for this activity                  *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void CellType::act(Cell *pc, double deltaT)
{
  // execute each unconditional action
  for(unsigned int i=0; i<unconditionals.size() && pc->isAlive(); i++)
    unconditionals[i]->doAction(pc, deltaT);

  // for each activity in list, check Condition and execute Action
  // if appropriate
  for(unsigned int i=0; i<activities.size() && pc->isAlive(); i++)
  {
    if (activities[i].condition->test(pc->getInternals(), deltaT))
      activities[i].action->doAction(pc, deltaT);
  }
}
/************************************************************************ 
 * update()                                                             *
 *   Carries out all sensing and processing for one cell in one         *
 *   timestep.                                                          *
 *                                                                      *
 * Parameters                                                           *
 *   Cell *c:		specific cell affected                          *
 *   double deltaT:	elapsed time for this activity                  *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void CellType::update(Cell *pc, double deltaT)
{
  // for each Sense function in list, invoke Sense::calculate
  // which updates the value for appropriate internal variable for Cell *pc
  // calculate may change Molecule values also
  for(unsigned int i=0; i<sensors.size(); i++)
    sensors[i]->calculate(pc, deltaT);

  // for each Process function in list, invoke Process::update
  for(unsigned int i=0; i<processes.size(); i++)
    processes[i]->update(pc, deltaT);

  // execute each unconditional action
  for(unsigned int i=0; i<unconditionals.size() && pc->isAlive(); i++)
    unconditionals[i]->doAction(pc, deltaT);

  // for each activity in list, check Condition and execute Action
  // if appropriate
  for(unsigned int i=0; i<activities.size() && pc->isAlive(); i++)
  {
    if (activities[i].condition->test(pc->getInternals(), deltaT))
      activities[i].action->doAction(pc, deltaT);
  }
}

/************************************************************************ 
 * operator<<                                                           *
 *   Output all data for this cell type, in human-friendly form   	*
 *                                                                      *
 * Parameters                                                           *
 *   ostream& s:	output stream                                   *
 *   const CellType& t:	reference to cell type to display	        *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
ostream& operator<<(ostream& outfile, const CellType& t)                   
{
    outfile << "name:  " << t.m_name << endl;
    outfile << "radius:  " << t.m_radius << endl;
    outfile << endl;

    return outfile;
}
