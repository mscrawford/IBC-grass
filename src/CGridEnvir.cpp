
#include "CGridEnvir.h"

#include <iostream>
#include <cassert>

using namespace std;

//------------------------------------------------------------------------------
/**
 * constructor
 */
CGridEnvir::CGridEnvir() : CEnvir(), CGrid() { }

//------------------------------------------------------------------------------
/**
 * Initiate new Run: Randomly set initial individuals.
 */
void CGridEnvir::InitRun()
{
	InitInds(); // Initialize individuals
}

void CGridEnvir::InitInds()
{
	const int no_init_seeds = 10;
	const double estab = 1.0;

	if (SRunPara::RunPara.mode == communityAssembly || SRunPara::RunPara.mode == catastrophicDisturbance)
	{
		// PFT Traits are read in GetSim()
		for (auto const& it : SPftTraits::pftTraitTemplates)
		{
			InitClonalSeeds(it.first, no_init_seeds, estab);
			PftSurvTime[it.first] = 0;
		}
	}
	else if (SRunPara::RunPara.mode == invasionCriterion)
	{
		assert(SPftTraits::pftTraitTemplates.size() == 2);

		string resident = SPftTraits::pftInsertionOrder[1];
		InitClonalSeeds(resident, no_init_seeds, estab);
		PftSurvTime[resident] = 0;
	}
}

void CGridEnvir::OneRun()
{
	output.print_param();

	if (SRunPara::RunPara.trait_out)
	{
		output.print_trait();
	}

	do {
		NewWeek();

		if (SRunPara::RunPara.verbose) cout << "y " << year << endl;

		OneYear();

		if (SRunPara::RunPara.mode == invasionCriterion && year == SRunPara::RunPara.Tmax_monoculture)
		{
			const int no_init_seeds = 100;
			const double estab = 1.0;

			string invader = SPftTraits::pftInsertionOrder[0];
			InitClonalSeeds(invader, no_init_seeds, estab);
			PftSurvTime[invader] = 0;
		}

		if (exitConditions())
		{
			break;
		}

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

	PlantLoop();            // Growth, dispersal, mortality

	if (year > 1)
	{
		Disturb();  //grazing and disturbance
	}

	if (SRunPara::RunPara.mode == catastrophicDisturbance 				// Catastrophic disturbance is on
			&& CEnvir::year == SRunPara::RunPara.CatastrophicDistYear 	// It is the disturbance year
			&& CEnvir::week == SRunPara::RunPara.CatastrophicDistWeek) 	// It is the disturbance week
	{
		RunCatastrophicDisturbance();
	}

	RemovePlants();         // remove dead plants

	if (SRunPara::RunPara.SeedRainType > 0 && week == 21)
	{
		SeedRain();
	}

	EstablishmentLottery(); // for seeds and ramets

	if (week == 20)
	{
		SeedMortAge(); // necessary to remove non-dormant seeds before autumn
	}

	if (week == WeeksPerYear)
	{
		Winter();           	// removal of above ground biomass and of dead plants
		SeedMortWinter();    	// winter seed mortality
	}

	if ((SRunPara::RunPara.weekly == 1 || week == 20) &&
			!(SRunPara::RunPara.mode == invasionCriterion &&
					CEnvir::year <= SRunPara::RunPara.Tmax_monoculture)) // Not a monoculture
	{

		CEnvir::output.print_srv_and_PFT(PlantList);

		if (SRunPara::RunPara.meta_out == 1)
		{
			CEnvir::output.print_meta();
		}

		if (SRunPara::RunPara.ind_out == 1)
		{
			CEnvir::output.print_ind(PlantList);
		}
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
	double n = 0;

	// For each PFT, we'll drop n seeds
	for (auto const& it : SPftTraits::pftTraitTemplates)
	{
		auto pft_name = it.first;
		switch (SRunPara::RunPara.SeedRainType)
		{
			case 1:
				n = SRunPara::RunPara.SeedInput;
				break;
			default:
				exit(1);
		}

		CGrid::InitClonalSeeds(pft_name, n, 1.0);
	}

}
