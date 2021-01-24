
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
 * file condition.h                                                     *
 * Declarations for virtual Condition class and derived classes		*
 ************************************************************************/

#ifndef COND_H
#define COND_H

#include <vector>
#include <cassert>
#include "random.h"

using namespace std;

class Cond {
  public:
    Cond() {};
    virtual ~Cond() {};
    virtual bool test(const vector<double> &values, double deltaT) const=0;

  private:
    // not used
    // Cond(const Cond &c);
    // Cond operator = (const Cond &c);
};

// calls RNG and compares result to fixed probability*deltaT
class CondFixedProb : public Cond 
{
  public:
    explicit CondFixedProb(double prob) : m_prob(prob) 
	{assert(prob>=0);assert(prob<=1);};
    // ~CondFixedProb();			// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {return sampleBernoulli(m_prob*deltaT);};

  private:
    double m_prob;

    // not used
    // CondFixedProb(const CondFixedProb &c);
    // CondFixedProb operator = (const CondFixedProb &c);
};

// calls RNG and compares result to prob*deltaT, where prob is a
// cell variable
class CondVarProb : public Cond 
{
  public:
    explicit CondVarProb(int index) : m_index(index) 
	{assert(m_index>=0);};
    // ~CondVarProb();			// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {assert(m_index<int(values.size()));
     return sampleBernoulli(values[m_index] * deltaT);};

  private:
    int m_index;

    // not used
    CondVarProb(const CondVarProb &c);
    CondVarProb operator = (const CondVarProb &c);
};

// CondAboveThr returns 1 if indexed value is greater than OR equal to the 
// threshold value; returns 0 otherwise
class CondAboveThr : public Cond 
{
  public:
    CondAboveThr(int index, double threshold) 
	: m_index(index), m_thr(threshold) 
	{assert(m_index>=0);};
    // ~CondAboveThr();		// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {assert(m_index<int(values.size()));
     return (values[m_index] >= m_thr);
    };

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_thr;		// threshold value 

    // not used
    CondAboveThr(const CondAboveThr &c);
    CondAboveThr operator = (const CondAboveThr &c);
};

// CondAboveVar returns 1 if the first indexed value is greater than OR equal 
// to the second indexed value; returns 0 otherwise
class CondAboveVar : public Cond 
{
  public:
    CondAboveVar(int index_var, int index_thr) 
	: m_index_var(index_var), m_index_thr(index_thr) 
	{assert(m_index_var>=0); assert(m_index_thr>=0);};
    // ~CondAboveVar();		// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {assert(m_index_var<int(values.size()));
     assert(m_index_thr<int(values.size()));
     return (values[m_index_var] >= values[m_index_thr]);
    };

  private:
    int m_index_var;		// index into cell's attribute list
				// for variable to test
    int m_index_thr;		// index into cell's attribute list
				// for reference parameter to compare to     

    // not used
    CondAboveVar(const CondAboveVar &c);
    CondAboveVar operator = (const CondAboveVar &c);
};

// CondBelowThr returns 1 if indexed value is less than OR equal to the 
// threshold value; returns 0 otherwise
class CondBelowThr : public Cond 
{
  public:
    CondBelowThr(int index, double threshold) 
	: m_index(index), m_thr(threshold) 
	{assert(m_index>=0);};
    // ~CondBelowThr();		// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {assert(m_index<int(values.size()));
     return (values[m_index] <= m_thr);
    };

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_thr;		// threshold value 

    // not used
    CondBelowThr(const CondBelowThr &c);
    CondBelowThr operator = (const CondBelowThr &c);
};

// CondBelowVar returns 1 if first indexed value is less than OR equal to the 
// second indexed value; returns 0 otherwise
class CondBelowVar : public Cond 
{
  public:
    CondBelowVar(int index_var, int index_thr) 
	: m_index_var(index_var), m_index_thr(index_thr) 
	{assert(m_index_var>=0); assert(m_index_thr>=0);};
    // ~CondBelowVar();		// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {assert(m_index_var<int(values.size()));
     assert(m_index_thr<int(values.size()));
     return (values[m_index_var] <= values[m_index_thr]);
    };

  private:
    int m_index_var;		// index into cell's attribute list
				// for variable to test                      
    int m_index_thr; 		// index into cells' attribute list
				// for parameter to compare to

    // not used
    CondBelowVar(const CondBelowVar &c);
    CondBelowVar operator = (const CondBelowVar &c);
};

// ANDs two conditions
class CondComposite : public Cond 
{
  public:
    CondComposite(Cond *pr1, Cond *pr2) 
	: m_pr1(pr1), m_pr2(pr2) 
	{assert(m_pr1); assert(m_pr2);};
    // ~CondComposite();		// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {return (m_pr1->test(values, deltaT) && m_pr2->test(values, deltaT));
    };

  private:
    Cond *m_pr1, *m_pr2;		// 'child' Conds                

    // not used
    CondComposite(const CondComposite &c);
    CondComposite operator = (const CondComposite &c);
};

// ORs two conditions
class CondOr : public Cond 
{
  public:
    CondOr(Cond *pr1, Cond *pr2) 
	: m_pr1(pr1), m_pr2(pr2) 
	{assert(m_pr1); assert(m_pr2);};
    // ~CondOr();		// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {return (m_pr1->test(values, deltaT) || m_pr2->test(values, deltaT));
    };

  private:
    Cond *m_pr1, *m_pr2;		// 'child' Conds                

    // not used
    CondOr(const CondOr &c);
    CondOr operator = (const CondOr &c);
};

#include "rate.h"

// calls Rate to calculate base probability, calls RNG on result*deltaT
class CondCalcProb : public Cond 
{
  public:
    explicit CondCalcProb(Rate *pr) : m_pr(pr) {};
    // ~CondCalcProb();			// use default destructor

    bool test(const vector<double> &values, double deltaT) const
    {double prob = m_pr->calculate(values);
     if (prob <= 0)
       return false;
     else if (prob >=1)
       return true;
     return sampleBernoulli(prob*deltaT);};

  private:
    Rate *m_pr;

    // not used
    CondCalcProb(const CondCalcProb &c);
    CondCalcProb operator = (const CondCalcProb &c);
};

#endif

