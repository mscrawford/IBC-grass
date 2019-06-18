
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
    const int no_init_seeds = 190;
    const double estab = 1.0;

    if (Parameters::parameters.mode == invasionCriterion)
    {
        assert(Traits::pftTraitTemplates.size() == 2);

        string resident = Traits::pftInsertionOrder[1];
        InitSeeds(resident, no_init_seeds, estab);
        PftSurvTime[resident] = 0;
    }
    else
    {
        // PFT Traits are read in GetSim()
        for (auto const& it : Traits::pftTraitTemplates)
        {
            InitSeeds(it.first, no_init_seeds, estab);
            PftSurvTime[it.first] = 0;
        }
    }

}

//-----------------------------------------------------------------------------

void GridEnvir::OneRun()
{
    output.print_parameter();

    if (Parameters::parameters.trait_out)
    {
        output.print_trait();
    }

    do {
        std::cout << "y " << year << std::endl;

        OneYear();

        if (Parameters::parameters.mode == invasionCriterion && year == Parameters::parameters.Tmax_monoculture)
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

    } while (++year <= Parameters::parameters.Tmax);

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

    if (Parameters::parameters.mode == eutrophication && week == Parameters::parameters.ExperimentStartWeek)
    {
        if (year == Parameters::parameters.ExperimentStartYear)
        {
            Parameters::parameters.meanBRes += Parameters::parameters.EutrophicationIntensity;
        }
        else if (year == Parameters::parameters.ExperimentStartYear + Parameters::parameters.ExperimentDuration)
        {
            Parameters::parameters.meanBRes -= Parameters::parameters.EutrophicationIntensity;
        }
    }

    SetCellResources();     // Restore/modulate cell resources

    CoverCells();          	// Calculate zone of influences (ZOIs)
    DistributeResource();   // Allot resources based on ZOI

    PlantLoop();            // Growth, dispersal, mortality

    if (year > 1)
    {
        RunYearlyDisturbances();  		// Grazing and disturbances
    }

    if (Parameters::parameters.mode == catastrophicDisturbance 					// Catastrophic disturbance is on
            && Environment::year == Parameters::parameters.DisturbanceYear      // It is the disturbance year
            && Environment::week == Parameters::parameters.DisturbanceWeek) 	// It is the disturbance week
    {
        RunSingletonDisturbance();
    }

    RemovePlants();    		// Remove decomposed plants and remove them from their genets

    if (Parameters::parameters.SeedRainType > 0 &&
            week == 21 &&
            !(Parameters::parameters.mode == SimNet &&
              year > Parameters::parameters.ExperimentStartYear))
    {
        SeedRain();
    }

    EstablishmentLottery(); // for seeds and ramets

    if (week == 20)
    {
        SeedMortalityAge(); // Remove non-dormant seeds before autumn
    }

    if (week == WeeksPerYear)
    {
        Winter();           	// removal of aboveground biomass and decomposed plants
        SeedMortalityWinter();  // winter seed mortality
    }

    if ((Parameters::parameters.weekly == 1 || week == 20) &&
            !(Parameters::parameters.mode == invasionCriterion &&
                    Environment::year <= Parameters::parameters.Tmax_monoculture)) // Not a monoculture
    {
        Environment::output.TotalShootmass.push_back(GetTotalAboveMass());
        Environment::output.TotalRootmass.push_back(GetTotalBelowMass());
        Environment::output.TotalNonClonalPlants.push_back(GetNPlants());
        Environment::output.TotalClonalPlants.push_back(GetNclonalPlants());
        Environment::output.TotalAboveComp.push_back(GetTotalAboveComp());
        Environment::output.TotalBelowComp.push_back(GetTotalBelowComp());

        Environment::output.print_populationSurvival_and_population(PlantList);

        if (Parameters::parameters.community_out == 1)
        {
            Environment::output.print_community(PlantList);
        }

        if (Parameters::parameters.individual_out == 1)
        {
            Environment::output.print_individual(PlantList);
        }
    }

}

//-----------------------------------------------------------------------------

bool GridEnvir::exitConditions()
{
    // Exit conditions do not exist with external seed input
    if (Parameters::parameters.SeedInput > 0)
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

    // For each PFT, we'll drop n seeds
    for (auto const& it : Traits::pftTraitTemplates)
    {
        auto pft_name = it.first;
        int n;

        switch (Parameters::parameters.SeedRainType)
        {
            case 1:
                n = Parameters::parameters.SeedInput;
                break;
            default:
                exit(1);
        }

        Grid::InitSeeds(pft_name, n, 1.0);
    }

}
