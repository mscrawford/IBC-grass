//---------------------------------------------------------------------------
//#pragma hdrstop

#include "CSeed.h"

#include <cassert>
#include <vector>

#include "Cell.h"
#include "Plant.h"
#include "CEnvir.h"
#include "RunPara.h"
#include "SPftTraits.h"

//---------------------------------------------------------------------------

//#pragma package(smart_init)

//---------------------------------------------------------------------------
//CSeed::CSeed(double x, double y, int ID, double m, double estab, int maxage)
//:xcoord(x),ycoord(y),mass(m),estab(estab)
//{
////   TypeID=ID;
//   Traits=*CPftTraits::PftList[ID-1];
//   cell=NULL;
//
//   Age=1;
//   remove=false;
//}

//---------------------------------------------------------------------------
///not used
///
//CSeed::CSeed(CSeed& seed)
//  :xcoord(seed.xcoord),ycoord(seed.ycoord),Age(seed.Age),cell(seed.cell),
//  remove(seed.remove),Traits(seed.Traits),estab(seed.estab),mass(seed.mass)
//{
//}

//---------------------------------------------------------------------------
///not used
///
//CSeed::CSeed(double x, double y, CPlant* plant)
//  :xcoord(x),ycoord(y),Age(1),cell(NULL),remove(false),
//  Traits(plant->Traits),estab(Traits->pEstab),mass(Traits->SeedMass)
//{
//}

//---------------------------------------------------------------------------
CSeed::CSeed(CPlant* plant, CCell* cell) :
		xcoord(plant->xcoord), ycoord(plant->ycoord), Age(1), cell(NULL), remove(
				false) {
	if (SRunPara::RunPara.indivVariationVer == on) {
		Traits = SPftTraits::createPftInstanceFromPftType(plant->Traits->name); // general
		Traits->varyTraits();
	} else if (SRunPara::RunPara.indivVariationVer == off) {
		assert(plant->Traits->myTraitType == SPftTraits::species);
		Traits = new SPftTraits(*plant->Traits);
	} else {
		exit(3);
	}

	estab = Traits->pEstab;
	mass = Traits->SeedMass;

	setCell(cell);

}

//---------------------------------------------------------------------------
CSeed::CSeed(double estab, SPftTraits* traits, CCell* cell) :
		xcoord(0), ycoord(0), Age(1), cell(NULL), remove(false), estab(estab) {

	if (SRunPara::RunPara.indivVariationVer == on) {
		Traits = SPftTraits::createPftInstanceFromPftType(traits->name); // general
		Traits->varyTraits();
	} else if (SRunPara::RunPara.indivVariationVer == off) {
		assert(traits->myTraitType == SPftTraits::species);
		Traits = new SPftTraits(*traits);
	} else {
		exit(3);
	}

	mass = Traits->SeedMass;

	setCell(cell);
	if (cell) {
		xcoord = (cell->x * SRunPara::RunPara.CellScale());
		ycoord = (cell->y * SRunPara::RunPara.CellScale());
	}

}

CSeed::~CSeed() {
	/**
	 * These assertions will trigger if the SimFile contains one parameterization with individual variation
	 * and one without, in the first year of the later. This is because the deconstructor is called during cleanup,
	 * which occurs at the beginning of the run, rather than the end of the last.
	 */
//	if (Traits->myTraitType == SPftTraits::individualized) {
//		assert(SRunPara::RunPara.indivVariationVer == on);
//	} else if (SRunPara::RunPara.indivVariationVer == off) {
//		assert(Traits->myTraitType == SPftTraits::species);
//	}
	delete Traits; // Every seed has its own copy of the Traits object.

}
//---------------------------------------------------------------------------
///not used
///
//CSeed::CSeed(double x, double y,double estab, SPftTraits* traits)
//  :xcoord(x),ycoord(y),estab(estab){
//   Traits=traits;
//   mass=Traits->SeedMass;
//   Age=1;
//   remove=false;
//   cell=NULL;
//}

//-----------------------------------------------------------------------------
void CSeed::setCell(CCell* cell) {
	if (this->cell == NULL) {
		this->cell = cell;
		this->cell->SeedBankList.push_back(this); //add to seed bank
	} else {
		cerr << "This seed is already on a cell." << endl;
	}
}

//---------------------------------------------------------------------------
///not used
///
//bool CSeed::Survive()
//{
//   if (Age<Traits->Dorm) return true;
//   else return false;
//}

//---------------------------------------------------------------------------
///not used
///
//void CSeed::SetAge(int age)
//{
//   Age=age;
//}

//---------------------------------------------------------------------------
bool GetSeedRemove(const CSeed* seed1) {
	return (!seed1->remove);
}

//-----------------------------------------------------------------------------
///sort plants ascending after TypeID
///\warning  compiler complains about temporal state of seed1 and seed2
///
int CompareTypeID(const CSeed* seed1, const CSeed* seed2) {
	return (seed1->Traits->TypeID < seed2->Traits->TypeID);
}

//-----------------------------------------------------------------------------
std::string CSeed::type() {
	return "CSeed";
}

//-----------------------------------------------------------------------------
std::string CSeed::pft() {
	return this->Traits->name;
}

//-eof----------------------------------------------------------------------------
