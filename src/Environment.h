#ifndef SRC_ENVIRONMENT_H_
#define SRC_ENVIRONMENT_H_

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include "Grid.h"
#include "Output.h"
#include "RandomGenerator.h"

class Environment
{

public:
	static RandomGenerator rng;
	static Output output;

	static std::vector<std::string> PftInitList; 	// list of Pfts used
	static std::map<std::string, int> PftSurvTime;	// array for survival times of PFTs (in years);

	const static int WeeksPerYear;  // number of weeks per year (constantly at value 30)

	static int week;	// current week (1-30)
	static int year;    // current year

	static int SimID;   // simulation-ID
	static int ComNr;	// Community identifier for multiple parameter settings of the same community.
	static int RunNr;   // repetition number

	Environment();
	~Environment();

	void GetSim(std::string data); 	// Simulation read in

	/*
	 * Helper function for comparing floating point numbers for equality
	 */
	inline static bool AreSame(double const a, double const b) {
	    return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
	}

};

#endif
