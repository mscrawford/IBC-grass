
#ifndef GridBaseH
#define GridBaseH

#include <string>
#include <vector>

#include "Cell.h"
#include "Plant.h"
#include "RunPara.h"

//!iterator type for plant list
typedef std::vector<CPlant*>::iterator plant_iter;

//!size type for plant list
typedef std::vector<CPlant*>::size_type plant_size;

//! Class with all spatial algorithms where plant individuals interact in space
/*! Functions for competition and plant growth are overwritten by inherited classes
 to include different degrees of size asymmetry and different concepts of niche differentiation
 */
class CGrid
{

	///List of Genets on Grid
	std::vector<CGenet*> GenetList;
	//clonal..
	void RametEstab(CPlant* plant);   ///< establish ramets
	void Resshare();                ///< share ressources among connected ramets
	void EstabLott_help(CSeed* seed);

protected:
	//! assigns grid cells to plants - which cell is covered by which plant
	virtual void CoverCells();
	//! removes dead plants from the grid and deletes them
	virtual void RemovePlants();
	virtual void DeletePlant(CPlant* plant1); //!< delete plant object
	//!  loop over all plants including growth, seed dispersal and mortality
	virtual void PlantLoop();
	//! distributes resource to each plant --> calls competition functions
	virtual void DistribResource();

	virtual int DispersSeeds(CPlant* plant);        //!<  seed dispersal
	//!  lottery competition for seedling establishment
	virtual void EstabLottery();
	//! calls seed mortality and mass removal of plants
	virtual void Winter();

	void ResetWeeklyVariables(); //!< clears list of plants that cover each cell
	void SeedMortAge();    //!<  seed mortality in winter due to ageing of seeds
	void SeedMortWinter();      //!< random seed mortality in winter
	//! disturbance --> calls grazing and gap formation functions
	bool Disturb();
	/// simulates aboveground herbivory
	void catastrophicDisturbance();
	void Grazing();
	/// simulates belowground grazing
	void GrazingBelGr();
	void Trampling();           //!< gap formation
	void Cutting(double CutHeight = 0);         //!< cutting of all plants to equal aboveground mass
	void CellsInit();           //!< initalization of cells
	void SetCellResource();     ///< set amount of resources the cells serve

public:
	std::vector<CPlant*> PlantList;    //!< List of plant individuals
	CCell** CellList;    //!<array of pointers to CCell

	CGrid(); //!< Konstruktor
	CGrid(std::string id); //!< Konstruktor for loading a file saved grid
	virtual ~CGrid(); //!< Destruktor
	virtual void resetGrid();

	double GetTotalAboveMass();
	double GetTotalBelowMass();

	//! initalization of clonal seeds
	virtual void InitClonalSeeds(SPftTraits* traits, const int n, double estab = 1.0);
	void DispersRamets(CPlant* plant); ///<initiate new ramets
	//service functions...
	int GetNclonalPlants();   ///< number of living clonal plants
	int GetNPlants();         ///< number of living non-clonal plants
	int GetNSeeds();			// number of seeds
	int GetNMotherPlants();   ///< number of living genets
	int GetCoveredCells();    ///< number of cells covered
	double GetNGeneration();  ///< number of Generations

};

///vector of cell indices increasing in distance to grid center
static std::vector<int> ZOIBase;
//! periodic boundary conditions
void Boundary(int& xx, int& yy);
/// test for emmigration
bool Emmigrates(int& xx, int& yy);
///dispersal kernel for seeds
void getTargetCell(int& xx, int& yy, const float mean, const float sd, double cellscale = 0);
//! distance between two points using Pythagoras
double Distance(const double& xx, const double& yy, const double& x = 0, const double& y = 0);
///compare two index-values in their distance to the center of grid
bool CompareIndexRel(int i1, int i2);

//---------------------------------------------------------------------------
#endif
