
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
 * file random.h                                                        *
 * Declarations for psuedo-random number routines                       * 
 ***********************************************************************/

#include <vector>
#include <algorithm>

using namespace std;

#ifndef RANDOM_H
#define RANDOM_H

class RandK {
  public:
    static double randk(long idum = 1);
    static void writeToFile(ofstream &outfile);
    static void readFromFile(ifstream &infile);

  protected:
    RandK();

  private:
    static int inext, inextp;
    static long ma[56];
    static int iff;

    // not used
    RandK(const RandK &rr);
    RandK operator = (const RandK &rr);
};

bool sampleBernoulli(double prob);
double gasdev();
double sampleGaussian(double mean, double variance);
	//I think what I've called variance here is really standard deviation

template<class T> void shuffle(vector<T>& vr)
{
  for (unsigned int j = vr.size()-1; j>0; j--)
  {
    double u = RandK::randk();
    int k = int(u*j);
    T temp = vr[j];
    vr[j] = vr[k];
    vr[k] = temp;
  }
}


#endif

