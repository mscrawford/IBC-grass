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
CCell::CCell(const unsigned int xx, const unsigned int yy, double ares, double bres) :
		x(xx), y(yy), AResConc(ares), BResConc(bres),
		aComp_weekly(0), bComp_weekly(0), occupied(false),
		PlantInCell(NULL)
{
   AbovePlantList.clear();
   BelowPlantList.clear();
   SeedBankList.clear();
   SeedlingList.clear();

   PftNIndA.clear();
   PftNIndB.clear();
   PftNSeedling.clear();

   const unsigned int index = xx*SRunPara::RunPara.CellNum + yy;
   AResConc=CEnvir::AResMuster.at(index);
   BResConc=CEnvir::BResMuster[index];
}
//---------------------------------------------------------------------------
/**
 * reset cell properties
 */
void CCell::clear()
{
   AbovePlantList.clear();
   BelowPlantList.clear();

   for (unsigned int i=0; i<SeedBankList.size();i++) delete SeedBankList[i];
   for (unsigned int i=0; i<SeedlingList.size();i++) delete SeedlingList[i];

   SeedBankList.clear();
   SeedlingList.clear();

   PftNIndA.clear();
   PftNIndB.clear();
   PftNSeedling.clear();

   occupied = false;
   PlantInCell = NULL;
}

CCell::~CCell()
{
   for (unsigned int i=0; i<SeedBankList.size();i++) delete SeedBankList[i];
   for (unsigned int i=0; i<SeedlingList.size();i++) delete SeedlingList[i];

   SeedBankList.clear();
   SeedlingList.clear();

   AbovePlantList.clear();
   BelowPlantList.clear();

   PftNSeedling.clear();
}

void CCell::SetResource(double Ares, double Bres)
{
   double SideLength = SRunPara::RunPara.CellScale();

   AResConc = Ares*(SideLength*SideLength);
   BResConc = Bres*(SideLength*SideLength);

}

double CCell::Germinate()
{
	double sum_SeedMass = 0;

	for (auto seed : SeedBankList)
	{
		if (CEnvir::rng.get01() < seed->estab)
		{
			// make a copy in seedling list
			SeedlingList.push_back(seed);
			PftNSeedling[seed->pft()]++;

			seed->remove = true;
		}
	}

   //remove germinated seeds from SeedBankList
   auto iter_rem = partition(SeedBankList.begin(), SeedBankList.end(), GetSeedRemove);
   SeedBankList.erase(iter_rem, SeedBankList.end());

   for (auto seed : SeedlingList)
   {
	   sum_SeedMass += seed->mass;
   }
   return sum_SeedMass;

}

//---------------------------------------------------------------------------
void CCell::RemoveSeedlings()
{
   PftNIndA.clear();
   PftNIndB.clear();
   PftNSeedling.clear();

   for (auto seedling : SeedlingList)
   {
      delete seedling;
   }
   SeedlingList.clear();
}

//---------------------------------------------------------------------------
void CCell::RemoveSeeds()
{
   auto irem = partition(SeedBankList.begin(), SeedBankList.end(), GetSeedRemove);

   for (auto iter = irem; iter != SeedBankList.end(); ++iter){
      CSeed* seed = *iter;
      delete seed;
   }
   SeedBankList.erase(irem, SeedBankList.end());
}

//-----------------------------------------------------------------------------
/**
ABOVEground competition takes global information on symmetry and version to
distribute the cell's resources. Resource competition Version is 1.

virtual function will be substituted by comp function from sub class
*/
void CCell::AboveComp()
{
   if (AbovePlantList.empty()) return;

   if (SRunPara::RunPara.AboveCompMode == asymtot)
   {
     //total asymmetry only for above plant competition
      sort(AbovePlantList.begin(), AbovePlantList.end(), CPlant::CompareShoot);
      CPlant* plant = *AbovePlantList.begin(); //biggest plant
      plant->Auptake += AResConc;
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
	for (auto plant : AbovePlantList)
	{
		comp_tot += plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, SRunPara::RunPara.Version);
	}

   //2. distribute resources
	for (auto plant : AbovePlantList)
	{
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
	for (auto plant : BelowPlantList)
	{
		comp_tot += plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, SRunPara::RunPara.Version);
	}
	//2. distribute resources
	for (auto plant : BelowPlantList)
	{
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
