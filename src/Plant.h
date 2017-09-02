#ifndef SRC_PLANT_H_
#define SRC_PLANT_H_

#include <vector>
#include <cmath>
#include <math.h>
#include <string>
#include <memory>

#include "Genet.h"
#include "Parameters.h"
#include "Traits.h"

static const double Pi = std::atan(1) * 4;

class Seed;
class Cell;
class Genet;

class Plant
{
private:
	Cell* cell;

	double ReproGrow(double uptake);
	double ShootGrow(double shres);
	double RootGrow(double rres);

	double mReproRamets;			// resources for ramet growth

public:
	std::unique_ptr<Traits> traits;	// PFT Traits
	std::weak_ptr<Genet> genet; 		// genet of the clonal plant

	static int staticID;
	int plantID;

	int x;
	int y;

	int age;
	double mShoot;   			// shoot mass
	double mRoot;    			// root mass
	double mRepro;    			// reproductive mass (converted to a discrete number of seeds)
	int lifetimeFecundity = 0; 	// The total accumulation of seeds

	int Ash_disc; 				// discrete above-ground ZOI area
	int Art_disc; 				// discrete below-ground ZOI area

	double Auptake; 			// uptake of above-ground resource in one time step
	double Buptake; 			// uptake below-ground resource one time step

	int isStressed;     			// counter for weeks with resource stress exposure
	bool isDead;      			// plant dead or alive?
	bool toBeRemoved;    			// Should the plant be removed from the PlantList?

	// Clonal
	std::vector< std::shared_ptr<Plant> > growingSpacerList;	// List of growing Spacer
	double spacerLengthToGrow;

	// Constructors
	Plant(const std::unique_ptr<Seed> & seed); 						// from a germinated seed
	Plant(double x, double y, const std::shared_ptr<Plant> & plant); 	// for clonal establishment
	~Plant();

	void Grow(); 									 // shoot-root resource allocation and plant growth in two layers
	void Kill();  									 // Mortality due to resource shortage or at random
	void DecomposeDead();     						 // calculate mass shrinkage of dead plants
	void WinterLoss(); 							 	 // removal of aboveground biomass in winter
	bool stressed() const;							 // return true if plant is stressed
	void weeklyReset();								 // reset competitive parameters that depend on current biomass
	double RemoveShootMass();  						 // removal of aboveground biomass by grazing
	void RemoveRootMass(const double mass_removed);  // removal of belowground biomass by root herbivory
	double comp_coef(const int layer, const int symmetry) const; // competition coefficient for a plant (for AboveComp and BelowComp)

	inline double minresA() const { return traits->mThres * Ash_disc * traits->Gmax; } // lower threshold of aboveground resource uptake
	inline double minresB() const { return traits->mThres * Art_disc * traits->Gmax; } // lower threshold of belowground resource uptake

	inline double GetMass() const { return mShoot + mRoot + mRepro; }

	inline double getHeight(double const c_height_conversion = 6.5) const {
		return pow(mShoot / (traits->LMR), 1 / 3.0) * c_height_conversion;
	}

	inline double getBiomassAtHeight(double const height, double const c_height_conversion = 6.5) const {
		return ( (pow(height, 3) * traits->LMR) / pow(c_height_conversion, 3) );
	}

	inline double Area_shoot()		{ return traits->SLA * pow(traits->LMR * mShoot, 2.0 / 3.0); } // ZOI area
	inline double Area_root()   	{ return traits->RAR * pow(mRoot, 2.0 / 3.0); }
	inline double Radius_shoot() 	{ return sqrt(traits->SLA * pow(traits->LMR * mShoot, 2.0 / 3.0) / Pi); } // ZOI radius
	inline double Radius_root() 	{ return sqrt(traits->RAR * pow(mRoot, 2.0 / 3.0) / Pi); }

	void setCell(Cell* cell);
	inline Cell* getCell() { return cell; }

	inline std::string pft() { return this->traits->PFT_ID; }

	inline void setGenet(std::weak_ptr<Genet> _genet) { this->genet = _genet; }
	inline std::weak_ptr<Genet> getGenet() { return genet; }

	void SpacerGrow();  			// spacer growth
	int ConvertReproMassToSeeds(); 	// returns number of seeds of one plant individual. Clears mRepro.
	int GetNRamets() const;  		// return number of ramets

	inline static double getPalatability(const std::shared_ptr<Plant> & p) {
		return p->mShoot * p->traits->GrazFraction();
	}

	inline static double getShootGeometry(const std::shared_ptr<Plant> & p) {
		return (p->mShoot / p->traits->LMR);
	}

	// return if plant should be removed
	inline static bool GetPlantRemove(const std::shared_ptr<Plant> & p) {
		return p->toBeRemoved;
	}

};

#endif
