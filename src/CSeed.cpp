
#include <cassert>
#include <iostream>
#include <vector>

#include "CSeed.h"
#include "CEnvir.h"

//---------------------------------------------------------------------------
CSeed::CSeed(CPlant* plant, CCell* _cell) :
		cell(NULL), xcoord(plant->xcoord), ycoord(plant->ycoord),
		Age(1), remove(false)
{
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

	setCell(_cell);

}

//---------------------------------------------------------------------------
CSeed::CSeed(double new_estab, SPftTraits* traits, CCell* _cell) :
		cell(NULL),
		xcoord(0), ycoord(0),
		Age(1), remove(false)
{

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
	this->estab = new_estab;

	setCell(_cell);
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
void CSeed::setCell(CCell* _cell)
{
	if (this->cell == NULL)
	{
		this->cell = _cell;
		this->cell->SeedBankList.push_back(this); // add to seed bank
	} else {
		std::cerr << "This seed is already on a cell." << std::endl;
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
