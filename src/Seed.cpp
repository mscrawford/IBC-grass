
#include <cassert>
#include <iostream>
#include <vector>

#include "Seed.h"
#include "Environment.h"

using namespace std;

//-----------------------------------------------------------------------------

/*
 * Constructor for normal reproduction
 */
Seed::Seed(const shared_ptr<Plant> & plant, Cell* _cell) :
		cell(NULL),
		age(0), toBeRemoved(false)
{
	traits = Traits::createTraitSetFromPftType(plant->traits->PFT_ID);

	if (Parameters::parameters.ITV == on) {
		traits->varyTraits();
	}

	pEstab = traits->pEstab;
	mass = traits->seedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}

//-----------------------------------------------------------------------------

/*
 * Constructor for initial establishment (with germination pre-set)
 */
Seed::Seed(std::string PFT_ID, Cell*_cell, double new_estab) :
		cell(NULL),
		age(0), toBeRemoved(false)
{
	traits = Traits::createTraitSetFromPftType(PFT_ID);

	if (Parameters::parameters.ITV == on) {
		traits->varyTraits();
	}

	pEstab = new_estab;
	mass = traits->seedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}
