#ifndef PlantH
#define PlantH

#include <vector>
#include <cmath>
#include <math.h>
#include <string>
#include <memory>

#include "CGenet.h"
#include "RunPara.h"
#include "SPftTraits.h"

const double Pi = std::atan(1) * 4;

class CSeed;
class CCell;
class CGenet;

class CPlant
{
private:
	CCell* cell;

	double ReproGrow(double uptake);
	double ShootGrow(double shres);
	double RootGrow(double rres);

	double mReproRamets;			// resources for ramet growth
	std::weak_ptr<CGenet> genet;  // genet of the clonal plant

public:
	std::unique_ptr<SPftTraits> Traits;	// PFT Traits

	static int numPlants;
	int plantID;

	int xcoord;   			// location of plant's central point
	int ycoord;   			// location of plant's central point

	int Age;
	double mshoot;   			// shoot mass
	double mroot;    			// root mass
	double mRepro;    			// reproductive mass (which is converted to seeds)
	int lifetimeFecundity = 0; 	// The total accumulation of seeds.

	int Ash_disc; 			// discrete above-ground ZOI area [number of cells covered * area of one cell]
	int Art_disc; 			// discrete below-ground ZOI area [number of cells covered * area of one cell]

	double Auptake; 			// uptake of above-ground resource in one time step
	double Buptake; 			// uptake below-ground resource one time step

	int stress;     			// counter for weeks with resource stress exposure
	bool dead;      			// plant dead or alive?
	bool remove;    			// Should the plant be removed from the PlantList?

	// Clonal
	std::vector< std::shared_ptr<CPlant> > growingSpacerList;	// List of growing Spacer
	double spacerLength;                    // real spacer length
	double spacerDirection;                 // spacer direction
	double spacerLengthToGrow;              // length to grow

	// Constructors
	CPlant(const std::unique_ptr<CSeed> & seed); 						// from a germinated seed
	CPlant(double x, double y, const std::shared_ptr<CPlant> & plant); 	// for clonal establishment
	~CPlant();

	void Grow2(); 									 // shoot-root resource allocation and plant growth in two layers
	void Kill();  									 // Mortality due to resource shortage or at random
	void DecomposeDead();     						 // calculate mass shrinkage of dead plants
	void WinterLoss(); 							 	 // removal of aboveground biomass in winter
	bool stressed();								 // return true if plant is stressed
	void weeklyReset();								 // reset competitive parameters that depend on current biomass
	double RemoveShootMass();  						 // removal of aboveground biomass by grazing
	void RemoveRootMass(const double& mass_removed); // removal of belowground biomass by root herbivory
	double comp_coef(const int layer, const int symmetry) const; // competition coefficient for a plant (for AboveComp and BelowComp)

	double minresA() { return Traits->mThres * Ash_disc * Traits->Gmax; } // lower threshold of aboveground resource uptake
	double minresB() { return Traits->mThres * Art_disc * Traits->Gmax; } // lower threshold of belowground resource uptake

	double GetMass() { return mshoot + mroot + mRepro; }

	double getHeight(double const& height_conversion_constant = 6.5) {
		return pow(mshoot / (Traits->LMR), 1 / 3.0) * height_conversion_constant; }

	// MSC: This is derived from "CPlant::getHeight"
	double getBiomassAtHeight(double const& height, double const& height_conversion_constant = 6.5) {
		return ( (pow(height, 3) * Traits->LMR) / pow(height_conversion_constant, 3) );
	}

	double Area_shoot()		{ return Traits->SLA * pow(Traits->LMR * mshoot, 2.0 / 3.0); } // ZOI area
	double Area_root()   	{ return Traits->RAR * pow(mroot, 2.0 / 3.0); }
	double Radius_shoot() 	{ return sqrt(Traits->SLA * pow(Traits->LMR * mshoot, 2.0 / 3.0) / Pi); } // ZOI radius
	double Radius_root() 	{ return sqrt(Traits->RAR * pow(mroot, 2.0 / 3.0) / Pi); }

	void setCell(CCell* cell);
	CCell* getCell() { return cell; }

	std::string pft() { return this->Traits->name; } // say what a pft you are

	void setGenet(std::weak_ptr<CGenet> genet);
	std::weak_ptr<CGenet> getGenet() { return genet; }

	void SpacerGrow();  // spacer growth
	int GetNSeeds(); 	// returns number of seeds of one plant individual. Clears mRepro.
	int GetNRamets();   // return number of ramets

	static double getPalatability(const std::shared_ptr<CPlant> & p) {
		return p->mshoot * p->Traits->GrazFraction();
	}

	static double getShootGeometry(const std::shared_ptr<CPlant> & p) {
		return (p->mshoot / p->Traits->LMR);
	}

	// return if plant should be removed (necessary to apply algorithms from STL)
	static bool GetPlantRemove(const std::shared_ptr<CPlant> & p)
	{
		return p->remove;
	}

};

#endif
