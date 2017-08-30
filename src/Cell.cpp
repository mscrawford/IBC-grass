//---------------------------------------------------------------------------

#include <iostream>
#include <algorithm>
#include <cassert>

#include "Cell.h"

#include "Environment.h"

using namespace std;

//-----------------------------------------------------------------------------

Cell::Cell(const unsigned int xx, const unsigned int yy) :
		x(xx), y(yy),
		AResConc(0), BResConc(0),
		aComp_weekly(0), bComp_weekly(0),
		occupied(false)
{
	AResConc = Parameters::params.meanARes;
	BResConc = Parameters::params.meanBRes;
}

//-----------------------------------------------------------------------------

Cell::~Cell()
{
	AbovePlantList.clear();
	BelowPlantList.clear();

	SeedBankList.clear();
	SeedlingList.clear();

	PftNIndA.clear();
	PftNIndB.clear();
}

//-----------------------------------------------------------------------------

void Cell::weeklyReset()
{
	AbovePlantList.clear();
	BelowPlantList.clear();

	PftNIndA.clear();
	PftNIndB.clear();

	SeedlingList.clear();
}

//-----------------------------------------------------------------------------

void Cell::SetResource(double Ares, double Bres)
{
   AResConc = Ares;
   BResConc = Bres;
}

//-----------------------------------------------------------------------------

double Cell::Germinate()
{
	double sum_SeedMass = 0;

	auto it = SeedBankList.begin();
	while ( it != SeedBankList.end() )
	{
		auto & seed = *it;
		if (Environment::rng.get01() < seed->pEstab)
		{
			sum_SeedMass += seed->mass;
			SeedlingList.push_back(std::move(seed)); // This seed germinates, add it to seedlings
			it = SeedBankList.erase(it); // Remove its iterator from the SeedBankList, which now holds only ungerminated seeds
		}
		else
		{
			++it;
		}
	}

	return sum_SeedMass;
}

//-----------------------------------------------------------------------------

void Cell::RemoveSeeds()
{
	SeedBankList.erase(
			std::remove_if(SeedBankList.begin(), SeedBankList.end(), Seed::GetSeedRemove),
			SeedBankList.end());
}

//-----------------------------------------------------------------------------

void Cell::AboveComp()
{
	if (AbovePlantList.empty())
		return;

	if (Parameters::params.AboveCompMode == asymtot)
	{
		weak_ptr<Plant> p_ptr =
				*std::max_element(AbovePlantList.begin(), AbovePlantList.end(),
						[](const weak_ptr<Plant> & a, const weak_ptr<Plant> & b)
						{
							auto _a = a.lock();
							auto _b = b.lock();

							return Plant::getShootGeometry(_a) < Plant::getShootGeometry(_b);
						});

		auto p = p_ptr.lock();

		assert(p);

		p->Auptake += AResConc;

		return;
	}

	int symm;
	if (Parameters::params.AboveCompMode == asympart)
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

		comp_tot += plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, Parameters::params.stabilization);
	}

	//2. distribute resources
	for (auto const& plant_ptr : AbovePlantList)
	{
		auto plant = plant_ptr.lock();
		assert(plant);

		comp_c = plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, Parameters::params.stabilization);
		plant->Auptake += AResConc * comp_c / comp_tot;
	}

	aComp_weekly = comp_tot;
}

//-----------------------------------------------------------------------------

void Cell::BelowComp()
{
	assert(Parameters::params.BelowCompMode != asymtot);

	if (BelowPlantList.empty())
		return;

	int symm;
	if (Parameters::params.BelowCompMode == asympart)
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

		comp_tot += plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, Parameters::params.stabilization);
	}
	//2. distribute resources
	for (auto const& plant_ptr : BelowPlantList)
	{
		auto plant = plant_ptr.lock();

		comp_c = plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, Parameters::params.stabilization);
		plant->Buptake += BResConc * comp_c / comp_tot;
	}

	bComp_weekly = comp_tot;
}

//---------------------------------------------------------------------------

double Cell::prop_res(const string type, const int layer, const int version) const
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
