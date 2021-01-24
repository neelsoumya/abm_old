
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
 *  file tallyActions.cc                                                *
 *  Definitions for Singleton class that tallies #uses of Action        *
 *  classes ActionDie, ActionDivide, and ActionAdmit                    *
 ************************************************************************/

#include "tallyActions.h"

// initialize single instance to null
TallyActions* TallyActions::m_instance = 0;

/************************************************************************ 
 *  getInstance()                                                       *
 *  Checks whether an instance has already been created.  If not, calls *
 *  constructor.                                                        *
 *                                                                      *
 *  Returns - pointer to (the only) instance of this class              *
 ************************************************************************/
TallyActions* TallyActions::getInstance ()
{
  if (m_instance == 0)
    m_instance = new TallyActions();

  return m_instance;
}

/************************************************************************ 
 *  TallyActions()                                                      *
 *    Constructor                                                       *
 *                                                                      *
 *  Returns - nothing                                                   *
 ************************************************************************/
TallyActions::TallyActions()
{
  // create empty vectors
}

/************************************************************************ 
 *  addName()                                                           *
 *    Adds an Action type to the list to be tracked.                    *
 *                                                                      *
 *  Returns - ID # of this Action type                                  *
 *  	which should be used when calling update                        *
 ************************************************************************/
int TallyActions::addName(string name)        
{
  // name only used when printing results - doesn't need to be unique
  m_names.push_back(name);
  m_tallies.push_back(0);           
  return m_names.size()-1;
}



