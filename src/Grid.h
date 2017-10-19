
#ifndef SRC_GRID_H_
#define SRC_GRID_H_

#include <string>
#include <vector>
#include <memory>

#include "Cell.h"
#include "Plant.h"
#include "Parameters.h"

//! Class with all spatial algorithms where plant individuals interact in space
/*! Functions for competition and plant growth are overwritten by inherited classes
 to include different degrees of size asymmetry and different concepts of niche differentiation
 */
class Grid
{

private:
    std::vector< std::shared_ptr<Genet> > GenetList;
    void establishRamets(const std::shared_ptr<Plant> plant); 	// establish ramets
    void shareResources();                						// share resources among connected ramets
    void establishSeedlings(const std::unique_ptr<Seed> & seed);

protected:
    void CoverCells();					// assigns grid cells to plants - which cell is covered by which plant
    void RemovePlants(); 				// removes dead plants from the grid and deletes them
    void PlantLoop();					// loop over all plants including growth, seed dispersal and mortality
    void DistributeResource();			// distributes resource to each plant --> calls competition functions
    void DisperseSeeds(const std::shared_ptr<Plant> & plant);
    void DisperseRamets(const std::shared_ptr<Plant> & plant); // initiate new ramets
    void EstablishmentLottery();		// lottery competition for seedling establishment
    void Winter();						// calls seed mortality and mass removal of plants
    void ResetWeeklyVariables(); 		// Clears list of plants that cover each cell
    void SeedMortalityAge();			// Kills seeds that are too old
    void SeedMortalityWinter();			// Kills seeds that die over winter
    void Disturb();						// Calls grazing (Above- and Belowground), trampling, and other disturbances
    void RunCatastrophicDisturbance(); 	// Removes some percentage of total plants and seeds
    void GrazingAbvGr(); 				// Aboveground grazing
    void GrazingBelGr();				// Belowground grazing
    void Cutting(double CutHeight = 0);
    void CellsInit();					// Creates the cells that make up the grid
    void SetCellResources();			// Populates the grid with resources (weekly)

public:
    Cell** CellList;    								// array of pointers to CCell
    std::vector< std::shared_ptr<Plant> > PlantList;    // plant individuals
    std::vector<int> below_biomass_history;

    Grid();
    ~Grid();

    double GetTotalAboveMass();
    double GetTotalBelowMass();
    double GetTotalAboveComp();
    double GetTotalBelowComp();

    void InitSeeds(std::string PFT_ID, const int n, const double estab);

    int GetNclonalPlants();   	// number of living clonal plants
    int GetNPlants();         	// number of living non-clonal plants
    int GetNSeeds();			// number of seeds

};

// vector of cell indices increasing in distance to grid center
static std::vector<int> ZOIBase;

// periodic boundary conditions
void Torus(int& xx, int& yy); // Change by reference

// dispersal kernel for seeds
void getTargetCell(int& xx, int& yy, const float mean, const float sd); // Change by reference

// Euclidean distance between two points
double Distance(const double xx, const double yy, const double x, const double y);

// compare two index-values in their distance to the center of grid
bool CompareIndexRel(const int a, const int b);

#endif
