#ifndef PlantH
#define PlantH

#include <vector>
#include <cmath>
#include <math.h>
#include <string>
#include <memory>

#include "CObject.h"
#include "CGenet.h"
#include "RunPara.h"
#include "SPftTraits.h"

const double Pi = std::atan(1) * 4;

//---------------------------------------------------------------------------
//! Structure to store all PFT Parameters

class CSeed;
class CCell;
class CGenet;

class CPlant: public CObject
{
protected:
	CCell* cell;

	virtual double ReproGrow(double uptake);
	virtual double ShootGrow(double shres);
	virtual double RootGrow(double rres);

	double mReproRamets;		// resources for ramet growth
	std::shared_ptr<CGenet> genet;              // genet of the clonal plant

public:
	std::shared_ptr<SPftTraits> Traits;	// PFT Traits

	int Age;

	static int numPlants;
	int plantID;

	double xcoord;   			// location of plant's central point
	double ycoord;   			// location of plant's central point

	double mshoot;   			// shoot mass
	double mroot;    			// root mass
	double mRepro;    			// reproductive mass (which is converted to seeds)
	int lifetimeFecundity = 0; 	// The total accumulation of seeds.

	double Ash_disc; 			// discrete above-ground ZOI area [number of cells covered * area of one cell]
	double Art_disc; 			// discrete below-ground ZOI area [number of cells covered * area of one cell]

	double Auptake; 			// uptake of above-ground resource in one time step
	double Buptake; 			// uptake below-ground resource one time step

	int stress;     			// counter for weeks with resource stress exposure
	bool dead;      			// plant dead or alive?
	bool remove;    			// trampled or not - should the plant be removed?

	// Clonal
	std::vector<CPlant*> growingSpacerList;	// List of growing Spacer
	double Spacerlength;                    // real spacer length
	double Spacerdirection;                 // spacer direction
	double spacerLengthToGrow;              // length to grow
	int Generation;                         // clonal generation

	// Constructors
	CPlant(CSeed* seed); 						// from a germinated seed
	CPlant(double x, double y, CPlant* plant); 	// for clonal establishment
	virtual ~CPlant();

	virtual std::string pft();   			// say what a pft you are

	// 2nd order properties
	double Area_shoot();  	// ZOI area aboveground
	double Area_root();   	// ZOI area belowground
	double Radius_shoot(); 	// ZOI radius aboveground
	double Radius_root();  	// ZOI radius belowground

	// competition coefficient for a plant (for AboveComp and BelowComp)
	double comp_coef(const int layer, const int symmetry) const;

	virtual bool stressed();	// return true if plant is stressed

	// lower threshold of aboveground resource uptake (light stress thresh.)
	virtual double minresA() {
		return Traits->mThres * Ash_disc * Traits->Gmax;
	}

	// lower threshold of belowground resource uptake (nutrient stress thresh.)
	virtual double minresB() {
		return Traits->mThres * Art_disc * Traits->Gmax;
	}

	//! shoot-root resource allocation and plant growth in two layers
	virtual void Grow2(); 			// shoot-root resource allocation and plant growth in two layers
	void Kill();  					// Mortality due to resource shortage or at random
	void DecomposeDead();     		// calculate mass shrinkage of dead plants
	void WinterLoss(); 				// removal of above-ground biomass in winter
	double RemoveMass();  			// removal of above-ground biomass by grazing
	void RemoveRootMass(const double mass_removed); 			// removal of belowground biomass by grazing

	void setCell(CCell* cell);		// define cell for plant
	inline CCell* getCell() {
		return cell;
	}
	;

	inline double GetMass() {
		return mshoot + mroot + mRepro;
	}
	;

/// \brief get plant's height
/// \param cheight mg vegetative plant mass per cm height
/// \note fkt meight be transfered to CPlant
/// \bug since mass relates to 3D-measurements as volume,
///   height has to be correlated with mass^(1/3)
///
	virtual double getHeight(double const height_conversion_constant = 6.5) {
		return pow(mshoot / (Traits->LMR), 1 / 3.0) * height_conversion_constant;
	}
	;

	// MSC: This is derived from "CPlant::getHeight"
	virtual double getBiomassAtHeight(double const height, double const height_conversion_constant = 6.5)
	{
		return ( (pow(height, 3) * Traits->LMR) / pow(height_conversion_constant, 3) );
	}

	virtual int GetNSeeds(); 		// returns number of seeds of one plant individual

	///set genet and add ramet to its list
	void setGenet(std::shared_ptr<CGenet> genet);

	std::shared_ptr<CGenet> getGenet() {
		return genet;
	}
	;

	void SpacerGrow();                  // spacer growth
	virtual int GetNRamets();           // return number of ramets

	//-----------------------------------------------------------------------------
	// functions that are used for STL algorithms (sort + partition)

	// return if plant should be removed (necessary to apply algorithms from STL)
	bool GetPlantRemove()
	{
		return (!this->remove);
	}
	;

	// sort plant individuals descending after shoot size * palatability
	static bool ComparePalat(const CPlant* plant1, const CPlant* plant2)
	{
		return ((plant1->mshoot * plant1->Traits->GrazFac())
				> (plant2->mshoot * plant2->Traits->GrazFac()));
	}
	;

	// sort plants descending after shoot size (mass*1/LMR)
	static bool CompareShoot(const CPlant* plant1, const CPlant* plant2)
	{
		return ((plant1->mshoot / plant1->Traits->LMR)
				> (plant2->mshoot / plant2->Traits->LMR));
	}
	;

	/// sort plants descending after root mass
	static bool CompareRoot(const CPlant* plant1, const CPlant* plant2)
	{
		return ((plant1->mroot) > (plant2->mroot));
	}
	;

};

#endif
