
#include <cassert>
#include <iostream>

#include "Parameters.h"
#include "Environment.h"

// Input Files
std::string Parameters::NamePftFile; 							// trait file for experiment species
std::string Parameters::NameSimFile = "data/in/SimFile.txt"; 	// file with simulation scenarios
std::string Parameters::outputPrefix = "default";

Parameters Parameters::parameters;

Parameters::Parameters() :
        weekly(0), individual_out(0), population_out(2), populationSurvival_out(1), trait_out(1), community_out(1),
        AboveCompMode(asympart), BelowCompMode(sym), stabilization(version1), mode(communityAssembly),
        Tmax_monoculture(10),
        ITV(off), ITVsd(0),
        GridSize(141),
        Tmax(100),
        seedMortality(0.5), SeedLongevity(1), winterDieback(0.5), backgroundMortality(0.007), litterDecomp(0.5),
        meanARes(100), meanBRes(100),
        rametEstab(1),
        AbvGrazProb(0), AbvGrazPerc(0), BiteSize(0.5), MassUngrazable(15300),
        BelGrazProb(0), BelGrazPerc(0), BelGrazResidualPerc(0), BelGrazThreshold(0.0667616), BelGrazAlpha(1), BelGrazWindow(10),
        CutHeight(0), NCut(0),
        DisturbanceYear(100), DisturbanceWeek(20), DisturbanceMortality(0),
        Aampl(0), Bampl(0),
        SeedInput(0), SeedRainType(0)
{

}

//-----------------------------------------------------------------------------

std::string Parameters::getSimID()
{

    std::string s =
            std::to_string(Environment::SimID) + "_" +
            std::to_string(Environment::ComNr) + "_" +
            std::to_string(Environment::RunNr);

    return s;
}
