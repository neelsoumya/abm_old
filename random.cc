
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
 * file random.cc                                                       *
 * Routines for random number generation and sampling                   * 
 ************************************************************************/

#include "random.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

// initialize static variables; reassigned on first call to randk or
// readFromFile
int RandK::inext = 0;
int RandK::inextp = 0;
long RandK::ma[56] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int RandK::iff = 0;

/************************************************************************
 * randk                                                                *
 * Justin Balthrop's implementation of ran3 from section 7.1 of 	*
 * Numerical Recipes in C - Knuth's recommendation for a portable 	*
 * random number generator, subtractive method.  My modifications were 	*
 * to make idum a long int, rather than a pointer to one, and to give 	*
 * it a default value so randk can be called with randk().              *
 * Now embedded in class to allow storage and retrieval of values used  *
 * to generate random numbers - for checkpointing.                      *
 *                                                                      *
 * Parameters:                                                          *
 *   long idum:		seed; pass negative value to initialize or      *
 *                      reinitialize the sequence (default value of 0   *
 *                      will still initialize the sequence using MSEED  *
 *                      alone if this is the first call)                *
 *                                                                      *
 * Returns:  next pseudo-random number between 0 and 1 (double)         *
 ************************************************************************/

#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)

double RandK::randk(long idum /* = 1 */)
{
    long mj,mk;
    int i,ii,k;

    // initialization of sequence
    if (idum < 0 || iff == 0) {
	iff=1;
	mj=labs(MSEED-labs(idum));
	mj %= MBIG;
	ma[55]=mj;
	mk=1;
	for (i=1;i<=54;i++) { 
	    ii=(21*i) % 55;
	    ma[ii]=mk;
	    mk=mj-mk;
	    if (mk < MZ) mk += MBIG;
	    mj=ma[ii];
	} 
	for (k=1;k<=4;k++) 
	    for (i=1;i<=55;i++) { 
		ma[i] -= ma[1+(i+30) % 55];
		if (ma[i] < MZ) ma[i] += MBIG;
	    } 
	inext=0;
	inextp=31;			// also a 'special' value
    } 
 
    // actual generation of uniform random deviate
    if (++inext == 56) inext=1;
    if (++inextp == 56) inextp=1;
    mj=ma[inext]-ma[inextp];
    if (mj < MZ) mj += MBIG;
    ma[inext]=mj;
    return mj*FAC;
}

/************************************************************************
 * RandK::writeToFile                                                   *
 *   Writes the values used in randk to keep track of random number	*
 *   sequence to an already-open file; used for checkpointing.        	*
 *                                                                      *
 * Parameters:                                                          *
 *   ofstream &outfile:	 already-open file to hold the values 		*
 *                                                                      *
 * Returns:  nothing                                                    *
 ************************************************************************/
void RandK::writeToFile(ofstream &outfile)
{
  outfile << inext << " " << inextp;
  for (int i=0; i<56; i++)
    outfile << " " << ma[i];
  outfile << endl << endl;
}

/************************************************************************
 * RandK::readFromFile                                                  *
 *   Reads the values used in randk to keep track of random number	*
 *   sequence from an already-open file; used to restart a simulation	*
 *   that was in progress.						*
 *                                                                      *
 * Parameters:                                                          *
 *   ifstream &infile:	already-open file that holds the values 	*
 *                                                                      *
 * Returns:  nothing                                                    *
 ************************************************************************/
void RandK::readFromFile(ifstream &infile)
{
  infile >> inext >> inextp;
  for (int i=0; i<56; i++)
    infile >> ma[i];
  iff=1;
}

/************************************************************************
 * sampleBernoulli                                                      *
 *    Routine to 'flip a weighted coin' - gets a random uniform deviate *
 * and compares it to an input probability to determine whether an      *
 * with that probability of occurence should happen.                    *
 * Random number generator (randk) should be initialized before calling *
 * sampleBernoulli.                                                     *
 *                                                                      *
 * Parameters:                                                          *
 *    double prob:	the probability of 'success'                    *
 *                                                                      *
 * Returns:  boolean - true if random number is < prob                  *
 ************************************************************************/
bool sampleBernoulli(double prob)
{
  double x = RandK::randk();
  if (x<prob)
    return true;

  return false;
}

/************************************************************************
 * gasdev                                                               *
 *    Routine to return a normally distributed variate with zero        *
 * mean and unit variance.  Based on Numerical Recipes routine -        *
 * This version does not take a seed - random number generator (randk)  *
 * should be initialized before calling gasdev.                         *
 *                                                                      *
 * Parameters:  none                                                    *
 *                                                                      *
 * Returns:  value with specified distribution                          *
 ************************************************************************/
double gasdev()
{
  static int iset=0;
  static double gset;
  double fac, rsq, v1, v2;
 
  if (iset == 0)		// no deviate already generated
  {
    do { // until get pair in unit circle
      // pick two uniform numbers in the square extending from -1 to + 1 
      // in each direction
      v1=2.0*RandK::randk()-1.0;
      v2=2.0*RandK::randk()-1.0;
      rsq = v1*v1 + v2*v2;
    } while (rsq >= 1.0 || rsq == 0.0);

    fac = sqrt(-2.0*log(rsq)/rsq);

    // Box-Muller transformation gives two deviates, save one
    gset = v1*fac;
    iset = 1;
    return v2*fac;
  }
  else				// use deviate generated last time
  {
    iset = 0;
    return gset;
  }
}

/************************************************************************
 * sampleGaussian                                                       *
 *    Routine to return a normally distributed variate with arbitrary   *
 * mean and variance.  Just uses gasdev and adjusts deviate to fit      *
 * specified distribution.						*
 *                                                                      *
 * Parameters:                                                          *
 *   double mean, variance:	describes sample distribution           *
 *                                                                      *
 * Returns:  value with specified distribution                          *
 ************************************************************************/
double sampleGaussian(double mean, double variance)
{
  if (variance == 0)
    return mean;
  else
    // get deviate from standard Gaussian, convert
    return variance*gasdev() + mean;
}


