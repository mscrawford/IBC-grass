/**\file
 \brief functions of class CGrid
 */

#include "CGrid.h"
#include "CEnvir.h"

#include <algorithm>
#include <iostream>
#include <cassert>

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
	int SideCells = SRunPara::RunPara.CellNum;
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
} //end CellsInit

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
	for (plant_iter p = PlantList.begin(); p < PlantList.end(); ++p) {
		delete *p;
	}
	PlantList.clear();
	CPlant::numPlants = 0;

	//Genet list..
	for (unsigned int i = 0; i < GenetList.size(); i++)
		delete GenetList[i];

	GenetList.clear();
	CGenet::staticID = 0;
}
//---------------------------------------------------------------------------
/**
 * CGrid destructor
 */
CGrid::~CGrid() {
	for (plant_iter iplant = PlantList.begin(); iplant < PlantList.end();
			++iplant) {
		CPlant* plant = *iplant;
		delete plant;
	};
	PlantList.clear();

	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* cell = CellList[i];
		delete cell;
	}
	delete[] CellList;

	for (unsigned int i = 0; i < GenetList.size(); i++)
		delete GenetList[i];
	GenetList.clear();
	CGenet::staticID = 0;

} //end ~CGrid

//-----------------------------------------------------------------------------

/**
 The clonal version of PlantLoop additionally to the CGrid-version
 disperses and grows the clonal ramets
 Growth (resource allocation and vegetative growth), seed dispersal and
 mortality of plants.
 */
void CGrid::PlantLoop()
{
	for (plant_iter it = PlantList.begin(); it < PlantList.end(); ++it)
	{
		CPlant* p = *it;

		if (SRunPara::RunPara.ITV == on)
			assert(p->Traits->myTraitType == SPftTraits::individualized);

		if (!p->dead)
		{
			p->Grow2();

			if (p->Traits->clonal)
			{
				DispersRamets(p); // ramet dispersal in every week
				p->SpacerGrow(); // if the plant has a growing spacer - grow it
			}

			//seed dispersal (clonal and non-clonal seeds)
			if (CEnvir::week > p->Traits->DispWeek)
			{
				DispersSeeds(p);
			}

			p->Kill();
		}
		p->DecomposeDead();
	}
} //plant loop

//-----------------------------------------------------------------------------
/**
 lognormal dispersal kernel
 Each Seed is dispersed after an log-normal dispersal kernel with mean and sd
 given by plant traits. The dispersal direction has no prevalence.

 \code
 double mean=plant->Traits->Dist*100;   //m -> cm
 double sd=plant->Traits->Dist*100;     //mean = std (simple assumption)

 double sigma=sqrt(log((sd/mean)*(sd/mean)+1));
 double mu=log(mean)-0.5*sigma;
 dist=exp(CEnvir::RandNumGen.normal(mu,sigma));

 double direction=2*Pi*CEnvir::rand01();
 int x=CEnvir::Round(plant->getCell()->x+cos(direction)*dist*CmToCell);
 int y=CEnvir::Round(plant->getCell()->y+sin(direction)*dist*CmToCell);
 \endcode
 */
void getTargetCell(int& xx, int& yy, const float mean, const float sd, double cellscale)
{
	double sigma = sqrt(log((sd / mean) * (sd / mean) + 1));
	double mu = log(mean) - 0.5 * sigma;
	double dist = exp(CEnvir::normrand(mu, sigma)); //RandNumGen.normal
	if (cellscale == 0)
		cellscale = SRunPara::RunPara.CellScale();
	double CmToCell = 1.0 / cellscale;

	//direction uniformly distributed
	double direction = 2 * Pi * CEnvir::rand01(); //rnumber;
	xx = round(xx + cos(direction) * dist * CmToCell);
	yy = round(yy + sin(direction) * dist * CmToCell);
}

//-----------------------------------------------------------------------------
/**
 Function disperses the seeds produced by a plant when seeds are to be
 released (at dispersal time - SclonalTraits::DispWeek).

 Each Seed is dispersed after an log-normal dispersal kernel
 in function getTargetCell().

 \date 2010-08-30 periodic boundary conditions are transformed
 to dispers seeds for LDD

 \return list of seeds to dispers per LDD
 */
int CGrid::DispersSeeds(CPlant* plant)
{
	int px = plant->getCell()->x, py = plant->getCell()->y;
	int NSeeds = 0;
	double dist = 0;
	int nb_LDDseeds = 0;
	int SideCells = SRunPara::RunPara.CellNum;

	NSeeds = plant->GetNSeeds();

	for (int j = 0; j < NSeeds; ++j) {
		int x = px, y = py; //remember the parent's position
		//negative exponential dispersal kernel
		//dist=RandNumGen->exponential(1/(plant->DispDist*100));   //m -> cm

		//lognormal dispersal kernel
		getTargetCell(x, y, plant->Traits->Dist * 100,        //m -> cm
		plant->Traits->Dist * 100);       //mean = std (simple assumption)
		//export LDD-seeds
		if (Emmigrates(x, y)) {
			nb_LDDseeds++;
			//Calculate distance between (x,y) and grid-center
			dist = Distance(x, y, SRunPara::RunPara.GridSize / 2,
					SRunPara::RunPara.GridSize / 2);
			dist = dist / 100;  //convert to m

			if (!SRunPara::RunPara.torus)
				continue;  //skip seed for absorbing boundaries
			else
				Boundary(x, y); //recalc position
		}

		CCell* cell = CellList[x * SideCells + y];
		new CSeed(plant, cell);
	} //for NSeeds
	return nb_LDDseeds;
} //end DispersSeeds

//---------------------------------------------------------------------------
void CGrid::DispersRamets(CPlant* plant) {
	double CmToCell = 1.0 / SRunPara::RunPara.CellScale();

	if (plant->Traits->clonal) //type() == "CclonalPlant")//only if its a clonal plant
	{
		//dispersal
		for (int j = 0; j < plant->GetNRamets(); ++j) {
			double dist = 0, direction; //, rdist;
			double mean, sd; //parameters for lognormal dispersal kernel

			//normal distribution for spacer length
			mean = plant->Traits->meanSpacerlength;   //cm
			sd = plant->Traits->sdSpacerlength; //mean = std (simple assumption)

			while (dist <= 0)
				dist = CEnvir::normrand(mean, sd);
			//direction uniformly distributed
			direction = 2 * Pi * CEnvir::rand01();
			int x = round(plant->getCell()->x + cos(direction) * dist * CmToCell);
			int y = round(plant->getCell()->y + sin(direction) * dist * CmToCell);

			Boundary(x, y);   //periodic boundary condition

			// save dist and direction in the plant
			CPlant *Spacer = new CPlant(x / CmToCell, y / CmToCell, plant);
			Spacer->SpacerlengthToGrow = dist;
			Spacer->Spacerlength = dist;
			Spacer->Spacerdirection = direction;
			plant->growingSpacerList.push_back(Spacer);
		}   //end for NRamets
	}  //end for if it a clonal plant
}  //end CGridclonal::DispersRamets()

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
void CGrid::CoverCells() {

	int index;
	int xhelp, yhelp;

	double CellScale = SRunPara::RunPara.CellScale();
	double CellArea = CellScale * CellScale;
	//loop for all plants
	for (plant_iter iplant = PlantList.begin(); iplant < PlantList.end();
			++iplant) {
		CPlant* plant = *iplant;

		double Ashoot = plant->Area_shoot() / CellArea;
		plant->Ash_disc = floor(plant->Area_shoot()) + 1;

		double Aroot = plant->Area_root() / CellArea;
		plant->Art_disc = floor(plant->Area_root()) + 1;

		double Amax = max(Ashoot, Aroot);

		for (int a = 0; a < Amax; a++) {
			//get current position: add plant pos with ZOIBase-pos
			xhelp = plant->getCell()->x + ZOIBase[a] / SRunPara::RunPara.CellNum
					- SRunPara::RunPara.CellNum / 2; //x;
			yhelp = plant->getCell()->y + ZOIBase[a] % SRunPara::RunPara.CellNum
					- SRunPara::RunPara.CellNum / 2; //y;
					/// \todo change to absorbing bound for upscaling
			Boundary(xhelp, yhelp);
			index = xhelp * SRunPara::RunPara.CellNum + yhelp;
			CCell* cell = CellList[index];

			//Aboveground****************************************************
			if (a < Ashoot) {
				//dead plants still shade others
				cell->AbovePlantList.push_back(plant);
				cell->PftNIndA[plant->pft()]++;
			}        //for <ashoot //Ende if

			//Belowground*****************************************************
			if (a < Aroot) {
				//dead plants do not compete for below ground resource
				if (!plant->dead) {
					cell->BelowPlantList.push_back(plant);
					cell->PftNIndB[plant->pft()]++;
				}
			}        //if <Aroot //Ende if
		}        // for <Amax
	}        //end of plant loop
}

//---------------------------------------------------------------------------
/**
 Resets all weekly variables of individual cells and plants (only in PlantList)

 Former called FreeCells()
 */
void CGrid::ResetWeeklyVariables()
{
	//loop for all cells
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];
		cell->AbovePlantList.clear();
		cell->BelowPlantList.clear();
		cell->RemoveSeedlings(); //remove seedlings and pft-counter
	}

	//loop for all plants
	for (plant_iter it = PlantList.begin(); it < PlantList.end(); ++it)
	{
		CPlant* p = *it;

		//reset weekly variables
		p->Auptake = 0;
		p->Buptake = 0;
		p->Ash_disc = 0;
		p->Art_disc = 0;
	}
}

//---------------------------------------------------------------------------
/**
 Distributes local resources according to local competition
 and shares them between connected ramets of clonal genets.
 */
void CGrid::DistribResource()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) { //loop for all cells
		CCell* cell = CellList[i];

		cell->AboveComp();
		cell->BelowComp();
	} //for all cells
	Resshare();  // resource sharing between connected ramets
}  //end distribResource

//----------------------------------------------------------------------------
/**
 Resource sharing between connected ramets on grid.
 */
void CGrid::Resshare() // resource sharing
{
	for (unsigned int i = 0; i < GenetList.size(); i++)
	{
		CGenet* Genet = GenetList[i];
		if (Genet->AllRametList.size() > 1) //!Genet->AllRametList.empty())
				{
			CPlant* plant = Genet->AllRametList.front();
			if (plant->Traits->clonal //type()=="CclonalPlant"
			&& plant->Traits->Resshare == true) { //only betwen connected ramets
				Genet->ResshareA();  //above ground
				Genet->ResshareB();  // below ground
			} //if Resshare true
		} //if genet>0
	} //for...
} //end CGridclonal::Resshare()

//-----------------------------------------------------------------------------
/**
 For each grid cell seeds from seed bank germinate and establish.
 Seedlings that do not establish will die.

 \note this function is \b completely reimplemented by CGridclonal

 \par revision
 I tried to make function faster skipping the seed-pft-collecting
 cumulative lists, but that possibly leads to pseudo-endless loops if
 seedling-number per cell is too high

 For each plant ramets establish and
 for each grid cell seeds from seed bank germinate and establish.
 Seedlings that do not establish will die.

 -# ramets establish if goal is reached (RametEstab())
 -# seeds establish from the seed bank during
 germination time (weeks 1-3, 22-25).
 -# seedlings that fail to establish will die

 \todo find a faster algorithm for choosing the winning seedling
 */
void CGrid::EstabLottery() {
	//Ramet establishment for all Plants
	for (int i = 0; i < PlantList.size(); i++) //for all Plants (before Rametestab)
			{
		CPlant* plant = PlantList[i];
		if (plant->Traits->clonal && !plant->dead) {
			RametEstab(plant);
		}
	}

	//for Seeds (for clonal and non-klonal plants)
	map<string, double> PftEstabProb;
	map<string, int> PftNSeedling;

	int gweek = CEnvir::week;
	if ((gweek >= 1 && gweek < 4) || (gweek > 21 && gweek <= 25)) // establishment only between week 1-4 and 21-25
	{
		double sum = 0;
		for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) // loop for all cells
		{
			CCell* cell = CellList[i];
			if (cell->AbovePlantList.empty() && !cell->SeedBankList.empty() && !cell->occupied) // germination if cell is uncovered
			{
				sum = cell->Germinate();

				if (sum > 0)  // if seeds germinated...
				{
					typedef map<string, int> mapType;
					for (mapType::const_iterator it = cell->PftNSeedling.begin();
							it != cell->PftNSeedling.end();
							++it) //for all PFTs
					{
						string pft = it->first;
						map<string, int>::iterator itr = cell->PftNSeedling.find(pft);
						if (itr != cell->PftNSeedling.end())
						{
							PftEstabProb[pft] = (double) itr->second * SPftTraits::getPftLink(pft)->SeedMass;
							PftNSeedling[pft] = itr->second; //(cell->PftNSeedling[pft]);
						}
					}

					// chose seedling that establishes at random
					double rnum = CEnvir::rand01() * sum; //random double between 0 and sum of seed mass

					for (mapType::const_iterator it =
							cell->PftNSeedling.begin();
							it != cell->PftNSeedling.end() && (!cell->occupied);
							++it) // for each type germinated
					{
						std::string pft = it->first;
						if (rnum < PftEstabProb[pft]) //random number < current types' estab Probability?
						{
							random_shuffle(cell->SeedlingList.begin(),
									partition(cell->SeedlingList.begin(),
											cell->SeedlingList.end(),
											bind2nd(mem_fun(&CSeed::SeedOfType), pft)));

							CSeed* seed = cell->SeedlingList.front();
							EstabLott_help(seed);
							cell->PftNSeedling[pft]--;

							continue; // if established, go to next cell

						}
						else
						{   //if not: subtrahiere   PftCumEstabProb[pft]
							rnum -= PftEstabProb.find(pft)->second;
						}   //und gehe zum n�chsten Typ
					}   //for all types in list
					cell->RemoveSeedlings();
				}   //if seedlings in cell
			}   //seeds in cell
		}   //for all cells
	}   //if between week 1-4 and 21-25
	PftEstabProb.clear();
	PftNSeedling.clear();
}   //end CGridclonal::EstabLottery()

/**
 * Establish new genet.
 * @param seed seed which germinates.
 */
void CGrid::EstabLott_help(CSeed* seed)
{
	CPlant* plant;
	CPlant* tempPlant = new CPlant(seed);

	CGenet *Genet = new CGenet();
	GenetList.push_back(Genet);

	tempPlant->setGenet(Genet);
	plant = tempPlant;
	PlantList.push_back(plant);
	tempPlant = NULL;
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
void CGrid::RametEstab(CPlant* plant) {
	//  using CEnvir::rand01;
	int RametListSize = plant->growingSpacerList.size();

	if (RametListSize == 0)
		return;

	for (int f = 0; f < RametListSize; f++)
	{
		CPlant* Ramet = plant->growingSpacerList[f];
		if (Ramet->SpacerlengthToGrow <= 0)
		{
			int x = round(Ramet->xcoord / SRunPara::RunPara.CellScale());
			int y = round(Ramet->ycoord / SRunPara::RunPara.CellScale());

			//find the number of the cell in the List with x,y
			CCell* cell = CellList[x * SRunPara::RunPara.CellNum + y];

			if (!cell->occupied) //establish if cell is not central point of another plant
			{
				Ramet->getGenet()->AllRametList.push_back(Ramet);
				Ramet->setCell(cell);

				PlantList.push_back(Ramet);

				//delete from list but not the element itself
				plant->growingSpacerList.erase(plant->growingSpacerList.begin() + f);

				//establishment success
				if (CEnvir::rand01() < (1.0 - SRunPara::RunPara.EstabRamet))
				{
					Ramet->dead = true; //tag:SA
				}
			} //if cell ist not occupied
			else //find another random cell in the area around
			{
				if (CEnvir::week < CEnvir::WeeksPerYear)
				{
					int factorx;
					int factory;
					do {
						factorx = CEnvir::nrand(5) - 2;
						factory = CEnvir::nrand(5) - 2;
					} while (factorx == 0 && factory == 0);

					double dist = Distance(factorx, factory);
					double direction = acos(factorx / dist);

					int x = round((Ramet->xcoord + factorx) / SRunPara::RunPara.CellScale());
					int y = round((Ramet->ycoord + factory) / SRunPara::RunPara.CellScale());

					Boundary(x, y);

					//new position, dist and direction
					Ramet->xcoord = x * SRunPara::RunPara.CellScale();
					Ramet->ycoord = y * SRunPara::RunPara.CellScale();
					Ramet->SpacerlengthToGrow = dist;
					Ramet->Spacerlength = dist;
					Ramet->Spacerdirection = direction;
				}
				if (CEnvir::week == CEnvir::WeeksPerYear)
				{
					delete Ramet;
					plant->growingSpacerList.erase(plant->growingSpacerList.begin() + f);
				}
			} //else
		} //end if pos reached
	} //loop for all Ramets
} //end CGridclonal::RametEstab()
//-----------------------------------------------------------------------------

/**
 * seedling mortality
 */
void CGrid::SeedMortAge()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i)
	{
		CCell* cell = CellList[i];

		for (seed_iter it = cell->SeedBankList.begin(); it != cell->SeedBankList.end(); ++it)
		{
			CSeed* seed = *it;

			if (seed->Age >= seed->Traits->Dorm)
				seed->remove = true;
		}
		cell->RemoveSeeds(); // removes and deletes all seeds with remove==true
	}   // for all cells
}   //end SeedMortAge

void CGrid::Disturb()
{
	if (PlantList.size() == 0)
		return;

	if (CEnvir::year < SRunPara::RunPara.catastrophicDistYear ||
			(CEnvir::year == SRunPara::RunPara.catastrophicDistYear && CEnvir::week < 21))
	{
		CGrid::above_biomass_history.push_back(GetTotalAboveMass());
		CGrid::below_biomass_history.push_back(GetTotalBelowMass());
	}

	if (CEnvir::rand01() < SRunPara::RunPara.GrazProb)
	{
		Grazing();
	}

	if (CEnvir::rand01() < SRunPara::RunPara.DistProb())
	{
		Trampling();
	}

	/**
	 * If:
	 * 		The probability is triggered
	 * 		The year is after the "belowground grazing start year"
	 * 		The year is either:
	 * 			Before the window expires
	 * 			There is no window and belowground infestation is permanent
	 *
	 */
	if (CEnvir::rand01() < SRunPara::RunPara.BelGrazProb
			&& CEnvir::year >= SRunPara::RunPara.BelGrazStartYear
			&& (CEnvir::year < SRunPara::RunPara.BelGrazStartYear + SRunPara::RunPara.BelGrazWindow
					|| SRunPara::RunPara.BelGrazWindow == 0))
	{
		GrazingBelGr();
	}

	if (SRunPara::RunPara.NCut > 0) {
		switch (SRunPara::RunPara.NCut) {
		case 1:
			if (CEnvir::week == 22)
				Cutting(SRunPara::RunPara.CutHeight);
			break;
		case 2:
			if ((CEnvir::week == 22) || (CEnvir::week == 10))
				Cutting(SRunPara::RunPara.CutHeight);
			break;
		case 3:
			if ((CEnvir::week == 22) || (CEnvir::week == 10)
					|| (CEnvir::week == 16))
				Cutting(SRunPara::RunPara.CutHeight);
			break;
		default:
			cerr << "CGrid::Disturb() - wrong input";
			exit(3);
		}
	}
} //end Disturb

void CGrid::catastrophicDisturbance()
{
		// Disturb plants
		for (auto i = PlantList.begin(); i < PlantList.end(); ++i)
		{
			CPlant* p = *i;

			if (p->dead) continue;

			if (CEnvir::rand01() < SRunPara::RunPara.CatastrophicPlantMortality) {
				p->dead = true;
			}
		}

		// Disturb seeds
		for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
			CCell* c = CellList[i];
			for (auto it = c->SeedBankList.begin(); it != c->SeedBankList.end(); ++it) {
				CSeed* s = *it;
				if (CEnvir::rand01() < SRunPara::RunPara.CatastrophicSeedMortality) {
					s->remove = true;
				}
			}

			c->RemoveSeeds(); //removes and deletes all seeds with remove==true
		} // for all cells
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
void CGrid::Grazing() {
	int SumCells = SRunPara::RunPara.GetSumCells();
	double CellScale = SRunPara::RunPara.CellScale();
	double ResidualMass = SRunPara::RunPara.MassUngraz * SumCells * CellScale * CellScale * 0.0001;
	double MaxMassRemove, TotalAboveMass, MassRemoved = 0;
	double grazprob;
	double Max;

	TotalAboveMass = GetTotalAboveMass();

	//maximal removal of biomass
	MaxMassRemove = TotalAboveMass * SRunPara::RunPara.PropRemove;
	MaxMassRemove = min(TotalAboveMass - ResidualMass, MaxMassRemove);

	while (MassRemoved < MaxMassRemove) {
		//calculate slope for individual grazing probability;
		//sort PlantList descending after mshoot/LMR
		sort(PlantList.begin(), PlantList.end(), CPlant::ComparePalat);
		//plant with highest grazing susceptibility
		CPlant* plant = *PlantList.begin();
		Max = plant->mshoot * plant->Traits->GrazFac();

		random_shuffle(PlantList.begin(), PlantList.end());

		plant_size i = 0;
		while (i < PlantList.size() && MassRemoved < MaxMassRemove)
		{
			CPlant* lplant = PlantList[i];
			grazprob = (lplant->mshoot * lplant->Traits->GrazFac()) / Max;

			if (CEnvir::rand01() < grazprob)
				MassRemoved += lplant->RemoveMass();

			++i;
		}
	}
} //end CGrid::Grazing()

//-----------------------------------------------------------------------------
/**
 Cutting of all plants on the patch

 \author Felix May (Jan2010)
 \change 28-10-2010 lw: quadriere LMR    #
 \change 18-11-2010 kk: gebe entfernte BM an Klassenvariable
 */
void CGrid::Cutting(double cut_height)
{
	CPlant* p;

	for (plant_size i = 0; i < PlantList.size(); i++) {
		p = PlantList[i];

		assert(cut_height != 0 && "In CGrid::Cutting, but cut_height is 0.");

		if (p->getHeight() > cut_height)
		{
			double biomass_at_height = p->getBiomassAtHeight(cut_height);

			p->mshoot = biomass_at_height;
			p->mRepro = 0.0;
		}
	}
} //end cutting

//-----------------------------------------------------------------------------
/**
 The plants on the whole grid are grazed according to the mode given
 A given percentage of RootMass is grazed.
 \param mode mode of belowground grazing
 \arg \c mode=0 general reduction of belowground biomass for all plants
 simultaneously (standard)
 \arg \c mode=1 stochastic reduction of belowground biomass of single plants
 until a given portion is reached (not implemented yet)
 \arg \c mode=2 as '1' but proportional to root mass
 \arg \c mode=3 as '1' but prop. to root mass and taste
 (SPftTraits::palat)
 \arg \c mode=4 as '1' but prop. to root taste (old:mass and 1/taste )
 \arg \c mode=5 as '1' but prop. to root density
 \arg \c mode=6 as '1' but prop. to type-specific root density
 \warning grazing priorities don't change for mode5 and mode6
 within one time step, as they do after first grazing loop
 for modes 1-4
 The inner loop is broken and a log-entry is done
 if no more roots to graze were found in one loop (to
 prevent endless loops).

 \since belowground herbivory simulations
 \bug m�glicher Absturz wenn Traits->palat==0 oder rootmass==0

 Additional mortality is assumed if root grazing of a  plant exceeds
 a threshold (only if mode>0).  -- obsolete

 \date Mar2011
 Spatial Heterogeneity is introduced. Only Plants of the left grid part
 are grazed belowground if Heterogeneity-flag is set. (only if mode>0)
 */
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

	assert(!CGrid::below_biomass_history.empty());

	double bt = sumRootMass(generateLivingPlants(PlantList)); // Total root biomass
	double fn_o = SRunPara::RunPara.BelGrazPerc * CGrid::below_biomass_history.back(); // Forage need (mg)
	double biomass_removed = 0;
	const double alpha = 2.0;

	// Functional response
	if (bt-fn_o < bt*SRunPara::RunPara.BelGrazResidualPerc)
	{
		fn_o = bt - bt*SRunPara::RunPara.BelGrazResidualPerc;
	}
	double fn = fn_o;

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
				biomass_removed += p->RemoveRootMass(biomass_to_remove);
			}
			assert(sumRootMass(livingPlants) > 0);
		}

		fn = leftovers;
	}

}

//-----------------------------------------------------------------------------
/**
 Round gaps are created randomly, and all plants therein are killed,
 until a certain \ref SRunPara::AreaEvent "Area" is trampled.
 If a cell is trampled twice it does not influence the number of
 disturbed patches.

 (Radius of disturbance currently is 10cm)

 \par revision
 Let ZOI be defined by a list sorted after ascending distance to center instead
 of searching a square defined by maximum radius.
 */
void CGrid::Trampling() {
	int xcell, ycell, xhelp, yhelp, index;   //central point
	//  using CEnvir::nrand;using CEnvir::Round;using SRunPara::RunPara;

	double radius = 10.0;                 //radius of disturbance [cm]
	double Apatch = (Pi * radius * radius);   //area of patch [cm�]
	//number of gaps
	int NTrample = floor(
			SRunPara::RunPara.AreaEvent * SRunPara::RunPara.GridSize
					* SRunPara::RunPara.GridSize / Apatch);
	//area of patch [cell number]
	Apatch /= SRunPara::RunPara.CellScale() * SRunPara::RunPara.CellScale();

	for (int i = 0; i < NTrample; ++i) {
		//get random center of disturbance
		xcell = CEnvir::nrand(SRunPara::RunPara.CellNum);
		ycell = CEnvir::nrand(SRunPara::RunPara.CellNum);

		for (int a = 0; a < Apatch; a++) {
			//get current position: add random center pos with ZOIBase-pos
			xhelp = xcell + ZOIBase[a] / SRunPara::RunPara.CellNum
					- SRunPara::RunPara.CellNum / 2;
			yhelp = ycell + ZOIBase[a] % SRunPara::RunPara.CellNum
					- SRunPara::RunPara.CellNum / 2;
			/// \todo change to absorbing bound for upscaling
			Boundary(xhelp, yhelp);
			index = xhelp * SRunPara::RunPara.CellNum + yhelp;
			CCell* cell = CellList[index];
			if (cell->occupied) {
				CPlant* plant = (CPlant*) cell->PlantInCell;
				plant->remove = true;
			}   //if occ
		}   //for all cells in patch
	}   //for all patches
}   //end CGrid::Trampling()

//-----------------------------------------------------------------------------
void CGrid::RemovePlants() {
	plant_iter irem = partition(PlantList.begin(), PlantList.end(), mem_fun(&CPlant::GetPlantRemove));
	for (plant_iter it = irem; it < PlantList.end(); ++it)
	{
		CPlant* plant = *it;
		DeletePlant(plant);
	}
	PlantList.erase(irem, PlantList.end());
}

//-----------------------------------------------------------------------------
/**
 Delete a plant from the grid and it's references in genet list and grid cell.
 */
void CGrid::DeletePlant(CPlant* p) {
	CGenet *Genet = p->getGenet();
	//search ramet in list and erase
	for (unsigned int j = 0; j < Genet->AllRametList.size(); j++)
	{
		CPlant* Ramet;
		Ramet = Genet->AllRametList[j];
		if (p == Ramet)
			Genet->AllRametList.erase(Genet->AllRametList.begin() + j);
	}   //for all ramets
	p->getCell()->occupied = false;
	p->getCell()->PlantInCell = NULL;

	delete p;
} //end CGridclonal::DeletePlant

//-----------------------------------------------------------------------------
void CGrid::Winter()
{
	RemovePlants();
	//mass removal in wintertime
	for (plant_iter p = PlantList.begin(); p < PlantList.end(); ++p)
	{
		(*p)->WinterLoss();
	}
}

//-----------------------------------------------------------------------------
void CGrid::SeedMortWinter()
{
	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) // loop for all cells
	{
		CCell* cell = CellList[i];
		for (seed_iter it = cell->SeedBankList.begin(); it != cell->SeedBankList.end(); ++it)
		{
			CSeed* seed = *it;
			if (CEnvir::rand01() < SRunPara::RunPara.mort_seeds)
			{
				seed->remove = true;
			} //if not seed survive
			else
				++seed->Age;
		} //for seeds in cell

		cell->RemoveSeeds();   //removes and deletes all seeds with remove==true
	}   // for all cells
}   //end CGrid::SeedMortWinter()

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
void CGrid::InitClonalSeeds(SPftTraits* traits, const int n, double estab)
{
	int x, y;
	int SideCells = SRunPara::RunPara.CellNum;

	for (int i = 0; i < n; ++i)
	{
		x = CEnvir::nrand(SideCells);
		y = CEnvir::nrand(SideCells);

		CCell* cell = CellList[x * SideCells + y];
		new CSeed(estab, traits, cell);
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
												/ (double) CEnvir::WeeksPerYear)
								+ CEnvir::AResMuster[i]),
				max(0.0,
						SRunPara::RunPara.Bampl
								* sin(
										2.0 * Pi * gweek
												/ (double) CEnvir::WeeksPerYear)
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
	const int Num = SRunPara::RunPara.CellNum;

	return Distance(i1 / Num, i1 % Num, Num / 2, Num / 2) <
			Distance(i2 / Num, i2 % Num, Num / 2, Num / 2);
}

//---------------------------------------------------------------------------
/**
 \param[in,out] xx  torus correction of x-coordinate
 \param[in,out] yy  torus correction of y-coordinate

 */
void Boundary(int& xx, int& yy)
{
	xx %= SRunPara::RunPara.CellNum;
	if (xx < 0)
		xx += SRunPara::RunPara.CellNum;

	yy %= SRunPara::RunPara.CellNum;
	if (yy < 0)
		yy += SRunPara::RunPara.CellNum;
}

//---------------------------------------------------------------------------
bool Emmigrates(int& xx, int& yy)
{
	if (xx < 0 || xx >= SRunPara::RunPara.CellNum)
		return true;
	if (yy < 0 || yy >= SRunPara::RunPara.CellNum)
		return true;
	return false;
}

//---------------------------------------------------------------------------
/**
 \return sum of all plants' aboveground biomass (shoot and fruit)
 */
double CGrid::GetTotalAboveMass() {
	double above_mass = 0;

	for (plant_iter i = PlantList.begin(); i < PlantList.end(); ++i) {
		CPlant* p = *i;
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

	for (plant_iter i = PlantList.begin(); i < PlantList.end(); ++i) {
		CPlant* p = *i;
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
	for (plant_iter i = PlantList.begin(); i < PlantList.end(); ++i) {
		CPlant* p = *i;
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
	for (plant_iter i = PlantList.begin(); i < PlantList.end(); ++i) {
		CPlant* p = *i;
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
		seedCount = seedCount + cell->SeedBankList.size();
	}

	return seedCount;
}

//-----------------------------------------------------------------------------
/**
 \return the number of genets with at least one ramet still alive
 */
int CGrid::GetNMotherPlants() //count genets
{
	int NMotherPlants = 0;
	if (GenetList.size() > 0) {
		for (unsigned int i = 0; i < GenetList.size(); i++) {
			CGenet* Genet = GenetList[i];
			if ((Genet->AllRametList.size() > 0)) {
				unsigned int g = 0;
				do {
					g++;
				} while ((Genet->AllRametList[g - 1]->dead)
						&& (g < Genet->AllRametList.size()));
				if (!Genet->AllRametList[g - 1]->dead)
					NMotherPlants++;
			} //for all ramets
		} //for all genets
	} //if there are genets
	return NMotherPlants;
} //end CGridclonal::GetNMotherPlants()
//------------------------------------------------------------------------------
/**
 Counts a cell covered if the list of aboveground ZOIs has length >0.

 \note Call the function after updating weekly ZOIs
 in function CGrid::CoverCells()

 \return the number of covered cells on grid
 */
int CGrid::GetCoveredCells() //count covered cells
{
	int NCellsAcover = 0;
	const int sumcells = SRunPara::RunPara.GetSumCells(); //hopefully faster
	for (int i = 0; i < sumcells; ++i) {
		if (CellList[i]->AbovePlantList.size() > 0)
			NCellsAcover++;
	} //for all cells
	return NCellsAcover;
} //end CGridclonal::GetCoveredCells()
//------------------------------------------------------------------------------
/**
 * calculate the mean number of generations per genet
 * @return mean number of generations per genet
 */
double CGrid::GetNGeneration() {
	double SumGeneration = 0;
	double Sum = 0;
	double highestGeneration;
	double MeanGeneration = 0;

	if (GenetList.size() > 0) {
		for (unsigned int i = 0; i < GenetList.size(); i++) {
			CGenet* Genet;
			Genet = GenetList[i];
			if ((Genet->AllRametList.size() > 0)) {
				highestGeneration = 0;
				for (unsigned int j = 0; j < Genet->AllRametList.size(); j++) {
					CPlant* Ramet;
					Ramet = Genet->AllRametList[j];
					highestGeneration = max(highestGeneration,
							double(Ramet->Generation));
				}
				SumGeneration += highestGeneration;
				Sum++;
			} //if genet has ramets
		} //for all ramets
		if (Sum > 0)
			MeanGeneration = (SumGeneration / Sum);
		//else MeanGeneration=0;
	}
	return MeanGeneration;
} //end CGridclonal::GetNGeneration()
