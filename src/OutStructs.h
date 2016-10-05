//---------------------------------------------------------------------------

#ifndef OutStructsH
#define OutStructsH

//#include <vector>
#include <map>
#include <string>
using namespace std;
//---------------------------------------------------------------------------
//! Structure with output data for each PFT
struct SPftOut
{
	struct SPftSingle{
		//     string name;        //!<list of active functional type names
		double totmass;     //!< total biomass
		double shootmass;
		double rootmass;
		double cover;
		int Nind;        //!< population size
		int Nseeds;       //!< number of seeds
		long LDDseeds[5]; //!< number of outside the grid in distance classes

		//can be activated as needed:
		//  --be careful to update constructors and destructors as well
		//   double meanmass;  //!< mean individual mass
		//   double ssmass;   //!< sum of squares of individual masses (for calculation of std)
		//   double sdmass;   //!< standard deviation (std) of individual masses
		//   double alloc;    //!< mean allocation coefficient
		//   double stress;   //!< mean stress counter
		//   double Ashoot;   //!< shoot area [cm�]
		//   double Aroot;    //!< root area  [cm�]
		//        double repromass; //!< reproductive mass (which can be cobverted to seeds
		//   int Ash_disc;     //!< discrete shoot area (in #cells covered instead of cm�)
		//   int Art_disc;     //!< discrete root area (in #cells covered instead of cm�)
		//   int NPlants;              ///< nb non-clonal plants
		//   int NclonalPlants;       ///< nb clonal plants
		//   int NGenets;             ///< Number of living genets
		//   double MPlants;          //!< total biomass non-clonal plants
		//   double MclonalPlants;          //!< total biomass clonal plants
		//   double MeanGenetsize;          ///< mean size of genets
		//   double MeanGeneration;         ///< mean number of generations
		SPftSingle();
		~SPftSingle(){};
	};

	int week;                  //!< week of the year (1-30)
	int year;
	map<string,SPftSingle*> PFT;                 //!< list of active PFTs
	SPftOut();
	~SPftOut();
};
//---------------------------------------------------------------------------

//! Structure with output data for the whole grid
struct SGridOut
{
	int year;
	int week;                 //!< week of the year (1-30)
	int PftCount;            //!< number surviving PFTs
	double totmass;          //!< total biomass (sum over all PFTs
	int Nind;                //!< number of plants
	double shannon;          //!< shannon diversity index
	double above_mass;       //!< total above-ground mass
	double below_mass;       //!< total below-ground mass
	double aresmean;         //!< mean above-ground resource availability
	double bresmean;         //!< mean below-ground resource availability
	double cutted;           ///< cutted biomass
	double bareGround;       ///< bare ground cover on grid
	double aComp;			/// Aboveground resource draw
	double bComp;				/// Belowground resource draw

	//clonal..
	int NPlants;              ///< nb non-clonal plants
	int NclonalPlants;       ///< nb clonal plants
	int NGenets;             ///< Number of living genets
	double MPlants;          //!< total biomass non-clonal plants
	double MclonalPlants;          //!< total biomass clonal plants
	double MeanGenetsize;          ///< mean size of genets
	double MeanGeneration;         ///< mean number of generations
	SGridOut();
};

//---------------------------------------------------------------------------

#endif
