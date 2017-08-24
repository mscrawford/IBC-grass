
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
	{
		ZOIBase[i] = i;
	}

	sort(ZOIBase.begin(), ZOIBase.end(), CompareIndexRel);
}

//---------------------------------------------------------------------------

CGrid::~CGrid()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];
		delete cell;
	}
	delete[] CellList;

	ZOIBase.clear();

	CPlant::numPlants = 0;
	CGenet::staticID = 0;
}

//-----------------------------------------------------------------------------

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
			CCell* cell = new CCell(x, y);
			CellList[index] = cell;
		}
	}
}

//-----------------------------------------------------------------------------

void CGrid::PlantLoop()
{
	for (auto const& p : PlantList)
	{
		if (SRunPara::RunPara.ITV == on)
			assert(p->Traits->myTraitType == SPftTraits::individualized);

		if (!p->dead)
		{
			p->Grow2();

			if (p->Traits->clonal)
			{
				DispersRamets(p);
				p->SpacerGrow();
			}

//			if (CEnvir::week >= p->Traits->DispWeek)
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
 * Disperses the seeds produced by a plant when seeds are to be released.
 * Each Seed is dispersed after an log-normal dispersal kernel in function getTargetCell().
 */
void CGrid::DispersSeeds(const std::shared_ptr<CPlant> & plant)
{
	int px = plant->getCell()->x;
	int py = plant->getCell()->y;
	int n = plant->ConvertReproMassToSeeds();

	for (int i = 0; i < n; ++i)
	{
		int x = px; // remember parent's position
		int y = py;

		// lognormal dispersal kernel. This function changes X & Y by reference!
		getTargetCell(x, y,
				plant->Traits->Dist * 100,  // meters -> cm
				plant->Traits->Dist * 100); // mean = std (simple assumption)

		Torus(x, y); // recalculates position for torus

		CCell* cell = CellList[x * SRunPara::RunPara.GridSize + y];

		cell->SeedBankList.push_back(make_unique<CSeed>(plant, cell));
	}
}

//---------------------------------------------------------------------------

void CGrid::DispersRamets(const std::shared_ptr<CPlant> & p)
{
	assert(p->Traits->clonal);

	if (p->GetNRamets() == 1)
	{
		double distance = abs(CEnvir::rng.getGaussian(p->Traits->meanSpacerlength, p->Traits->sdSpacerlength));

		// uniformly distributed direction
		double direction = 2 * Pi * CEnvir::rng.get01();
		int x = round(p->getCell()->x + cos(direction) * distance);
		int y = round(p->getCell()->y + sin(direction) * distance);

		// periodic boundary condition
		Torus(x, y);

		// save distance and direction in the plant
		std::shared_ptr<CPlant> Spacer = make_shared<CPlant>(x, y, p);
		Spacer->spacerLengthToGrow = distance; // This spacer now has to grow to get to its new cell
		p->growingSpacerList.push_back(Spacer);
	}
}

//--------------------------------------------------------------------------
/**
 * This function calculates ZOI of all plants on grid.
 * Each grid-cell gets a list of plants influencing the above- (alive and dead plants) and
 * belowground (alive plants only) layers.
 */
void CGrid::CoverCells()
{
	for (auto const& plant : PlantList)
	{
		double Ashoot = plant->Area_shoot();
		plant->Ash_disc = floor(Ashoot) + 1;

		double Aroot = plant->Area_root();
		plant->Art_disc = floor(Aroot) + 1;

		double Amax = max(Ashoot, Aroot);

		for (int a = 0; a < Amax; a++)
		{
			int x = plant->getCell()->x
					+ ZOIBase[a] / SRunPara::RunPara.GridSize
					- SRunPara::RunPara.GridSize / 2;
			int y = plant->getCell()->y
					+ ZOIBase[a] % SRunPara::RunPara.GridSize
					- SRunPara::RunPara.GridSize / 2;

			Torus(x, y);

			CCell* cell = CellList[x * SRunPara::RunPara.GridSize + y];

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

//-----------------------------------------------------------------------------
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

	for (auto const& p : PlantList)
	{
		p->weeklyReset();
	}
}

//---------------------------------------------------------------------------
/**
 * Distributes local resources according to local competition
 * and shares them between connected ramets of clonal genets.
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
 * Resource sharing between connected ramets
 */
void CGrid::Resshare()
{
	for (auto const& Genet : GenetList)
	{
		if (Genet->AllRametList.size() > 1) // A ramet cannot share with itself
		{
			auto ramet = Genet->AllRametList.front().lock();
			assert(ramet);
			assert(ramet->Traits->clonal);

			if (ramet->Traits->Resshare)
			{
				Genet->ResshareA();
				Genet->ResshareB();
			}
		}
	}
}

//-----------------------------------------------------------------------------

void CGrid::EstablishmentLottery()
{
	// Explicit use of indexes rather than iterators because RametEstab
	// adds to PlantList, thereby sometimes invalidating them
	for (std::vector< shared_ptr<CPlant> >::size_type i = 0; i < PlantList.size(); ++i)
	{
		auto const& plant = PlantList[i];

		if (plant->Traits->clonal && !plant->dead)
		{
			RametEstab(plant);
		}
	}

	int w = CEnvir::week;
	if ( !( (w >= 1 && w < 5) || (w > 21 && w <= 25) ) ) // establishment is only between weeks 1-4 and 21-25
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

		double n = CEnvir::rng.get01() * sumSeedMass;
		for (auto const& itr : cell->SeedlingList)
		{
			n -= itr->mass;
			if (n <= 0)
			{
				EstablishSeedling(itr);
				break;
			}
		}
		cell->SeedlingList.clear();
	}
}

//-----------------------------------------------------------------------------

void CGrid::EstablishSeedling(const std::unique_ptr<CSeed> & seed)
{
	shared_ptr<CPlant> p = make_shared<CPlant>(seed);

	shared_ptr<CGenet> Genet = make_shared<CGenet>();
	GenetList.push_back(Genet);

	Genet->AllRametList.push_back(p);
	p->setGenet(Genet);

	PlantList.push_back(p);
}

//-----------------------------------------------------------------------------

void CGrid::RametEstab(const std::shared_ptr<CPlant> & plant)
{
	auto ramet_itr = plant->growingSpacerList.begin();

	while (ramet_itr != plant->growingSpacerList.end())
	{
		const auto& Ramet = *ramet_itr;

		if (Ramet->spacerLengthToGrow > 0) // This ramet still has to grow more, keep it.
		{
			ramet_itr++;
			continue;
		}

		CCell* cell = CellList[Ramet->xcoord * SRunPara::RunPara.GridSize + Ramet->ycoord];

		if (!cell->occupied)
		{
			if (CEnvir::rng.get01() < SRunPara::RunPara.EstabRamet)
			{
				// This ramet successfully establishes into a plant
				auto Genet = Ramet->getGenet().lock();
				assert(Genet);

				Genet->AllRametList.push_back(Ramet);
				Ramet->setCell(cell);
				PlantList.push_back(Ramet);
			}

			// Regardless of establishment success, the iterator is removed from growingSpacerList
			ramet_itr = plant->growingSpacerList.erase(ramet_itr);
		}
		else
		{
			if (CEnvir::week == CEnvir::WeeksPerYear)
			{
				// It is winter so this ramet dies over the winter
				ramet_itr = plant->growingSpacerList.erase(ramet_itr);
			}
			else
			{
				// This ramet will find a nearby cell; keep it
				int _x, _y;
				do
				{
					_x = CEnvir::rng.getUniformInt(5) - 2;
					_y = CEnvir::rng.getUniformInt(5) - 2;
				} while (_x == 0 && _y == 0);

				int x = round(Ramet->xcoord + _x);
				int y = round(Ramet->ycoord + _y);

				Torus(x, y);

				Ramet->xcoord = x;
				Ramet->ycoord = y;
				Ramet->spacerLengthToGrow = Distance(_x, _y, 0, 0);

				ramet_itr++;
			}

		}
	}
}

//-----------------------------------------------------------------------------

void CGrid::SeedMortAge()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];

		for (auto const& seed : cell->SeedBankList)
		{
			if (seed->Age >= seed->Traits->Dorm)
			{
				seed->remove = true;
			}
		}
		cell->RemoveSeeds();
	}
}

//-----------------------------------------------------------------------------

void CGrid::Disturb()
{

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

//-----------------------------------------------------------------------------

void CGrid::RunCatastrophicDisturbance()
{
	for (auto const& p : PlantList)
	{
		if (p->dead)
			continue;

		if (CEnvir::rng.get01() < SRunPara::RunPara.CatastrophicPlantMortality)
		{
			p->dead = true;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 The plants on the whole grid are grazed according to
 their relative grazing susceptibility until the given "proportion of removal"
 is reached or the grid is completely grazed.
 (Aboveground mass that is ungrazable - see Schwinning and Parsons (1999):
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
		auto p = *std::max_element(PlantList.begin(), PlantList.end(),
						[](const shared_ptr<CPlant> & a, const shared_ptr<CPlant> & b)
						{
							return CPlant::getPalatability(a) < CPlant::getPalatability(b);
						});

		double max_palatability = CPlant::getPalatability(p);

		std::shuffle( PlantList.begin(), PlantList.end(), CEnvir::rng.getRNG() );

		for (auto const& plant : PlantList)
		{
			if (MassRemoved >= MaxMassRemove)
			{
				break;
			}

			double grazProb = CPlant::getPalatability(plant) / max_palatability;

			if (CEnvir::rng.get01() < grazProb)
				MassRemoved += plant->RemoveShootMass();
		}
	}
}

//-----------------------------------------------------------------------------
/**
 * Cutting of all plants on the patch to a uniform height.
 */
void CGrid::Cutting(double cut_height)
{
	for (auto const& i : PlantList)
	{
		if (i->getHeight() > cut_height)
		{
			double biomass_at_height = i->getBiomassAtHeight(cut_height);

			i->mshoot = biomass_at_height;
			i->mRepro = 0.0;
		}
	}
}

//-----------------------------------------------------------------------------

void CGrid::GrazingBelGr()
{
	auto sumLivingRootMass = [](const vector< shared_ptr<CPlant> > & l)
	{
		double r = 0;
		for (auto const& p : l)
		{
			if ( !p->dead )
			{
				r += p->mroot;
			}
		}
		return r;
	};

	assert(!CGrid::below_biomass_history.empty());

	double bt = sumLivingRootMass(PlantList);
	const double alpha = 2.0;

	std::vector<double> rolling_mean;
	vector<double>::size_type historySize = 60; // in Weeks
	if (CGrid::below_biomass_history.size() > historySize)
	{
		rolling_mean = std::vector<double>(CGrid::below_biomass_history.end() - historySize, CGrid::below_biomass_history.end());
	}
	else
	{
		rolling_mean = std::vector<double>(CGrid::below_biomass_history.begin(), CGrid::below_biomass_history.end());
	}

	double fn_o = SRunPara::RunPara.BelGrazPerc *
					( accumulate(rolling_mean.begin(), rolling_mean.end(), 0) / rolling_mean.size() );

	// Functional response
	if (bt - fn_o < bt * SRunPara::RunPara.BelGrazResidualPerc)
	{
		fn_o = bt - bt * SRunPara::RunPara.BelGrazResidualPerc;
	}

	CEnvir::output.blwgrnd_graz_pressure_history.push_back(fn_o);

	double fn = fn_o;
	double t_br = 0; // total biomass removed
	while (ceil(t_br) < fn_o)
	{
		double bite = 0;
		for (auto const& p : PlantList)
		{
			if (!p->dead)
			{
				bite += pow(p->mroot / bt, alpha) * fn;
			}
		}
		bite = fn / bite;

		double br = 0; // Biomass removed this iteration
		double leftovers = 0; // When a plant is eaten to death, this is the overshoot from the algorithm
		for (auto const& p : PlantList)
		{
			if (p->dead)
			{
				continue;
			}

			double biomass_to_remove = pow(p->mroot / bt, alpha) * fn * bite;

			if (biomass_to_remove >= p->mroot)
			{
				leftovers = leftovers + (biomass_to_remove - p->mroot);
				br = br + p->mroot;
				p->mroot = 0;
				p->dead = true;
			}
			else
			{
				p->RemoveRootMass(biomass_to_remove);
				br = br + biomass_to_remove;
			}
		}

		t_br = t_br + br;
		bt = bt - br;
		fn = leftovers;

		assert(bt >= 0);
	}
}

//-----------------------------------------------------------------------------

void CGrid::RemovePlants()
{
	// Delete the CPlant shared_pointers
	PlantList.erase(
			std::remove_if(PlantList.begin(), PlantList.end(),
					[] (const shared_ptr<CPlant> & p)
					{
						if ( CPlant::GetPlantRemove(p) )
						{
							p->getCell()->occupied = false;
							return true;
						}
						return false;
					}),
					PlantList.end());


	// Clear out dead pointers in the AllRametsList held by each genet
	auto eraseExpiredRamet = []( const std::weak_ptr<CPlant> & r )
	{
		if (r.expired())
		{
			return true;
		}
		return false;
	};

	std::for_each(GenetList.begin(), GenetList.end(),
			[eraseExpiredRamet] (std::shared_ptr<CGenet> const& g)
			{
				auto& r = g->AllRametList;
				r.erase(std::remove_if(r.begin(), r.end(), eraseExpiredRamet), r.end());
			});

	// Delete any empty genets
	GenetList.erase(
			std::remove_if(GenetList.begin(), GenetList.end(),
					[] (const shared_ptr<CGenet> & g)
					{
						if (g->AllRametList.empty())
						{
							return true;
						}
						return false;
					}),
					GenetList.end());
}

//-----------------------------------------------------------------------------

void CGrid::Winter()
{
	RemovePlants();
	for (auto const& p : PlantList)
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
		for (auto const& seed : cell->SeedBankList)
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
 * Set a number of randomly distributed clonal Seeds of a specific trait-combination on the grid.
 */
void CGrid::InitClonalSeeds(string PFT_ID, const int n, const double estab)
{
	for (int i = 0; i < n; ++i)
	{
		int x = CEnvir::rng.getUniformInt(SRunPara::RunPara.GridSize);
		int y = CEnvir::rng.getUniformInt(SRunPara::RunPara.GridSize);

		CCell* cell = CellList[x * SRunPara::RunPara.GridSize + y];

		cell->SeedBankList.push_back(make_unique<CSeed>(PFT_ID, cell, estab));
	}
}

//---------------------------------------------------------------------------

/**
 * Weekly sets cell's resources. Above- and belowground variation during the year.
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
								+ SRunPara::RunPara.meanARes),
				max(0.0,
						SRunPara::RunPara.Bampl
								* sin(
										2.0 * Pi * gweek
												/ double(CEnvir::WeeksPerYear))
								+ SRunPara::RunPara.meanBRes));
	}
}

//-----------------------------------------------------------------------------

double Distance(const double xx, const double yy, const double x, const double y)
{
	return sqrt((xx - x) * (xx - x) + (yy - y) * (yy - y));
}

//-----------------------------------------------------------------------------

bool CompareIndexRel(const int i1, const int i2)
{
	const int n = SRunPara::RunPara.GridSize;

	return Distance(i1 / n, i1 % n, n / 2, n / 2) < Distance(i2 / n, i2 % n, n / 2, n / 2);
}

//---------------------------------------------------------------------------
/*
 * Accounts for the gridspace being torus
 */
void Torus(int& xx, int& yy)
{
	xx %= SRunPara::RunPara.GridSize;
	if (xx < 0)
	{
		xx += SRunPara::RunPara.GridSize;
	}

	yy %= SRunPara::RunPara.GridSize;
	if (yy < 0)
	{
		yy += SRunPara::RunPara.GridSize;
	}
}

//---------------------------------------------------------------------------

double CGrid::GetTotalAboveMass()
{
	double above_mass = 0;
	for (auto const& p : PlantList)
	{
		above_mass += p->mshoot + p->mRepro;
	}
	return above_mass;
}

//---------------------------------------------------------------------------

double CGrid::GetTotalBelowMass()
{
	double below_mass = 0;
	for (auto const& p : PlantList)
	{
		below_mass += p->mroot;
	}
	return below_mass;
}

//-----------------------------------------------------------------------------

int CGrid::GetNclonalPlants()
{
	int NClonalPlants = 0;
	for (auto const& p : PlantList)
	{
		if (p->Traits->clonal && !p->dead)
		{
			NClonalPlants++;
		}
	}
	return NClonalPlants;
}

//-----------------------------------------------------------------------------

int CGrid::GetNPlants() //count non-clonal plants
{
	int NPlants = 0;
	for (auto const& p : PlantList)
	{
		//only if its a non-clonal plant
		if (!p->Traits->clonal && !p->dead)
		{
			NPlants++;
		}
	}
	return NPlants;
}

//-----------------------------------------------------------------------------

int CGrid::GetNSeeds()
{
	int seedCount = 0;
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];
		seedCount = seedCount + int(cell->SeedBankList.size());
	}

	return seedCount;
}
