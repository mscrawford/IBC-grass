//---------------------------------------------------------------------------

#ifndef LCGH
#define LCGH

//---------------------------------------------------------------------------

/// uniformly distributed random number between 0 and 1
double combinedLCG(void);

/// normal distributed random number with mean and sd
double normcLCG(double,double); //double mean,double sd);

///init random number generator with two large integers
void initLCG(long, long);

/// poisson distributed random number with lambda
int poissonLCG(const double lambda);

#endif
