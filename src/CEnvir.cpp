
#include <iostream>
#include <string>
#include <sstream>

#include "CEnvir.h"

using namespace std;

const int CEnvir::WeeksPerYear = 30;
int CEnvir::week;
int CEnvir::year;

int CEnvir::SimNr;
int CEnvir::ComNr;
int CEnvir::RunNr;

Output CEnvir::output;
RandomGenerator CEnvir::rng;

std::vector<std::string> CEnvir::PftInitList;	// list of PFTs used
std::map<std::string, int> CEnvir::PftSurvTime;	// how long each PFT lives

//-----------------------------------------------------------------------------

CEnvir::CEnvir()
{
	CEnvir::rng = RandomGenerator();
	SRunPara::RunPara = SRunPara();

	year = 1;
	week = 1;

	SimNr = 0;
	ComNr = 0;
	RunNr = 0;
}

//-----------------------------------------------------------------------------

CEnvir::~CEnvir()
{
	SPftTraits::pftTraitTemplates.clear();
	SPftTraits::pftInsertionOrder.clear();

	output.cleanup();

	PftInitList.clear();
	PftSurvTime.clear();
}

//-----------------------------------------------------------------------------

void CEnvir::GetSim(string data)
{
	/////////////////////////
	// Read in simulation parameters

	int IC_version;
	int mode;

	std::stringstream ss(data);

	ss	>> SimNr 										// Simulation number
		>> ComNr 										// Community number
		>> IC_version 									// Stabilizing mechanisms
		>> mode											// (0) Community assembly (normal), (1) invasion criterion, (2) catastrophic disturbance
		>> SRunPara::RunPara.ITVsd 						// Standard deviation of intraspecific variation
		>> SRunPara::RunPara.Tmax 						// End of run year
		>> SRunPara::RunPara.meanARes 					// Aboveground resources
		>> SRunPara::RunPara.meanBRes  					// Belowground resources
		>> SRunPara::RunPara.GrazProb 					// Aboveground grazing: probability
		>> SRunPara::RunPara.PropRemove 				// Aboveground grazing: proportion of biomass removed
		>> SRunPara::RunPara.BelGrazProb 				// Belowground grazing: probability
		>> SRunPara::RunPara.BelGrazPerc 				// Belowground grazing: proportion of biomass removed
		>> SRunPara::RunPara.CatastrophicPlantMortality // Catastrophic Disturbance: Percent of plant removal during Catastrophic Disturbance
		>> SRunPara::RunPara.CatastrophicDistWeek		// Catastrophic Disturbance: Week of the disturbance
		>> SRunPara::RunPara.SeedRainType				// Seed Rain: Off/On/Type
		>> SRunPara::RunPara.SeedInput					// Seed Rain: Number of seeds to input per SeedRain event
		>> SRunPara::RunPara.weekly						// Output: Weekly output rather than yearly
		>> SRunPara::RunPara.ind_out					// Output: Individual-level output
		>> SRunPara::RunPara.PFT_out					// Output: PFT-level output
		>> SRunPara::RunPara.srv_out					// Output: End-of-run survival output
		>> SRunPara::RunPara.trait_out					// Output: Trait-level output
		>> SRunPara::RunPara.meta_out					// Output: Meta-level output
		>> SRunPara::NamePftFile 						// Input: Name of input community (PFT intialization) file
		;

	// set intraspecific competition version, intraspecific trait variation version, and competition modes
	switch (IC_version)
	{
	case 0:
		SRunPara::RunPara.Version = version1;
		break;
	case 1:
		SRunPara::RunPara.Version = version2;
		break;
	case 2:
		SRunPara::RunPara.Version = version3;
		break;
	default:
		break;
	}

	switch (mode)
	{
	case 0:
		SRunPara::RunPara.mode = communityAssembly;
		break;
	case 1:
		SRunPara::RunPara.mode = invasionCriterion;
		break;
	case 2:
		SRunPara::RunPara.mode = catastrophicDisturbance;
		break;
	default:
		cerr << "Invalid mode parameterization" << endl;
		exit(1);
	}

	if (SRunPara::RunPara.mode == invasionCriterion)
	{
		SRunPara::RunPara.Tmax += SRunPara::RunPara.Tmax_monoculture;
	}

	if (SRunPara::RunPara.ITVsd > 0)
	{
		SRunPara::RunPara.ITV = on;
	}
	else
	{
		SRunPara::RunPara.ITV = off;
	}

	if (SRunPara::RunPara.BelGrazPerc > 0)
	{
		SRunPara::RunPara.BelGrazResidualPerc = exp(-1 * (SRunPara::RunPara.BelGrazPerc / 0.0651));
	}

	////////////////////
	// Setup PFTs
	SRunPara::NamePftFile = "data/in/" + SRunPara::NamePftFile;
	SPftTraits::ReadPFTDef(SRunPara::NamePftFile);

	////////////////////
	// Design output file names
	const string dir = "data/out/";
	const string fid = SRunPara::RunPara.outputPrefix;

	string param = 	dir + fid + "_param.csv";
	string trait = 	dir + fid + "_trait.csv";
	string srv = 	dir + fid + "_srv.csv";
	string PFT = 	dir + fid + "_PFT.csv";
	string ind = 	dir + fid + "_ind.csv";
	string meta =   dir + fid + "_meta.csv";

	output.setupOutput(param, trait, srv, PFT, ind, meta);
}

