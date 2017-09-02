
#include <iostream>
#include <string>
#include <sstream>

#include "Environment.h"

using namespace std;

const int Environment::WeeksPerYear = 30;
int Environment::week;
int Environment::year;

int Environment::SimID;
int Environment::ComNr;
int Environment::RunNr;

Output Environment::output;
RandomGenerator Environment::rng;

std::vector<std::string> Environment::PftInitList;		// list of PFTs used
std::map<std::string, int> Environment::PftSurvTime;	// how long each PFT lives

//-----------------------------------------------------------------------------

Environment::Environment()
{
	Environment::rng = RandomGenerator();
	Parameters::params = Parameters();

	year = 1;
	week = 1;

	SimID = 0;
	ComNr = 0;
	RunNr = 0;
}

//-----------------------------------------------------------------------------

Environment::~Environment()
{
	Traits::pftTraitTemplates.clear();
	Traits::pftInsertionOrder.clear();

	output.cleanup();

	PftInitList.clear();
	PftSurvTime.clear();
}

//-----------------------------------------------------------------------------

void Environment::GetSim(string data)
{
	/////////////////////////
	// Read in simulation parameters

	int IC_version;
	int mode;

	std::stringstream ss(data);

	ss	>> SimID 											// Simulation number
		>> ComNr 											// Community number
		>> IC_version 										// Stabilizing mechanisms
		>> mode												// (0) Community assembly (normal), (1) invasion criterion, (2) catastrophic disturbance
		>> Parameters::params.ITVsd 						// Standard deviation of intraspecific variation
		>> Parameters::params.Tmax 							// End of run year
		>> Parameters::params.meanARes 						// Aboveground resources
		>> Parameters::params.meanBRes 	 					// Belowground resources
		>> Parameters::params.AbvGrazProb 					// Aboveground grazing: probability
		>> Parameters::params.AbvPropRemoved 				// Aboveground grazing: proportion of biomass removed
		>> Parameters::params.BelGrazProb 					// Belowground grazing: probability
		>> Parameters::params.BelGrazPerc 					// Belowground grazing: proportion of biomass removed
		>> Parameters::params.BelGrazAlpha					// For sensitivity analysis of Belowground Grazing algorithm
		>> Parameters::params.BelGrazHistorySize			// For sensitivity analysis of Belowground Grazing algorithm
		>> Parameters::params.CatastrophicPlantMortality	// Catastrophic Disturbance: Percent of plant removal during Catastrophic Disturbance
		>> Parameters::params.CatastrophicDistWeek			// Catastrophic Disturbance: Week of the disturbance
		>> Parameters::params.SeedRainType					// Seed Rain: Off/On/Type
		>> Parameters::params.SeedInput						// Seed Rain: Number of seeds to input per SeedRain event
		>> Parameters::params.weekly						// Output: Weekly output rather than yearly
		>> Parameters::params.ind_out						// Output: Individual-level output
		>> Parameters::params.PFT_out						// Output: PFT-level output
		>> Parameters::params.srv_out						// Output: End-of-run survival output
		>> Parameters::params.trait_out						// Output: Trait-level output
		>> Parameters::params.aggregated_out				// Output: Meta-level output
		>> Parameters::NamePftFile 							// Input: Name of input community (PFT intialization) file
		;

	// set intraspecific competition version, intraspecific trait variation version, and competition modes
	switch (IC_version)
	{
	case 0:
		Parameters::params.stabilization = version1;
		break;
	case 1:
		Parameters::params.stabilization = version2;
		break;
	case 2:
		Parameters::params.stabilization = version3;
		break;
	default:
		break;
	}

	switch (mode)
	{
	case 0:
		Parameters::params.mode = communityAssembly;
		break;
	case 1:
		Parameters::params.mode = invasionCriterion;
		break;
	case 2:
		if (Parameters::params.CatastrophicPlantMortality > 0)
		{
			Parameters::params.mode = catastrophicDisturbance;
		}
		else
		{
			Parameters::params.mode = communityAssembly;
		}
		break;
	default:
		cerr << "Invalid mode parameterization" << endl;
		exit(1);
	}

	if (Parameters::params.mode == invasionCriterion)
	{
		Parameters::params.Tmax += Parameters::params.Tmax_monoculture;
	}

	if (Parameters::params.ITVsd > 0)
	{
		Parameters::params.ITV = on;
	}
	else
	{
		Parameters::params.ITV = off;
	}

	if (Parameters::params.BelGrazPerc > 0)
	{
		Parameters::params.BelGrazResidualPerc = exp(-1 * (Parameters::params.BelGrazPerc / 0.0651));
	}

	////////////////////
	// Setup PFTs
	Parameters::NamePftFile = "data/in/" + Parameters::NamePftFile;
	Traits::ReadPFTDef(Parameters::NamePftFile);

	////////////////////
	// Design output file names
	const string dir = "data/out/";
	const string fid = Parameters::params.outputPrefix;

	string param = 	dir + fid + "_param.csv";
	string trait = 	dir + fid + "_trait.csv";
	string srv = 	dir + fid + "_srv.csv";
	string PFT = 	dir + fid + "_PFT.csv";
	string ind = 	dir + fid + "_ind.csv";
	string aggregated =   dir + fid + "_aggregated.csv";

	output.setupOutput(param, trait, srv, PFT, ind, aggregated);
}

