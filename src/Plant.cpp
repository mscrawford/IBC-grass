
#include <cassert>
#include <iostream>

#include "Plant.h"
#include "Environment.h"
#include "Traits.h"

using namespace std;

//-----------------------------------------------------------------------------
/**
 * constructor - germination
 *
 * If a seed germinates, the new plant inherits its parameters.
 * Genet has to be defined externally.
 */

int Plant::staticID = 0;

Plant::Plant(const unique_ptr<Seed> & seed) :
		cell(NULL), mReproRamets(0), genet(),
		plantID(++staticID), x(0), y(0),
		age(0), mRepro(0), Ash_disc(0), Art_disc(0), Auptake(0), Buptake(0),
		isStressed(0), isDead(false), toBeRemoved(false),
		spacerLengthToGrow(0)
{

	traits = Traits::copyTraitSet(seed->traits);

	if (Parameters::params.ITV == on) {
		assert(traits->myTraitType == Traits::individualized);
	} else {
		assert(traits->myTraitType == Traits::species);
	}

	mShoot = traits->m0;
	mRoot = traits->m0;

	//establish this plant on cell
	setCell(seed->getCell());
	if (cell)
	{
		x = cell->x;
		y = cell->y;
	}
}

//-----------------------------------------------------------------------------
/**
 * Clonal Growth - The new Plant inherits its parameters from 'plant'.
 * Genet is the same as for plant
 */
Plant::Plant(double x, double y, const std::shared_ptr<Plant> & plant) :
		cell(NULL), mReproRamets(0), genet(plant->genet),
		plantID(++staticID), x(x), y(y),
		age(0), mRepro(0), Ash_disc(0), Art_disc(0), Auptake(0), Buptake(0),
		isStressed(0), isDead(false), toBeRemoved(false),
		spacerLengthToGrow(0)
{

	traits = Traits::copyTraitSet(plant->traits);

	if (Parameters::params.ITV == on) {
		assert(traits->myTraitType == Traits::individualized);
	} else {
		assert(traits->myTraitType == Traits::species);
	}

	mShoot = traits->m0;
	mRoot = traits->m0;
}

//---------------------------------------------------------------------------

Plant::~Plant()
{
	growingSpacerList.clear();
}

void Plant::weeklyReset()
{
	Auptake = 0;
	Buptake = 0;
	Ash_disc = 0;
	Art_disc = 0;
}

//-----------------------------------------------------------------------------

void Plant::setCell(Cell* _cell) {
	assert(this->cell == NULL && _cell != NULL);

	this->cell = _cell;
	this->cell->occupied = true;
}

//---------------------------------------------------------------------------
/**
 * Growth of reproductive organs (seeds and spacer).
 */
double Plant::ReproGrow(double uptake)
{
	double VegRes;

	//fixed Proportion of resource to seed production
	if (mRepro <= traits->allocSeed * mShoot)
	{
		double SeedRes = uptake * traits->allocSeed;
		double SpacerRes = uptake * traits->allocSpacer;

		// during the seed-production-weeks
		if (Environment::week >= traits->flowerWeek && Environment::week < traits->dispersalWeek)
		{
			//seed production
			double dm_seeds = max(0.0, traits->growth * SeedRes);
			mRepro += dm_seeds;

			//clonal growth
			double d = max(0.0, min(SpacerRes, uptake - SeedRes)); // for large AllocSeed, resources may be < SpacerRes, then only take remaining resources
			mReproRamets += max(0.0, traits->growth * d);

			VegRes = uptake - SeedRes - d;
		}
		else
		{
			VegRes = uptake - SpacerRes;
			mReproRamets += max(0.0, traits->growth * SpacerRes);
		}

	}
	else
	{
		VegRes = uptake;
	}

	return VegRes;
}

//-----------------------------------------------------------------------------
/**
 * Growth of the spacer.
 */
void Plant::SpacerGrow() {

	if (growingSpacerList.size() == 0 || Environment::AreSame(mReproRamets, 0))
	{
		return;
	}

	double mGrowSpacer = mReproRamets / growingSpacerList.size(); //resources for one spacer

	for (auto const& Spacer : growingSpacerList)
	{
		Spacer->spacerLengthToGrow = max(0.0, Spacer->spacerLengthToGrow - (mGrowSpacer / traits->mSpacer));
	}

	mReproRamets = 0;
}

//-----------------------------------------------------------------------------
/**
 * two-layer growth
 * -# Resources for fecundity are allocated
 * -# According to the resources allocated and the respiration needs
 * shoot- and root-growth are calculated.
 * -# Stress-value is in- or decreased according to the uptake
 *
 * adapted growth formula with correction factor for the conversion rate
 * to simulate implicit biomass reduction via root herbivory
 */
void Plant::Grow() //grow plant one timestep
{
	double dm_shoot, dm_root, alloc_shoot;
	double LimRes, ShootRes, RootRes, VegRes;

	/********************************************/
	/*  dm/dt = growth*(c*m^p - m^q / m_max^r)  */
	/********************************************/

	// which resource is limiting growth?
	LimRes = min(Buptake, Auptake); // two layers
	VegRes = ReproGrow(LimRes);

	// allocation to shoot and root growth
	alloc_shoot = Buptake / (Buptake + Auptake); // allocation coefficient

	ShootRes = alloc_shoot * VegRes;
	RootRes = VegRes - ShootRes;

	// Shoot growth
	dm_shoot = this->ShootGrow(ShootRes);

	// Root growth
	dm_root = this->RootGrow(RootRes);

	mShoot += dm_shoot;
	mRoot += dm_root;

	if (stressed())
	{
		++isStressed;
	}
	else if (isStressed > 0)
	{
		--isStressed;
	}

}

//-----------------------------------------------------------------------------
/**
 * shoot growth : dm/dt = growth*(c*m^p - m^q / m_max^r)
 */
double Plant::ShootGrow(double shres)
{

	double Assim_shoot;
	double Resp_shoot;

	// exponents for growth function
	double p = 2.0 / 3.0;
	double q = 2.0;
	double r = 4.0 / 3.0;

	Assim_shoot = traits->growth * min(shres, traits->Gmax * Ash_disc); //growth limited by maximal resource per area -> similar to uptake limitation
	Resp_shoot = traits->growth * traits->SLA * pow(traits->LMR, p) * traits->Gmax * pow(mShoot, q) / pow(traits->maxMass, r); //respiration proportional to mshoot^2

	return max(0.0, Assim_shoot - Resp_shoot);

}

//-----------------------------------------------------------------------------
/**
 * root growth : dm/dt = growth*(c*m^p - m^q / m_max^r)
 */
double Plant::RootGrow(double rres)
{
	double Assim_root, Resp_root;

	//exponents for growth function
	double q = 2.0;
	double r = 4.0 / 3.0;

	Assim_root = traits->growth * min(rres, traits->Gmax * Art_disc); //growth limited by maximal resource per area -> similar to uptake limitation
	Resp_root = traits->growth * traits->Gmax * traits->RAR * pow(mRoot, q) / pow(traits->maxMass, r);  //respiration proportional to root^2

	return max(0.0, Assim_root - Resp_root);
}

//-----------------------------------------------------------------------------

bool Plant::stressed() const
{
	return (Auptake / 2.0 < minresA()) || (Buptake / 2.0 < minresB());
}

//-----------------------------------------------------------------------------
/**
 * Kill plant depending on stress level and base mortality. Stochastic process.
 */
void Plant::Kill()
{
	assert(traits->memory >= 1);

	double pmort = (double(isStressed) / double(traits->memory)) + Parameters::params.backgroundMortality; // stress mortality + random background mortality

	if (Environment::rng.get01() < pmort)
	{
		isDead = true;
	}
}

//-----------------------------------------------------------------------------
/**
 * Litter decomposition with deletion at 10mg.
 */
void Plant::DecomposeDead() {

	assert(isDead);

	const double minmass = 10; // mass at which dead plants are removed

	mRepro = 0;
	mShoot *= Parameters::params.litterDecomp;
	mRoot *= Parameters::params.litterDecomp;

	if (GetMass() < minmass)
	{
		toBeRemoved = true;
	}
}

//-----------------------------------------------------------------------------
/**
 * If the plant is alive and it is dispersal time, the function returns
 * the number of seeds produced during the last weeks.
 * Subsequently the allocated resources are reset to zero.
 */
int Plant::ConvertReproMassToSeeds()
{
	int NSeeds = 0;

	if (!isDead)
	{
		NSeeds = floor(mRepro / traits->seedMass);

		mRepro = 0;
	}

	lifetimeFecundity += NSeeds;

	return NSeeds;
}

//-----------------------------------------------------------------------------
/**
 returns the number of new spacer to set:
 - 1 if there are enough clonal-growth resources and spacer list is empty
 - 0 otherwise
 */
int Plant::GetNRamets() const
{
	if (mReproRamets > 0 &&
			!isDead &&
			growingSpacerList.size() == 0) {
		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
/**
 * Remove half shoot mass and seed mass from a plant.
 */
double Plant::RemoveShootMass()
{
	double mass_removed = 0;

	if (mShoot + mRepro > 1) // only remove mass if shootmass > 1 mg
	{
		mass_removed = Parameters::params.BiteSize * mShoot + mRepro;
		mShoot *= 1 - Parameters::params.BiteSize;
		mRepro = 0;
	}

	return mass_removed;
}

//-----------------------------------------------------------------------------
/**
 * Remove root mass from a plant.
 */
void Plant::RemoveRootMass(const double mass_removed)
{
	assert(mass_removed <= mRoot);

	mRoot -= mass_removed;

	if (Environment::AreSame(mRoot, 0)) {
		isDead = true;
	}
}

//-----------------------------------------------------------------------------
/**
 * Winter dieback of aboveground biomass. Aging of Plant.
 */
void Plant::WinterLoss()
{
	mShoot *= 1 - Parameters::params.winterDieback;
	mRepro = 0;
	age++;
}

//-----------------------------------------------------------------------------
/**
 * Competitive strength of plant.
 * @param layer above- (1) or belowground (2) ZOI
 * @param symmetry Symmetry of competition
 * (symmetric, partial asymmetric, complete asymmetric )
 * @return competitive strength
 */
double Plant::comp_coef(const int layer, const int symmetry) const
{
	switch (symmetry)
	{
	case 1:
		if (layer == 1)
			return traits->Gmax;
		if (layer == 2)
			return traits->Gmax;
		break;
	case 2:
		if (layer == 1)
			return mShoot * traits->CompPowerA();
		if (layer == 2)
			return mRoot * traits->CompPowerB();
		break;
	default:
		cerr << "CPlant::comp_coef() - wrong input";
		exit(1);
	}

	return -1;
}
