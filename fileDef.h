
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
 * file fileDef.h                                                       *
 * Declarations for FileDef class                                       * 
 * Handles reading Tissue model definition from file                    *
 ***********************************************************************/

#ifndef FILEDEF_H
#define FILEDEF_H

#include <fstream>
#include <vector>

using namespace std;

class Tissue;
class Molecule;
class CellType;
class Action;
class Rate;
class Cond;

class FileDef {	
  public:
    //--------------------------- CREATORS --------------------------------- 
    FileDef() {};		
    // copy constructor not used
    // ~FileDef();			// use default destructor
 
    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    // This actually manipulates the Tissue object
    void defineFromFile(Tissue *tp, const char *filename);

    //--------------------------- ACCESSORS --------------------------------

  private:
    vector<CellType *> m_celltypes;	// track CellTypes while assembling

    // private functions called by defineFromFile()
    void readTypeNames(Tissue *pt, ifstream &infile);
    void readMolType(Tissue *pt, ifstream &infile);
    void readCellType(Tissue *pt, ifstream &infile);
    void readAttribute(CellType *pct, Tissue *pt, ifstream &infile);
    void readSense(CellType *pct, Tissue *pt, ifstream &infile);
    void readProcess(CellType *pct, Tissue *pt, ifstream &infile);
    void readActivity(CellType *pct, Tissue *pt, ifstream &infile);
    Action *readAction(CellType *pct, Tissue *pt, ifstream &infile);
    Rate *readRate(Tissue *pt, CellType *pct, ifstream &infile);
    Cond *readCond(char buff[20], Tissue *pt, CellType *pct, 
   	ifstream &infile);
    Molecule *readMolName(Tissue *pt, ifstream &infile);
    int readCellName(Tissue *pt, ifstream &infile);
    CellType *getCellType(const string& type_name);

    // not used
    FileDef(const FileDef &h);    // copy constructor should not be used
    FileDef operator = (const FileDef &h);    // assignment should not be used
};

#endif

