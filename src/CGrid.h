
#ifndef GridBaseH
#define GridBaseH

#include <string>
#include <vector>

#include "Cell.h"
#include "Plant.h"
#include "RunPara.h"

//! Class with all spatial algorithms where plant individuals interact in space
/*! Functions for competition and plant growth are overwritten by inherited classes
 to include different degrees of size asymmetry and different concepts of niche differentiation
 */
class CGrid
{

private:
	std::vector< std::shared_ptr<CGenet> > GenetList;
	void RametEstab(CPlant* plant);   	// establish ramets
	void Resshare();                	// share resources among connected ramets
	void EstabLott_help(CSeed* seed);

protected:
	virtual void CoverCells();			// assigns grid cells to plants - which cell is covered by which plant
	virtual void RemovePlants(); 		// removes dead plants from the grid and deletes them
	virtual void DeletePlant(CPlant* plant1);
	virtual void PlantLoop();			// loop over all plants including growth, seed dispersal and mortality
	virtual void DistribResource();		// distributes resource to each plant --> calls competition functions
	virtual void DispersSeeds(CPlant* plant);
	virtual void EstabLottery();		// lottery competition for seedling establishment
	virtual void Winter();				// calls seed mortality and mass removal of plants

	void ResetWeeklyVariables(); 		// Clears list of plants that cover each cell
	void SeedMortAge();					// Kills seeds that are too old
	void SeedMortWinter();				// Kills seeds that die over winter
	void Disturb();						// Calls grazing (Above- and Belowground), trampling, and other disturbances
	void RunCatastrophicDisturbance(); 	// Removes some percentage of total plants and seeds
	void Grazing(); 					// Aboveground grazing
	void GrazingBelGr();				// Belowground grazing
	void Cutting(double CutHeight = 0);
	void CellsInit();					// Creates the cells that make up the grid
	void SetCellResource();				// Populates the grid with resources (weekly)

public:
	std::vector<CPlant*> PlantList;    	// List of plant individuals
	CCell** CellList;    				// array of pointers to CCell
	std::vector<int> above_biomass_history;
	std::vector<int> below_biomass_history;

	CGrid();
	virtual ~CGrid();
	virtual void resetGrid();

	double GetTotalAboveMass();
	double GetTotalBelowMass();

	virtual void InitClonalSeeds(std::shared_ptr<SPftTraits> traits, const int n, double estab = 1.0);
	void DispersRamets(CPlant* plant); 	// initiate new ramets

	int GetNclonalPlants();   	// number of living clonal plants
	int GetNPlants();         	// number of living non-clonal plants
	int GetNSeeds();			// number of seeds

};

///vector of cell indices increasing in distance to grid center
static std::vector<int> ZOIBase;

//! periodic boundary conditions
void Boundary(int& xx, int& yy);

/// test for emmigration
bool Emmigrates(int& xx, int& yy);

///dispersal kernel for seeds
void getTargetCell(int& xx, int& yy, const float mean, const float sd);

//! distance between two points using Pythagoras
double Distance(const double& xx, const double& yy, const double& x = 0, const double& y = 0);

///compare two index-values in their distance to the center of grid
bool CompareIndexRel(int i1, int i2);

//---------------------------------------------------------------------------
#endif
