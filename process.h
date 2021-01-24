
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
 * file process.h                                                 	*
 * Declarations for virtual Process class and derived classes		*
 ************************************************************************/

#ifndef PROCESS_H
#define PROCESS_H

#include "cell.h"
#include "rate.h"
#include "condition.h"
#include "random.h"
#include "util.h"

class Process {
  public:
    Process() {};
    virtual ~Process() {};   

    virtual void update(Cell *cell, double deltaT) = 0;

  protected:

  private:
    // not used
    // Process(const Process &rp);
    // Process operator = (const Process &rp);
};
				 

// allows updating of internal variables, e.g. intracellular signalling;
// decay of some internal signal due to metabolism (negative Rate), etc.
// changes value by adding Rate*deltaT to current value
class ProcessUpdate : public Process {
  public:
    ProcessUpdate(Rate * rateFunc, int index) : 
	m_rateFunc(rateFunc), m_index(index) {}; 		
    // copy constructor not used
    // use Process's destructor only - nothing else to delete         

    void update(Cell *cell, double deltaT) {
      double change = m_rateFunc->calculate(cell->getInternals()) * deltaT;
      cell->setValue(m_index, cell->getValue(m_index)+change);
    };
  
  private:
    Rate *m_rateFunc;
    int m_index;		// index of Cell attriibute to modify

    // not used
    ProcessUpdate(const ProcessUpdate &rp);
    ProcessUpdate operator = (const ProcessUpdate &as);
};

// Similar to ProcessUpdate, but sets min and max values on the result.
class ProcessUpdateBounded : public Process {
  public:
    ProcessUpdateBounded(Rate * rateFunc, int index, double min, double max) : 
	m_rateFunc(rateFunc), m_index(index), m_min(min), m_max(max)
	{}; 		
    // copy constructor not used
    // use Process's destructor only - nothing else to delete         

    void update(Cell *cell, double deltaT) {
      double change = m_rateFunc->calculate(cell->getInternals()) * deltaT;
      double result = cell->getValue(m_index) + change;
      if (result < m_min)
        result = m_min;
      else if (result > m_max)
	result = m_max;
      cell->setValue(m_index, result);
    };
  
  private:
    Rate *m_rateFunc;
    int m_index;		// index of Cell attriibute to modify
    double m_min, m_max;

    // not used
    ProcessUpdateBounded(const ProcessUpdateBounded &as);
    ProcessUpdateBounded operator = (const ProcessUpdateBounded &as);
};

// replaces current value of internal variable with new value
// generally used to calculate a rate or probability
// does not multiply by deltaT
class ProcessReplace : public Process {
  public:
    ProcessReplace(Rate * rateFunc, int index) : 
	m_rateFunc(rateFunc), m_index(index) {}; 		
    // copy constructor not used
    // use Process's destructor only - nothing else to delete         

    void update(Cell *cell, double deltaT) {
      double change = m_rateFunc->calculate(cell->getInternals());
      cell->setValue(m_index, change);
    };
  
  private:
    Rate *m_rateFunc;
    int m_index;		// index of Cell attribute to modify

    // not used
    ProcessReplace(const ProcessReplace &rp);
    ProcessReplace operator = (const ProcessReplace &rp);
};

// Updates an indexed attribute that takes on one of only two values - a high 
// and a low.  If the value is currently low (high), applies the 'lo2hi' 
// ('hi2lo') condition to determine whether to change the value to high (low).
// Does not multiply by deltaT.
class ProcessToggle : public Process {
  public:
    ProcessToggle(double low, double high, Cond * condlo2hi, Cond * condhi2lo,
		    int index) : 
	m_loval(low), m_hival(high), m_cond_lo2hi(condlo2hi), 
    	m_cond_hi2lo(condhi2lo), m_index(index) {};
    // copy constructor not used
    // use Process's destructor only - nothing else to delete         

    void update(Cell *cell, double deltaT) {
      double current = cell->getValue(m_index);
      if (current == m_loval)  
	if (m_cond_lo2hi->test(cell->getInternals(), deltaT))
          cell->setValue(m_index, m_hival);
        else;	// no change
      else if (current == m_hival)
	if (m_cond_hi2lo->test(cell->getInternals(), deltaT))
          cell->setValue(m_index, m_loval);
        else;	// no change
      else 
	error("ProcessToggle:  unexpected value", current);
    };
  
  private:
    double m_loval, m_hival;
    Cond *m_cond_lo2hi, *m_cond_hi2lo;
    int m_index;		// index of Cell attribute to modify

    // not used
    ProcessToggle(const ProcessToggle &rp);
    ProcessToggle operator = (const ProcessToggle &rp);
};

// Similar to ProcessToggle, but this version gets the low and high values
// from Cell attributes.
// Updates an indexed attribute that takes on one of only two values - a high 
// and a low.  If the value is currently low (high), applies the 'lo2hi' 
// ('hi2lo') condition to determine whether to change the value to high (low).
// Does not multiply by deltaT.
class ProcessToggleVar : public Process {
  public:
    ProcessToggleVar(int loindex, int hiindex, Cond * condlo2hi, 
		    Cond * condhi2lo, int index) : 
	m_loindex(loindex), m_hiindex(hiindex), m_cond_lo2hi(condlo2hi), 
    	m_cond_hi2lo(condhi2lo), m_index(index) {};
    // copy constructor not used
    // use Process's destructor only - nothing else to delete         

    void update(Cell *cell, double deltaT) {
      double current = cell->getValue(m_index);
      double loval = cell->getValue(m_loindex);
      double hival = cell->getValue(m_hiindex);
      if (current == loval)  
	if (m_cond_lo2hi->test(cell->getInternals(), deltaT))
          cell->setValue(m_index, hival);
        else;	// no change
      else if (current == hival)
	if (m_cond_hi2lo->test(cell->getInternals(), deltaT))
          cell->setValue(m_index, loval);
        else;	// no change
      else 
	error("ProcessToggleVar:  unexpected value", current);
    };
  
  private:
    int m_loindex, m_hiindex;
    Cond *m_cond_lo2hi, *m_cond_hi2lo;
    int m_index;		// index of Cell attribute to modify

    // not used
    ProcessToggleVar(const ProcessToggleVar &rp);
    ProcessToggleVar operator = (const ProcessToggleVar &rp);
};

#include "tallyActions.h"

// Allows updating of internal variable through a pseudo birth-death process. 
// Value of internal variable should always be a whole number.
// Allows updating of internal variable through a pseudo birth-death process. 
// Value of internal variable should always be a whole number.
// This routine assumes that the time step is small enough that probabilities
// of division and death are just n*r*deltaT, where n is the current population;
// r is the population division or death rate, and deltaT is the time step.
class ProcessBD : public Process {
  public:
    ProcessBD(int index, double bprob, double dprob) :
        m_index(index), m_bp(bprob), m_dp(dprob) 
    { m_tap = TallyActions::getInstance();
      m_birthid = m_tap->addName("ProcessBD-birth");
      m_deathid = m_tap->addName("ProcessBD-death");
    };
    // copy constructor not used
    // use Process's destructor only - nothing else to delete

    void update(Cell *cell, double deltaT) {
      // get current value n and a random number
      int n = int(cell->getValue(m_index));
      double r = RandK::randk();

      // need to know that n*(m_bp+m_dp)*deltaT can't be > 1
      assert( (n*(m_bp+m_dp)*deltaT) <= 1 );

      if (r < n*m_bp*deltaT) { 			// add one
        cell->setValue(m_index, n+1);
	m_tap->update(m_birthid);
      }
      else if (r < n*(m_bp+m_dp)*deltaT) {	// subtract one
        cell->setValue(m_index, n-1);
	m_tap->update(m_deathid);
      }
      // else do nothing
    };

  private:
    int m_index;                // index of Cell attriibute to modify
    double m_bp;		// individual birth probability = m_bp*deltaT
    double m_dp;		// individual death probability = m_dp*deltaT
    TallyActions *m_tap;	// object to tally intracellular births/deaths
    int m_birthid, m_deathid;	// ids to use with TallyAction object

    // not used
    ProcessBD(const ProcessBD &rp);
    ProcessBD operator = (const ProcessBD &rp);
};

// Allows updating of internal variable through a birth-death process. 
// Value of internal variable should always be a whole number.
// This routine assumes that the time step is small enough that probabilities
// of division and death are just n*r*deltaT, where n is the current population;
// r is the population division or death rate, and deltaT is the time step.
// This version uses internal Cell variables for the division and death rates
// - it's up to the user to make sure that those rates are appropriately 
// bounded and that the time step chosen is small enough
// to accomodate the maximum value of those rates.
class ProcessBDvar : public Process {
  public:
    ProcessBDvar(int index, int bpindex, int dpindex) :
        m_index(index), m_bpindex(bpindex), m_dpindex(dpindex) 
    { m_tap = TallyActions::getInstance();
      m_birthid = m_tap->addName("ProcessBD-birth");
      m_deathid = m_tap->addName("ProcessBD-death");
    };
    // copy constructor not used
    // use Process's destructor only - nothing else to delete

    void update(Cell *cell, double deltaT) {
      // get current value n and a random number
      int n = int(cell->getValue(m_index));
      double r = RandK::randk();

      // get probability factors
      double bp = cell->getValue(m_bpindex);
      double dp = cell->getValue(m_dpindex);

      // need to know that n*(bp+dp)*deltaT can't be > 1
      assert(n*(bp+dp)*deltaT<=1);

      if (r < n*bp*deltaT) { 			// add one
        cell->setValue(m_index, n+1);
	m_tap->update(m_birthid);
      }
      else if (r < n*(bp+dp)*deltaT) {	// subtract one
        cell->setValue(m_index, n-1);
	m_tap->update(m_deathid);
      }
      // else do nothing
    };

  private:
    int m_index;                // index of Cell attriibute to modify
    int m_bpindex;		// index of birth probability factor
    int m_dpindex;		// index of death probability factor
    TallyActions *m_tap;	// object to tally intracellular births/deaths
    int m_birthid, m_deathid;	// ids to use with TallyAction object

    // not used
    ProcessBDvar(const ProcessBDvar &rp);
    ProcessBDvar operator = (const ProcessBDvar &rp);
};

#endif

