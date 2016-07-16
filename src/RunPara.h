/**\file
 \brief definition of struct SRunPara and enums CompMode and CompVersion
 */
//---------------------------------------------------------------------------
#ifndef RunParaH
#define RunParaH

#include <iostream>
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
 * \arg normal \c traditional model: drops seeds of every PFT and lets them compete
 * \arg invasionCriteria \c Requires exactly two PFTs: The first grows to carrying capacity, and the second is introduced
 */
enum invasionVersion {
	normal, invasionCriteria
};

/** MSC
 * \arg on \c variation is taken from "indivVariationSD" and will be applied to the independently parameterized traits.
 * \arg off \c there is no individual variation.
 */
enum individualVariationVersion {
	off, on
};

//---------------------------------------------------------------------------
//! Structure with all scenario parameters
struct SRunPara {
public:
	//Input Files
	static std::string NamePftFile;   ///< Filename of PftTrait-File
	static std::string NameSimFile;  ///< Filename of Simulation-File

	static SRunPara RunPara;  //!> scenario parameters

	CompMode AboveCompMode; //!<0 = symmetric; 1 = partial asymmetry; 2 = total asymmetry
	CompMode BelowCompMode; //!<0 = symmetric; 1 = partial asymmetry; 2 = total asymmetry


	int SPAT; // Do we record the spatial grid?
	int SPATyear; // Which year do we record the spatial grid?
	int PFT; // Do we record the PFT output?
	int COMP; // Do we record the competition grid?

	//! niche differentiation
	/*!
	 * 0 = no difference between intra- and interspecific competition
	 * 1 = higher effects of intraspecific competition
	 * 2 = lower resource availability for intraspecific competition
	 */
	CompVersion Version;
	invasionVersion Invasion; // MSC
	individualVariationVersion indivVariationVer; // MSC
	double indivVariationSD; // MSC
	int invasionTmax; // MSC: Hard coded in the initialization code for now.
	int GridSize;     //!< side length in cm
	int CellNum;      //!< side length in cells
	bool torus;       //!< boundary behavior
	int Tmax;         //!< simulation time
	int NPft;         //!< number of plant functional types
	double mort_seeds;     //!< seed mortality per year (in winter)
	double DiebackWinter; //!< portion of aboveground biomass to be removed in winter
	double mort_base;      //!< basic mortality per week
	double LitterDecomp;   //!< weekly litter decomposition rate
	double meanARes;       //!< mean above-ground resource availability
	double meanBRes;       //!< below-ground resourcer availability
	double EstabRamet;      //!< probability of ramet establishment (1)

	/** @name GrazParam
	 *  Grazing parameters
	 */
///@{
	double GrazProb;   //!< grazing probability per week
	double PropRemove; //!< proportion of above ground mass removed by grazing
	double BitSize;   //!< Bit size of macro-herbivore
	double MassUngraz;   //!< biomass ungrazable 15300[mg DW/m�]
///@}

	/** @name BGHerbParam
	 *  Belowground-Herbivory parameters
	 */
///@{
	int BelGrazMode; //!< mode of belowground grazing \sa CGrid::GrazingBelGr
	///belowground grazing probability per week
	///\since belowground herbivory simulations
	bool Rootherb; //!< indirect effect of belowground herbovory via correction of conversion rate parameter grow
	double BelGrazProb;
	//! proportion of belowground biomass removed by grazing
	///\since belowground herbivory simulations
	double BelPropRemove;
	///threshold for additional below_herbivory-mortality  (default is one)
	double BGThres;
	bool HetBG;         //!< flag for heterogenous belowground grazing
///@}

	/** @name CutParam
	 *  Cutting parameters
	 */
///@{
	double CutMass;     //!< plant aboveground biomass for plants with LMR = 1.0
	int NCut;          //!< number cuts per year
///@}

	/** @name TramplingParam
	 *  Trampling parameters
	 */
///@{
	double DistAreaYear;   //!< fraction of grid area disturbed per year;
	double AreaEvent;      //!< fraction of grid area disturbed in one event
	inline double DistProb() {
		return DistAreaYear / AreaEvent / 30.0;
	}
	;
///@}

//   double cv_res;     //!< coefficient of resource variation between years (not used)
	double Aampl;    //!< within year above-ground resource amplitude (not used)
	double Bampl;   //!<  within year above-ground resource amplitude (not used)
	std::string PftFile; //!< File with PFT trait parameter in Folder "Input"

	double SeedInput; //!< number of seeds introduced per PFT per year or seed mass introduced per PFT
	int SeedRainType;  //!< mode of seed input: 0 - no seed rain;
					   //!< 1 - SeedInput specifies total seed NUMBER + equal number of seeds for each PFT;
					   //!< 2 - SeedInput specifies total seed MASS + equal number of seeds for each PFT;
					   //!< 3 - SeedInput specifies total seed NUMBER + equal seed mass for each PFT;
					   //!< 4 - SeedInput specifies total seed MASS + equal seed mass for each PFT;
					   //!< 5 - SeedInput specifies total seed NUMBER + non-clonal plants get x-times more seeds than clonal plants

					   //!< 99 - SeedInput specifies FACTOR for seed numbers in PftTraits-File
					   //!< 111 - SeedInput specifies FACTOR for seed numbers in InitPFTdat-File

	SRunPara();
	inline double CellScale() {
		return GridSize / (double) CellNum;
	}
	;
	inline int GetGridSize() const {
		return CellNum;
	}
	;
	inline int GetSumCells() const {
		return CellNum * CellNum;
	}
	;
	std::string toString();  ///<
	static std::string headerToString();
	std::string getFileID(); ///<ID-string for current run
	void setRunPara(std::string def);
};
//---------------------------------------------------------------------------
#endif
