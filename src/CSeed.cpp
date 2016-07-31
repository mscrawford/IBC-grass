//---------------------------------------------------------------------------
//#pragma hdrstop

#include <cassert>
#include <vector>

#include "CSeed.h"
#include "Cell.h"
#include "Plant.h"
#include "CEnvir.h"
#include "RunPara.h"
#include "SPftTraits.h"

//---------------------------------------------------------------------------

//#pragma package(smart_init)

//---------------------------------------------------------------------------
CSeed::CSeed(CPlant* plant, CCell* cell) :
		xcoord(plant->xcoord), ycoord(plant->ycoord), Age(1), cell(NULL), remove(
				false) {
	if (SRunPara::RunPara.ITV == on) {
		Traits = SPftTraits::createPftInstanceFromPftType(plant->Traits->name); // general
		Traits->varyTraits();
	} else if (SRunPara::RunPara.ITV == off) {
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

	if (SRunPara::RunPara.ITV == on) {
		Traits = SPftTraits::createPftInstanceFromPftType(traits->name); // general
		Traits->varyTraits();
	} else if (SRunPara::RunPara.ITV == off) {
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

/**
 * Destructor. Every seed contains its own trait set.
 */
CSeed::~CSeed() {
	delete Traits;
}

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
bool GetSeedRemove(const CSeed* seed1) {
	return (!seed1->remove);
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
