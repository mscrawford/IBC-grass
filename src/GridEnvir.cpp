
#include <iostream>
#include <cassert>
#include "GridEnvir.h"

using namespace std;

//------------------------------------------------------------------------------

GridEnvir::GridEnvir() : Environment(), Grid() { }

//------------------------------------------------------------------------------
/**
 * Initiate new Run: Randomly set initial individuals.
 */
void GridEnvir::InitRun()
{
	InitInds();
}

//-----------------------------------------------------------------------------

void GridEnvir::InitInds()
{
	const int no_init_seeds = 10;
	const double estab = 1.0;

	if (Parameters::params.mode == communityAssembly || Parameters::params.mode == catastrophicDisturbance)
	{
		// PFT Traits are read in GetSim()
		for (auto const& it : Traits::pftTraitTemplates)
		{
			InitSeeds(it.first, no_init_seeds, estab);
			PftSurvTime[it.first] = 0;
		}
	}
	else if (Parameters::params.mode == invasionCriterion)
	{
		assert(Traits::pftTraitTemplates.size() == 2);

		string resident = Traits::pftInsertionOrder[1];
		InitSeeds(resident, no_init_seeds, estab);
		PftSurvTime[resident] = 0;
	}
}

//-----------------------------------------------------------------------------

void GridEnvir::OneRun()
{
	output.print_param();

	if (Parameters::params.trait_out)
	{
		output.print_trait();
	}

	do {
		std::cout << "y " << year << std::endl;

		OneYear();

		if (Parameters::params.mode == invasionCriterion && year == Parameters::params.Tmax_monoculture)
		{
			const int no_init_seeds = 100;
			const double estab = 1.0;

			string invader = Traits::pftInsertionOrder[0];
			InitSeeds(invader, no_init_seeds, estab);
			PftSurvTime[invader] = 0;
		}

		if (exitConditions())
		{
			break;
		}

	} while (++year <= Parameters::params.Tmax);

}

//-----------------------------------------------------------------------------

void GridEnvir::OneYear()
{
	week = 1;

	do {
		//std::cout << "y " << year << " w " << week << std::endl;

		OneWeek();

		if (exitConditions()) break;

	} while (++week <= WeeksPerYear);
}

//-----------------------------------------------------------------------------

void GridEnvir::OneWeek()
{

	ResetWeeklyVariables(); // Clear ZOI data
	SetCellResources();      // Restore/modulate cell resources

	CoverCells();           // Calculate zone of influences (ZOIs)
	DistributeResource();      // Allot resources based on ZOI

	PlantLoop();            // Growth, dispersal, mortality

	if (year > 1)
	{
		Disturb();  		// Grazing and disturbances
	}

	if (Parameters::params.mode == catastrophicDisturbance 				// Catastrophic disturbance is on
			&& Environment::year == Parameters::params.CatastrophicDistYear 	// It is the disturbance year
			&& Environment::week == Parameters::params.CatastrophicDistWeek) 	// It is the disturbance week
	{
		RunCatastrophicDisturbance();
	}

	RemovePlants();    		// Remove decomposed plants and remove them from their genets

	if (Parameters::params.SeedRainType > 0 && week == 21)
	{
		SeedRain();
	}

	EstablishmentLottery(); // for seeds and ramets

	if (week == 20)
	{
		SeedMortalityAge(); 		// Remove non-dormant seeds before autumn
	}

	if (week == WeeksPerYear)
	{
		Winter();           // removal of aboveground biomass and decomposed plants
		SeedMortalityWinter();   // winter seed mortality
	}

	if ((Parameters::params.weekly == 1 || week == 20) &&
			!(Parameters::params.mode == invasionCriterion &&
					Environment::year <= Parameters::params.Tmax_monoculture)) // Not a monoculture
	{

		Environment::output.print_srv_and_PFT(PlantList);

		if (Parameters::params.meta_out == 1)
		{
			Environment::output.print_aggregated(PlantList);
		}

		if (Parameters::params.ind_out == 1)
		{
			Environment::output.print_ind(PlantList);
		}
	}

}

//-----------------------------------------------------------------------------

bool GridEnvir::exitConditions()
{
	// Exit conditions do not exist with external seed input
	if (Parameters::params.SeedInput > 0)
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

//-----------------------------------------------------------------------------

void GridEnvir::SeedRain()
{

	string PFT_ID;
	double n = 0;

	// For each PFT, we'll drop n seeds
	for (auto const& it : Traits::pftTraitTemplates)
	{
		auto pft_name = it.first;
		switch (Parameters::params.SeedRainType)
		{
			case 1:
				n = Parameters::params.SeedInput;
				break;
			default:
				exit(1);
		}

		Grid::InitSeeds(pft_name, n, 1.0);
	}

}
