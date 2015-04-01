/**\file
 \brief functions and static defaults of environmental classes

 Version for Ines' invasion experiments: adapt GetSim, Run and Init functions
 of class CClonalGridEnvir (further: CClonalGridEnvir::exitConditions()).
 \date 03/2010

 Version for Linas exploratoria experiments - no invasion; realistic PFTs
 \date 07/2010
 */
//---------------------------------------------------------------------------
#include "CEnvir.h"
#include <iostream>
#include <time.h>
#include <iomanip>
#include <sstream>

using namespace std;
//-CEnvir: Init static variables--------------------------------------------------------------------------
int CEnvir::week = 0;
int CEnvir::year = 1;
int CEnvir::WeeksPerYear = 30;

//Output Files
string CEnvir::NamePftOutFile = "Output/PftOut.txt";
string CEnvir::NameGridOutFile = "Output/GridOut.txt";
string CEnvir::NameSurvOutFile = "Output/SurvOutGraz.txt";
string CEnvir::NameLogFile = "Output/LogUpSc.log";
string CEnvir::NameClonalOutFile = "Output/clonalOut.txt";

string SSR::NameLDDFile1 = "Output/SeedOut1_2.csv";
string SSR::NameLDDFile2 = "Output/SeedOut2_3.csv";
string SSR::NameLDDFile3 = "Output/SeedOut4_5.csv";
string SSR::NameLDDFile4 = "Output/SeedOut9_10.csv";
string SSR::NameLDDFile5 = "Output/SeedOut19_20.csv";

int CEnvir::NRep = 1;        //!> number of replications -> read from SimFile;
int CEnvir::SimNr = 0;
int CEnvir::ComNr = 0;
int CEnvir::RunNr = 0;
vector<double> CEnvir::AResMuster;
vector<double> CEnvir::BResMuster;
map<string, long> CEnvir::PftInitList;  //!< list of Pfts used
map<string, double> SSR::PftSeedRainList;

//---------------------------------------------------------------------------
/**
 * constructor for virtual class
 */
CEnvir::CEnvir() :
		NCellsAcover(0), init(1), endofrun(false) {
	ReadLandscape();
	ACover.assign(SRunPara::RunPara.GetSumCells(), 0);
	BCover.assign(SRunPara::RunPara.GetSumCells(), 0);
}
//---------------------------------------------------------------------------
/**
 * constructor -
 * Restore an previously saved grid.
 * @param id file name stub of saves grid
 */
CEnvir::CEnvir(string id) :
		NCellsAcover(0), init(1), endofrun(false) {
//read file
	string dummi = (string) "Save/E_" + id + ".sav";
	ifstream loadf(dummi.c_str());
	string d;
	loadf >> year >> week;
	string dummi2;
	loadf >> SRunPara::NamePftFile >> dummi2; //NameClonalPftFile must be read separately
//  getline(loadf,d);
	getline(loadf, dummi2);
	getline(loadf, dummi2);
	SRunPara::RunPara.setRunPara(dummi2);
	ReadLandscape();
	ACover.assign(SRunPara::RunPara.GetSumCells(), 0);
	BCover.assign(SRunPara::RunPara.GetSumCells(), 0);
}
//------------------------------------------------------------------------------
/**
 * destructor -
 * free summarizing data sets
 */
CEnvir::~CEnvir() {
	for (unsigned int i = 0; i < GridOutData.size(); i++)
		delete GridOutData[i];
	for (unsigned int i = 0; i < PftOutData.size(); i++)
		delete PftOutData[i];
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
	AResMuster.assign(SRunPara::RunPara.GetSumCells(),
			SRunPara::RunPara.meanARes);
	BResMuster.assign(SRunPara::RunPara.GetSumCells(),
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
int CEnvir::GetSim(const int pos, string file) {

	//Open SimFile,
	ifstream SimFile(SRunPara::NameSimFile.c_str(), ios::binary);
	if (!SimFile.good()) {
		cerr << ("error opening SimFile");
		exit(3);
	}
	cout << "SimFile: " << SRunPara::NameSimFile << endl;
	int lpos = pos;
	if (pos == 0) {  //read header
		string line, file_id; // file_id not used here
		getline(SimFile, line);
		int SimNrMax; // dummi
		SimFile >> SimNrMax >> NRep >> file_id;
//    file_id.erase (file_id.begin(), file_id.begin()+1);
//    file_id.erase (file_id.end()-1, file_id.end());
		getline(SimFile, line);
		getline(SimFile, line);
		lpos = SimFile.tellg();
	}
	SimFile.clear();
	SimFile.seekg(lpos, std::ios_base::beg);

	//ist position g�ltig?, wenn nicht -- Abbruch
	if (!SimFile.good())
		return -1;

	int IC_version, acomp, bcomp, invasionVersion, individualVariation;

	SimFile >> SimNr
			>> ComNr
			>> IC_version
			>> invasionVersion; // MSC

	// MSC:
	switch (invasionVersion) {
	case 0:
		// Case 0: Completely traditional run, no invaders.
		SRunPara::RunPara.Invasion = normal;
		break;
	case 1:
		// Case 1: Drop a small number of seedling invaders into a monoculture of one PFT.
		// The monoculture and the "invasion" are run for the same number of years.
		// The number of seedling invaders is hard coded right now.
		// Each seedling invader is dropped into an "empty spot" in the grid. Therefore
		// it's important that the number isn't too big, or else one may run out of free spots.
		SRunPara::RunPara.Invasion = invasionCriteria;
		break;
	default:
		break;
	}

//	>> dummi     // RunPara.Layer

//	>> RunPara.Version - enum types cannot be read with >>
//  >> acomp
//  >> RunPara.AboveCompMode
//  >> bcomp
//  >> RunPara.BelowCompMode
//  >> RunPara.BelGrazMode   //mode of belowground grazing   --> submodel with direct biomass removal, Katrins version
//  >> RunPara.GridSize
//  >> RunPara.CellNum

	SimFile
	>> individualVariation
	>> SRunPara::RunPara.indivVariationSD;

	if (SRunPara::RunPara.Invasion == invasionCriteria) {
		SimFile >> SRunPara::RunPara.invasionTmax;
		SRunPara::RunPara.Tmax = SRunPara::RunPara.invasionTmax;
	} else {
		SimFile >> SRunPara::RunPara.Tmax;
	}

	SimFile
//	>> RunPara.NPft
	>> SRunPara::RunPara.meanARes
	>> SRunPara::RunPara.meanBRes
	>> SRunPara::RunPara.GrazProb
	>> SRunPara::RunPara.PropRemove
//	>> SRunPara::RunPara.MassUngraz // Residual Mass ungrazable
//	>> SRunPara::RunPara.BitSize           //Grazing bit size
	>> SRunPara::RunPara.DistAreaYear      //Trampling
	>> SRunPara::RunPara.AreaEvent         //Trampling
	>> SRunPara::RunPara.NCut       //Cutting NCut
	>> SRunPara::RunPara.CutMass    //Cutting cutmass
//	>> RunPara.mort_seeds
	>> SRunPara::RunPara.SeedRainType
	>> SRunPara::RunPara.SeedInput //seed number / mass per grid
	>> SRunPara::RunPara.SPAT
	>> SRunPara::RunPara.SPATyear
	>> SRunPara::RunPara.PFT
	>> SRunPara::RunPara.COMP
	>> SRunPara::NamePftFile
//  >> RunPara.PftFile
//	>> RunPara.BGThres
	;

	SRunPara::NamePftFile = (string) "Input/" + SRunPara::NamePftFile;
	//---------standard parameter:
	//grazing intensity
//  SRunPara::RunPara.PropRemove=0.5;
	//no trampling
	//above- and belowground competition -> acomp=asymetric bcomp=symetric
	acomp = 1;
	bcomp = 0;
//	IC_version = 1; //RunPara.Version assume higher intraspecific competition

	//--------------------------------
	// set version and competition  modes - in this way because of enum types!

//    SRunPara::RunPara.Version=version;  // old code does not work because of problems with enumeration -> switch from integer to enumeration necessary
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

	switch (individualVariation) {
	case 0:
		SRunPara::RunPara.indivVariationVer = off;
		break;
	case 1:
		SRunPara::RunPara.indivVariationVer = on;
		break;
	default:
		break;
	}

// SRunPara::RunPara.AboveCompMode=acomp;
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

//      SRunPara::RunPara.BelowCompMode=bcomp;
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
//	cout << SRunPara::RunPara.toString() << endl;
	//Open InitFile,
	SPftTraits::ReadPFTDef(SRunPara::NamePftFile);

	//set valid OFile names
	string fid = SRunPara::RunPara.getFileID();
	//set file names
	NamePftOutFile = (string) "Output/Pft-" + fid + ".txt";
	NameGridOutFile = (string) "Output/Grd-" + fid + ".txt";
	NameSurvOutFile = (string) "Output/Srv-" + fid + ".txt";
	NameLogFile = (string) "Output/Log-" + fid + ".log";
	SSR::NameLDDFile1 = (string) "Output/LDD-1-2_" + fid + ".txt";
	SSR::NameLDDFile2 = (string) "Output/LDD-2-3_" + fid + ".txt";
	SSR::NameLDDFile3 = (string) "Output/LDD-3-5_" + fid + ".txt";
	SSR::NameLDDFile4 = (string) "Output/LDD-9-10_" + fid + ".txt";
	SSR::NameLDDFile5 = (string) "Output/LDD-19-20_" + fid + ".txt";

	return SimFile.tellg();
} //end  CEnvir::GetSim

string CEnvir::headerToString() {
	std::stringstream mystream;
	mystream
		<< "SimNr" << "\t"
		<< "ComNr" << "\t"
		<< "RunNr" << "\t"
		<< "year" << "\t"
		<< "week" << "\t"
		;
	return mystream.str();
}

string CEnvir::toString() {
	std::stringstream mystream;
	mystream
		<< SimNr << "\t"
		<< ComNr << "\t"
		<< RunNr << "\t"
		<< year << "\t"
		<< week << "\t"
		;
	return mystream.str();
}

//------------------------------------------------------------------------------
/**
 * refresh environment
 *
 * \todo is it used?
 * /
 void CEnvir::clearResults(){
 InitRun();
 ACover.assign(SRunPara::RunPara.GetSumCells(),0);
 BCover.assign(SRunPara::RunPara.GetSumCells(),0);
 NCellsAcover=0;
 }
 */
//------------------------------------------------------------------------------
/**
 * refresh output data.
 * \todo used?
 */
void CEnvir::InitRun() {
	for (unsigned int i = 0; i < GridOutData.size(); i++)
		delete GridOutData[i];
	for (unsigned int i = 0; i < PftOutData.size(); i++)
		delete PftOutData[i];
	PftInitList.clear();
	PftSurvTime.clear();
//   PftWeek=NULL;GridWeek=NULL;
	PftOutData.clear();
	GridOutData.clear();
	endofrun = false;
	//set resources
	ReadLandscape();

}
//------------------------------------------------------------------------------
/**
 * File Output..
 *
 * Choose whether to write every year, once at end of run, or discrete slots between.
 * At end of Run all data or only last year's data can be written
 */
void CEnvir::WriteOFiles() {
	// if (year%10==1){// modulo... output every n time steps
	// if (year==11||year==31){//output in discrete time steps
	WriteGridComplete(false);
	WritePftComplete(false);
//	WriteclonalOutput();
	if (SRunPara::RunPara.Invasion == normal && year == SRunPara::RunPara.Tmax) {	//output at end of run
		WriteSurvival();
//		this->WritePftSeedOutput();	//write all year's output at once
	} else if (SRunPara::RunPara.Invasion == invasionCriteria && year == SRunPara::RunPara.invasionTmax + SRunPara::RunPara.Tmax) {
		WriteSurvival();
//		this->WritePftSeedOutput();	//write all year's output at once
	}
}
//------------------------------------------------------------------------------
/**
 * File Output - grid-wide summaries
 * @param allYears write all data or only the last entry?
 */
void CEnvir::WriteGridComplete(bool allYears) {
	//Open GridFile, write header
	ofstream GridOutFile(NameGridOutFile.c_str(), ios::app);
	if (!GridOutFile.good()) {
		cerr << ("Fehler beim �ffnen GridOutFile");
		exit(3);
	}
	GridOutFile.seekp(0, ios::end);
	long size = GridOutFile.tellp();
	if (size == 0) {
		GridOutFile
				<< "SimNr" << "\t"
				<< "ComNr" << "\t"
				<< "RunNr" << "\t"
				<< "Year" << "\t"
				<< SRunPara::headerToString()
				<< "totalMass" << "\t"
				<< "NInd" << "\t"
				<< "abovemass" << "\t"
				<< "belowmass" << "\t"
				<< "BareGround" << "\t"
				<< "mean_ares" << "\t"
				<< "mean_bres" << "\t"
				<< "shannon" << "\t"
				<< "meanShannon" << "\t"
				<< "nPFT" << "\t"
				<< "meanNPFT" << "\t"
				<< "Cutted" << "\t"
				<< "nNonClonal" << "\t"
				<< "nClonal" << "\t"
				<< "NGenets" << "\t"
				<< endl;
	}

	vector<SGridOut>::size_type i = 0;
	if (!allYears)
		i = GridOutData.size() - 1;
	for (i; i < GridOutData.size(); ++i) {
		GridOutFile
				<< SimNr << "\t"
				<< ComNr << "\t"
				<< RunNr << "\t"
				<< i << "\t"
				<< SRunPara::RunPara.toString()
				<< GridOutData[i]->totmass << '\t'
				<< GridOutData[i]->Nind << '\t'
				<< GridOutData[i]->above_mass << '\t'
				<< GridOutData[i]->below_mass << '\t'
				<< GridOutData[i]->bareGround << '\t'
				<< GridOutData[i]->aresmean << '\t'
				<< GridOutData[i]->bresmean << '\t'
				<< GridOutData[i]->shannon << '\t'
				<< GetMeanShannon(10) << '\t'
				<< GridOutData[i]->PftCount << '\t'
				<< GetMeanNPFT(10) << '\t'
				<< GridOutData[i]->cutted << '\t'
				<< GridOutData.back()->NPlants << '\t'
				<< GridOutData.back()->NclonalPlants << '\t'
				<< GridOutData.back()->NGenets
				<< endl;
	}
	GridOutFile.close();
} //WriteGridComplete
//------------------------------------------------------------------------------
/**
 * File output - summarizing PFT information
 changed in version 100715 for type-flexible Output
 \param allYears write all data or only the last entry?
 */
void CEnvir::WritePftComplete(bool allYears) {

	if (SRunPara::RunPara.PFT == 1)
	{
		//Open PftFile, write header and initial conditions
		ofstream PftOutFile(NamePftOutFile.c_str(), ios_base::app);
		if (!PftOutFile.good()) {
			cerr << ("Fehler beim �ffnen PftOutFile");
			exit(3);
		}

		PftOutFile.seekp(0, ios::end);
		long size = PftOutFile.tellp();
		if (size == 0) {
			PftOutFile
					<< "SimNr" << "\t"
					<< "ComNr" << "\t"
					<< "RunNr" << "\t"
					<< "Year" << "\t"
					<< SRunPara::headerToString()
					<< SPftTraits::headerToString()
					<< "Nind" << "\t"
					<< "Nseeds" << "\t"
					<< "cover" << "\t"
					<< "rootmass" << "\t"
					<< "shootmass" << "\t"
					<< endl;
		}

		vector<SPftOut>::size_type i = 0;
		if (!allYears)
			i = PftOutData.size() - 1;
		for (i; i < PftOutData.size(); ++i)
		{
			typedef map<string, SPftOut::SPftSingle*> mapType;
			for (mapType::const_iterator it = PftOutData[i]->PFT.begin();
					it != PftOutData[i]->PFT.end(); ++it)
			{

				SPftTraits* traits = SPftTraits::PftLinkList.find(it->first)->second;
				PftOutFile
							<< SimNr << "\t"
							<< ComNr << "\t"
							<< RunNr << "\t"
							<< i << "\t"
							<< SRunPara::RunPara.toString()
							<< traits->toString()
							<< it->second->Nind << "\t"
							<< it->second->Nseeds << "\t"
							<< it->second->cover << "\t"
							<< it->second->rootmass << "\t"
							<< it->second->shootmass << "\t"
							<< endl;
			}
		}
		PftOutFile.close();
	}
} //end WritePftComplete()
//------------------------------------------------------------------------------
/**
 * File Output - survival Info of PFTs
 */
void CEnvir::WriteSurvival() {
	WriteSurvival(RunNr, SimNr, ComNr);
}
/**
 * File Output - survival Info of PFTs
 *
 * @param str Output file name stub
 */
void CEnvir::WriteSurvival(string str) {
	string dummi = NameSurvOutFile.substr(0, NameSurvOutFile.length() - 4);
	NameSurvOutFile = dummi + str + ".txt";
	WriteSurvival();
}
/**
 File-Output - Documentation of type-specific survival statistics.
 */
void CEnvir::WriteSurvival(int runnr, int simnr, int comnr) {
	ofstream SurvOutFile(NameSurvOutFile.c_str(), ios_base::app);
	if (!SurvOutFile.good()) {
		cerr << ("Fehler beim �ffnen SurvFile");
		exit(3);
	}

	SurvOutFile.seekp(0, ios::end);
	long size = SurvOutFile.tellp();
	if (size == 0) {
		SurvOutFile
		<< CEnvir::headerToString()
		<< SRunPara::headerToString()
		<< SPftTraits::headerToString()
		<< "mPop" << "\t"
		<< "cPop" << "\t"
		<< "TE" << "\t"
		<< endl;
	}

	typedef map<string, int> mapType;
	for (mapType::const_iterator it = PftSurvTime.begin();
			it != PftSurvTime.end(); ++it) {
		SPftTraits* traits = SPftTraits::getPftLink(it->first);
		SurvOutFile	<< CEnvir::toString()
					<< SRunPara::RunPara.toString()
					<< traits->toString()
					<< GetMeanPopSize(it->first, 10) << '\t' // mean of 10 years
					<< GetCurrPopSize(it->first) << '\t'
					<< it->second << '\t'
					<< endl;
	}
//     SurvOutFile<<"\n";
} //end writeSurvival
//---------------------------------------------------------------------------
/**
 * Write clonal Output
 */
void CEnvir::WriteclonalOutput() {
	//get data
	//write data in the clonalOut file
	ofstream clonOut(NameClonalOutFile.c_str(), ios_base::app);
//    {
	if (!clonOut.good()) {
		cerr << ("Fehler beim �ffnen ClonalOutFile");
		exit(3);
	}
	clonOut.seekp(0, ios::end);
	long size = clonOut.tellp();
	if (size == 0)
		clonOut << "SimNr\tComNr\tRun\tweek\tnon-ClPlants\tClPlants\tclones\n";
	clonOut << SimNr << "\t"
			<< ComNr << "\t"
			//       <<Pfttype+1<<"\t"
			//       <<clonaltype+1<<"\t"
			<< RunNr + 1 << "\t" << GetT() << "\t"
			<< GridOutData.back()->NPlants << "\t"
			<< GridOutData.back()->NclonalPlants << "\t" //GetNclonalPlants()
			<< GridOutData.back()->NGenets //GetNMotherPlants()
			<< endl;    //schreibt z.b: 1 4
//    }
	clonOut.close();
}    //end CClonalGridEnvir::clonalOutput()
//---------------------------------------------------------------------------
/**
 * file output of seed rain (current list)
 *
 * \author FM - seed rain option
 */
void CEnvir::WritePftSeedOutput() {
	string PFTname;
//   size_t pos1;

	// File 1 ----------------------------------------------------------------
	//Open PftFiles , write header and initial conditions
	ofstream LDDFile1(SSR::NameLDDFile1.c_str(), ios_base::app);
	if (!LDDFile1.good()) {
		cerr << ("Fehler beim �ffnen LDDFile");
		exit(3);
	}

	LDDFile1.seekp(0, ios::end);
	long size = LDDFile1.tellp();
	if (size == 0) {
		LDDFile1 << "Sim\tComNr\tRun\tTime";
		LDDFile1 << "\t";

		for (map<string, SPftTraits*>::const_iterator it =
				SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {
			PFTname = it->first;
			LDDFile1 << PFTname << "\t";
		}
		LDDFile1 << "\n";
	}

	typedef map<string, SPftTraits*> mapType;
	map<string, SPftOut::SPftSingle*>::const_iterator pos;

	for (vector<PftOut>::size_type i = 0; i < PftOutData.size(); ++i) {

		LDDFile1 << SimNr << '\t'
				 << ComNr << "\t"
				 << RunNr << '\t' << i + 1;

		for (mapType::const_iterator it = SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {

			string pft_name = it->first;
			pos = PftOutData[i]->PFT.find(pft_name);
			if (pos == PftOutData[i]->PFT.end())
				LDDFile1 << "\t" << 0;
			else
				LDDFile1 << "\t" << PftOutData[i]->PFT[pft_name]->LDDseeds[0];

		}
		LDDFile1 << "\n";
	}
	LDDFile1.close();

	// File 2 ----------------------------------------------------------------
	//Open PftFiles , write header and initial conditions
	ofstream LDDFile2(SSR::NameLDDFile2.c_str(), ios_base::app);
	if (!LDDFile2.good()) {
		cerr << ("Fehler beim �ffnen LDDFile");
		exit(3);
	}

	LDDFile2.seekp(0, ios::end);
	size = LDDFile2.tellp();
	if (size == 0) {
		LDDFile2 << "Sim\tRun\tTime";
		LDDFile2 << "\t";

		for (mapType::const_iterator it = SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {
			PFTname = it->first;
			LDDFile2 << PFTname << "\t";
		}
		LDDFile2 << "\n";
	}

	for (vector<PftOut>::size_type i = 0; i < PftOutData.size(); ++i) {

		LDDFile2 << SimNr << '\t' << RunNr << '\t' << i + 1;

		for (mapType::const_iterator it = SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {

			string pft_name = it->first;
			pos = PftOutData[i]->PFT.find(pft_name);
			if (pos == PftOutData[i]->PFT.end())
				LDDFile2 << "\t" << 0;
			else
				LDDFile2 << "\t" << PftOutData[i]->PFT[pft_name]->LDDseeds[1];

		}
		LDDFile2 << "\n";
	}
	LDDFile2.close();

	// File 3 ----------------------------------------------------------------
	//Open PftFiles , write header and initial conditions
	ofstream LDDFile3(SSR::NameLDDFile3.c_str(), ios_base::app);
	if (!LDDFile3.good()) {
		cerr << ("Fehler beim �ffnen LDDFile");
		exit(3);
	}

	LDDFile3.seekp(0, ios::end);
	size = LDDFile3.tellp();
	if (size == 0) {
		LDDFile3 << "Sim\tRun\tTime";
		LDDFile3 << "\t";

		for (map<string, SPftTraits*>::const_iterator it =
				SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {
			PFTname = it->first;
			LDDFile3 << PFTname << "\t";
		}
		LDDFile3 << "\n";
	}

	for (vector<PftOut>::size_type i = 0; i < PftOutData.size(); ++i) {

		LDDFile3 << SimNr << '\t' << RunNr << '\t' << i + 1;

		for (mapType::const_iterator it = SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {

			string pft_name = it->first;
			pos = PftOutData[i]->PFT.find(pft_name);
			if (pos == PftOutData[i]->PFT.end())
				LDDFile3 << "\t" << 0;
			else
				LDDFile3 << "\t" << PftOutData[i]->PFT[pft_name]->LDDseeds[2];

		}
		LDDFile3 << "\n";
	}
	LDDFile3.close();

	// File 4 ----------------------------------------------------------------
	//Open PftFiles, write header and initial conditions
	ofstream LDDFile4(SSR::NameLDDFile4.c_str(), ios_base::app);
	if (!LDDFile4.good()) {
		cerr << ("Fehler beim �ffnen LDDFile");
		exit(3);
	}

	LDDFile4.seekp(0, ios::end);
	size = LDDFile4.tellp();
	if (size == 0) {
		LDDFile4 << "Sim\tRun\tTime";
		LDDFile4 << "\t";

		for (map<string, SPftTraits*>::const_iterator it =
				SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {
			PFTname = it->first;
			LDDFile4 << PFTname << "\t";
		}
		LDDFile4 << "\n";
	}

	for (vector<PftOut>::size_type i = 0; i < PftOutData.size(); ++i) {

		LDDFile4 << SimNr << '\t' << RunNr << '\t' << i + 1;

		for (mapType::const_iterator it = SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {

			string pft_name = it->first;
			pos = PftOutData[i]->PFT.find(pft_name);
			if (pos == PftOutData[i]->PFT.end())
				LDDFile4 << "\t" << 0;
			else
				LDDFile4 << "\t" << PftOutData[i]->PFT[pft_name]->LDDseeds[3];

		}
		LDDFile4 << "\n";
	}
	LDDFile4.close();

	// File 3 ----------------------------------------------------------------
	//Open PftFiles , write header and initial conditions
	ofstream LDDFile5(SSR::NameLDDFile5.c_str(), ios_base::app);
	if (!LDDFile5.good()) {
		cerr << ("Fehler beim �ffnen LDDFile");
		exit(3);
	}

	LDDFile5.seekp(0, ios::end);
	size = LDDFile5.tellp();
	if (size == 0) {
		LDDFile5 << "Sim\tRun\tTime";
		LDDFile5 << "\t";

		for (map<string, SPftTraits*>::const_iterator it =
				SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {
			PFTname = it->first;
			LDDFile5 << PFTname << "\t";
		}
		LDDFile5 << "\n";
	}

	for (vector<PftOut>::size_type i = 0; i < PftOutData.size(); ++i) {

		LDDFile5 << SimNr << '\t' << RunNr << '\t' << i + 1;

		for (mapType::const_iterator it = SPftTraits::PftLinkList.begin();
				it != SPftTraits::PftLinkList.end(); ++it) {

			string pft_name = it->first;
			pos = PftOutData[i]->PFT.find(pft_name);
			if (pos == PftOutData[i]->PFT.end())
				LDDFile5 << "\t" << 0;
			else
				LDDFile5 << "\t" << PftOutData[i]->PFT[pft_name]->LDDseeds[4];

		}
		LDDFile5 << "\n";
	}
	LDDFile5.close();

}   //end WritePftSeedOutput()

//---------------------------------------------------------------------------
//----------------------------------
/**
 * File Output - append text to log file
 * @param text string to append
 * @param filename filename
 */
void CEnvir::AddLogEntry(string text, string filename) {
	ofstream LogFile(filename.c_str(), ios_base::app);
	if (!LogFile.good()) {
		cerr << ("Fehler beim �ffnen LogFile");
		exit(3);
	}

	LogFile << text;
}
//----------------------------------
/**
 * File Output - append numbers to log file
 * @param nb number to append
 * @param filename filename
 */
void CEnvir::AddLogEntry(float nb, string filename) {
	ofstream LogFile(filename.c_str(), ios_base::app);
	if (!LogFile.good()) {
		cerr << ("Fehler beim �ffnen LogFile");
		exit(3);
	}

	LogFile << " " << nb;
}

//---------------------------------------------------------------------------
/**
 * extract mean Shannon Diversity
 * @param years time steps to accumulate
 * @return Shannon Diversity value
 */
double CEnvir::GetMeanShannon(int years) {
	double sum = 0, count = 0;

	//int start=(GridOutData.size()-1)-years*32;
	int start = (GridOutData.size() - 1) - years;

	for (vector<SGridOut>::size_type i = start + 1; i < GridOutData.size();
			++i) {
		sum += GridOutData[i]->shannon;
		count++;
	}
	return sum / count;
}
//---------------------------------------------------------------------------
/**
 * extract mean PFT number
 * @param years time steps to accumulate
 * @return number of PFTs
 */
double CEnvir::GetMeanNPFT(int years) {
	double sum = 0, count = 0;
	int start = (GridOutData.size() - 1) - years;

	for (vector<SGridOut>::size_type i = start + 1; i < GridOutData.size();
			++i) {
		sum += GridOutData[i]->PftCount;
		count++;
	}
	return sum / count;
}
//---------------------------------------------------------------------------
/**
 * extract current population size
 * @param pft PFT asked for
 * @return population size of PFT pft
 */
double CEnvir::GetCurrPopSize(string pft) {
	if (PftOutData.back()->PFT.find(pft) == PftOutData.back()->PFT.end())
		return 0;
	SPftOut::SPftSingle* entry = PftOutData.back()->PFT.find(pft)->second;
	return entry->Nind;
}
//-------------------------------------------------------
/**
 * extract mean population size
 * @param pft PFT asked for
 * @param x time steps to accumulate
 * @return mean population size of PFT pft
 */
double CEnvir::GetMeanPopSize(string pft, int x) {
	//only if output once a year
	if (PftOutData.size() < x)
		return -1;
	double sum = 0;
	for (unsigned int i = (PftOutData.size() - x); i < PftOutData.size(); i++)
		if (PftOutData[i]->PFT.find(pft) != PftOutData[i]->PFT.end()) {
			SPftOut::SPftSingle* entry = PftOutData[i]->PFT.find(pft)->second;
			sum += entry->Nind;
		}
	return (sum / x);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/**
 * summarize seed mass distribution in seed rain
 * \author FM for seed rain option (numbers oriented to PFT categories)
 */
void SSR::GetNPftSeedsize() {

	//calculate number of PFTs with small, medium and large seeds
	for (int i = 0; i < 3; ++i)
		NPftSeedSize[i] = 0;

	string PFT_ID;
//   size_t posc;
	SPftTraits *pfttraits;

	for (map<string, SPftTraits*>::const_iterator it =
			SPftTraits::PftLinkList.begin();
			it != SPftTraits::PftLinkList.end(); ++it) {

		PFT_ID = it->first;
		pfttraits = SPftTraits::getPftLink(PFT_ID);
		if (pfttraits->SeedMass <= 0.1)
			NPftSeedSize[0]++;       //small
		else if (pfttraits->SeedMass >= 1.0)
			NPftSeedSize[2]++;   //large
		else
			NPftSeedSize[1]++;   //if (pfttraits->SeedMass == 0.3) //medium size
	}
}

//---------------------------------------------------------------------------
/**
 * summarize clonality distribution in seed rain
 * \author FM for seed rain option
 */
void SSR::GetNPftSeedClonal() {

	//calculate number of PFTs with small, medium and large seeds
	for (int i = 0; i < 2; ++i)
		NPftClonal[i] = 0;

	string PFT_ID;
//   size_t posc;
	SPftTraits *traits;

	for (map<string, SPftTraits*>::const_iterator it =
			SPftTraits::PftLinkList.begin();
			it != SPftTraits::PftLinkList.end(); ++it) {

		PFT_ID = it->first;
		traits = SPftTraits::getPftLink(PFT_ID);
		if (!traits->clonal)
			NPftClonal[0]++; //non-clonal
		else
			NPftClonal[1]++;                 //clonal
	}
}

//eof
