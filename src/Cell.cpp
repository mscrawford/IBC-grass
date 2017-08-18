//---------------------------------------------------------------------------

#include <iostream>
#include <algorithm>
#include <cassert>

#include "Cell.h"
#include "CEnvir.h"

using namespace std;

//-----------------------------------------------------------------------------
/**
 * constructor
 * @param xx x-coordinate on grid
 * @param yy y-coordinate on grid
 * @param ares aboveground resources
 * @param bres belowground resources
 */
CCell::CCell(const unsigned int xx, const unsigned int yy) :
		x(xx), y(yy),
		AResConc(0), BResConc(0),
		aComp_weekly(0), bComp_weekly(0),
		occupied(false)
{
	int index = xx * SRunPara::RunPara.GridSize + yy;
	AResConc = CEnvir::AResMuster[index];
	BResConc = CEnvir::BResMuster[index];
}

CCell::~CCell()
{
	AbovePlantList.clear();
	BelowPlantList.clear();

	SeedBankList.clear();
	SeedlingList.clear();

	PftNIndA.clear();
	PftNIndB.clear();
}

void CCell::clear()
{
	AbovePlantList.clear();
	BelowPlantList.clear();

	SeedBankList.clear();
	SeedlingList.clear();

	PftNIndA.clear();
	PftNIndB.clear();

	occupied = false;
}

void CCell::weeklyReset()
{
	AbovePlantList.clear();
	BelowPlantList.clear();

	PftNIndA.clear();
	PftNIndB.clear();

	SeedlingList.clear();
}

void CCell::SetResource(double Ares, double Bres)
{
   AResConc = Ares;
   BResConc = Bres;
}

double CCell::Germinate()
{
	double sum_SeedMass = 0;

	auto it = SeedBankList.begin();
	while ( it != SeedBankList.end() )
	{
		auto & seed = *it;
		if (CEnvir::rng.get01() < seed->estab)
		{
			sum_SeedMass += seed->mass;
			SeedlingList.push_back(std::move(seed)); // This seed germinates, add it to seedlings
			SeedBankList.erase(it); // Remove its iterator from the SeedBankList, which now holds only ungerminated seeds
		}
		else
		{
			++it;
		}
	}

	return sum_SeedMass;
}

//---------------------------------------------------------------------------
void CCell::RemoveSeeds()
{
	SeedBankList.erase(
			std::remove_if(SeedBankList.begin(), SeedBankList.end(), CSeed::GetSeedRemove),
			SeedBankList.end());
}

//-----------------------------------------------------------------------------
/**
ABOVEground competition takes global information on symmetry and version to
distribute the cell's resources. Resource competition Version is 1.

virtual function will be substituted by comp function from sub class
*/
void CCell::AboveComp()
{
	if (AbovePlantList.empty())
		return;

	if (SRunPara::RunPara.AboveCompMode == asymtot)
	{
		weak_ptr<CPlant> p_ptr =
				*std::max_element(AbovePlantList.begin(), AbovePlantList.end(),
						[](const weak_ptr<CPlant> & a, const weak_ptr<CPlant> & b)
						{
							auto _a = a.lock();
							auto _b = b.lock();

							assert(_a);
							assert(_b);

							return CPlant::getShootGeometry(_a) < CPlant::getShootGeometry(_b);
						});

		auto p = p_ptr.lock();

		assert(p);

		p->Auptake += AResConc;

		return;
	}

	int symm;
	if (SRunPara::RunPara.AboveCompMode == asympart)
	{
		symm = 2;
	}
	else
	{
		symm = 1;
	}

	double comp_tot = 0;
	double comp_c = 0;

	//1. sum of resource requirement
	for (auto const& plant_ptr : AbovePlantList)
	{
		auto plant = plant_ptr.lock();
		assert(plant);

		comp_tot += plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, SRunPara::RunPara.Version);
	}

	//2. distribute resources
	for (auto const& plant_ptr : AbovePlantList)
	{
		auto plant = plant_ptr.lock();
		assert(plant);

		comp_c = plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, SRunPara::RunPara.Version);
		plant->Auptake += AResConc * comp_c / comp_tot;
	}

	aComp_weekly = comp_tot;
}

//-----------------------------------------------------------------------------
/**
BELOWground competition takes global information on symmetry and version to
distribute the cell's resources.
 Resource competition Version is 1.

virtual function will be substituted by comp function from sub class
*/
void CCell::BelowComp()
{
	assert(SRunPara::RunPara.BelowCompMode != asymtot);

	if (BelowPlantList.empty())
		return;

	int symm;
	if (SRunPara::RunPara.BelowCompMode == asympart)
	{
		symm = 2;
	}
	else
	{
		symm = 1;
	}

	double comp_tot = 0;
	double comp_c = 0;

	//1. sum of resource requirement
	for (auto const& plant_ptr : BelowPlantList)
	{
		auto plant = plant_ptr.lock();
		assert(plant);

		comp_tot += plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, SRunPara::RunPara.Version);
	}
	//2. distribute resources
	for (auto const& plant_ptr : BelowPlantList)
	{
		auto plant = plant_ptr.lock();
		assert(plant);

		comp_c = plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, SRunPara::RunPara.Version);
		plant->Buptake += BResConc * comp_c / comp_tot;
	}

	bComp_weekly = comp_tot;
}

//---------------------------------------------------------------------------
/**
  \param type     Plant_functional_Type-ID
  \param layer    above(1)- or below(2)ground
  \param version  one of [0,1,2]
  \since revision
*/
double CCell::prop_res(const string type, const int layer, const int version) const
{
	switch (version)
	{
	case 0:
		return 1;
		break;
	case 1:
		if (layer == 1)
		{
			map<string, int>::const_iterator noa = PftNIndA.find(type);
			if (noa != PftNIndA.end())
			{
				return 1.0 / sqrt(noa->second);
			}
		}
		if (layer == 2)
		{
			map<string, int>::const_iterator nob = PftNIndB.find(type);
			if (nob != PftNIndB.end())
			{
				return 1.0 / sqrt(nob->second);
			}
		}
		break;
	case 2:
		if (layer == 1)
		{
			return PftNIndA.size() / (1.0 + PftNIndA.size());
		}
		if (layer == 2)
		{
			return PftNIndB.size() / (1.0 + PftNIndB.size());
		}
		break;
	default:
		cerr << "CCell::prop_res() - wrong input";
		exit(3);
		break;
	}
	return -1;
}
