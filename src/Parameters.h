
#ifndef SRC_PARAMETERS_H_
#define SRC_PARAMETERS_H_

#include <string>

// Enumeration type to specify size asymmetry/symmetry of competition
enum CompMode { sym, asympart, asymtot };

// Enumeration type to specify the competition version describing interspecific niche differentiation
/**
 * version1: no difference between intra- and interspecific competition
 * version2: higher effects of intraspecific competition
 * version3: lower resource availability for intraspecific competition
 */
enum stabilizationMode { version1, version2, version3 };

enum ITV_mode { off, on };

enum experimentType { communityAssembly, invasionCriterion, catastrophicDisturbance, eutrophication };

//---------------------------------------------------------------------------

class Parameters
{

public:
    static Parameters parameters;			// Static scenario parameters structure

    // Input Files
    static std::string NamePftFile;   	// Filename of PftTrait-File
    static std::string NameSimFile;  	// Filename of Simulation-File
    static std::string outputPrefix;	// Prefix for the output file, empty is single core, something user-supplied if multi-core

    int weekly;
    int individual_out;
    int population_out;
    int populationSurvival_out;
    int trait_out;
    int community_out;

    // Competition mode
    CompMode AboveCompMode; // 0 = symmetric; 1 = partial asymmetry; 2 = total asymmetry
    CompMode BelowCompMode; // 0 = symmetric; 1 = partial asymmetry; 2 = total asymmetry

    /* niche differentiation
     * 0 = no difference between intra- and interspecific competition
     * 1 = higher effects of intraspecific competition
     * 2 = lower resource availability for intraspecific competition
     */
    stabilizationMode stabilization;

    /* Type of experiment
     * 0 = Community assembly (traditional)
     * 1 = Invasion criterion (Run a monoculture for 10 years, then introduce the other species.
     * 	   Repeat with other species as the monoculture)
     * 2 = Catastrophic disturbances (At a given week/year, some proportion of plants will be killed).
     */
    experimentType mode;

    // Invasion criterion
    int Tmax_monoculture; // Duration of the monoculture phase

    // Intraspecific trait variation
    ITV_mode ITV;
    double ITVsd;

    // Gridspace
    int GridSize;     			// side length in cm

    // General parameters
    int Tmax;         			// simulation time
    double seedMortality;     	// seed mortality per year (in winter)
    int SeedLongevity;         // number of years a seed can persist in the seed bank
    double winterDieback; 		// portion of aboveground biomass to be removed in winter
    double backgroundMortality; // basic mortality per week
    double litterDecomp;   		// weekly litter decomposition rate
    double meanARes;      		// mean above-ground resource availability
    double meanBRes;       		// below-ground resourcer availability
    double rametEstab;     		// probability of ramet establishment (1)

    // Aboveground herbivory
    double AbvGrazProb;   		// grazing probability per week
    double AbvGrazPerc; 		// proportion of above ground mass removed by grazing
    double BiteSize;   			// Bite size of macro-herbivore
    double MassUngrazable;   	// biomass ungrazable 15300[mg DW/m^2]

    // Belowground herbivory
    double BelGrazProb;
    double BelGrazPerc;
    double BelGrazResidualPerc;
    double BelGrazThreshold;
    double BelGrazAlpha;
    int BelGrazWindow;

    // Mowing
    double CutHeight;  // Height to cut plants to
    int NCut;          // number cuts per year

    // Catastrophic disturbance
    int DisturbanceYear;
    int DisturbanceWeek;
    double DisturbanceMortality;

    int ExperimentStartYear;
    int ExperimentStartWeek;
    int ExperimentDuration;

    int EutrophicationIntensity;

    bool AbvHerbExclusion;
    bool BelHerbExclusion;

    // Resource variation
    double Aampl;   // within year above-ground resource amplitude
    double Bampl;   // within year below-ground resource amplitude

    // Seed Rain
    int SeedInput;    // number of seeds introduced per PFT per year or seed mass introduced per PFT
    int SeedRainType; // mode of seed input: 0 - no seed rain, 1 - some number of seeds

    // Constructor
    Parameters();

    inline int getGridArea() const { return GridSize * GridSize; }

    std::string getSimID(); // Merge ID for data sets
};

#endif
