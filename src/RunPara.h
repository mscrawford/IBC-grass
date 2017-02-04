/**\file
 \brief definition of struct SRunPara and enums CompMode and CompVersion
 */
//---------------------------------------------------------------------------
#ifndef RunParaH
#define RunParaH

#include <string>

//---------------------------------------------------------------------------
//! Enumeration type to specify size asymmetry/symmetry of competition
enum CompMode {
	sym, asympart, asymtot
};

//! Enumeration type to specify the competition version describing interspecific niche differentiation
/**
 \arg version1 \c no difference between intra- and interspecific competition
 \arg version2 \c higher effects of intraspecific competition
 \arg version3 \c lower resource availability for intraspecific competition
 */
enum CompVersion {
	version1, version2, version3
};

/** MSC
 * \arg on \c variation is taken from "ITVsd" and will be applied to the independently parameterized traits.
 * \arg off \c there is no individual variation.
 */
enum ITV_version {
	off, on
};

enum experimentType {
	communityAssembly, invasionCriterion, catastrophicDisturbance
};

//---------------------------------------------------------------------------
//! Structure with all scenario parameters
struct SRunPara
{

public:

	static const bool verbose = true;

	// Input Files
	static std::string NamePftFile;   	///< Filename of PftTrait-File
	static std::string NameSimFile;  	///< Filename of Simulation-File

	int weekly;
	int ind_out;
	int PFT_out;
	int srv_out;

	static SRunPara RunPara;	//!> scenario parameters

	void cleanRunPara();

	// Competition mode
	CompMode AboveCompMode; //!<0 = symmetric; 1 = partial asymmetry; 2 = total asymmetry
	CompMode BelowCompMode; //!<0 = symmetric; 1 = partial asymmetry; 2 = total asymmetry

	/* niche differentiation
	 * 0 = no difference between intra- and interspecific competition
	 * 1 = higher effects of intraspecific competition
	 * 2 = lower resource availability for intraspecific competition
	 */
	CompVersion Version;

	/* Type of experiment
	 * 0 = Community assembly (traditional)
	 * 1 = Invasion criterion (Run a monoculture for 20 years, then introduce the other species.
	 * 	   Repeat with other species as the monoculture)
	 * 2 = Catastrophic disturbances (At a given year, some proportion of seeds/plants will be killed).
	 */
	experimentType mode;

	// Invasion criterion
	int Tmax_monoculture; // Duration of the monoculture phase

	// Intraspecific trait variation
	ITV_version ITV; 	// MSC
	double ITVsd; 		// MSC

	// Gridspace
	int GridSize;     //!< side length in cm
	int CellNum;      //!< side length in cells
	bool torus;       //!< boundary behavior

	// General parameters
	int Tmax;         		//!< simulation time
	double mort_seeds;     	//!< seed mortality per year (in winter)
	double DiebackWinter; 	//!< portion of aboveground biomass to be removed in winter
	double mort_base;      	//!< basic mortality per week
	double LitterDecomp;   	//!< weekly litter decomposition rate
	double meanARes;      	//!< mean above-ground resource availability
	double meanBRes;       	//!< below-ground resourcer availability
	double EstabRamet;     	//!< probability of ramet establishment (1)

	// Aboveground herbivory
	double GrazProb;   		//!< grazing probability per week
	double PropRemove; 		//!< proportion of above ground mass removed by grazing
	double BitSize;   		//!< Bit size of macro-herbivore
	double MassUngraz;   	//!< biomass ungrazable 15300[mg DW/m�]

	// Belowground herbivory
	double BelGrazResidualPerc;
	int BelGrazStartYear;
	int BelGrazWindow;
	double BelGrazProb;
	double BelGrazPerc;

	// Mowing
	double CutHeight;  //!< Height to cut plants to
	int NCut;          //!< number cuts per year

	// Catastrophic disturbance
	int catastrophicDistYear;
	double CatastrophicPlantMortality;
	double CatastrophicSeedMortality;

	// Trampling parameters
	double DistAreaYear;   //!< fraction of grid area disturbed per year;
	double AreaEvent;      //!< fraction of grid area disturbed in one event
	inline double DistProb()
	{
		return DistAreaYear / AreaEvent / 30.0;
	};

	// Resource variation
	double Aampl;   //!< within year above-ground resource amplitude (not used)
	double Bampl;   //!<  within year above-ground resource amplitude (not used)

	// Seed Rain
	double SeedInput; //!< number of seeds introduced per PFT per year or seed mass introduced per PFT
	int SeedRainType; //!< mode of seed input: 0 - no seed rain;
					  //!< 1 - SeedInput specifies total seed NUMBER + equal number of seeds for each PFT;

	// Functions

	// Constructor
	SRunPara();

	inline double CellScale() {
		return GridSize / (double) CellNum;
	};

	inline int GetGridSize() const {
		return CellNum;
	};

	inline int GetSumCells() const {
		return CellNum * CellNum;
	};

	std::string getSimID(); // Merge ID for data sets
	std::string getFileID(); // Output filename (Repetitions go into the same files)
};
//---------------------------------------------------------------------------
#endif
