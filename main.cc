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
 * file main.cc                                                         *
 * Main file for CyCells program                                        *
 ************************************************************************/
									
#include <cstdio>			// for sprintf
#include <getopt.h>			// for command-line options
#include <iostream>			// for cout
#include "tissue.h"
#include "history.h"
#include "fileDef.h"
#include "fileInit.h"
#include "tallyActions.h"
#include "util.h"

using namespace std;

//---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  Tissue tissue;	// sets seed from clock, but may be overriden	
  int c;		// options

  // default values for all command line parameters
  char *def_file = "test.def";
  char *init_file = "test.init";
  char *history_file = "test.history";
  char *detail_file = "test.detail";
  char *attr_file = 0;
  long seed = 0;
  double duration=10, deltaT=1, deltaW=1, deltaV=0;
  double maxCells = 10000000;

  // bookkeeping
  double lastsample=-1, lastdetail=-1;

  // command line interface:
  // -d def-file -i init_file -o output_file -s seed -t duration -e stepsize
  // -f detail-file -w history-stepsize -v detail-stepsize -c max-cells

  // process all command line options; each of these just overwrites
  // defaults set above - the values are actually used below
  while ((c = getopt(argc, argv, "hd:i:o:a:s:t:e:c:f:w:v:")) != EOF)
  {
    switch (c)
    {
      case 'd':		// definition file
	def_file = optarg;
	break;
      case 'i':		// initialization file
	init_file = optarg;
	break;
      case 'o':		// history output file
    	history_file = optarg; 	
	break;
      case 'a':		// input file for attributes to track
    	attr_file = optarg; 	
	break;
      case 's':		// seed         
	seed = strtol(optarg, NULL, 10);
	break;
      case 't':		// duration
 	duration = strtod(optarg, NULL);
	break;
      case 'e':		// time step size
	deltaT = strtod(optarg, NULL);
	break;
      case 'c':
	maxCells = strtod(optarg, NULL);
	break;
      case 'f':		// detail output file (cell/concentration data)
    	detail_file = optarg; 	
	break;
      case 'w':		// history step size
	deltaW = strtod(optarg, NULL);
	break;
      case 'v':		// detailed output step size
	deltaV = strtod(optarg, NULL);
	break;
      case 'h':		// help         
	cout << "usage:  textsim [-h] [-d def_file] [-i init_file] "
	     << "[-o output_file] [-s seed] [-t duration] [-e timestep] " 
	     << "[-f detail_file] [-w history_interval] [-v detail_interval] "
	     << endl;
	exit(0);
    }
  }

  // override default seed set in Tissue constructor 
  // with command line seed if there is one
  // Note that init file may also have seed or RNG settings - if so, they
  // will override the other seed specifications
  if (seed) tissue.setSeed(seed);	

  FileDef defParser;
  defParser.defineFromFile(&tissue, def_file);	
  FileInit initParser;
  initParser.initFromFile(&tissue, init_file);	


  // if sim volume 'gridded-up'; make sure timestep not too big for gridsize
  // (so cells can't eat more than what should be in their sensing volume)
//  int size = tissue.getGridSize();
//  double D = tissue.getMaxDiffRate();
//  if (size && D)
//  {
//    double constraint = size*size / D;
//    if ( deltaT > constraint )
//      error("Error:  timestep too big for gridsize");
//  }

  // set up history structure, record initial conditions
  History history(tissue, attr_file);
  history.sample(tissue);
  lastsample = tissue.getTime();

  char file[200];
  if (deltaV)
  {
    // record detailed initial simulation values
    sprintf(file, "%s.%d", detail_file, 0);
    tissue.writeData(file);
    lastdetail = tissue.getTime();
  }

  int numsteps = int(duration/deltaT);
  int writeInterval = (deltaW <= deltaT)? 1 : int(deltaW/deltaT); 
  int detailInterval = numsteps+1;	// default interval > simlength
  if (deltaV)
    detailInterval = (deltaV <= deltaT)? 1 : int(deltaV/deltaT); 

  for (int i=1; i<=numsteps && tissue.getNumCells()<maxCells; i++)
  {
    tissue.update(deltaT);
    if (i % writeInterval == 0)
    {
      history.sample(tissue);
      lastsample = tissue.getTime();
    }
    if (i % detailInterval == 0)
    {
      lastdetail = tissue.getTime();
      sprintf(file, "%s.%d", detail_file, int(lastdetail));
      tissue.writeData(file);
    }
  }

  // get final history/detail data, if missed above
  double t = tissue.getTime();
  if (t != lastsample)
    history.sample(tissue);
  if ( deltaV && (t != lastdetail) )
  {
    sprintf(file, "%s.%d", detail_file, int(t));
    tissue.writeData(file);
  }

  // open output file, write history
  ofstream outfile(history_file);
  if (!outfile)
    error("Error:  could not open history file", history_file);
  outfile << history;
  outfile.close();

  // write action tallies
  TallyActions *tap = TallyActions::getInstance();
  char actname[200];
  sprintf(actname, "%s.actions", history_file);
  ofstream actfile(actname);
  if (!actfile)
    error("Error:  could not open action file", actname);
  actfile << *tap;
  actfile.close();
}

