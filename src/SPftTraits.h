
#ifndef SPFTTRAITS_H_
#define SPFTTRAITS_H_

#include <map>
#include <string>
#include <vector>
#include <memory>

/**
 * Structure to store all PFT Parameters
 */
class SPftTraits {

public:

	enum traitType
	{
		species,
		individualized
	};

//general
	static std::map< std::string, std::unique_ptr<SPftTraits> > pftTraitTemplates; // links of PFTs (SPftTraits) used
	static std::vector< std::string > pftInsertionOrder;

	traitType myTraitType; 	// The default trait set is a species -- only after being varied is it individualized.
	std::string name;    	// name of functional type

//morphology
	double LMR;     // leaf mass ratio (LMR) (leaf mass per shoot mass) [0;1] 1 -> only leafs, 0 -> only stem
	double SLA;     // specific leaf area (SLA) equal to cshoot in the model description (leaf area per leaf mass)
	double RAR;     // root area ratio (root area per root mass) equal to croot in the model description
	double m0;      // initial masses of root and shoot
	double MaxMass; // maximum individual mass

//seed reproduction
	double AllocSeed;  // constant proportion that is allocated to seeds between FlowerWeek and DispWeek
	double SeedMass;   // Seed mass (mass of ONE seed)
	double Dist;       // mean dispersal distance (and standard deviation of the dispersal kernel
	int Dorm;          // maximum seed longevity
	double pEstab;     // annual probability of establishment

//competitive strength
	double Gmax;
	double palat;   // Palatability -> susceptibility towards grazing

	// above-ground competitive ability
	inline double CompPowerA() const {
		return 1.0 / LMR * Gmax;
	}
	;

	// below-ground competitive ability
	inline double CompPowerB() const {
		return Gmax;
	}
	;

//grazing response
	// fraction of above-ground biomass removal if a plant is grazed
	inline double GrazFraction() const {
		return 1.0 / LMR * palat;
	}
	;

//stress tolerance
	double memory; // equal to surv_max in the model description -> maximal time of survival under stress
	double mThres; // Fraction of maximum uptake that is considered as resource stress
	double growth; // concersion rate  resource -> biomass [mass/resource unit]

//phenology
	int FlowerWeek; // week of start of seed oroduction
	int DispWeek;   // week of seed dispersal (and end of seed production)

//clonality...
	bool clonal;   				// is this plant clonal at all?
	double meanSpacerlength;  	// mean spacer length [cm]
	double sdSpacerlength;    	// sd spacer length [cm]
	double AllocSpacer; 		// proportion of ressource invested in ramet growth -> for annual and biannual species this should not be=AllocSeed, because this is then way to high
	bool Resshare;              // do established ramets share their resources?
	double mSpacer;  			// resources for 1 cm spacer (default=70)

//functions..
	SPftTraits();
	SPftTraits(const SPftTraits& s);

	void varyTraits();
	static void ReadPFTDef(const std::string& file);
	static std::unique_ptr<SPftTraits> createTraitSetFromPftType(std::string type);
	static std::unique_ptr<SPftTraits> copyTraitSet(const std::unique_ptr<SPftTraits> & t);

};

#endif /* SPFTTRAITS_H_ */
