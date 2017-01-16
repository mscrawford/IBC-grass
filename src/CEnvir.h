/**\file
 \brief definition of environmental classes and result structures PftOut and GridOut
 */
//---------------------------------------------------------------------------
#ifndef environmentH
#define environmentH

#include <map>
#include <iostream>
#include <string>
#include <vector>

#include "CGrid.h"
#include "Output.h"
#include "LCG.h"


//---------------------------------------------------------------------------
/// virtual Basic Results Class with general static simulation parameters
/** The class contains
 - simulation-wide (static) information on
 - Names of in- and output-files,
 - an Random Number Generator (plus some service functions), and
 - a template for above-and belowground resources as well as
 - current simulation status (current year, week etc.)
 - variables storing result information on grid and single pfts
 - functions
 - collecting and writing results to output-files
 - reading-in Resource data
 - core function OneWeek(), running a week of the simulation
 \par time scales of the simulations:
 - 1 step = 1 week
 - 1 year = 30 weeks
 */
class CEnvir
{

public:
	static std::map<std::string, long> PftInitList;  //!< list of Pfts used
	static std::map<std::string, int> PftSurvTime; //!< array for survival times of PFTs [years];

	static std::vector<double> AResMuster; //!< mean above-ground resource availability [resource units per cm^2]
	static std::vector<double> BResMuster; //!< mean below-ground resource availability [resource units per cm^2]

	static int week;        ///< current week (0-30)
	static int year;        ///< current year
	static int WeeksPerYear;  ///< number of weeks per year (constantly at value 30)
	static int NRep;        //!< number of replications -> read from SimFile;
	static int SimNr;       ///< simulation-ID
	static int ComNr;///< Community identifier for multiple parameter settings of the same community.
	static int RunNr;       ///< repetition number

	static Output output;

	//Functions
	CEnvir();
	CEnvir(std::string id);  ///<load saved parameter set and state info
	virtual ~CEnvir();

	//! read in fractal below-ground resource distribution (not used)
	static void ReadLandscape();
	///reads simulation environment from file
	void GetSim(std::string data);
	/// returns absolute time horizon
	static int GetT() {
		return (year - 1) * WeeksPerYear + week;
	}
	;
	/// reset time
	static void ResetT() {
		year = 1;
		week = 0;
	}
	;
	/// set new week
	static void NewWeek() {
		week++;
		if (week > WeeksPerYear) {
			week = 1;
			year++;
		};
	}
	;

	/**
	 * "Random"
	 */

	///get a uniformly distributed random number (0-n)
	inline static int nrand(int n) {
		return combinedLCG() * n;
	}
	;

	///get a uniformly distributed random number (0-1)
	inline static double rand01() {
		return combinedLCG();
	}
	;

	///get a uniformly distributed random number (0-1)
	inline static double normrand(double mean, double sd) {
		return normcLCG(mean, sd);
	}
	;

	/**
	 * \name core simulation functions (virtual)
	 * Functions needed for simulation runs.
	 * To be defined in inheriting classes.
	 */
	virtual void InitRun();   ///<init a new run
	virtual void OneWeek()=0;  //!< calls all weekly processes
	virtual void OneYear()=0; ///<runs one year in default mode
	virtual void OneRun()=0;  ///<runs one simulation run in default mode

};
//---------------------------------------------------------------------------
#endif
