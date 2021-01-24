
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
 * file rate.h                                                          *
 * Declarations for virtual Rate class and derived classes   		*
 * Originally used just to calculate secretion rates; now used as	*
 * general-purpose functions (no checking done here to make sure the  	*
 * values generated are appropriate for those purposes).		*
 * Different concrete classes implement different functions, which are  *
 * applied to one variable out of a parameter list.                     *
 ************************************************************************/

#ifndef RATE_H
#define RATE_H

#include <vector>
#include <cmath>
#include <cassert>

using namespace std;

class Rate {
  public:
    Rate() {};
    virtual ~Rate() {};
    virtual double calculate(const vector<double> &values) const=0;

  private:
    // not used
    // Rate(const Rate &rr);
    // Rate operator = (const Rate &rr);
};

class RateFixed : public Rate
{
  public:
    explicit RateFixed(double rate) : m_rate(rate) {};
    // ~RateFixed();                    // use default destructor

    double calculate(const vector<double> &values) const
    {return m_rate;};

  private:
    double m_rate;

    // not used
    RateFixed(const RateFixed &r);
    RateFixed operator = (const RateFixed &r);
};

class RateVar : public Rate 
{
  public:
    RateVar(int index) 
	: m_index(index) 
	{assert(m_index>=0);};
    // ~RateVar();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index<int(values.size()));
     return values[m_index];};

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation

    // not used
    RateVar(const RateVar &r);
    RateVar operator = (const RateVar &r);
};

class RateLinear : public Rate 
{
  public:
    RateLinear(int index, double slope, double yinter) 
	: m_index(index), m_slope(slope), m_yinter(yinter) 
	{assert(m_index>=0);};
    // ~RateLinear();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index<int(values.size()));
     return (m_slope*values[m_index] + m_yinter);};

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_slope, m_yinter;	// define line 

    // not used
    RateLinear(const RateLinear &r);
    RateLinear operator = (const RateLinear &r);
};

class RateChoppedLinear : public Rate 
{
  public:
    RateChoppedLinear(int index, double slope, double yinter, 
		double min, double max) : 
	m_index(index), m_slope(slope), m_yinter(yinter), m_min(min), m_max(max)
	{assert(m_index>=0);};
    // ~RateChoppedLinear();		// use default destructor

    double calculate(const vector<double> &values) const {
      assert(m_index<int(values.size()));
      double rate = (m_slope*values[m_index] + m_yinter);
      if (rate < m_min) rate = m_min;
      else if (rate > m_max) rate = m_max;
      return rate;
    };

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_slope, m_yinter;	// define line 
    double m_min, m_max;	// define 'chopping' off points

    // not used
    RateChoppedLinear(const RateChoppedLinear &r);
    RateChoppedLinear operator = (const RateChoppedLinear &r);
};

class RateProduct : public Rate 
{
  public:
    RateProduct(int index1, int index2) 
	: m_index1(index1), m_index2(index2)
	{assert(m_index1>=0); assert(m_index2>=0);};
    // ~RateProduct();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index1<int(values.size()));
     assert(m_index2<int(values.size()));
     return (values[m_index1] * values[m_index2]);};

  private:
    int m_index1, m_index2;	// index into cell's attribute list
				// for parameters involved in this calculation

    // not used
    RateProduct(const RateProduct &r);
    RateProduct operator = (const RateProduct &r);
};

// Value returned by RateSaturating varies from 0 to max as the variable
// used increases from 0
// General form:
// maxRate*x/(halfSat+x)
class RateSaturating : public Rate 
{
  public:
    RateSaturating(int index, double maxRate, double halfSat) 
	: m_index(index), m_maxRate(maxRate), m_halfSat(halfSat) 
	{assert(m_index>=0);};
    // ~RateSaturating();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index<int(values.size())); 
     double x = values[m_index];
     return ( m_maxRate*x / (x + m_halfSat) ); };

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_maxRate, m_halfSat; // define saturating function

    // not used
    RateSaturating(const RateSaturating &r);
    RateSaturating operator = (const RateSaturating &r);
};


// Value returned by RateInhibiting varies from max to 0, as the variable
// used increases from 0
// General form:
// maxRate*c/(c+x)
class RateInhibiting : public Rate 
{
  public:
    RateInhibiting(int index, double maxRate, double c) 
	: m_index(index), m_maxRate(maxRate), m_c(c) 
	{assert(m_index>=0);};
    // ~RateInhibiting();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index<int(values.size())); 
     return ( m_maxRate * m_c / ( values[m_index] + m_c) ); };

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_maxRate; 		// max rate returned               
    double m_c; 		// constant in inhibition equation

    // not used
    RateInhibiting(const RateInhibiting &r);
    RateInhibiting operator = (const RateInhibiting &r);
};

// RateRelSat is a modification of the Saturating form that allows for
// interaction between two cytokines, with allowance made for differences
// in relative contributions of the two terms.  
// General form: // maxRate*x/(x + f*y + halfSat)
// Note that this is generally used when y inhibits an x-induced rate; f is 
// positive.  Cooperation between x and y could be represented with a negative
// f value, but this is problematic if f*y can be > halfSat.
// Below, index1 is used to find x and index2 is used to find y
class RateRelSat : public Rate 
{
  public:
    RateRelSat(int index1, int index2, double maxRate, double halfSat,
	double f) : m_index1(index1), m_index2(index2), m_maxRate(maxRate), 
	m_halfSat(halfSat), m_f(f)
	{assert(m_index1>=0); assert(m_index2>=0);};
    // ~RateRelSat();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index1<int(values.size())); 
     assert(m_index2<int(values.size())); 
     double x = values[m_index1];
     double y = values[m_index2];
     return ( m_maxRate*x / (x + m_f*y + m_halfSat) ); };

  private:
    int m_index1, m_index2;	// indices into cell's attribute list
				// for parameters involved in this calculation
    double m_maxRate, m_halfSat; // as in usual saturating function
    double m_f;			// accounts for relative effect of second
				// variable

    // not used
    RateRelSat(const RateRelSat &r);
    RateRelSat operator = (const RateRelSat &r);
};

// RateRelInh is a modification of the Inhibiting form that allows for
// interaction between two cytokines, with allowance made for differences
// in relative contributions of the two terms.  General form:
// maxRate*c/(x + f*y + c)
// f is usually positive; both x and y inhibit.
class RateRelInh : public Rate 
{
  public:
    RateRelInh(int index1, int index2, double maxRate, double c, double f) 
	: m_index1(index1), m_index2(index2), m_maxRate(maxRate), m_c(c),
	  m_f(f) {assert(m_index1>=0); assert(m_index2>=0);};
    // ~RateRelInh();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index1<int(values.size())); 
     assert(m_index2<int(values.size())); 
     double x = values[m_index1];
     double y = values[m_index2];
     return ( m_maxRate * m_c / ( x + m_f*y + m_c) ); };

  private:
    int m_index1, m_index2;	// indices into cell's attribute list
				// for parameters involved in this calculation
    double m_maxRate; 		// max rate returned               
    double m_c;			// constant in inhibition equation
    double m_f;			// accounts for relative contribution of 
				// second variable

    // not used
    RateRelInh(const RateRelInh &r);
    RateRelInh operator = (const RateRelInh &r);
};

// RateSynergy is a modification of the Saturating form that allows for
// synergy between two cytokines, but with the second cytokine unable to have
// any effect without the first.
// General form:
// maxRate*x*(1+fy)/(x*(1+fy)+c)
// f should be ~1/(normal y value)
class RateSynergy : public Rate 
{
  public:
    RateSynergy(int index1, int index2, double maxRate, double c, double f) 
	: m_index1(index1), m_index2(index2), m_maxRate(maxRate), m_c(c),
	  m_f(f) {assert(m_index1>=0); assert(m_index2>=0);};
    // ~RateSynergy();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index1<int(values.size())); 
     assert(m_index2<int(values.size())); 
     double x = values[m_index1];
     double y = values[m_index2];
     return ( m_maxRate * x * (1 + m_f*y) / ( x * (1 + m_f*y) + m_c) ); };

  private:
    int m_index1, m_index2;	// indices into cell's attribute list
				// for parameters involved in this calculation
    double m_maxRate; 		// max rate returned               
    double m_c;			// constant in inhibition equation
    double m_f;			// accounts for relative contribution of 
				// second variable

    // not used
    RateSynergy(const RateSynergy &r);
    RateSynergy operator = (const RateSynergy &r);
};

// Value returned by RateSigmoid varies from 0 to 1, with value of
// 1/2 at 'threshold', and steepness parameter sigma determining
// how quickly value changes away from threshold
// General form:
// 1/(1 + exp(-sigma (x - threshold)))
class RateSigmoid : public Rate 
{
  public:
    RateSigmoid(int index, double threshold, double sigma)
	: m_index(index), m_thr(threshold), m_sigma(sigma) 
	{assert(m_index>=0);};
    // ~RateSigmoid();		// use default destructor

    double calculate(const vector<double> &values) const
    {assert(m_index<int(values.size())); 
     double x = values[m_index];
     return ( 1 / (1 + exp(-m_sigma*(x-m_thr))) ); };

  private:
    int m_index;		// index into cell's attribute list
				// for parameter involved in this calculation
    double m_thr, m_sigma; 	// define position and shape of sigmoid  

    // not used
    RateSigmoid(const RateSigmoid &r);
    RateSigmoid operator = (const RateSigmoid &r);
};

// multiplies results from two member Rates - not to be confused with 
// RateProduct, which multiplies two cell variables
class RateComposite : public Rate
{
  public:
    RateComposite(Rate *pr1, Rate *pr2)
        : m_pr1(pr1), m_pr2(pr2)
        {assert(m_pr1); assert(m_pr2);};
    // ~RateComposite();                // use default destructor

    double calculate(const vector<double> &values) const
    {return (m_pr1->calculate(values)*m_pr2->calculate(values));    };

  private:
    Rate *m_pr1, *m_pr2;                // 'child' Rates

    // not used
    RateComposite(const RateComposite &r);
    RateComposite operator = (const RateComposite &r);
};

#endif

