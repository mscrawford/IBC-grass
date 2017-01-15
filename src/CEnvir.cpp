/**\file
 \brief functions and static defaults of environmental classes

 Version for Ines' invasion experiments: adapt GetSim, Run and Init functions
 of class CClonalGridEnvir (further: CClonalGridEnvir::exitConditions()).
 \date 03/2010

 Version for Linas exploratoria experiments - no invasion; realistic PFTs
 \date 07/2010
 */
//---------------------------------------------------------------------------

#include <iostream>

#include "CEnvir.h"

using namespace std;

//-CEnvir: Init static variables-----------------------------------------------
int CEnvir::week = 0;
int CEnvir::year = 1;
int CEnvir::WeeksPerYear = 30;

int CEnvir::NRep = 1;        //!> number of replications -> read from SimFile;
int CEnvir::SimNr;
int CEnvir::ComNr;
int CEnvir::RunNr;

Output CEnvir::output;

vector<double> CEnvir::AResMuster;
vector<double> CEnvir::BResMuster;

map<string, long> CEnvir::PftInitList;  //!< list of Pfts used
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
}
//------------------------------------------------------------------------------
/**
 Function defined global muster resources, set to gridcells at beginning
 of each Year. At the moment only evenly dirtributed single values for above-
 and belowground ressources are implemented.
 Later the function can read source files of values <100\% autocorrelated or
 generate some noise around fixed values etc..
 */
void CEnvir::ReadLandscape() {
	//100% autocorrelated values
	AResMuster.clear();
	BResMuster.clear();

	AResMuster = vector<double>(SRunPara::RunPara.GetSumCells(), SRunPara::RunPara.meanARes);
	BResMuster = vector<double>(SRunPara::RunPara.GetSumCells(), SRunPara::RunPara.meanBRes);
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
int CEnvir::GetSim(const int pos, string file) {

	//Open SimFile,
	ifstream SimFile(SRunPara::NameSimFile.c_str(), ios::binary);
	if (!SimFile.good()) {
		cerr << ("error opening SimFile");
		exit(3);
	}

	int lpos = pos;
	if (pos == 0)
	{  //read header
		string line; // file_id not used here
		getline(SimFile, line);
		SimFile >> NRep;
		getline(SimFile, line);
		getline(SimFile, line);
		lpos = SimFile.tellg();
	}
	SimFile.clear();
	SimFile.seekg(lpos, std::ios_base::beg);

	if (!SimFile.good())
		return -1;

	int IC_version;
	int acomp = 1; // aboveground competition is asymmetric by default
	int bcomp = 0; // belowground competition is symmetric by default

	SimFile
		>> SimNr // Simulation number
		>> ComNr // Community number
		>> IC_version // Stabilizing mechanisms
		>> SRunPara::RunPara.ITVsd // Standard deviation of intraspecific variation
		>> SRunPara::RunPara.Tmax // End of run year
		>> SRunPara::RunPara.meanARes // Aboveground resources
		>> SRunPara::RunPara.meanBRes  // Belowground resources
		>> SRunPara::RunPara.GrazProb // Aboveground grazing: probability
		>> SRunPara::RunPara.PropRemove // Aboveground grazing: proportion of biomass removed
		>> SRunPara::RunPara.BelGrazProb // Belowground grazing: probability
		>> SRunPara::RunPara.BelGrazStartYear // Belowground grazing: year of herbivory introduction
		>> SRunPara::RunPara.BelGrazWindow // Belowground grazing: timespan in which herbivory takes place
		>> SRunPara::RunPara.BelGrazMode // Belowground grazing: mode
		>> SRunPara::RunPara.BelGrazGrams // Belowground grazing: proportion of biomass removed
		>> SRunPara::RunPara.catastrophicDistYear // Year for catastrophic disturbace. Removes all aboveground biomass after seeds are dropped.
		>> SRunPara::RunPara.CatastrophicPlantMortality
		>> SRunPara::RunPara.CatastrophicSeedMortality
		>> SRunPara::RunPara.SeedRainType
		>> SRunPara::RunPara.SeedInput
		>> SRunPara::RunPara.weekly
		>> SRunPara::RunPara.ind_out
		>> SRunPara::RunPara.PFT_out
		>> SRunPara::NamePftFile // Name of PFT input file
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

	if (SRunPara::RunPara.ITVsd > 0)
	{
		SRunPara::RunPara.ITV = on;
	}
	else
	{
		SRunPara::RunPara.ITV = off;
	}

	switch (acomp) {
	case 0:
		SRunPara::RunPara.AboveCompMode = sym;
		break;
	case 1:
		SRunPara::RunPara.AboveCompMode = asympart;
		break;
	case 2:
		SRunPara::RunPara.AboveCompMode = asymtot;
		break;
	default:
		break;
	}

	switch (bcomp) {
	case 0:
		SRunPara::RunPara.BelowCompMode = sym;
		break;
	case 1:
		SRunPara::RunPara.BelowCompMode = asympart;
		break;
	case 2:
		SRunPara::RunPara.BelowCompMode = asymtot;
		break;
	default:
		break;
	}

	//Open InitFile,
	SRunPara::NamePftFile = "data/in/" + SRunPara::NamePftFile;
	SPftTraits::ReadPFTDef(SRunPara::NamePftFile);

	// Design output file names

	const string dir = "data/out/";
	const string fid = SRunPara::RunPara.getFileID();

	string param = dir + fid + "_param.txt";
	string trait = dir + fid + "_trait.txt";
	string srv = dir + fid + "_srv.txt";
	string PFT = dir + fid + " _PFT.txt";
	string ind = dir + fid + "_ind.txt";

	output.setupOutput(param, trait, srv, PFT, ind);

	output.print_param();
	output.print_trait();

	return SimFile.tellg();
}

//------------------------------------------------------------------------------
/**
 * refresh output data.
 */
void CEnvir::InitRun() {
	PftInitList.clear();
	PftSurvTime.clear();

	//set resources
	ReadLandscape();
}
