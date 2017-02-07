/** \brief LCG.cpp random number generator
 source: http://www.c-plusplus.de/forum/39335-full (posted by Mady 12:54:00 15.12.2002)

 Es gab ein Problem mit der Gleichverteilung der Zufallszahlen. L�sung (in C):

 Das Problem liegt an der Implementation der rand()-Funktion auf Deinem System.
 Vielleicht versuchst Du einen anderen Generator, der gleichm��iger Verteilt.
 Dieser Generator stammt aus 'Angewandte Kryptographie' und liefert einen
 'zuf�lligen' Wert zwischen 0 und 1 zur�ck.

 LCG= Linear Congruential Generator

 \code a testcode would be..
 #include <cstdio>
 #include <cstdlib>
 #include <ctime>

 #include "LCG.h"

 enum { max_test = 1000 };

 int main(void)
 {
 int a[4] = { 0 };
 int i;

 initLCG(time(NULL), 3487234); // 3487234 ist 'zuf�llig' gew�hlt

 for (i = 0; i < max_test; ++i)
 {
 a[(int)(combinedLCG() * (sizeof (a) / sizeof (a[0])))]++;
 }

 for (i = 0; i < sizeof (a) / sizeof (a[0]); ++i)
 {
 printf("a[%d]: %d (%0.2f%%)\n", i, a[i], (double)a[i] / (double)max_test * 100.0);
 }

 return 0;
 }

 \endcode
 */

#include "LCG.h"
#include <math.h> //for sqrt and log

static long s1 = 1;
static long s2 = 1;

#define MODMULT(a, b, c, m, s) q = s/a; s = b*(s-a*q)-c*q; if (s < 0) s += m;

double combinedLCG(void)
{
	long q, z;

	MODMULT(53668, 40014, 12211, 2147483563L, s1)
	MODMULT(52774, 40692, 3791, 2147483399L, s2)
	z = s1 - s2;
	if (z < 1)
		z += 2147483562;
	return z * 4.656613e-10;
}

/**
 * \return normally distributed random numbers
 *
 * Follows the algorithm
 * of the 'Marsaglia polar method', an modification of the 'Box-Muller'
 * algorithm.
 * \param mean mean
 * \param sd   standard deviation
 * */
double normcLCG(double mean, double sd)
{
	double u = combinedLCG() * 2 - 1;
	double v = combinedLCG() * 2 - 1;
	double r = u * u + v * v;
	if (r == 0 || r > 1)
		return normcLCG(mean, sd);
	double c = sqrt(-2 * log(r) / r);
	return (u * c) * sd + mean;

}

void initLCG(long init_s1, long init_s2)
{
	s1 = init_s1;
	s2 = init_s2;
}

/**
 *

 The Poisson distribution describes the probability of observing k events at a given length of time if the events occur independently at a constant rate lambda. It is often neccessary to simulate random variables with a Poisson distribution, especially in physics Monte Carlo simulations. Sadly most random number generators only give uniformly distributed random numbers, however using a uniform random number pU~U(0,1) one can calculate a Poisson distributed random number pPo~Po(lambda).

 The idea is simple, given lambda, calculate the cumulative probability S(n) of observing n or less events for all k>=0, and generate a uniform random number pU. The transformed random number is the first n for which pU >=S(n).

 This method is some times called the cumulant method and works for most probability distributions, but is most handy when calculating S(n) is easy. Using the Poisson distribution function the sum can be written as

 S(n)= Sum_k=0,n e-lambda lambda*n/k!

 In an implementation this way of calculating S(n) is slow, especially the factorial and exponent parts of the expression. To optimize one can use the iterative form of the distribution:

 P(n+1)= P(n)lambda/k   where   P(0)= e-lambda

 It is also clever to put an upper limit on k for the unlikely case that pU=1 or else one might get stuck in an infinite loop. If lambda is constant for many calls it might be a good idea to precalculate all S(n) and store them in a lookup table.

 Below is a C function that implements the idea. Note that you have to provide your own UniformRandomNumber(). (Code written by Inexpensive, free to use as you like)
 */
int poissonLCG(const double lambda)
{
	int k = 0;                          //Counter
	const int max_k = 1000;           //k upper limit
	double p = combinedLCG();         //uniform random number
	double P = exp(-lambda);          //probability
	double sum = P;                     //cumulant
	if (sum >= p)
		return 0;             //done allready
	for (k = 1; k < max_k; ++k)
	{         //Loop over all k:s
		P *= lambda / double(k);            //Calc next prob
		sum += P;                         //Increase cumulant
		if (sum >= p)
			break;              //Leave loop
	}

	return k;                         //return random number
}

//eof
