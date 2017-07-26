
#include "CGrid.h"
#include "CEnvir.h"
#include "RandomGenerator.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <memory>
#include <math.h>

using namespace std;

//---------------------------------------------------------------------------
CGrid::CGrid()
{
	CellsInit();

	ZOIBase = vector<int>(SRunPara::RunPara.GetSumCells(), 0);

	for (unsigned int i = 0; i < ZOIBase.size(); i++)
		ZOIBase[i] = i;

	sort(ZOIBase.begin(), ZOIBase.end(), CompareIndexRel);

	CGrid::above_biomass_history = vector<int>();
	CGrid::below_biomass_history = vector<int>();
}

//-----------------------------------------------------------------------------
/**
 Initiate grid cells.

 \note call only once or delete cell objects before;
 better reset cells (resetGrid()) to start a new environment
 */
void CGrid::CellsInit()
{
	int index;
	int SideCells = SRunPara::RunPara.GridSize;
	CellList = new CCell*[SideCells * SideCells];

	for (int x = 0; x < SideCells; x++)
	{
		for (int y = 0; y < SideCells; y++)
		{
			index = x * SideCells + y;
			CCell* cell = new CCell(x, y, CEnvir::AResMuster[index], CEnvir::BResMuster[index]);
			CellList[index] = cell;
		}
	}
}

//---------------------------------------------------------------------------
/**
 Clears the grid from Plants and resets cells.
 */
void CGrid::resetGrid()
{
	//cells...
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* cell = CellList[i];
		cell->clear();
	}

	//plants...
	for (auto p : PlantList) {
		delete p;
	}
	PlantList.clear();
	CPlant::numPlants = 0;

	GenetList.clear();
	CGenet::staticID = 0;
}
//---------------------------------------------------------------------------
/**
 * CGrid destructor
 */
CGrid::~CGrid()
{
	for (auto p : PlantList)
	{
		delete p;
	}
	PlantList.clear();

	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* cell = CellList[i];
		delete cell;
	}
	delete[] CellList;

	GenetList.clear();
}

//-----------------------------------------------------------------------------

/**
 The clonal version of PlantLoop additionally to the CGrid-version
 disperses and grows the clonal ramets
 Growth (resource allocation and vegetative growth), seed dispersal and
 mortality of plants.
 */
void CGrid::PlantLoop()
{
	for (auto p : PlantList)
	{
		if (SRunPara::RunPara.ITV == on)
			assert(p->Traits->myTraitType == SPftTraits::individualized);

		if (!p->dead)
		{
			p->Grow2();

			if (p->Traits->clonal)
			{
				DispersRamets(p); 	// ramet dispersal in every week
				p->SpacerGrow(); 	// if the plant has a growing spacer - grow it
			}

			//seed dispersal (clonal and non-clonal seeds)
			if (CEnvir::week > p->Traits->DispWeek)
			{
				DispersSeeds(p);
			}

			p->Kill();
		}
		else
		{
			p->DecomposeDead();
		}
	}
}

//-----------------------------------------------------------------------------
/**
 lognormal dispersal kernel
 Each Seed is dispersed after an log-normal dispersal kernel with mean and sd
 given by plant traits. The dispersal direction has no prevalence.
 */
void getTargetCell(int& xx, int& yy, const float mean, const float sd)
{
	double sigma = sqrt(log((sd / mean) * (sd / mean) + 1));
	double mu = log(mean) - 0.5 * sigma;
	double dist = exp(CEnvir::rng.getGaussian(mu, sigma));

	// direction uniformly distributed
	double direction = 2 * Pi * CEnvir::rng.get01();
	xx = round(xx + cos(direction) * dist);
	yy = round(yy + sin(direction) * dist);
}

//-----------------------------------------------------------------------------
/**
 Function disperses the seeds produced by a plant when seeds are to be
 released (at dispersal time - SclonalTraits::DispWeek).

 Each Seed is dispersed after an log-normal dispersal kernel
 in function getTargetCell().

 \date 2010-08-30 periodic boundary conditions are transformed
 to disperse seeds for LDD

 \return list of seeds to disperse per LDD
 */
void CGrid::DispersSeeds(CPlant* plant)
{
	int px = plant->getCell()->x;
	int py = plant->getCell()->y;
	int NSeeds = 0;
	double dist = 0;

	NSeeds = plant->GetNSeeds();

	for (int j = 0; j < NSeeds; ++j)
	{
		int x = px;
		int y = py; //remember the parent's position

		// lognormal dispersal kernel. This function changes X & Y by reference!
		getTargetCell(x, y, plant->Traits->Dist * 100, plant->Traits->Dist * 100); //mean = std (simple assumption)

		//export LDD-seeds
		if (Emmigrates(x, y))
		{
			//Calculate distance between (x,y) and grid-center
			dist = Distance(x, y, SRunPara::RunPara.GridSize / 2, SRunPara::RunPara.GridSize / 2);
			dist = dist / 100;  //convert to m
			Boundary(x, y); //recalc position for torus
		}

		CCell* cell = CellList[x * SRunPara::RunPara.GridSize + y];

		shared_ptr<CSeed> seed = make_shared<CSeed>(plant, cell);
		cell->SeedBankList.push_back(seed);
	}
}

//---------------------------------------------------------------------------
void CGrid::DispersRamets(CPlant* plant)
{
	assert(plant->Traits->clonal);

	if (plant->GetNRamets() == 1)
	{
		double distance = -1;

		while (distance <= 0)
		{
			distance = CEnvir::rng.getGaussian(
					plant->Traits->meanSpacerlength,
					plant->Traits->sdSpacerlength);
		}

		// uniformly distributed direction
		double direction = 2 * Pi * CEnvir::rng.get01();
		int x = round(plant->getCell()->x + cos(direction) * distance);
		int y = round(plant->getCell()->y + sin(direction) * distance);

		// periodic boundary condition
		Boundary(x, y);

		// save distance and direction in the plant
		CPlant *Spacer = new CPlant(x, y, plant);
		Spacer->spacerLengthToGrow = distance;
		Spacer->spacerLength = distance;
		Spacer->spacerDirection = direction;
		plant->growingSpacerList.push_back(Spacer);
	}
}

//--------------------------------------------------------------------------
/**
 This function calculates ZOI of all plants on grid.
 Each grid-cell gets a list
 of plants influencing the above- (alive and dead individuals) and
 belowground (alive plants only) layers.

 \par revision
 Let ZOI be defined by a list sorted after ascending distance to center instead
 of searching a square defined by maximum radius.
 */
void CGrid::CoverCells()
{
	for (auto plant : PlantList)
	{
		double Ashoot = plant->Area_shoot();
		plant->Ash_disc = floor(Ashoot) + 1;

		double Aroot = plant->Area_root();
		plant->Art_disc = floor(Aroot) + 1;

		double Amax = max(Ashoot, Aroot);

		for (int a = 0; a < Amax; a++)
		{
			//get current position: add plant pos with ZOIBase-pos
			int xhelp = plant->getCell()->x
					+ ZOIBase[a] / SRunPara::RunPara.GridSize
					- SRunPara::RunPara.GridSize / 2;
			int yhelp = plant->getCell()->y
					+ ZOIBase[a] % SRunPara::RunPara.GridSize
					- SRunPara::RunPara.GridSize / 2;

			Boundary(xhelp, yhelp);

			CCell* cell = CellList[xhelp * SRunPara::RunPara.GridSize + yhelp];

			// Aboveground
			if (a < Ashoot)
			{
				// dead plants still shade others
				cell->AbovePlantList.push_back(plant);
				cell->PftNIndA[plant->pft()]++;
			}

			// Belowground
			if (a < Aroot)
			{
				// dead plants do not compete for below ground resource
				if (!plant->dead)
				{
					cell->BelowPlantList.push_back(plant);
					cell->PftNIndB[plant->pft()]++;
				}
			}
		}
	}
}

/**
 * Resets all weekly variables of individual cells and plants (only in PlantList)
 */
void CGrid::ResetWeeklyVariables()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];
		cell->weeklyReset();
	}

	for (auto p : PlantList)
	{
		p->weeklyReset();
	}
}

//---------------------------------------------------------------------------
/**
 Distributes local resources according to local competition
 and shares them between connected ramets of clonal genets.
 */
void CGrid::DistribResource()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];

		cell->AboveComp();
		cell->BelowComp();
	}
	Resshare();
}

//----------------------------------------------------------------------------
/**
 Resource sharing between connected ramets
 */
void CGrid::Resshare()
{
	for (auto Genet : GenetList)
	{
		if (Genet->AllRametList.size() > 1) // A ramet cannot share with itself
		{
			auto plant = Genet->AllRametList.front(); // To ensure that the has the "resource sharing" trait
			assert(plant->Traits->clonal);
			if (plant->Traits->Resshare)
			{
				Genet->ResshareA();
				Genet->ResshareB();
			}
		}
	}
}

void CGrid::EstablishmentLottery()
{
	for (auto plant : PlantList)
	{
		if (plant->Traits->clonal && !plant->dead)
		{
			RametEstab(plant);
		}
	}

	int w = CEnvir::week;
	if ( !( (w >= 1 && w < 4) || (w > 21 && w <= 25) ) ) // establishment is only between weeks 1-4 and 21-25
	{
		return;
	}

	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];

		if (!cell->AbovePlantList.empty() || cell->SeedBankList.empty() || cell->occupied)
		{
			continue;
		}

		double sumSeedMass = cell->Germinate();

		if ( CEnvir::AreSame(sumSeedMass, 0) ) // No seeds germinated
		{
			continue;
		}

		shared_ptr<CSeed> winningSeed;
		double n = CEnvir::rng.get01() * sumSeedMass;
		for (auto itr : cell->SeedlingList)
		{
			n -= itr->mass;
			if (n <= 0)
			{
				winningSeed = itr;
				break;
			}
		}
		EstablishSeedling(winningSeed);
		cell->SeedlingList.clear();
	}
}

/**
 * Establish new genet.
 * @param seed seed which germinates.
 */
void CGrid::EstablishSeedling(shared_ptr<CSeed> seed)
{
	CPlant* p = new CPlant(seed);
	shared_ptr<CGenet> Genet = make_shared<CGenet>();
	GenetList.push_back(Genet);
	p->setGenet(Genet);
	PlantList.push_back(p);
}
//-----------------------------------------------------------------------------
/**
 Establishment of ramets. If spacer is readily grown tries to settle on
 current cell.
 If it is already occupied it finds a new goal nearby.
 If the cell is empty, the ramet establishes:
 cell information is set and the ramet is added to the global plant list,
 the genet's ramet list as well as erased
 from the spacer list of the mother plant.
 */
void CGrid::RametEstab(CPlant* plant)
{
	vector<CPlant*> rametsToKeep;

	for (auto Ramet : plant->growingSpacerList)
	{
		if (Ramet->spacerLengthToGrow > 0)
		{
			rametsToKeep.push_back(Ramet);
			continue;
		}

		int x = round(Ramet->xcoord);
		int y = round(Ramet->ycoord);
		CCell* cell = CellList[x * SRunPara::RunPara.GridSize + y];

		if (!cell->occupied)
		{
			Ramet->getGenet()->AllRametList.push_back(Ramet);
			Ramet->setCell(cell);

			PlantList.push_back(Ramet);

			if ( CEnvir::rng.get01() < (1.0 - SRunPara::RunPara.EstabRamet) )
			{
				Ramet->dead = true;
			}
		}
		else
		{
			if (CEnvir::week == CEnvir::WeeksPerYear)
			{
				delete Ramet;
			}
			else
			{
				int factorx;
				int factory;
				do
				{
					factorx = CEnvir::rng.getUniformInt(5) - 2;
					factory = CEnvir::rng.getUniformInt(5) - 2;
				} while (factorx == 0 && factory == 0);

				double dist = Distance(factorx, factory);
				double direction = acos(factorx / dist);

				x = round(Ramet->xcoord + factorx);
				y = round(Ramet->ycoord + factory);

				Boundary(x, y);

				//new position, dist and direction
				Ramet->xcoord = x;
				Ramet->ycoord = y;
				Ramet->spacerLengthToGrow = dist;
				Ramet->spacerLength = dist;
				Ramet->spacerDirection = direction;

				rametsToKeep.push_back(Ramet);
			}

		}
	}

	plant->growingSpacerList = rametsToKeep;
}

/**
 * Seed mortality
 */
void CGrid::SeedMortAge()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];

		for (auto seed : cell->SeedBankList)
		{
			if (seed->Age >= seed->Traits->Dorm)
			{
				seed->remove = true;
			}
		}
		cell->RemoveSeeds();
	}
}

void CGrid::Disturb()
{
	if (PlantList.size() == 0)
		return;

	CGrid::above_biomass_history.push_back(GetTotalAboveMass());
	CGrid::below_biomass_history.push_back(GetTotalBelowMass());

	if (CEnvir::rng.get01() < SRunPara::RunPara.GrazProb) {
		GrazingAbvGr();
	}

	if (CEnvir::rng.get01() < SRunPara::RunPara.BelGrazProb) {
		GrazingBelGr();
	}

	if (SRunPara::RunPara.NCut > 0) {
		switch (SRunPara::RunPara.NCut) {
		case 1:
			if (CEnvir::week == 22)
				Cutting(SRunPara::RunPara.CutHeight);
			break;
		case 2:
			if (CEnvir::week == 22 || CEnvir::week == 10)
				Cutting(SRunPara::RunPara.CutHeight);
			break;
		case 3:
			if (CEnvir::week == 22 || CEnvir::week == 10 || CEnvir::week == 16)
				Cutting(SRunPara::RunPara.CutHeight);
			break;
		default:
			cerr << "CGrid::Disturb() - wrong input";
			exit(1);
		}
	}
}

void CGrid::RunCatastrophicDisturbance()
{
	// Disturb plants
	for (auto p : PlantList)
	{
		if (p->dead)
			continue;

		if (CEnvir::rng.get01() < SRunPara::RunPara.CatastrophicPlantMortality)
		{
			p->dead = true;
		}
	}

	// Disturb seeds
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];

		for (auto seed : cell->SeedBankList)
		{
			if (CEnvir::rng.get01() < SRunPara::RunPara.CatastrophicSeedMortality)
			{
				seed->remove = true;
			}
		}

		cell->RemoveSeeds();
	}
}

//-----------------------------------------------------------------------------
/**
 The plants on the whole grid are grazed according to
 their relative grazing susceptibility until the given
 \ref SRunPara::PropRemove "proportion of removal"
 is reached or the grid is completely grazed.
 (Above ground mass that is ungrazable - see Schwinning and Parsons (1999):
 15,3 g/m�  * 1.6641 m� = 25.5 g)

 */
void CGrid::GrazingAbvGr()
{
	double ResidualMass = SRunPara::RunPara.MassUngraz * SRunPara::RunPara.GetSumCells() * 0.0001;

	double TotalAboveMass = GetTotalAboveMass();

	double MaxMassRemove = min(TotalAboveMass - ResidualMass, TotalAboveMass * SRunPara::RunPara.PropRemove);
	double MassRemoved = 0;

	while (MassRemoved < MaxMassRemove)
	{
		sort(PlantList.begin(), PlantList.end(), CPlant::ComparePalat);

		CPlant* plant = *PlantList.begin();

		double max = plant->mshoot * plant->Traits->GrazFac();

		random_shuffle(PlantList.begin(), PlantList.end());

		for (auto lplant : PlantList)
		{
			if (MassRemoved >= MaxMassRemove)
			{
				break;
			}

			double grazingProb = (lplant->mshoot * lplant->Traits->GrazFac()) / max;

			if (CEnvir::rng.get01() < grazingProb)
				MassRemoved += lplant->RemoveMass();
		}
	}
}

//-----------------------------------------------------------------------------
/**
 * Cutting of all plants on the patch to a uniform height.
 */
void CGrid::Cutting(double cut_height)
{
	for (auto i : PlantList)
	{
		if (i->getHeight() > cut_height)
		{
			double biomass_at_height = i->getBiomassAtHeight(cut_height);

			i->mshoot = biomass_at_height;
			i->mRepro = 0.0;
		}
	}
}

void CGrid::GrazingBelGr() {

	auto generateLivingPlants = [](vector<CPlant*> l) {
		auto it = l.begin();
		while (it != l.end()) {
			CPlant* p = *it;
			if (p->dead) {
				it = l.erase(it);
			} else {
				++it;
			}
		}
		return l;
	};

	auto sumRootMass = [](const vector<CPlant*> & l) {
		double total_root_mass = 0;
		for (auto i = l.begin(); i != l.end(); ++i) {
			CPlant* p = *i;
			total_root_mass += p->mroot;
		}
		return total_root_mass;
	};

	auto mean = [](const vector<double> & l) {
		double t = 0;
		for (auto it = l.begin(); it != l.end(); ++it)
		{
			t += *it;
		}
		return t / l.size();
	};

	assert(!CGrid::below_biomass_history.empty());

	double bt = sumRootMass(generateLivingPlants(PlantList)); // Total root biomass
	double biomass_removed = 0;
	const double alpha = 2.0;

	double fn_o;
	if (CGrid::below_biomass_history.size() > 60)
	{
		std::vector<double> rolling_mean(CGrid::below_biomass_history.end() - 60, // parameterize this...
										 CGrid::below_biomass_history.end());
		fn_o = SRunPara::RunPara.BelGrazPerc * mean(rolling_mean);
	}
	else
	{
		std::vector<double> rolling_mean(CGrid::below_biomass_history.begin(), // parameterize this...
										 CGrid::below_biomass_history.end());
		fn_o = SRunPara::RunPara.BelGrazPerc * mean(rolling_mean);
	}

	// Functional response
	if (bt-fn_o < bt*SRunPara::RunPara.BelGrazResidualPerc)
	{
		fn_o = bt - bt*SRunPara::RunPara.BelGrazResidualPerc;
	}
	double fn = fn_o;

	CEnvir::output.blwgrnd_graz_pressure_history.push_back(fn_o);

	while (ceil(biomass_removed) < fn_o)
	{
		vector<CPlant*> livingPlants = generateLivingPlants(PlantList);
		bt = sumRootMass(livingPlants);

		double bite = 0;
		for (auto i = livingPlants.begin(); i != livingPlants.end(); ++i)
		{
			CPlant* p = *i;
			bite += pow(p->mroot / bt, alpha) * fn;
		}
		bite = fn / bite;

		double leftovers = 0; // When a plant is eaten to death, this is the overshoot from the algorithm
		for (auto i = livingPlants.begin(); i != livingPlants.end(); ++i)
		{
			CPlant* p = *i;

			double biomass_to_remove = pow(p->mroot / bt, alpha) * fn * bite;

			if (biomass_to_remove > p->mroot)
			{
				leftovers += (biomass_to_remove - p->mroot);
				biomass_removed += p->mroot;
				p->mroot = 0;
				p->dead = true;
			}
			else
			{
				p->RemoveRootMass(biomass_to_remove);
				biomass_removed += biomass_to_remove;
			}
			assert(sumRootMass(livingPlants) > 0);
		}

		fn = leftovers;
	}
}

//-----------------------------------------------------------------------------
void CGrid::RemovePlants() {

	auto removePlant = [] (CPlant* p) {
		if ( CPlant::GetPlantRemove(p) )
		{
			auto Genet = p->getGenet();
			Genet->AllRametList.erase(
					remove(Genet->AllRametList.begin(), Genet->AllRametList.end(), p),
					Genet->AllRametList.end());

			p->getCell()->occupied = false;
			p->getCell()->PlantInCell = NULL;
			delete p;

			return true;
		}
		return false;
	};

	PlantList.erase(std::remove_if(PlantList.begin(), PlantList.end(), removePlant),
					PlantList.end());
}

//-----------------------------------------------------------------------------
void CGrid::Winter()
{
	RemovePlants();
	for (auto p : PlantList)
	{
		p->WinterLoss();
	}
}

//-----------------------------------------------------------------------------
void CGrid::SeedMortWinter()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];
		for (auto seed : cell->SeedBankList)
		{
			if (CEnvir::rng.get01() < SRunPara::RunPara.mort_seeds)
			{
				seed->remove = true;
			}
			else
			{
				++seed->Age;
			}
		}

		cell->RemoveSeeds();
	}
}

//-----------------------------------------------------------------------------
/**
 Set a number of randomly distributed clonal Seeds (CclonalSeed) of a specific
 trait-combination on the grid.

 \param traits   SPftTraits of the seeds to be set
 \param cltraits SclonalTraits of the seeds to be set
 \param n        number of seeds to be set
 \param estab    seed establishment (CSeed) - default is 1
 \since 2010-09-10 estab rate for seeds can be modified (default is 1.0)

 */
void CGrid::InitClonalSeeds(shared_ptr<SPftTraits> traits, const int n, double estab)
{
	for (int i = 0; i < n; ++i)
	{
		int x = CEnvir::rng.getUniformInt(SRunPara::RunPara.GridSize);
		int y = CEnvir::rng.getUniformInt(SRunPara::RunPara.GridSize);

		CCell* cell = CellList[x * SRunPara::RunPara.GridSize + y];

		shared_ptr<CSeed> seed = make_shared<CSeed>(estab, traits, cell);
		cell->SeedBankList.push_back(seed);
	}
} //end CGridclonal::clonalSeedsInit()

//---------------------------------------------------------------------------
/**
 Weekly sets cell's resources - above- and belowground variation during the
 year.

 At the moment Ampl and Bampl are set zero,
 so ressources are temporally constant.
 */
void CGrid::SetCellResource()
{
	int gweek = CEnvir::week;

	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* cell = CellList[i];
		cell->SetResource(
				max(0.0,
						(-1.0) * SRunPara::RunPara.Aampl
								* cos(
										2.0 * Pi * gweek
												/ double(CEnvir::WeeksPerYear))
								+ CEnvir::AResMuster[i]),
				max(0.0,
						SRunPara::RunPara.Bampl
								* sin(
										2.0 * Pi * gweek
												/ double(CEnvir::WeeksPerYear))
								+ CEnvir::BResMuster[i]));
	}
}     //end SetCellResource
//-----------------------------------------------------------------------------
/**
 \note does not account for possible nearer distance-values if torus is assumed
 (not a case at the moment)

 \return eucledian distance between two pairs of coordinates (xx,yy) and (x,y)

 */
double Distance(const double& xx, const double& yy, const double& x, const double& y)
{
	return sqrt((xx - x) * (xx - x) + (yy - y) * (yy - y));
}

bool CompareIndexRel(int i1, int i2)
{
	const int n = SRunPara::RunPara.GridSize;

	return Distance(i1 / n, i1 % n, n / 2, n / 2) < Distance(i2 / n, i2 % n, n / 2, n / 2);
}

//---------------------------------------------------------------------------
/**
 \param[in,out] xx  torus correction of x-coordinate
 \param[in,out] yy  torus correction of y-coordinate

 */
void Boundary(int& xx, int& yy)
{
	xx %= SRunPara::RunPara.GridSize;
	if (xx < 0)
		xx += SRunPara::RunPara.GridSize;

	yy %= SRunPara::RunPara.GridSize;
	if (yy < 0)
		yy += SRunPara::RunPara.GridSize;
}

//---------------------------------------------------------------------------
bool Emmigrates(int& xx, int& yy)
{
	if (xx < 0 || xx >= SRunPara::RunPara.GridSize)
		return true;
	if (yy < 0 || yy >= SRunPara::RunPara.GridSize)
		return true;
	return false;
}

//---------------------------------------------------------------------------
/**
 \return sum of all plants' aboveground biomass (shoot and fruit)
 */
double CGrid::GetTotalAboveMass() {
	double above_mass = 0;

	for (auto p : PlantList)
	{
		above_mass += p->mshoot + p->mRepro;
	}
	return above_mass;
}
//---------------------------------------------------------------------------
/**
 \return sum of all plants' belowground biomass (roots)
 */
double CGrid::GetTotalBelowMass() {
	double below_mass = 0;

	for (auto p : PlantList)
	{
		below_mass += p->mroot;
	}
	return below_mass;
}
//-----------------------------------------------------------------------------
/**
 \return number of clonal plants on grid
 */
int CGrid::GetNclonalPlants() //count clonal plants
{
	int NClonalPlants = 0;
	for (auto p : PlantList)
	{
		if (p->Traits->clonal && !p->dead)
		{
			NClonalPlants++;
		}
	}
	return NClonalPlants;
} //end CGridclonal::GetNclonalPlants()

//-----------------------------------------------------------------------------
/**
 * \return number of non-clonal plants on grid
 */
int CGrid::GetNPlants() //count non-clonal plants
{
	int NPlants = 0;
	for (auto p : PlantList)
	{
		//only if its a non-clonal plant
		if (!p->Traits->clonal && !p->dead)
		{
			NPlants++;
		}
	}
	return NPlants;
} //end CGridclonal::GetNPlants()

int CGrid::GetNSeeds()
{
	int seedCount = 0;
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* cell = CellList[i];
		seedCount = seedCount + int(cell->SeedBankList.size());
	}

	return seedCount;
}
