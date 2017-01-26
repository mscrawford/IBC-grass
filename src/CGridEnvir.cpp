
#include "CGridEnvir.h"

#include <iostream>

using namespace std;

//------------------------------------------------------------------------------
/**
 * constructor
 */
CGridEnvir::CGridEnvir() : CEnvir(), CGrid() {
	ReadLandscape();
}

//------------------------------------------------------------------------------
/**
 * destructor
 */
CGridEnvir::~CGridEnvir() {

}

//------------------------------------------------------------------------------
/**
 Initiate new Run: reset grid and randomly set initial individuals.
 */
void CGridEnvir::InitRun() {
	CEnvir::InitRun();

	resetGrid();

	InitInds(SRunPara::NamePftFile); // Initialize individuals
}

//------------------------------------------------------------------------------

/**      new, flexible version of initialisation that permits to read clonal and other traits from one file

 this function reads a file, introduces PFTs and initializes seeds on grid
 after file data.

 (each PFT on file gets 10 seeds randomly set on grid)
 \since 2012-04-18

 \param n position of type initiates for monoculture
 \param file file name of simulation definitions
 */
void CGridEnvir::InitInds(string file) {

	const int no_init_seeds = 10;

	// PFT Traits are read in GetSim()
	for (map<string, SPftTraits*>::iterator it = SPftTraits::PftLinkList.begin();
			it != SPftTraits::PftLinkList.end();
			++it)
	{
		SPftTraits* traits = it->second;
		InitClonalSeeds(traits, no_init_seeds);
		PftInitList[traits->name] += no_init_seeds;
		PftSurvTime[traits->name] = 0;
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

	output.print_param();
	output.print_trait();

	do {
		this->NewWeek();

		if (SRunPara::RunPara.verbose) cout << "y " << year << endl;

		OneYear();

		if (exitConditions()) break;

	} while (year < SRunPara::RunPara.Tmax);

}  // end OneSim

//------------------------------------------------------------------------------
/**
 * calculate one year's todos.
 */
void CGridEnvir::OneYear()
{
	do {
//		if (SRunPara::RunPara.verbose) cout << "y " << year << " w " << week << endl;
		OneWeek();

		if (exitConditions()) break;

	} while (++week <= WeeksPerYear);
} // end OneYear

//------------------------------------------------------------------------------
/**
 calculation of one week's todos
 */
void CGridEnvir::OneWeek()
{

	ResetWeeklyVariables(); //cell loop, removes data from cells
	SetCellResource();      //variability between weeks
	CoverCells();           //plant loop
	DistribResource();      //cell loop, resource uptake and competition
	PlantLoop();            //Growth, Dispersal, Mortality
	RemovePlants();         //remove trampled plants
	EstabLottery();         //for seeds and ramets

	if (week == 20)
	{
		SeedMortAge(); // necessary to remove non-dormant seeds before autumn
	}

	if (SRunPara::RunPara.catastrophicDistYear > 0 				// Catastrophic disturbance is on
	&& CEnvir::year == SRunPara::RunPara.catastrophicDistYear 	// It is the disturbance year
	&& CEnvir::week == 21) 										// It is the disturbance week
	{
		catastrophicDisturbance();
	}

	if (year > 1)
	{
		Disturb();  //grazing and disturbance
	}

	if (SRunPara::RunPara.weekly == 1 || week == 21)
	{
		CEnvir::output.print_srv_and_PFT(PlantList, CellList);

		if (SRunPara::RunPara.ind_out == 1)
		{
			CEnvir::output.print_ind(PlantList);
		}
	}

	if (SRunPara::RunPara.SeedRainType > 0 && week == 21)
	{
		SeedRain();
	}

	if (week == WeeksPerYear)
	{
		Winter();           	//removal of above ground biomass and of dead plants
		SeedMortWinter();    	//winter seed mortality
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
bool CGridEnvir::exitConditions()
{
	// Exit conditions do not exist with external seed input
	if (SRunPara::RunPara.SeedInput > 0)
		return false;

	int NPlants = GetNPlants();
	int NClPlants = GetNclonalPlants();
	int NSeeds = GetNSeeds();

	// if no more individuals existing
	if ((NPlants + NClPlants + NSeeds) == 0) {
		return true; //extinction time
	}

	return false;
} //end CClonalGridEnvir::exitConditions()

//------------------------------------------------------------------------------
/**
 * annual seed rain
 *
 * \author FM - seed rain option
 */
void CGridEnvir::SeedRain() {
	string PFT_ID;
	SPftTraits *traits;
	double n = 0;

	// For each PFT, we'll drop N seeds
	for (map<string, long>::const_iterator it = PftInitList.begin(); it != PftInitList.end(); ++it)
	{

		PFT_ID = it->first;
		traits = SPftTraits::getPftLink(PFT_ID);

		switch (SRunPara::RunPara.SeedRainType)
		{
			case 1:
				n = SRunPara::RunPara.SeedInput;
		}

		CGrid::InitClonalSeeds(traits, n, traits->pEstab);
	}

}
