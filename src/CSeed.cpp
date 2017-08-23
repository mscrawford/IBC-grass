
#include <cassert>
#include <iostream>
#include <vector>

#include "CSeed.h"
#include "CEnvir.h"

using namespace std;

//-----------------------------------------------------------------------------

/*
 * Constructor for normal reproduction
 */
CSeed::CSeed(const shared_ptr<CPlant> & plant, CCell* _cell) :
		cell(NULL),
		Age(0), remove(false)
{
	Traits = SPftTraits::createTraitSetFromPftType(plant->Traits->name);

	if (SRunPara::RunPara.ITV == on) {
		Traits->varyTraits();
	}

	estab = Traits->pEstab;
	mass = Traits->SeedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}

//-----------------------------------------------------------------------------

/*
 * Constructor for initial establishment (with germination pre-set)
 */
CSeed::CSeed(std::string PFT_ID, CCell*_cell, double new_estab) :
		cell(NULL),
		Age(0), remove(false)
{
	Traits = SPftTraits::createTraitSetFromPftType(PFT_ID);

	if (SRunPara::RunPara.ITV == on) {
		Traits->varyTraits();
	}

	estab = new_estab;
	mass = Traits->SeedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}
