
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
 * file fileInit.h                                                      *
 * Declarations for FileInit class                                      * 
 * Handles reading Tissue model initialization data from file           *
 ***********************************************************************/

#ifndef FILEINIT_H
#define FILEINIT_H

#include <fstream>

using namespace std;

class Tissue;

class FileInit {	
  public:
    //--------------------------- CREATORS --------------------------------- 
    FileInit() {};		
    // copy constructor not used
    // ~FileInit();			// use default destructor
 
    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    // This actually manipulates the Tissue object
    void initFromFile(Tissue *tp, const char *filename);

    //--------------------------- ACCESSORS --------------------------------

  private:
    // private functions called by defineFromFile()
    void readGeometry(Tissue *pt, ifstream &s);
    void readCells(Tissue *pt, ifstream &infile);

    // not used
    FileInit(const FileInit &h);    	// copy constructor should not be used
    FileInit operator = (const FileInit &h);    // assignment should not be used
};

#endif

