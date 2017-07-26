
#include "CGridEnvir.h"

#include <iostream>
#include <cassert>

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

	InitInds(); // Initialize individuals
}

void CGridEnvir::InitInds()
{
	const int no_init_seeds = 10;

	if (SRunPara::RunPara.mode == communityAssembly || SRunPara::RunPara.mode == catastrophicDisturbance)
	{
		// PFT Traits are read in GetSim()
		for (auto it = SPftTraits::PftLinkList.begin(); it != SPftTraits::PftLinkList.end(); ++it)
		{
			shared_ptr<SPftTraits> traits = it->second;
			InitClonalSeeds(traits, no_init_seeds);
			PftInitList[traits->name] += no_init_seeds;
			PftSurvTime[traits->name] = 0;
		}
	}
	else if (SRunPara::RunPara.mode == invasionCriterion)
	{
		assert(SPftTraits::PftLinkList.size() == 2);

		string resident = SPftTraits::pftInsertionOrder[1];
		shared_ptr<SPftTraits> traits = SPftTraits::PftLinkList.find(resident)->second;

		InitClonalSeeds(traits, no_init_seeds);

		PftInitList[traits->name] += no_init_seeds;
		PftSurvTime[traits->name] = 0;
	}

}

void CGridEnvir::OneRun() {

	ResetT();

	output.print_param();

	if (SRunPara::RunPara.trait_out)
	{
		output.print_trait();
	}

	do {
		this->NewWeek();

		if (SRunPara::RunPara.verbose) cout << "y " << year << endl;

		OneYear();

		if (SRunPara::RunPara.mode == invasionCriterion && year == SRunPara::RunPara.Tmax_monoculture)
		{
			const int no_init_seeds = 100;
			string invader = SPftTraits::pftInsertionOrder[0];
			shared_ptr<SPftTraits> traits = SPftTraits::PftLinkList.find(invader)->second;
			InitClonalSeeds(traits, no_init_seeds);
			PftInitList[traits->name] += no_init_seeds;
			PftSurvTime[traits->name] = 0;
		}

		if (exitConditions()) break;

	} while (year < SRunPara::RunPara.Tmax);

}

void CGridEnvir::OneYear()
{
	do {
//		if (SRunPara::RunPara.verbose) cout << "y " << year << " w " << week << endl;

		OneWeek();

		if (exitConditions()) break;

	} while (++week <= WeeksPerYear);
}

void CGridEnvir::OneWeek()
{

	ResetWeeklyVariables(); // cell loop, removes data from cells
	SetCellResource();      // variability between weeks

	CoverCells();           // plant loop
	DistribResource();      // cell loop, resource uptake and competition
	PlantLoop();            // Growth, Dispersal, Mortality
	RemovePlants();         // remove trampled plants
	EstablishmentLottery(); // for seeds and ramets

	if (week == 20)
	{
		SeedMortAge(); // necessary to remove non-dormant seeds before autumn
	}

	if (SRunPara::RunPara.mode == catastrophicDisturbance 				// Catastrophic disturbance is on
			&& CEnvir::year == SRunPara::RunPara.CatastrophicDistYear 	// It is the disturbance year
			&& CEnvir::week == SRunPara::RunPara.CatastrophicDistWeek) 	// It is the disturbance week
	{
		RunCatastrophicDisturbance();
	}

	if (year > 1)
	{
		Disturb();  //grazing and disturbance
	}

	if ((SRunPara::RunPara.weekly == 1 || week == 20) &&
			!(SRunPara::RunPara.mode == invasionCriterion &&
					CEnvir::year <= SRunPara::RunPara.Tmax_monoculture)) // Not a monoculture
	{

		CEnvir::output.print_srv_and_PFT(PlantList);

		CEnvir::output.print_meta();

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
		Winter();           	// removal of above ground biomass and of dead plants
		SeedMortWinter();    	// winter seed mortality
	}

}

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
}


void CGridEnvir::SeedRain()
{

	string PFT_ID;
	shared_ptr<SPftTraits> traits;
	double n = 0;

	// For each PFT, we'll drop n seeds
	for (map<string, long>::const_iterator it = PftInitList.begin();
			it != PftInitList.end();
			++it)
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
