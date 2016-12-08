/*
 * CGridEnvir.cpp
 *
 *  Created on: 24.04.2014
 *      Author: KatrinK
 */

#include "CGridEnvir.h"
#include <sstream>
#include <iostream>
using namespace std;

//------------------------------------------------------------------------------
/**
 * constructor
 */
CGridEnvir::CGridEnvir() :
		CEnvir(), CGrid() {
	ReadLandscape();
}

//------------------------------------------------------------------------------
/**
 * destructor
 */
CGridEnvir::~CGridEnvir() {
//  for (unsigned int i=0;i<ClonOutData.size();i++)  delete ClonOutData[i];
}

//------------------------------------------------------------------------------
/**
 Initiate new Run: reset grid and randomly set initial individuals.
 */
void CGridEnvir::InitRun() {
	CEnvir::InitRun();
	resetGrid();

	//set initial plants on grid...
	//new: read from File
	InitInds(SRunPara::NamePftFile);

//  cout <<" sum of types: "<<PftInitList.size()<<endl;
}

//------------------------------------------------------------------------------

/**      new, flexible version of initialisation that permits to read clonal and other traits from one file

 this function reads a file, introduces PFTs and initializes seeds on grid
 after file data.

 (each PFT on file gets 10 seeds randomly set on grid)
 \since 2012-04-18

 \param n position of type initiates for monoculture
 \param file file name of simulation definitions
 \todo init pft defs elsewhere
 */
void CGridEnvir::InitInds(string file) {

	const int no_init_seeds = 10;

	// PFT Traits are read in GetSim()
	for (map<string, SPftTraits*>::iterator var = SPftTraits::PftLinkList.begin();
			var != SPftTraits::PftLinkList.end();
			++var)
	{
		SPftTraits* traits = var->second; // MSC you could vary the traits here...
		InitClonalSeeds(traits, no_init_seeds);
		PftInitList[traits->name] += no_init_seeds;
		PftSurvTime[traits->name] = 0;
		if(SRunPara::RunPara.verbose) cout << "Initializing " << no_init_seeds << " seeds of PFT: " << traits->name << endl;
		CEnvir::SeedRainGr.PftSeedRainList[traits->name] = SRunPara::RunPara.SeedInput;
	}
} //initialization based on file

//------------------------------------------------------------------------------
/**
 one run of simulations

 environmental prefenences are stored in SRunPara::RunPara

 \since 2012/08 seed rain added to test peak theory
 */
void CGridEnvir::OneRun() {

	ResetT(); // reset time

	do {
		this->NewWeek();
		if(SRunPara::RunPara.verbose) cout << "y " << year << endl;

		OneYear();

		WriteOFiles();

		this->writeSpatialGrid();

		if (endofrun)
			break;

	} while (year < SRunPara::RunPara.Tmax);

}  // end OneSim

//------------------------------------------------------------------------------
/**
 * calculate one year's todos.
 */
void CGridEnvir::OneYear() {
	do {
		if(SRunPara::RunPara.verbose) cout << "y " << year << " w " << week << endl;
		OneWeek();
		exitConditions();
		if (endofrun)
			break;
	} while (++week <= WeeksPerYear);
} // end OneYear

//------------------------------------------------------------------------------
/**
 calculation of one week's todos

 \since 2010-07-07 no invasion version
 */
void CGridEnvir::OneWeek() {

	ResetWeeklyVariables(); //cell loop, removes data from cells
	SetCellResource();      //variability between weeks
	CoverCells();           //plant loop
	DistribResource();      //cell loop, resource uptake and competition
	PlantLoop();            //Growth, Dispersal, Mortality

	if (year > 1)
		Disturb();  //grazing and disturbance

	RemovePlants();         //remove trampled plants

	EstabLottery();         //for seeds and ramets

	if (week == 20)
		SeedMortAge(); //necessary to remove non-dormant seeds before autumn

	if (week == WeeksPerYear) {     //at end of year ...
		Winter();           //removal of above ground biomass and of dead plants
		SeedMortWinter();    //winter seed mortality
	}

	if (week == 20) {        //general output
		GetOutput();   //calculate output variables
	}

	if (SRunPara::RunPara.catastrophicDistYear > 0 // Catastrophic disturbance is on
	&& CEnvir::year == SRunPara::RunPara.catastrophicDistYear // It is the disturbance year
	&& CEnvir::week == 21) // It is the disturbance week
	{
		catastrophicDisturbance();
	}

	if ((SRunPara::RunPara.SeedRainType > 0) && (week == 21)) //seed rain in seed dispersal week
		SeedRain();

	if (week == 30) {
		//get cutted biomass
		GetOutputCutted();
		//clonal output
//      GetClonOutput();   //calculate output variables - now in week 20
	}

}   //end CClonalGridEnvir::OneWeek()

//---------------------------------------------------------------------------
/**
 Exit conditions for runs,
 where clonal Plants migrate into a non-clonal community or
 vice versa.
 \since clonal version

 changed for Lina's realistic-pft-experiments
 */
int CGridEnvir::exitConditions() {
	int currTime = GetT();
	int NPlants = GetNPlants();   //+GetNclonalPlants();
	int NClPlants = GetNclonalPlants();
	int NSeeds = GetNSeeds();

//    if no more individuals existing
	if ((NPlants + NClPlants + NSeeds) == 0) {
		endofrun = true;
		return currTime; //extinction time
	}
	return 0;
} //end CClonalGridEnvir::exitConditions()

//---------------------------------------------------------------------------
/**
 calculate Output-variables and store in intern 'database'

 changed in Version 100715 - for  type-flexible Output
 */
void CGridEnvir::GetOutput() //PftOut& PftData, SGridOut& GridData)
{
	string pft_name;
	double prop_PFT;

	SPftOut* PftWeek = new SPftOut();
	SGridOut* GridWeek = new SGridOut();

	//calculate sums
	for (plant_iter iplant = PlantList.begin(); iplant < PlantList.end();
			++iplant) {
		CPlant* plant = *iplant;
		pft_name = plant->pft();
		//suche pft...
		if (!plant->dead) {
//following lines adapted from Internet
// http://stackoverflow.com/questions/936999/what-is-the-default-constructor-for-c-pointer
//CHANGED BY KK
			map<string, SPftOut::SPftSingle*>::const_iterator pos =
					PftWeek->PFT.find(pft_name);
			SPftOut::SPftSingle* mi;
			if (pos == PftWeek->PFT.end())
				PftWeek->PFT[pft_name] = new SPftOut::SPftSingle();
			mi = PftWeek->PFT.find(pft_name)->second;
			mi->totmass += plant->GetMass();
			++mi->Nind;
			mi->shootmass += plant->mshoot;
			mi->rootmass += plant->mroot;
		}
	}
	//calculate mean values
	typedef map<string, SPftOut::SPftSingle*> mapType;

	for (mapType::const_iterator it = PftWeek->PFT.begin();
			it != PftWeek->PFT.end(); ++it) {
		if (it->second->Nind >= 1) {

			//calculate shannon index and proportion of each PFT
			prop_PFT = (double) it->second->Nind / PlantList.size();
			GridWeek->shannon += (-1) * prop_PFT * log(prop_PFT);
		}
		// cover mit Funktion find()  f�llen -- da sonst evtl adressierungsfehler
		string type = it->first;
		double cover = this->PftCover.find(it->first)->second;
		it->second->cover = cover;

		GridWeek->totmass += it->second->totmass;
		GridWeek->above_mass += it->second->shootmass;
		GridWeek->below_mass += it->second->rootmass;
		GridWeek->Nind += it->second->Nind;

		//add LDDSeeds to PftWeek
		for (int d = 0; d < NDistClass; ++d) {
			PftWeek->PFT[it->first]->LDDseeds[d] =
					(*LDDSeeds)[it->first].NSeeds[d];
			(*LDDSeeds)[it->first].NSeeds[d] = 0;
		}

	}

	//summarize seeds on grid...
	int sumcells = SRunPara::RunPara.GetSumCells();
	for (int i = 0; i < sumcells; ++i) {
		CCell* cell = CellList[i];
		for (seed_iter iter = cell->SeedBankList.begin();
				iter < cell->SeedBankList.end(); ++iter) {
			string pft = (*iter)->pft();
			if (!PftWeek->PFT[pft])
				PftWeek->PFT[pft] = new SPftOut::SPftSingle();
			++PftWeek->PFT[pft]->Nseeds;
		}
	}

	double sum_above = 0, sum_below = 0;
	for (int i = 0; i < sumcells; ++i) {
		CCell* cell = CellList[i];
		sum_above += cell->AResConc;
		sum_below += cell->BResConc;
	}
	GridWeek->aresmean = sum_above / sumcells;
	GridWeek->bresmean = sum_below / sumcells;

	double t_acomp = 0;
	double t_bcomp = 0;
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* cell = CellList[i];
		double acomp = cell->aComp_weekly;
		double bcomp = cell->bComp_weekly;

		t_acomp += acomp;
		t_bcomp += bcomp;
	}
	GridWeek->aComp = t_acomp / SRunPara::RunPara.GetSumCells();
	GridWeek->bComp = t_bcomp / SRunPara::RunPara.GetSumCells();

	this->GetClonOutput(*GridWeek);

	NCellsAcover = GetCoveredCells();
	GridWeek->bareGround = 1.0
			- (double(NCellsAcover) / (SRunPara::RunPara.GetSumCells())); //bare ground

	PftOutData.push_back(PftWeek);
	GridWeek->PftCount = PftSurvival(); //get PFT results
	GridOutData.push_back(GridWeek);
} //end CClonalGridEnvir::GetOutput(
//---------------------------------------------------------------------------
/**
 * get and reset amount of cutted biomass
 *
 * Appends Output struct
 */
void CGridEnvir::GetOutputCutted() {
	SGridOut* GridWeek = GridOutData.back();
	//store cutted biomass and reset value for next mowing
	GridWeek->cutted = this->getCuttedBM();
	this->resetCuttedBM();

}
//---------------------------------------------------------------------------
/**
 * get clonal variables (grid wide)
 */
void CGridEnvir::GetClonOutput(SGridOut& GridData) //PftOut& PftData, )
{
	GridData.NclonalPlants = GetNclonalPlants();
	GridData.NGenets = GetNMotherPlants();
	GridData.MeanGeneration = GetNGeneration();
	GridData.NPlants = GetNPlants();
}

//------------------------------------------------------------------------------
/**
 Saves Pft survival times and returns number of surviving PFTs

 changed in version 100716 for type-flexible Output
 \return number of surviving PFTs
 */
int CGridEnvir::PftSurvival() {
	typedef map<string, long> mapType;
	for (mapType::const_iterator it = PftInitList.begin();
			it != PftInitList.end(); ++it) {

		if (PftOutData.back()->PFT.find(it->first) == PftOutData.back()->PFT.end()) {  // PFT is extinct
			// That is, it isn't in the output container.
			// vergleiche mit letztem Stand
			if (PftSurvTime.find(it->first)->second == 0) //wenn vorher noch existent
				PftSurvTime[it->first] = CEnvir::year; //GetT(); //get current time
		} else { //PFT still exists
			PftSurvTime[it->first] = 0;
		} // [it->first];
	}
	return PftOutData.back()->PFT.size(); //count_pft;
} //endPftSurvival
//-Auswertung--------------------------------------------------------------------------
int CGridEnvir::getGridACover(int i) {
	return CellList[i]->getCover(1);
}
int CGridEnvir::getGridBCover(int i) {
	return CellList[i]->getCover(2);
}

///returns grid cover of a given type
/**
 * Get grid-wide cover of PFT in question.
 *
 * To get bare ground, ask for type 'bare'.
 * @param type PFT in question
 * @return cover of PFT type
 */
double CGridEnvir::getTypeCover(const string type) const {
	//for each cell
	double number = 0;
	const long int sumcells = SRunPara::RunPara.GetSumCells();
	for (long int i = 0; i < sumcells; ++i) {
		number += getTypeCover(i, type);

	}
	return number / sumcells;
}

/**
 * portion of type in question at cell 'i'
 * \warning ohne Adress�berpr�fung
 * \sa CCell::getCover()
 * @param i cell index in CellList
 * @param type PFT in question
 * @return cover at position i
 */
double CGridEnvir::getTypeCover(const int i, const string type) const {
	return CellList[i]->getCover(type);
}

/**
 * MSC
 * Saves the spatial state of the grid (one entry for each plant)
 */
void CGridEnvir::writeSpatialGrid() {
	if (SRunPara::RunPara.SPAT == 1 &&
			(CEnvir::year == SRunPara::RunPara.SPATyear || SRunPara::RunPara.SPATyear == 0))
	{
		string fn = "data/out/Spat-" +
				std::to_string(CEnvir::SimNr) + "_" +
				std::to_string(CEnvir::ComNr) + "_" +
				std::to_string(CEnvir::RunNr) + ".txt";

		((CGrid*) this)->writeSpatialGrid(fn);
	}

	if (SRunPara::RunPara.COMP == 1)
	{
		string fn = "data/out/Comp-" +
				std::to_string(CEnvir::SimNr) + "_" +
				std::to_string(CEnvir::ComNr) + "_" +
				std::to_string(CEnvir::RunNr) + ".txt";

		((CGrid*) this) -> writeCompetitionGrid(fn);
	}
}

//------------------------------------------------------------------------------
/**
 * annual seed rain
 *
 * \author FM - seed rain option
 */
void CGridEnvir::SeedRain() {
	string PFT_ID, PFTtype, Cltype;
	SPftTraits *traits;
	double n = 0;
	cout << "In SeedRain. " << endl;
	// For each PFT, we'll drop N seeds
	for (map<string, long>::const_iterator it = PftInitList.begin();
			it != PftInitList.end(); ++it)
	{

		PFT_ID = it->first;
		traits = SPftTraits::getPftLink(PFT_ID);

		switch (SRunPara::RunPara.SeedRainType)
		{
			case 1:
				n = SRunPara::RunPara.SeedInput;
		}

		cout << "Initializing " << n << " seeds of PFT type: " << PFT_ID << endl;
		CGrid::InitClonalSeeds(traits, n, traits->pEstab);
	}

}

//	string PFT_ID, PFTtype, Cltype;
//	//size_t posc;
//	SPftTraits *pfttraits;
////   SclonalTraits *cltraits;
//	double nseeds = 0;
//
//	double SeedFracClonal = 0.1; //ratio between seed input for clonal and non-clonal seeds;
//
//	//for all plants..
//	for (map<string, long>::const_iterator it = PftInitList.begin();
//			it != PftInitList.end(); ++it) {
//
//		PFT_ID = it->first;
//
////      cltraits=getClLink(PFT_ID);
//		pfttraits = SPftTraits::getPftLink(PFT_ID);
//
//		switch (SRunPara::RunPara.SeedRainType) {
//		// [SeedInput] == seed NUMBER & equal seed NUMBER for each PFT
//		case 1:
//			nseeds = SRunPara::RunPara.SeedInput
//					/ (CEnvir::SeedRainGr.NPftSeedSize[0]
//							+ CEnvir::SeedRainGr.NPftSeedSize[1]
//							+ CEnvir::SeedRainGr.NPftSeedSize[2]);
//			break;
//			// [SeedInput] == seed MASS & equal seed NUMBER for each PFT
//		case 2:
//			nseeds = SRunPara::RunPara.SeedInput
//					/ (CEnvir::SeedRainGr.NPftSeedSize[0] * 0.1
//							+ CEnvir::SeedRainGr.NPftSeedSize[1] * 0.3
//							+ CEnvir::SeedRainGr.NPftSeedSize[2] * 1.0);
//			break;
//			// [SeedInput] == seed MASS & equal seed NUMBER for each PFT
//		case 3:
//			nseeds = SRunPara::RunPara.SeedInput
//					/ (CEnvir::SeedRainGr.NPftSeedSize[0]
//							+ CEnvir::SeedRainGr.NPftSeedSize[1] / 3.0
//							+ CEnvir::SeedRainGr.NPftSeedSize[2] / 10.0) * 0.1
//					/ pfttraits->SeedMass;
//			break;
//			// [SeedInput] == seed MASS& equal seed MASS  for each PFT
//		case 4:
//			nseeds = SRunPara::RunPara.SeedInput
//					/ (CEnvir::SeedRainGr.NPftSeedSize[0]
//							+ CEnvir::SeedRainGr.NPftSeedSize[1]
//							+ CEnvir::SeedRainGr.NPftSeedSize[2])
//					/ pfttraits->SeedMass;
//			break;
//			// [SeedInput] == seed NUMBER & fixed ratio of seed numbers for clonal and non-clonal PFTS
//		case 5:
//			nseeds =
//					SRunPara::RunPara.SeedInput
//							/ (CEnvir::SeedRainGr.NPftClonal[0]
//									+ CEnvir::SeedRainGr.NPftClonal[1]
//											* SeedFracClonal);
//			break;
////disabled..
////        // [SeedInput] == factor for pft-specific seed rain & number of seeds is PFT specific and specified in PFTTraits file
////         case 99: nseeds = SRunPara::RunPara.SeedInput*pfttraits->SeedRainGr;
////            break;
//		case 111:
//			nseeds = SRunPara::RunPara.SeedInput
//					* CEnvir::SeedRainGr.PftSeedRainList[PFT_ID];
//			break;
//		default:
//			nseeds = 0;
//		}
//
//		int nseeds2 = poissonLCG(nseeds); //random number from poisson distribution
//		//int nseeds2 = Round(nseeds);
//		CGrid::InitClonalSeeds(pfttraits, nseeds2, pfttraits->pEstab); //,cltraits
//	}
//}

//eof
