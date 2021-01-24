
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
 * file sense.h                                                         *
 * Declarations for virtual Sense class and derived classes   		*
 * Different concrete classes implement different functions, which are  *
 * used to update one variable out of a list (corresponding to a single *
 * cell's internal variables) and may also modify the extracellular     *
 * environment.                                                         *
 ************************************************************************/

#ifndef SENSE_H
#define SENSE_H

class Cell;
class Cells;
class Molecule;


class Sense {
  public:
    virtual ~Sense() {};
    virtual void calculate(Cell *cell, double deltaT) = 0;
};

class SensePhag : public Sense
{
  public:
    SensePhag(int pattr, int targettype, double dist, int Rattr, double thr,
		    Cells *cells);
    // ~SensePhag();			// use default destructor

    void calculate(Cell *cell, double deltaT);

  private:
    int m_pattr;		// index of Cell attribute to modify
    int m_targetType;		// index of CellType to look for
    double m_dist;		// maximum distance 'detectable'
    int m_Rattr;		// index of attribute storing #receptors
    double m_thr;		// threshold value on #receptors for binding
    Cells *m_cells;		// access to Cells routine getTarget       

    // not used
    SensePhag(const SensePhag &r);
    SensePhag operator = (const SensePhag &r);
};

// SenseCognate simply checks whether there is a neighbor of the appropriate
// type; sets an internal (boolean) variable accordingly.
class SenseCognate : public Sense
{
  public:
    SenseCognate(int pattr, int targettype, double dist, Cells *cells);
    // ~SenseCognate();			// use default destructor

    void calculate(Cell *cell, double deltaT);

  private:
    int m_pattr;		// index of Cell attribute to modify
    int m_targetType;		// index of CellType to look for
    double m_dist;		// maximum distance 'detectable'
    Cells *m_cells;		// access to Cells routine getTarget       

    // not used
    SenseCognate(const SenseCognate &r);
    SenseCognate operator = (const SenseCognate &r);
};

class SenseCopyConc : public Sense
{
  public:
    SenseCopyConc(int index, Molecule *field);
    // ~SenseCopyConc();			// use default destructor

    void calculate(Cell *cell, double deltaT);

  private:
    int m_index;		// index of Cell attribute to modify
    Molecule *m_field;		// access to molecular concentration sensed

    // not used
    SenseCopyConc(const SenseCopyConc &r);
    SenseCopyConc operator = (const SenseCopyConc &r);
};

// unlike above, SenseBindxx would actually modify the molecular concentration
// as well as Cell's perception of it. (except maybe SenseBindEq)
// note: there are a lot of different equations for binding based on 
// different assumptions - choices listed in order of increasing complexity

// SenseBindEq
// Uses equilibrium assumption to calculate number of bound complexes
// Does NOT take ligand depletion into account  
// don't need to convert concentration to #molecules
// Valid when change in ligand concentration is insignificant
// Lauffenberger equation 2-13
// C = L_total*R_total/(K_D + L_total)
// 2 constants, 1 variable 
// doesn't actually use old value of C     

// SenseBindIrr
// Lauffenberger doesn't address irreversible binding directly; 
// seems more closely related to predator consumption of prey
// modified from Lauffenberger equation 2-8, but with k_r = 0
// deltaC = k_f*L_free*R_free = k_f*L_free*(R_total-C)
// 2 constants, 2 variables 
// have to translate between concentration and #molecules to remove bound
// ligand

// SenseBindRev
// sensing as reversible molecular binding, as described by
// Lauffenberger equation 2-8
// changes the molecular concentration as well as the cell data
class SenseBindRev : public Sense
{
  public:
    // constructor
    SenseBindRev(int index, Molecule *field, double kf, double kr, double r);
    // index - of Cell variable representing #bound receptors
    // field - identifies Molecule object representing ligand
    // kf - forward rate constant:	
      // kf will generally be expressed in 1/((moles/liter)*min)
      // I need it in 1/((moles/ml)*sec)
    // kr - reverse rate constant, in 1/sec 
    // r - fixed number of receptors available for this cell type

    // ~SenseBindRev();			// use default destructor

    void calculate(Cell *cell, double deltaT);

  private:
    int m_index;
    Molecule *m_field;
    double m_kf;	// association rate constant, 1/((moles/ml)*time)
    double m_kr;	// dissociation rate constant, 1/time
    double m_r;		// total number of receptors for this ligand
			// note - this could be another cell variable

    // not used
    SenseBindRev(const SenseBindRev &r);
    SenseBindRev operator = (const SenseBindRev &r);
};

// SenseConsume
// Sensing as strictly internalizing ligand. 
// Consumption rate is a saturating function of the ligand concentration,
// based on Tushinski/Stanley data and an equation for that 
// saturating function based on Lauffenburger's quasi-steady-state model
// for receptor trafficking.
// Local molecular concentration is changed by this rate * deltaT;
// just rate (in molecules/sec) is stored in internal Cell variable -    
// other Cell Actions may depend on this rate, not the total amount consumed
// (or alternatively, on the rate 'most recently' consumed)
// NOTE:  it is important, when estimating the parameters for this function
// to also estimate the corresponding 'sensing volume' of one cell at the
// timestep used - the volume of one sim grid should not be smaller than this 
// volume.
class SenseConsume : public Sense
{
  public:
    // constructor
    SenseConsume(int index, Molecule *field, double maxRate, double halfSat);
    // index - of Cell variable representing current binding rate
    // field - identifies Molecule object representing ligand
    // maxRate - maximum possible consumption rate       
    // halfSat - half-saturation constant; b in the equation:
	// rate = maxRate * conc / (b + conc)       
 
    // ~SenseConsume();			// use default destructor

    void calculate(Cell *cell, double deltaT);

  private:
    int m_index;
    Molecule *m_field;
    double m_maxRate; 	// #molecules/sec
    double m_halfSat; 	// Moles/ml        

    // not used
    SenseConsume(const SenseConsume &r);
    SenseConsume operator = (const SenseConsume &r);
};

// SenseConsumeIndiv
// Same as SenseConsume, but allows for a different maxRate for each cell.
class SenseConsumeIndiv : public Sense
{
  public:
    // constructor
    SenseConsumeIndiv(int index, Molecule *field, int rate_index, 
	double halfSat);
    // index - of Cell variable representing current binding rate
    // field - identifies Molecule object representing ligand
    // rate_index - index of Cell variable that stores maximum possible 
	// consumption rate       
    // halfSat - half-saturation constant; b in the equation:
	// rate = maxRate * conc / (b + conc)       
 
    // ~SenseConsumeIndiv();			// use default destructor

    void calculate(Cell *cell, double deltaT);

  private:
    int m_index;
    Molecule *m_field;
    int m_rateIndex;
    double m_halfSat; 	// Moles/ml        

    // not used
    SenseConsumeIndiv(const SenseConsumeIndiv &r);
    SenseConsumeIndiv operator = (const SenseConsumeIndiv &r);
};
#endif

