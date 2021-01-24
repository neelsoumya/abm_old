
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
 * file fileDef.cc                                                      * 
 * Routines for reading model definition from file                      *    
 ************************************************************************/

#include "fileDef.h"							
#include <iostream>
#include "tissue.h"
#include "molecule.h"
#include "cellType.h"
#include "sense.h"
#include "rate.h"
#include "condition.h"
#include "action.h"
#include "process.h"

using namespace std;

class Cells;

#define REVNO 8	    // revision number for definition file format

/************************************************************************ 
 * defineFromFile()                         				*
 *   Reads model parameters, but not data, from specified input file.   *
 *									*
 * Parameters          			 				*
 *									*
 * Returns - nothing               					*
 ************************************************************************/
void FileDef::defineFromFile(Tissue *pt, const char *filename)
{
  // open file
  ifstream infile(filename);
  if (!infile)
    error("FileDef:  could not open file", filename);

  char buff[20];		// for input strings 

  // check file format; should read:  #DefFormat REVNO 
  infile >> buff;
  if (strcmp(buff, "#DefFormat") != 0)
    error("FileDef:  expected first line to read #DefFormat n");
  int num;
  infile >> num;
  if (num != REVNO)
    error("FileDef:  expected file format", REVNO);

  // next bit should be list of names of cell types
  infile >> buff;
  if (strcmp(buff, "cell_names") == 0)
    readTypeNames(pt, infile);
  else
    error("Error in def file; expecting cell_names");

  // loop through file looking for model component keywords
  while (!infile.eof())
  {
    infile >> buff;		
    // skip comments

    if (strcmp(buff, "molecule_type") == 0)
      readMolType(pt, infile);

    else if (strcmp(buff, "cell_type") == 0)
      readCellType(pt, infile);

    else	// unknown type
      error("FileDef: unknown keyword ", buff);

    infile >> ws;	 	// skip whitespace
  } 

  infile.close();
}

/************************************************************************
 * readTypeNames()                                                      *
 *   Reads cell type names from already-open file; creates CellType     * 
 *   object and adds to list - further definition will be handled by    *
 *   readCellType.                                                      *
 *                                                                      *
 * Parameters                                                           *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileDef::readTypeNames(Tissue *pt, ifstream &infile)
{
  char buff[20];

  // look for opening { - assume followed by a space          

  infile >> buff;
  if (strcmp(buff, "{") != 0)
    error("FileDef:  cell type name list should begin with {");

  infile >> buff;          
  while (buff[0] != '}')
  {
    // create default CellType with this name 
    CellType *pct = new CellType(buff);

    // call Tissue to add this CellType to list
    pt->addCellType(pct);

    // add to list - need to retain pointer to allow further definition
    m_celltypes.push_back(pct);

    // read next name or ending }
    infile >> buff;
  }	// end cell name list        
}

/************************************************************************
 * readMolType()                                                        *
 *   Reads molecule definition from already-open file; creates Molecule *
 *   object; calls Tissue to add new object to its list		        *
 *                                                                      *
 * Parameters                                                           *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileDef::readMolType(Tissue *pt, ifstream &infile)
{
  char buff[20];

  // read name
  infile >> buff;          
cout << "molecule type name:  " << buff << endl;

  // check that name doesn't already exist
  if (pt->getMolecule(buff))
    error("FileDef: attempting to redefine Molecule type ", buff);

  // create default Molecule with this name 
  Molecule *pm = new Molecule(buff);

  // read other parameters & set accordingly
  // look for opening { - assume followed by a space          
  infile >> buff;
  if (strcmp(buff, "{") != 0)
    error("FileDef:  molecule type definition should begin with {");
 
  infile >> buff;

  double rate; 
  while (buff[0] != '}')
  {
    if (strcmp(buff, "diffusion_rate") == 0)
    {
      infile >> rate;
      pm->setDiffRate(rate);
    }

    else if (strcmp(buff, "decay_rate") == 0)
    {
      infile >> rate;
      pm->setDecayRate(rate);
    }
 
    else
      error("FileDef molecule type definition:  unknown keyword ", buff); 
  
    infile >> buff;
  }   // end of molecule definition loop

  // call Tissue to add this Molecule to list
  pt->addMolType(pm);
}

/************************************************************************
 * readCellType()                                                       *
 *   Reads cell type definition from already-open file; adds various    * 
 *   attributes to CellType.                                            *
 *                                                                      *
 * Parameters                                                           *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileDef::readCellType(Tissue *pt, ifstream &infile)
{
  char buff[20];

  // read name
  infile >> buff;          
cout << "cell type name:  " << buff << endl;

  // get cell type pointer from internal list
  CellType *pct = getCellType(buff);
  if (!pct)
    error("FileDef:  can't find cell type ", buff);

  // read other parameters & set accordingly
  // look for opening { - assume followed by a space          

  infile >> buff;
  if (strcmp(buff, "{") != 0)
    error("FileDef:  cell type definition should begin with {");
 
  infile >> buff;

  while (buff[0] != '}')
  {
    if (strcmp(buff, "radius") == 0)
    {
      double radius;
      infile >> radius;
      pct->setRadius(radius);
    }
    else if (strcmp(buff, "speed") == 0)
    {
      double speed;
      infile >> speed;
      pct->setSpeed(speed);
    }
    else if (strcmp(buff, "attribute") == 0)
      readAttribute(pct, pt, infile);

    else if (strcmp(buff, "sense") == 0)
      readSense(pct, pt, infile);

    else if (strcmp(buff, "process") == 0)
      readProcess(pct, pt, infile);

    else if (strcmp(buff, "action") == 0)
      readActivity(pct, pt, infile);

    else
      error("FileDef cell type definition:  unknown keyword ", buff);

    infile >> buff;
  }	// end cell type definition
}

/************************************************************************
 * readAttribute()                                                      *
 *   Reads an individual 'attribute' of a cell type; this corresponds   * 
 *   to an internal variable of each Cell of this type.  The parameters *
 *   read here determine how that variable should be initialized        *
 *                                                                      *
 * Parameters                                                           *
 *   CellType *pct:   		cell type to be modified                *
 *   Tissue *pt:   		tissue model cell type belongs to       *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileDef::readAttribute(CellType *pct, Tissue *pt, ifstream &infile)
{
  // line should read:  att_name init_flag init_param1 init_param2
  //			rand_flag rand_param1 rand_param2
  char name[20];

  // read attribute name
  infile >> name;          
cout << "attribute name:  " << name << endl;

  // check that name doesn't already exist
  if (pct->getAttributeIndex(name) >= 0)
    error("FileDef: attempting to redefine attribute", name);

  char buff[20];	// for initialization/randomization flags
  double init1, init2=0, rand1, rand2=0;
  CellType::Dist initFlag, randFlag;

  // read info about how to initialize this attribute
  infile >> buff;
  if (strcmp(buff, "fixed") == 0)
  {
    initFlag = CellType::FIXED;
    infile >> init1;
  }
  else if (strcmp(buff, "uniform") == 0)
  {
    initFlag = CellType::UNIFORM;
    infile >> init1 >> init2;
  }
  else if (strcmp(buff, "gaussian") == 0)
  {
    initFlag = CellType::GAUSSIAN;
    infile >> init1 >> init2;
  }
  else if (strcmp(buff, "lognormal") == 0)
  {
    initFlag = CellType::LOGNORMAL;
    infile >> init1 >> init2;
  }
  else
    error("FileDef attribute initialization:  unknown keyword", buff);

  // read info about how to randomize this attribute
  infile >> buff;
  if (strcmp(buff, "fixed") == 0)
  {
    randFlag = CellType::FIXED;
    infile >> rand1;
  }
  else if (strcmp(buff, "uniform") == 0)
  {
    randFlag = CellType::UNIFORM;
    infile >> rand1 >> rand2;
  }
  else if (strcmp(buff, "gaussian") == 0)
  {
    randFlag = CellType::GAUSSIAN;
    infile >> rand1 >> rand2;
  }
  else if (strcmp(buff, "lognormal") == 0)
  {
    randFlag = CellType::LOGNORMAL;
    infile >> rand1 >> rand2;
  }
  else
    error("FileDef attribute randomization:  unknown keyword ", buff);

  pct->addAttribute(name, initFlag, init1, init2, randFlag, rand1, rand2);
}

/************************************************************************
 * readSense()                                                          *
 *   Reads an individual sense function defined for one cell type;      * 
 *   creates the Sense object and adds it to that CellType's list       *
 *                                                                      *
 * Parameters                                                           *
 *   CellType *pct:   		cell type to be modified                *
 *   Tissue *pt:   		tissue model cell type belongs to       *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileDef::readSense(CellType *pct, Tissue *pt, ifstream &infile)
{
  // line should read:  param_name sense_type [parameters]               

  char buff[20];

  // read attribute name, get corresponding array index
  infile >> buff;
  int index = pct->getAttributeIndex(buff);

  // read type of function 
  infile >> buff;		// type     
cout << "sense type:  " << buff << endl;

  // determine type of action and read parameters accordingly
  Sense *ps;
  if (strcmp(buff, "phag") == 0)
  {
    // expect cell_type_name dist receptor_attribute receptor_threshold
    int tindex = readCellName(pt, infile);
    Cells *cells = pt->getCellsPtr();
    double dist, thr;
    infile >> dist;
    infile >> buff;
    int Rattr = pct->getAttributeIndex(buff);
    infile >> thr;
    ps = new SensePhag(index, tindex, dist, Rattr, thr, cells);
    pct->addSense(ps);
  }
  else if (strcmp(buff, "cognate") == 0)
  {
    // expect cell_type_name dist 
    int tindex = readCellName(pt, infile);
    Cells *cells = pt->getCellsPtr();
    double dist;
    infile >> dist;
    ps = new SenseCognate(index, tindex, dist, cells);
    pct->addSense(ps);
  }
  else if (strcmp(buff, "copy_conc") == 0)
  {
    // expect molecule name to follow keyword
    Molecule *pm = readMolName(pt, infile);
    ps = new SenseCopyConc(index, pm);
    pct->addSense(ps);
  }
  else if (strcmp(buff, "bind_rev") == 0)
  {
    // expect molecule name, k_f, k_r and total #receptors to follow keyword
    Molecule *pm = readMolName(pt, infile);
    double kf, kr;
    int r;
    infile >> kf;			// in 1/((moles/liter)*min)
					// need 1/((moles/ml)*sec)
    infile >> kr;			// in 1/min; need 1/sec
    infile >> r;	
    ps = new SenseBindRev(index, pm, kf*16.67, kr/60, r);
    pct->addSense(ps);
  }
  else if (strcmp(buff, "consume") == 0)
  {
    // expect molecule name, max rate, and half-saturation constant
    Molecule *pm = readMolName(pt, infile);
    double rate, sat;
    infile >> rate >> sat;	// in #molecules/sec, Moles/ml 
    ps = new SenseConsume(index, pm, rate, sat);
    pct->addSense(ps);
  }
  else if (strcmp(buff, "consume-indiv") == 0)
  {
    // expect molecule name, name of Cell attribute that stores max rate, 
    // and value for half-saturation constant after keyword
    Molecule *pm = readMolName(pt, infile);
    infile >> buff;		// Cell attribute storing max rate
    int rateIndex = pct->getAttributeIndex(buff);
    double sat;
    infile >> sat;	// in #molecules/sec, Moles/ml 
    ps = new SenseConsumeIndiv(index, pm, rateIndex, sat);
    pct->addSense(ps);
  }
  else
    error("FileDef:  unknown sensing function type", buff);
}

/************************************************************************
 * readProcess()                                                        *
 *   Reads an process description; creates the Process object and 	*
 *   adds it to the CellType object.					*
 *                                                                      *
 * Parameters                                                           *
 *   CellType *pct:   		cell type to be modified                *
 *   Tissue *pt:   		tissue model cell type belongs to       *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - pointer to Action object                                   *
 ************************************************************************/
void FileDef::readProcess(CellType *pct, Tissue *pt, ifstream &infile)
{
  // line should read:  param_name process_type rate_info              

  char buff[20];

  // read attribute name, get corresponding array index
  infile >> buff;
  int index = pct->getAttributeIndex(buff);

  // read type of function 
  infile >> buff;		// type     
cout << "process type:  " << buff << endl;

  Process *pp;
  // determine type of process and read parameters accordingly
  if (strcmp(buff, "update") == 0)	// original version
  { // just Rate           
    Rate *pr = readRate(pt, pct, infile);
    pp = new ProcessUpdate(pr, index);
    pct->addProcess(pp);
  }
  else if (strcmp(buff, "update_bounded") == 0)	// bounded
  { // Rate min max
    Rate *pr = readRate(pt, pct, infile);
    double min, max;		// lower and upper bounds
    infile >> min >> max;               
    pp = new ProcessUpdateBounded(pr, index, min, max);
    pct->addProcess(pp);
  }
  else if (strcmp(buff, "replace") == 0) // not cumulative
  { // just Rate            
    Rate *pr = readRate(pt, pct, infile);
    pp = new ProcessReplace(pr, index);
    pct->addProcess(pp);
  }
  else if (strcmp(buff, "toggle") == 0) // not cumulative
  { // two values and two conditions
    double loval, hival;
    infile >> loval >> hival;               
    infile >> buff;			// condition type
    Cond *pc1 = readCond(buff, pt, pct, infile);
    infile >> buff;			// condition type
    Cond *pc2 = readCond(buff, pt, pct, infile);
    pp = new ProcessToggle(loval, hival, pc1, pc2, index);
    pct->addProcess(pp);
  }
  else if (strcmp(buff, "toggle_var") == 0) // not cumulative
  { // two index attributes and two conditions
    infile >> buff;
    int index1 = pct->getAttributeIndex(buff);
    infile >> buff;
    int index2 = pct->getAttributeIndex(buff);
    infile >> buff;			// condition type
    Cond *pc1 = readCond(buff, pt, pct, infile);
    infile >> buff;			// condition type
    Cond *pc2 = readCond(buff, pt, pct, infile);
    pp = new ProcessToggleVar(index1, index2, pc1, pc2, index);
    pct->addProcess(pp);
  }
  else if (strcmp(buff, "bd") == 0)
  { // birth_prob death_prob
    double bprob, dprob;
    infile >> bprob >> dprob;
    pp = new ProcessBD(index, bprob, dprob);
    pct->addProcess(pp);
  }
  else if (strcmp(buff, "bd_var") == 0)
  { // birth_prob_index death_prob_index
    infile >> buff;		// attribute name for birth rate
    int bpi = pct->getAttributeIndex(buff);
    infile >> buff;		// attribute name for death rate
    int dpi = pct->getAttributeIndex(buff);
    pp = new ProcessBDvar(index, bpi, dpi);
    pct->addProcess(pp);
  }
}

/************************************************************************
 * readActivity()                                                       *
 *   Reads an action defined for a cell type; determines whether it is  *
 *   conditional or not, calls the appropriate functions to construct   *
 *   the Action or Activity object to add to the appropriate CellType   *
 *   list    								*
 *                                                                      *
 * Parameters                                                           *
 *   CellType *pct:   		cell type to be modified                *
 *   Tissue *pt:   		tissue model cell type belongs to       *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void FileDef::readActivity(CellType *pct, Tissue *pt, ifstream &infile)
{
  Action *pa = readAction(pct, pt, infile);

  char buff[20];		
  infile >> buff;			// condition type
  if (strcmp(buff, "always") == 0)
    pct->addAction(pa);
  else
  {
    Cond *pc = readCond(buff, pt, pct, infile);
    pct->addActivity(pc, pa);
  }
}

/************************************************************************
 * readAction()                                                         *
 *   Reads an individual action; creates the Action object and returns  *
 *   a pointer to it.							*
 *                                                                      *
 * Parameters                                                           *
 *   CellType *pct:   		cell type to be modified                *
 *   Tissue *pt:   		tissue model cell type belongs to       *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - pointer to Action object                                   *
 ************************************************************************/
Action *FileDef::readAction(CellType *pct, Tissue *pt, ifstream &infile)
{
  // line should read:  action_type [parameter] rate_info              
  // Some actions take a cell or molecule type parameter, some don't.
  // rate_info may be a Cond, a Rate, or just a value used as a rate.
  // (note that a probabilistic condition make actually take a Rate - 
  // readCond will handle that).
  // Secrete Actions take Rates, other Actions take conditions

  char buff[20];		
  infile >> buff;			// action type
cout << "action type:  " << buff << endl;

  Action *pa;

  // determine type of action and read parameters accordingly
  if (strcmp(buff, "secrete") == 0)
  {
    Molecule *pm = readMolName(pt, infile);
    Rate *pr = readRate(pt, pct, infile);
    pa = new ActionSecrete(pr, pm);
  }
  else if (strcmp(buff, "secrete_fixed") == 0)
  {
    Molecule *pm = readMolName(pt, infile);
    double rate;
    infile >> rate;		
    pa = new ActionSecreteFixed(pm, rate);
  }
  else if (strcmp(buff, "secrete_var") == 0)
  {
    Molecule *pm = readMolName(pt, infile);
    infile >> buff;		// attribute name
    int index = pct->getAttributeIndex(buff);
    pa = new ActionSecreteVar(pm, index);
  }
  else if (strcmp(buff, "secrete_burst") == 0)
  {
    Molecule *pm = readMolName(pt, infile);
    infile >> buff;		// attribute name
    int index = pct->getAttributeIndex(buff);
    pa = new ActionSecreteBurst(pm, index);
  }
  else if (strcmp(buff, "die") == 0)
  {
    pa = new ActionDie();
  }
  else if (strcmp(buff, "change") == 0)
  {
    // read cell type name, get its index
    int index = readCellName(pt, infile);
    pa = new ActionChange(index);
  }
  else if (strcmp(buff, "divide") == 0)
  {
    // read cell daughter cell type name, get its index
    int tindex = readCellName(pt, infile);
    Cells *cells = pt->getCellsPtr();
    pa = new ActionDivide(cells, tindex);
  }
  else if (strcmp(buff, "admit") == 0)
  {
    int index = readCellName(pt, infile);
    infile >> buff;		// initialization scheme
    bool flag;
    if (strcmp(buff, "init") == 0) flag = true;
    else if (strcmp(buff, "rand") == 0) flag = false;
    else error("FileDef: unknown initialization option for action admit");
    double dist = pct->getRadius();
    Cells *cells = pt->getCellsPtr();
    pa = new ActionAdmit(index, dist, flag, cells);
  }
  else if (strcmp(buff, "admit_gradient") == 0)
  {
    int index = readCellName(pt, infile);
    infile >> buff;		// initialization scheme
    bool flag;
    if (strcmp(buff, "init") == 0) flag = true;
    else if (strcmp(buff, "rand") == 0) flag = false;
    else error("FileDef: unknown init option for action admit_gradient");
    Molecule *pm = readMolName(pt, infile);
    double dist = pct->getRadius();
    Cells *cells = pt->getCellsPtr();
    pa = new ActionAdmitGradient(index, pm, dist, flag, cells);
  }
  else if (strcmp(buff, "admit_mult") == 0)
  {
    int index = readCellName(pt, infile);
    infile >> buff;		// initialization scheme
    bool flag;
    if (strcmp(buff, "init") == 0) flag = true;
    else if (strcmp(buff, "rand") == 0) flag = false;
    else error("FileDef: unknown initialization option for action admit_mult");
    double dist = pct->getRadius();
    Cells *cells = pt->getCellsPtr();
    Rate *pr = readRate(pt, pct, infile);
    pa = new ActionAdmitMult(index, dist, flag, cells, pr);
  }
  else if (strcmp(buff, "move_randomly") == 0)
  {
    pa = new ActionMoveRandomly();
  }
  else if (strcmp(buff, "move_randomly2D") == 0)
  {
    pa = new ActionMoveRandomly2D();
  }
  else if (strcmp(buff, "move_chem") == 0)
  {
    infile >> buff;         // molecule name
    Molecule *pm = pt->getMolecule(buff);
    double min;
    infile >> min;		// min concentration requirement
    double r = pct->getRadius();
    pa = new ActionMoveChemotaxis(pm, min, r);
  }
  else if (strcmp(buff, "move_chem2D") == 0)
  {
    infile >> buff;         // molecule name
    Molecule *pm = pt->getMolecule(buff);
    double min;
    infile >> min;		// min concentration requirement
    double r = pct->getRadius();
    pa = new ActionMoveChemotaxis2D(pm, min, r);
  }
  else if (strcmp(buff, "composite") == 0)
  { // read two more actions - for each, read name then call readAction
    Action *pa1 = readAction(pct, pt, infile);
    Action *pa2 = readAction(pct, pt, infile);
    pa = new ActionComposite(pa1, pa2);
  }
  else
    error("FileDef:  unknown action type", buff);

  return pa;
}

/************************************************************************
 * readMolName()                                                        *
 *   Reads name of molecule type from file, returns pointer to it if    * 
 *   it exists                						*
 *                                                                      *
 * Parameters                                                           *
 *   Tissue *pt:      		allows access to molecule types         *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - pointer to Molecule object                                 *
 ************************************************************************/
Molecule *FileDef::readMolName(Tissue *pt, ifstream &infile)
{
  char buff[20];
  infile >> buff;         // molecule name
  Molecule *pm = pt->getMolecule(buff);
  if (!pm)
    error("FileDef::readCellType - can't find molecule", buff);
  return pm;
}

/************************************************************************
 * readCellName()                                                       *
 *   Reads name of cell type from file, returns it's index in Cells     *
 *   list if it exists                                                  * 
 *                                                                      *
 * Parameters                                                           *
 *   Tissue *pt:      		allows access to molecule types         *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - pointer to cell type                                       *
 ************************************************************************/
int FileDef::readCellName(Tissue *pt, ifstream &infile)
{
  char buff[20];
  infile >> buff;		
  int index = pt->getCellTypeIndex(buff);
  if (index < 0)
    error("FileDef::readCellName - can't find cell Type", buff);
  return index;
}

/************************************************************************
 * readRate()                                                           *
 *   Reads a Rate type (function name) and the appropriate parameters   * 
 *   for that type; constructs new Rate object                          *
 *                                                                      *
 * Parameters                                                           *
 *   Tissue *pt:      		for access to cell types                *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - pointer to new Rate object                                 *
 ************************************************************************/
Rate *FileDef::readRate(Tissue *pt, CellType *pct, ifstream &infile)
{
  // line should read:  rate_type parameters                        
  // where # and type of parameters depend on the rate_type

  char buff[20];
  infile >> buff;		// rate type     

  Rate *pr;
  // determine rate function and read parameters accordingly
  if (strcmp(buff, "fixed") == 0)
  {
    double rate;
    infile >> rate;
    // construct Rate object
    pr = new RateFixed(rate);         
  }

  else if (strcmp(buff, "var") == 0)
  {
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // construct Rate object
    pr = new RateVar(index);         
  }

  else if (strcmp(buff, "linear") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read slope & yintercept
    double slope, yinter;
    infile >> slope >> yinter;
    // construct Rate object
    pr = new RateLinear(index, slope, yinter);         
  }

  else if (strcmp(buff, "chopped") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read slope, yintercept, min, and max
    double slope, yinter, min, max;
    infile >> slope >> yinter >> min >> max;
    // construct Rate object
    pr = new RateChoppedLinear(index, slope, yinter, min, max);         
  }

  else if (strcmp(buff, "product") == 0)
  {
    // read attribute names, get corresponding array index
    infile >> buff;
    int index1 = pct->getAttributeIndex(buff);
    infile >> buff;
    int index2 = pct->getAttributeIndex(buff);
    // construct Rate object
    pr = new RateProduct(index1, index2);         
  }

  else if (strcmp(buff, "sigmoid") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read threshold and steepness
    double thr, sigma;
    infile >> thr >> sigma;
    // construct Rate object
    pr = new RateSigmoid(index, thr, sigma);         
  }

  else if (strcmp(buff, "saturating") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read max, half-sat
    double max, halfSat;
    infile >> max >> halfSat;
    // construct Rate object
    pr = new RateSaturating(index, max, halfSat);         
  }

  else if (strcmp(buff, "inhibiting") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read max, inhibition constant
    double max, c;
    infile >> max >> c;
    // construct Rate object
    pr = new RateInhibiting(index, max, c);         
  }

  else if (strcmp(buff, "relsat") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index1 = pct->getAttributeIndex(buff);
    infile >> buff;
    int index2 = pct->getAttributeIndex(buff);
    // read max, half-sat, scale factor
    double max, halfSat, f;
    infile >> max >> halfSat >> f;
    // construct Rate object
    pr = new RateRelSat(index1, index2, max, halfSat, f);         
  }

  else if (strcmp(buff, "relinh") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index1 = pct->getAttributeIndex(buff);
    infile >> buff;
    int index2 = pct->getAttributeIndex(buff);
    // read max, constant, scale factor
    double max, c, f;
    infile >> max >> c >> f;
    // construct Rate object
    pr = new RateRelInh(index1, index2, max, c, f);         
  }

  else if (strcmp(buff, "synergy") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index1 = pct->getAttributeIndex(buff);
    infile >> buff;
    int index2 = pct->getAttributeIndex(buff);
    // read max, half-sat, scale factor
    double max, halfSat, f;
    infile >> max >> halfSat >> f;
    // construct Rate object
    pr = new RateSynergy(index1, index2, max, halfSat, f);         
  }

  else if (strcmp(buff, "composite") == 0)
  {
    // need to read and create two child Rates, then composite Rate
    Rate *pr1 = readRate(pt, pct, infile);
    Rate *pr2 = readRate(pt, pct, infile);
    // construct Rate object
    pr = new RateComposite(pr1, pr2);          
  }

  else
    error("FileDef cell action definition:  unknown rate function", buff);

  return pr;
}

/************************************************************************
 * readCond()                                                           *
 *   Reads a Cond type (function name) and the appropriate parameters   * 
 *   for that type; constructs new Cond object                          *
 *                                                                      *
 * Parameters                                                           *
 *   char buff[20]:		already-read condition type
 *   Tissue *pt:      		for access to cell types                *
 *   ifstream &infile:		open file containing data               *
 *                                                                      *
 * Returns - pointer to new Cond object                                 *
 ************************************************************************/
Cond *FileDef::readCond(char buff[20], Tissue *pt, CellType *pct, 
		ifstream &infile)
{
  // line should read:  rate_type parameters                        
  // where # and type of parameters depend on the rate_type

  Cond *pc;
  // determine condition type and read parameters accordingly
  if (strcmp(buff, "fixed") == 0)
  {
    double prob;
    infile >> prob;
    pc = new CondFixedProb(prob);         
  }

  else if (strcmp(buff, "var_prob") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    pc = new CondVarProb(index);         
  }

  else if (strcmp(buff, "calc_prob") == 0)
  {
    Rate *pr = readRate(pt, pct, infile);
    pc = new CondCalcProb(pr);         
  }

  else if (strcmp(buff, "gte") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read threshold           
    double thr;
    infile >> thr;
    // construct Cond object
    pc = new CondAboveThr(index, thr);         
  }

  else if (strcmp(buff, "gte_var") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read index to threshold variable 
    infile >> buff;
    int thr_index = pct->getAttributeIndex(buff);
    // construct Cond object
    pc = new CondAboveVar(index, thr_index);         
  }

  else if (strcmp(buff, "lte") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read threshold           
    double thr;
    infile >> thr;
    // construct Cond object
    pc = new CondBelowThr(index, thr);         
  }

  else if (strcmp(buff, "lte_var") == 0)
  {
    // read attribute name, get corresponding array index
    infile >> buff;
    int index = pct->getAttributeIndex(buff);
    // read index to threshold variable 
    infile >> buff;
    int thr_index = pct->getAttributeIndex(buff);
    // construct Cond object
    pc = new CondBelowVar(index, thr_index);         
  }

  else if (strcmp(buff, "composite") == 0)
  {
    // need to read and create two child Conds, then composite Cond
    infile >> buff;
    Cond *pr1 = readCond(buff, pt, pct, infile);
    infile >> buff;
    Cond *pr2 = readCond(buff, pt, pct, infile);
    // construct Cond object
    pc = new CondComposite(pr1, pr2);          
  }
  else if (strcmp(buff, "or") == 0)
  {
    // need to read and create two child Conds, then or Cond
    infile >> buff;
    Cond *pr1 = readCond(buff, pt, pct, infile);
    infile >> buff;
    Cond *pr2 = readCond(buff, pt, pct, infile);
    // construct Cond object
    pc = new CondOr(pr1, pr2);          
  }

  else
    error("FileDef cell action definition: unknown condition type", buff);

  return pc;
}

/************************************************************************ 
 * getCellType()                                                        *
 * Finds a cell type by name, returns pointer                           *
 *                                                                      *
 * Parameters                                                           *
 *   string type_name;          identifies cell type being modified     *
 *                                                                      *
 * Returns - pointer to CellType object                                 *
 ************************************************************************/
CellType *FileDef::getCellType(const string& type_name)
{
  for (unsigned int i=0; i<m_celltypes.size(); i++)
  {
    CellType *pct = m_celltypes[i];
    if (pct->isMatch(type_name))   
      return pct;
  }

  // if we got this far, we didn't find it
  return 0;
}

