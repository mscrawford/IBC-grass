
#include <cassert>
#include <iostream>
#include <vector>

#include "CSeed.h"
#include "CEnvir.h"

using namespace std;

CSeed::CSeed(shared_ptr<CPlant> plant, CCell* _cell) :
		cell(NULL),
		Age(0), remove(false)
{
	this->Traits = SPftTraits::createTraitSetFromPftType(plant->Traits->name);

	estab = Traits->pEstab;
	mass = Traits->SeedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}

CSeed::CSeed(double new_estab, shared_ptr<SPftTraits> traits, CCell*_cell) :
		cell(NULL),
		Age(0), remove(false)
{
	this->Traits = SPftTraits::createTraitSetFromPftType(traits->name);

	estab = new_estab;
	mass = Traits->SeedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}

CSeed::~CSeed() {

}
