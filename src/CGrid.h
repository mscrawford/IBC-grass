
#ifndef GridBaseH
#define GridBaseH

#include <string>
#include <vector>
#include <memory>

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
	void RametEstab(const std::shared_ptr<CPlant> & plant); // establish ramets
	void Resshare();                						// share resources among connected ramets
	void EstablishSeedling(const std::unique_ptr<CSeed> & seed);

protected:
	void CoverCells();					// assigns grid cells to plants - which cell is covered by which plant
	void RemovePlants(); 				// removes dead plants from the grid and deletes them
	void PlantLoop();					// loop over all plants including growth, seed dispersal and mortality
	void DistribResource();				// distributes resource to each plant --> calls competition functions
	void DispersSeeds(const std::shared_ptr<CPlant> & plant);
	void DispersRamets(const std::shared_ptr<CPlant> & plant); 	// initiate new ramets
	void EstablishmentLottery();		// lottery competition for seedling establishment
	void Winter();						// calls seed mortality and mass removal of plants
	void ResetWeeklyVariables(); 		// Clears list of plants that cover each cell
	void SeedMortAge();					// Kills seeds that are too old
	void SeedMortWinter();				// Kills seeds that die over winter
	void Disturb();						// Calls grazing (Above- and Belowground), trampling, and other disturbances
	void RunCatastrophicDisturbance(); 	// Removes some percentage of total plants and seeds
	void GrazingAbvGr(); 				// Aboveground grazing
	void GrazingBelGr();				// Belowground grazing
	void Cutting(double CutHeight = 0);
	void CellsInit();					// Creates the cells that make up the grid
	void SetCellResource();				// Populates the grid with resources (weekly)

public:
	CCell** CellList;    								// array of pointers to CCell
	std::vector< std::shared_ptr<CPlant> > PlantList;   // List of plant individuals
	std::vector<int> above_biomass_history;
	std::vector<int> below_biomass_history;

	CGrid();
	~CGrid();

	double GetTotalAboveMass();
	double GetTotalBelowMass();

	void InitClonalSeeds(std::string PFT_ID, const int n, const double estab);

	int GetNclonalPlants();   	// number of living clonal plants
	int GetNPlants();         	// number of living non-clonal plants
	int GetNSeeds();			// number of seeds

};

///vector of cell indices increasing in distance to grid center
static std::vector<int> ZOIBase;

// periodic boundary conditions
void Torus(int& xx, int& yy);

// dispersal kernel for seeds
void getTargetCell(int& xx, int& yy, const float mean, const float sd);

// Euclidean distance between two points
double Distance(const double xx, const double yy, const double x, const double y);

// compare two index-values in their distance to the center of grid
bool CompareIndexRel(const int i1, const int i2);

#endif
