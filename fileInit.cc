
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
 * file fileInit.cc                                                     * 
 * Routines for reading model initialization from file                  *    
 ************************************************************************/

#include "fileInit.h"						
#include <iostream>
#include "tissue.h"
#include "random.h"
#include "util.h"

using namespace std;

#define REVNO 4

/************************************************************************ 
 * initFromFile()                           				*
 *   Reads model initialization data from specified input file.   	*
 *									*
 * Parameters          			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void FileInit::initFromFile(Tissue *pt, const char *filename)
{
  // open file
  ifstream infile(filename);
  if (!infile)
    error("FileInit:  could not open file", filename);

  char buff[20];			// for input strings 
  int num;				// for input values
  Cells *pcs = pt->getCellsPtr();	// for access to Cells routines

   // check file format; should read:  #InitFormat REVNO
  infile >> buff;
  if (strcmp(buff, "#InitFormat") != 0)
    error("FileInit:  expected first line to read #InitFormat n");
  infile >> num;
  if (num != REVNO)
    error("FileInit:  expected format", REVNO);

  // loop through file to find initialization keywords
  while (!infile.eof())
  {
    infile >> ws;		// skip leading whitespace

    infile >> buff;		// read keyword 

    if (strcmp( buff, "geometry") == 0)
      readGeometry(pt, infile);
    else if (strcmp( buff, "timestamp:") == 0)
    {
      double value;
      infile >> value;
      pt->setTime(value);
cout << "set time to " << value << endl;
    }
    else if (strcmp( buff, "seed:") == 0)
    {
      long int value;
      infile >> value;
      pt->setSeed(value);
cout << "set seed to " << value << endl;
    }
    else if (strcmp( buff, "rnginfo:") == 0)
      RandK::readFromFile(infile);
    else if (strcmp( buff, "molecule_uniform:") == 0)
    {
      infile >> buff;
      Molecule *pm = pt->getMolecule(buff);
      if (!pm)
	error("FileInit:  can't find molecule type", buff);
      Molecule::Conc value;
      double stddev;
      infile >> value >> stddev;
      pm->setUniformConc(value, stddev);
    }
    else if (strcmp( buff, "molecule_reset:") == 0)
    {
      infile >> buff;
      double interval;		
      Molecule::Conc value;
      double stddev;
      infile >> interval >> value >> stddev;
      pt->setMolReset(buff, interval, value, stddev);
    }
    else if (strcmp( buff, "molecule_detail:") == 0)
    {
      infile >> buff;
      Molecule *pm = pt->getMolecule(buff);
      if (!pm)
	  error("FileInit:  can't find molecule type", buff);
      pm->initFromFile(infile);
    }
    else if (strcmp( buff, "cell_detail:") == 0)
    {
      pcs->initFromFile(infile);
    }
    else if (strcmp( buff, "cell_list:") == 0)
      // read individual cell info - not organized by type
      readCells(pt, infile);
    else if (strcmp( buff, "cell_sheet:") == 0)
    {
      double zpos;
      infile >> buff >> zpos;			// type name, zpos
      pcs->addSheet(buff, zpos); 
cout << "added sheet at z = " << zpos << endl;
    }
    else if (strcmp( buff, "cell_hexsheet:") == 0)
    {
      double zpos;
      infile >> buff >> zpos ;			// type name, zpos
      pcs->addHexSheet(buff, zpos); 
cout << "added hex sheet at z = " << zpos << endl;
    }
    else if (strcmp( buff, "cell_hexmix:") == 0)
    {
      // type1 type2 perc1 zpos
      char buff2[20];			
      double perc, zpos;
      infile >> buff >> buff2 >> perc >> zpos ;			
      pcs->addHexMix(buff, buff2, perc, zpos); 
cout << "added hex mix at z = " << zpos << endl;
    }
    else if (strcmp( buff, "cell_grid:") == 0)
    {
      int size;
      infile >> buff >> size;	
      pcs->addGrid(buff, size); 
cout << "added grid of size " << size << endl;
    }
    else if (strcmp( buff, "cell_mixed_grid:") == 0)
    {
      int size;
      char buff2[20];			
      infile >> buff >> buff2 >> size;	
      pcs->addMixedGrid(buff, buff2, size); 
cout << "added mixed grid of size " << size << endl;
    }
    else if (strcmp( buff, "cell_grid2D:") == 0)
    {
      int size;
      double zpos;
      infile >> buff >> size >> zpos;	
      pcs->addGrid2D(buff, size, zpos); 
cout << "added 2D grid of size " << size << " at z = " << zpos << endl;
    }
    else if (strcmp( buff, "cell_count:") == 0)
    {
      infile >> buff >> num;		// type name and number of cells
      pcs->addRandomly(buff, num); 
cout << "added " << num << " " << buff << " cells" << endl;
    }
    else if (strcmp( buff, "cell_count2D:") == 0)
    {
      double zpos;
      infile >> buff >> num >> zpos;	// type name, number and z position
      pcs->addRandomly2D(buff, num, zpos); 
cout << "added " << num << " " << buff << " cells" << endl;
    }
    else	// unknown type
      error("FileInit:  unknown keyword", buff);

    infile >> ws;	 	// skip whitespace

  }	// end loop through file

  pcs->initialize();

  // close file
  infile.close();
}

/************************************************************************
 * readCells()                                                          *
 *   Reads list of individual cells from already-open file; calls       *
 *   Tissue object to add them to model       		                *
 *                                                                      *
 * Parameters                                                           *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileInit::readCells(Tissue *pt, ifstream &infile)
{
  char buff[20];
  int i, count;
  double x, y, z;                // for reading cell position
  Cells *pcs = pt->getCellsPtr();

  // read and store number of cells  
  infile >> count;                      // use temp variable because
                                        // addCell updates num_new_cells
                                        // and mergeNew updates num_cells
cout << "read number of cells = " << count << endl;
  for (i=0; i<count; i++)
  {
    // read info as written in writeDefinition
    infile >> buff;                     // type name
    infile >> x;
    infile >> y;
    infile >> z;

cout << "read:  " << buff << " at " << x << " " << y << " " << z << endl;
    pcs->addCell(buff, SimPoint(x,y,z), false);
  }
}

/************************************************************************
 * readGeometry()                                                       *
 *   Reads geometry definition from already-open file; calls Tissue     *
 *   object to set geometry                                             *
 *                                                                      *
 * Parameters                                                           *
 *   Tissue *pt:                points to Tissue to be initialized      *
 *   ifstream &infile:          open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileInit::readGeometry(Tissue *pt, ifstream &s)
{
  // read size info 
  char ch;
  int xsize, ysize, zsize, molgridsize, cellgridsize;	// all sizes in microns

  s >> xsize;

  s >> ch;
  if (ch != 'x')
    error("FileInit:  missing 'x' in geometry description");

  s >> ysize;

  s >> ch;
  if (ch != 'x')
    error("FileInit:  missing 'x' in geometry description");

  s >> zsize;

  char buff[20];
  s >> buff;            // "microns"
  if (strcmp(buff, "microns;") != 0)
    error("FileInit:  missing 'microns;' in geometry description");

  s >> buff;            //  "mol_res:" 
  if (strcmp(buff, "mol_res:") != 0)
    error("FileInit:  missing 'mol_res:' in geometry description");

  s >> molgridsize;

  s >> buff;            //  "cell_res:" 
  if (strcmp(buff, "cell_res:") != 0)
    error("FileInit:  missing 'cell_res:' in geometry description");

  s >> cellgridsize;

  // Tissue routine will make sure all internal geometry refs are consistent
  pt->setGeometry(xsize, ysize, zsize, molgridsize, cellgridsize);       
}


