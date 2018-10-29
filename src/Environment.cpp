
#include <iostream>
#include <string>
#include <sstream>

#include "Environment.h"

using namespace std;

const int Environment::WeeksPerYear = 30;
int Environment::week;
int Environment::year;

int Environment::SimID;
int Environment::ComNr;
int Environment::RunNr;

Output Environment::output;
RandomGenerator Environment::rng;

std::vector<std::string> Environment::PftInitList;		// list of PFTs used
std::map<std::string, int> Environment::PftSurvTime;	// how long each PFT lives

//-----------------------------------------------------------------------------

Environment::Environment()
{
    Environment::rng = RandomGenerator();
    Parameters::parameters = Parameters();

    year = 1;
    week = 1;

    SimID = 0;
    ComNr = 0;
    RunNr = 0;
}

//-----------------------------------------------------------------------------

Environment::~Environment()
{
    Traits::pftTraitTemplates.clear();
    Traits::pftInsertionOrder.clear();

    output.cleanup();

    PftInitList.clear();
    PftSurvTime.clear();
}

//-----------------------------------------------------------------------------

void Environment::GetSim(string data)
{
    /////////////////////////
    // Read in simulation parameters

    int IC_version;
    int mode;

    std::stringstream ss(data);

    ss	>> SimID 											// Simulation number
        >> ComNr 											// Community number
        >> IC_version 										// Stabilizing mechanisms
        >> mode												// (0) Community assembly (normal), (1) invasion criterion, (2) catastrophic disturbance
        >> Parameters::parameters.ITVsd 					// Standard deviation of intraspecific variation
        >> Parameters::parameters.Tmax 						// End of run year
        >> Parameters::parameters.meanARes 					// Aboveground resources
        >> Parameters::parameters.meanBRes 	 				// Belowground resources
        >> Parameters::parameters.AbvGrazProb 				// Aboveground grazing: probability per time step
        >> Parameters::parameters.AbvGrazPerc 				// Aboveground grazing: proportion of biomass removed
        >> Parameters::parameters.BelGrazProb 				// Belowground grazing: probability per time step
        >> Parameters::parameters.BelGrazPerc 				// Belowground grazing: proportion of biomass removed
        >> Parameters::parameters.BelGrazThreshold          // Belowground grazing: Threshold value determining when functional response is triggered
        >> Parameters::parameters.BelGrazAlpha				// Belowground grazing: For sensitivity analysis of Belowground Grazing algorithm
        >> Parameters::parameters.BelGrazWindow             // Belowground grazing: For sensitivity analysis of Belowground Grazing algorithm
        >> Parameters::parameters.DisturbanceMortality      // Catastrophic Disturbance: Percent of plant removal during Catastrophic Disturbance
        >> Parameters::parameters.DisturbanceWeek			// Catastrophic Disturbance: Week of the disturbance
        >> Parameters::parameters.ExperimentDuration        // EutrophicationDuration: Number of years during which there is a higher BRes
        >> Parameters::parameters.EutrophicationIntensity   // EutrophicationIntensity: Number of belowground resource units added per week
        >> Parameters::parameters.AbvHerbExclusion
        >> Parameters::parameters.BelHerbExclusion
        >> Parameters::parameters.SeedLongevity             // Seed Bank: Number of years a seed can persist in the seed bank
        >> Parameters::parameters.SeedRainType              // Seed Rain: Off/On/Type
        >> Parameters::parameters.SeedInput					// Seed Rain: Number of seeds to input per SeedRain event
        >> Parameters::parameters.weekly					// Output: Weekly output rather than yearly
        >> Parameters::parameters.individual_out			// Output: Individual-level output
        >> Parameters::parameters.population_out			// Output: PFT-level output
        >> Parameters::parameters.populationSurvival_out	// Output: End-of-run survival output
        >> Parameters::parameters.trait_out					// Output: Trait-level output
        >> Parameters::parameters.community_out				// Output: Meta-level output
        >> Parameters::NamePftFile 							// Input: Name of input community (PFT intialization) file
        ;

    // set intraspecific competition version, intraspecific trait variation version, and competition modes
    switch (IC_version)
    {
    case 0:
        Parameters::parameters.stabilization = version1;
        break;
    case 1:
        Parameters::parameters.stabilization = version2;
        break;
    case 2:
        Parameters::parameters.stabilization = version3;
        break;
    default:
        break;
    }

    switch (mode)
    {
    case 0:
        Parameters::parameters.mode = communityAssembly;
        break;
    case 1:
        Parameters::parameters.mode = invasionCriterion;
        break;
    case 2:
        if (Parameters::parameters.DisturbanceMortality > 0)
        {
            Parameters::parameters.mode = catastrophicDisturbance;
        }
        else
        {
            Parameters::parameters.mode = communityAssembly;
        }
        break;
    case 3:
        Parameters::parameters.mode = eutrophication;
        break;
    default:
        cerr << "Invalid mode parameterization" << endl;
        exit(1);
    }

    if (Parameters::parameters.mode == invasionCriterion)
    {
        Parameters::parameters.Tmax += Parameters::parameters.Tmax_monoculture;
    }

    if (Parameters::parameters.ITVsd > 0)
    {
        Parameters::parameters.ITV = on;
    }
    else
    {
        Parameters::parameters.ITV = off;
    }

    if (Parameters::parameters.BelGrazPerc > 0)
    {
        Parameters::parameters.BelGrazResidualPerc = exp(-1 * (Parameters::parameters.BelGrazPerc / Parameters::parameters.BelGrazThreshold));
    }

    ////////////////////
    // Setup PFTs
    Parameters::NamePftFile = "data/in/" + Parameters::NamePftFile;
    Traits::ReadPFTDef(Parameters::NamePftFile);

    ////////////////////
    // Design output file names
    const string dir = "data/out/";
    const string fid = Parameters::parameters.outputPrefix;

    string parameter = 	dir + fid + "_parameter.csv";
    string trait = 	dir + fid + "_trait.csv";
    string populationSurvival = 	dir + fid + "_populationSurvival.csv";
    string population = 	dir + fid + "_population.csv";
    string individual = 	dir + fid + "_individual.csv";
    string community =   dir + fid + "_community.csv";

    output.setupOutput(parameter, trait, populationSurvival, population, individual, community);
}

