
#include <iostream>
#include <string>
#include <sstream>

#include "CEnvir.h"

using namespace std;

int CEnvir::week 			= 0;
int CEnvir::year 			= 1;
int CEnvir::WeeksPerYear 	= 30;

int CEnvir::NRep; // number of replications -> read from SimFile;
int CEnvir::SimNr;
int CEnvir::ComNr;
int CEnvir::RunNr;

Output CEnvir::output;

RandomGenerator CEnvir::rng;

vector<double> CEnvir::AResMuster;
vector<double> CEnvir::BResMuster;

map<string, long> CEnvir::PftInitList;	// list of Pfts used
map<string, int> CEnvir::PftSurvTime;

//---------------------------------------------------------------------------
/**
 * constructor for virtual class
 */
CEnvir::CEnvir() {
	ReadLandscape();
}

//------------------------------------------------------------------------------
/**
 * destructor -
 * free summarizing data sets
 */
CEnvir::~CEnvir()
{
	SPftTraits::PftLinkList.clear();

	SPftTraits::pftInsertionOrder.clear();
}
//------------------------------------------------------------------------------
/**
 Function defined global muster resources, set to gridcells at beginning
 of each Year. At the moment only evenly dirtributed single values for above-
 and belowground resources are implemented.
 Later the function can read source files of values <100\% autocorrelated or
 generate some noise around fixed values etc..
 */
void CEnvir::ReadLandscape() {
	//100% autocorrelated values
	AResMuster.clear();
	BResMuster.clear();

	AResMuster = vector<double>(vector<double>::size_type(SRunPara::RunPara.GetSumCells()),
								SRunPara::RunPara.meanARes);
	BResMuster = vector<double>(vector<double>::size_type(SRunPara::RunPara.GetSumCells()),
								SRunPara::RunPara.meanBRes);
}  //end ReadLandscape

//------------------------------------------------------------------------------
/**
 * File input - get run parameters from file
 * - one line per setting;
 * nb repetitions can be specified.
 *
 * After reading one line of Simfile,
 * PFT definitions are read too via SPftTraits::ReadPFTDef()
 *
 \param pos file position to start reading
 \param file name of input file (obsolete, NameSimFile is used)
 \return file position of next simulation set
 */
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
		>> SRunPara::RunPara.BelGrazResidualPerc 		// Belowground grazing: mode
		>> SRunPara::RunPara.BelGrazPerc 				// Belowground grazing: proportion of biomass removed
		>> SRunPara::RunPara.CatastrophicPlantMortality // Catastrophic Disturbance: Percent of plant removal during Catastrophic Disturbance
		>> SRunPara::RunPara.CatastrophicSeedMortality	// Catastrophic Disturbance: Percent of seed removal during Catastrophic Disturbance
		>> SRunPara::RunPara.SeedRainType				// Seed Rain: Off/On/Type
		>> SRunPara::RunPara.SeedInput					// Seed Rain: Number of seeds to input per SeedRain event
		>> SRunPara::RunPara.weekly						// Output: Weekly output rather than yearly
		>> SRunPara::RunPara.ind_out					// Output: Individual-level output
		>> SRunPara::RunPara.PFT_out					// Output: PFT-level output
		>> SRunPara::RunPara.srv_out					// Output: End-of-run survival output
		>> SRunPara::RunPara.trait_out					// Output: Trait-level output
		>> SRunPara::NamePftFile 						// Input: Name of input community (PFT intialization) file
		;

	// set intraspecific competition version, intraspecific trait variation version, and competition modes
	switch (IC_version) {
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

	switch (mode) {
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


	SRunPara::RunPara.validateRunPara();

	////////////////////
	// Setup PFTs
	SRunPara::NamePftFile = "data/in/" + SRunPara::NamePftFile;
	SPftTraits::ReadPFTDef(SRunPara::NamePftFile);

	////////////////////
	// Design output file names
	const string dir = "data/out/";
	const string fid = SRunPara::RunPara.getFileID();

	string param = 	dir + fid + "_param.csv";
	string trait = 	dir + fid + "_trait.csv";
	string srv = 	dir + fid + "_srv.csv";
	string PFT = 	dir + fid + "_PFT.csv";
	string ind = 	dir + fid + "_ind.csv";

	output.setupOutput(param, trait, srv, PFT, ind);
}

//------------------------------------------------------------------------------
/**
 * refresh output data.
 */
void CEnvir::InitRun()
{
	PftInitList.clear();
	PftSurvTime.clear();

	// set resources
	ReadLandscape();
}
