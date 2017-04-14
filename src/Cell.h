//---------------------------------------------------------------------------
#ifndef CellH
#define CellH
//---------------------------------------------------------------------------
#include "Plant.h"
#include "CSeed.h"

#include <vector>

//! iterator type for seed list
typedef std::vector<CSeed*>::iterator seed_iter;

//! class for cell objects (surprisingly)
class CCell {

public:
	int x;  //!< x location [grid cells]
	int y;  //!< y location [grid cells]

	double AResConc;  //!< above-ground resource availability
	double BResConc;  //!< below-ground resource availability

	double aComp_weekly;
	double bComp_weekly;

	bool occupied;  //!< is the cell occupied by any plant?

	CPlant* PlantInCell; //!< pointer to plant individual that has its central point in the cell (if any)
//   CSeed* seed;     //!< pointer to seed object

	std::vector<CPlant*> AbovePlantList; //!< List of all plant individuals that cover the cell ABOVE ground
	std::vector<CPlant*> BelowPlantList; //!< List of all plant individuals that cover the cell BELOW ground

	std::vector<CSeed*> SeedBankList; //!< List of all (ungerminated) seeds in the cell
	std::vector<CSeed*> SeedlingList; //!< List of all freshly germinated seedlings in the cell

	//! array with individual numbers of each PFT covering the cell above-ground
	/*! necessary for niche differentiation version 2
	 */
	std::map<std::string, int> PftNIndA;
	//! array with individual numbers of each PFT covering the cell below-ground
	/*! necessary for niche differentiation version 2
	 */
	std::map<std::string, int> PftNIndB;

	//! array of seedling number of each PFT
	//int *PftNSeedling;
	std::map<std::string, int> PftNSeedling;

	CCell(const unsigned int xx,
			const unsigned int yy,
			double ares = 0,
			double bres = 0);

	virtual ~CCell(); //!< Destructor

	void clear(); ///<reset
	void SetResource(double Ares, double Bres); ///<set resources
	double Germinate(); ///<on-cell germination
	void RemoveSeedlings(); ///<remove dead seedlings
	void RemoveSeeds(); ///<remove dead seeds
	void GetNPft();     //!< calculates number of individuals of each PFT
	//! competition function for size symmetric above-ground resource competition
	/*! function is overwritten if inherited class with different competitive
	 size-asymmetry of niche differentiation is used*/
	virtual void AboveComp();
	//! competition function for size symmetric below-ground resource competition
	/*! function is overwritten if inherited class with different competitive
	 size-asymmetry of niche differentiation is used*/
	virtual void BelowComp();

	///portion cell resources the plant is gaining
	double prop_res(const std::string type, const int layer, const int version) const;

	void SortTypeID();  //!< sort individuals after PFT ID

};

//---------------------------------------------------------------------------
#endif
