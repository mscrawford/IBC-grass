//---------------------------------------------------------------------------
//#pragma hdrstop

#include "Plant.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cassert>
#include <iomanip>

#include "Cell.h"
#include "CEnvir.h"
#include "CGrid.h"
#include "CSeed.h"
#include "SPftTraits.h"

//-----------------------------------------------------------------------------
/**
 * constructor - germination
 *
 * If a seed germinates, the new plant inherits its parameters.
 * Genet has to be defined externally.
 */

int CPlant::numPlants = 0;

CPlant::CPlant(CSeed* seed) :
		xcoord(seed->xcoord), ycoord(seed->ycoord), Age(0), plantID(
				++numPlants), Aroots_all(0), Aroots_type(0), mRepro(0), Ash_disc(
				0), Art_disc(0), Auptake(0), Buptake(0), dead(false), remove(
				false), stress(0), cell(NULL), mReproRamets(0), Spacerlength(0), Spacerdirection(
				0), Generation(1), SpacerlengthToGrow(0), genet(
		NULL) {

	Traits = new SPftTraits(*seed->Traits);

	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC

	mshoot = Traits->m0;
	mroot = Traits->m0;

	//establish this plant on cell
	setCell(seed->getCell());
	if (cell) {
		xcoord = (cell->x * SRunPara::RunPara.CellScale());
		ycoord = (cell->y * SRunPara::RunPara.CellScale());
	}
	growingSpacerList.clear();

}

//-----------------------------------------------------------------------------
/**
 Clonal Growth - The new Plant inherits its parameters from 'plant'.
 Genet is the same as for plant, Generation is by one larger than
 that of plant.

 \note For clonal growth:
 cell has to be set and plant has to be added to genet list
 when ramet establishes.

 \since revision
 */
CPlant::CPlant(double x, double y, CPlant* plant) :
		xcoord(x), ycoord(y), Age(0), plantID(++numPlants), Aroots_all(0), Aroots_type(
				0), mRepro(0), Ash_disc(0), Art_disc(0), Auptake(0), Buptake(0), dead(
				false), remove(false), stress(0), cell(NULL), mReproRamets(0), Spacerlength(
				0), Spacerdirection(0), Generation(plant->Generation + 1), SpacerlengthToGrow(
				0), genet(plant->genet) {

	Traits = new SPftTraits(*plant->Traits);

	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC

	mshoot = Traits->m0;
	mroot = Traits->m0;

	growingSpacerList.clear();
}

//---------------------------------------------------------------------------
/**
 * destructor
 * TODO use iterators instead
 */
CPlant::~CPlant() {
	for (unsigned int i = 0; i < growingSpacerList.size(); ++i)
		delete growingSpacerList[i];
	growingSpacerList.clear();

	delete Traits;
}

//---------------------------------------------------------------------------
///set genet and add ramet to its list
void CPlant::setGenet(CGenet* genet) {
	if (this->genet == NULL) {
		this->genet = genet;
		this->genet->AllRametList.push_back(this);
	}
}

//---------------------------------------------------------------------------
/**
 plant report (incl. clonal information)

 \note direction not reportet; mReproRamets not reportet - weekly transfered
 directly to Spacerlength
 \author KK
 \date 120905
 */
string CPlant::toString(bool benchmarkFecundity) {

	if (benchmarkFecundity) {
		lifetimeFecundity += yearlyFecundity;
	}

	// MSC
	std::stringstream dummi;
	dummi << plantID
			<< '\t' << xcoord
			<< '\t' << ycoord
			<< '\t' << Age
			<< '\t' << mshoot
			<< '\t' << mroot
			<< '\t' << mRepro
			<< '\t' << yearlyFecundity
			<< '\t' << lifetimeFecundity
			<< '\t' << Radius_shoot()
			<< '\t' << Radius_root()
			<< '\t' << stress
			<< '\t' << dead
			<< '\t' << Traits->toString();

//	generation number and genet - ID
//	if (this->Traits->clonal) {
//		dummi << "\t" << Generation << '\t' << genet->number;
//		// Spacer info Length and Length-to-grow  (only for first spacer)
//		if (growingSpacerList.size() > 0) {
//			dummi << '\t' << this->growingSpacerList[0]->Spacerlength << '\t'
//					<< this->growingSpacerList[0]->SpacerlengthToGrow;
//		}
//	}

	// You should print out the Generation if it's a clonal plant, and the
	// Spacerlengths, maybe?

	if (benchmarkFecundity) {
		yearlyFecundity = 0;
	}

	return dummi.str();
}

string CPlant::headerToString() {
	std::stringstream dummi;
	dummi << "plantID"
			<< '\t' << "xcoord"
			<< '\t' << "ycoord"
			<< '\t' << "Age"
			<< '\t' << "mshoot"
			<< '\t' << "mroot"
			<< '\t' << "mRepro"
			<< '\t' << "yearlyFecundity"
			<< '\t' << "lifetimeFecundity"
			<< '\t' << "rShoot"
			<< '\t' << "rRoot"
			<< '\t' << "stress"
			<< '\t' << "dead"
			<< '\t' << SPftTraits::headerToString();
	return dummi.str();
}

//-----------------------------------------------------------------------------
/**
 * join cell to plant object
 *
 * \param
 */
void CPlant::setCell(CCell* cell) {
	if (this->cell == NULL && cell != NULL) {
		this->cell = cell;
		this->cell->occupied = true;
		this->cell->PlantInCell = this;
	}
}

//-----------------------------------------------------------------------------
string CPlant::type() {
	return "CPlant";
}

//-----------------------------------------------------------------------------
/**
 * Say, what PFT you are
 * @return PFT name
 */
string CPlant::pft() {
	return this->Traits->name;
}

//---------------------------------------------------------------------------
/**
 * Growth of reproductive organs (seeds and spacer).
 *
 * Function adapted to annual plants with AllocSeed of 1.
 * @param uptake Resource uptake of plant object.
 * @return resources available for individual needs.
 * \author FM, IS adapted by HP
 */
double CPlant::ReproGrow(double uptake) {
	double SpacerRes, SeedRes, VegRes, dm_seeds, dummy1;
	//fixed Proportion of resource to seed production
	if (mRepro <= Traits->AllocSeed * mshoot)  //calculate mRepro for every week
			{
		SeedRes = uptake * Traits->AllocSeed;
		SpacerRes = uptake * Traits->AllocSpacer;

		int pweek = CEnvir::week;

		//during the seed-production-weeks
		if ((pweek >= Traits->FlowerWeek) && (pweek < Traits->DispWeek)) {
			//seed production
			dm_seeds = max(0.0, Traits->growth * SeedRes);
			mRepro += dm_seeds;

			//clonal growth
			dummy1 = max(0.0, min(SpacerRes, uptake - SeedRes)); // for large AllocSeed, resources may be < SpacerRes, then only take remaining resources
			mReproRamets += max(0.0, Traits->growth * dummy1);
			VegRes = uptake - SeedRes - dummy1;

		} else {
			VegRes = uptake - SpacerRes;
			mReproRamets += max(0.0, Traits->growth * SpacerRes);
		}

	} else
		VegRes = uptake;
	return VegRes;
}

//-----------------------------------------------------------------------------
/**
 * Growth of the spacer.
 */
void CPlant::SpacerGrow() {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC

	double mGrowSpacer = 0;
	int SpacerListSize = this->growingSpacerList.size();

	if (SpacerListSize == 0)
		return;
	if ((mReproRamets > 0)) {
		mGrowSpacer = (mReproRamets / SpacerListSize); //resources for one spacer

		for (int g = 0; g < (SpacerListSize); g++) { //loop for all growing Spacer of one plant
			CPlant* Spacer = this->growingSpacerList[g];

			double lengthtogrow = Spacer->SpacerlengthToGrow;
			lengthtogrow -= (mGrowSpacer / Traits->mSpacer); //spacer growth
			Spacer->SpacerlengthToGrow = max(0.0, lengthtogrow);

			//Estab for all growing Spacers in the last week of the year
			if ((CEnvir::week == CEnvir::WeeksPerYear)
					&& (Spacer->SpacerlengthToGrow > 0)) {
				double direction = Spacer->Spacerdirection;
				double complDist = Spacer->Spacerlength; //should be positive
				double dist = (complDist - Spacer->SpacerlengthToGrow);
				double CmToCell = 1.0 / SRunPara::RunPara.CellScale();
				int x2 = CEnvir::Round(
						this->cell->x + cos(direction) * dist * CmToCell);
				int y2 = CEnvir::Round(
						this->cell->y + sin(direction) * dist * CmToCell);

				/// \todo change boundary conditions
				//Boundary conditions (types MUST match exactly)
				Boundary(x2, y2);
//             CGrid::Boundary(x2,y2);

				//beachte dass CellNum nicht immer == CellSize
				Spacer->xcoord = x2 / CmToCell;
				Spacer->ycoord = y2 / CmToCell;
				Spacer->SpacerlengthToGrow = 0;
			}  //end if pweek==WeeksPerYear
		}   //end List of Spacers
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
void CPlant::Grow2() //grow plant one timestep
{
	double dm_shoot, dm_root, alloc_shoot;
//   double Assim_shoot, Resp_shoot, Assim_root, Resp_root;
	double LimRes, ShootRes, RootRes, VegRes;

	/********************************************/
	/*  dm/dt = growth*(c*m^p - m^q / m_max^r)  */
	/********************************************/

	//which resource is limiting growth ?
	LimRes = min(Buptake, Auptake);   //two layers
	VegRes = ReproGrow(LimRes);

	//allocation to shoot and root growth
	alloc_shoot = Buptake / (Buptake + Auptake); //allocation coefficient

	ShootRes = alloc_shoot * VegRes;
	RootRes = VegRes - ShootRes;

	//Shoot growth
	dm_shoot = this->ShootGrow(ShootRes);

	//Root growth
	dm_root = this->RootGrow(RootRes);

	mshoot += dm_shoot;
	mroot += dm_root;

	if (stressed())
		++stress;
	else if (stress > 0)
		--stress;
}

//-----------------------------------------------------------------------------
/**
 shoot growth
 dm/dt = growth*(c*m^p - m^q / m_max^r)
 */
double CPlant::ShootGrow(double shres) {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC

	double Assim_shoot, Resp_shoot;
	double p = 2.0 / 3.0, q = 2.0, r = 4.0 / 3.0; //exponents for growth function
	Assim_shoot = Traits->growth * min(shres, Traits->Gmax * Ash_disc); //growth limited by maximal resource per area -> similar to uptake limitation
	Resp_shoot = Traits->growth * Traits->SLA * pow(Traits->LMR, p)
			* Traits->Gmax * pow(mshoot, q) / pow(Traits->MaxMass, r); //respiration proportional to mshoot^2
	return max(0.0, Assim_shoot - Resp_shoot);
}

//-----------------------------------------------------------------------------
/**
 root growth

 dm/dt = growth*(c*m^p - m^q / m_max^r)
 */
double CPlant::RootGrow(double rres) {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC

	double Assim_root, Resp_root;
	double q = 2.0, r = 4.0 / 3.0; //exponents for growth function
	Assim_root = Traits->growth * min(rres, Traits->Gmax * Art_disc); //growth limited by maximal resource per area -> similar to uptake limitation
	Resp_root = Traits->growth * Traits->Gmax * Traits->RAR * pow(mroot, q)
			/ pow(Traits->MaxMass, r);  //respiration proportional to root^2

	return max(0.0, Assim_root - Resp_root);
}

//-----------------------------------------------------------------------------
/**
 identify resource stressing situation

 \return true if plant is stressed

 \note May et al. (2009) documented this part as
 \verbatim
 delta_res<Traits->mThres*Ash/rt_disc*Traits->Gmax
 \endverbatim
 but his code was
 \code
 (Auptake<Traits->mThres*Ash_disc*Traits->Gmax*2)
 || (Buptake<Traits->mThres*Art_disc*Traits->Gmax*2);
 \endcode
 as described in his diploma thesis

 \date 2012-07-31  code splitted by KK
 */
bool CPlant::stressed() {
//   return (Auptake<Traits->mThres*Ash_disc*Traits->Gmax)
//       || (Buptake<Traits->mThres*Art_disc*Traits->Gmax);
	return (Auptake / 2.0 < minresA()) || (Buptake / 2.0 < minresB());
}
//-----------------------------------------------------------------------------
/**
 * Kill plant depending on stress level and base mortality. Stochastic process.
 */
void CPlant::Kill() {
	// resource deficiency mortality
	// pmin->random background mortality
	const double pmin = SRunPara::RunPara.mort_base;  // 0.007;
	assert(Traits->memory != 0);
	double pmort = (double) stress / Traits->memory + pmin; // stress mortality + random background mortality
	if (CEnvir::rand01() < pmort)
		dead = true;
}
//-----------------------------------------------------------------------------
/**
 * Litter decomposition with deletion at 10mg.
 */
void CPlant::DecomposeDead() {
	const double minmass = 10; // mass at which dead plants are removed
	const double rate = SRunPara::RunPara.LitterDecomp; //0.5;

	if (dead) {
		mRepro = 0;
		mshoot *= rate;
		mroot *= rate;
		if (GetMass() < minmass)
			remove = true;
	}
}

//-----------------------------------------------------------------------------
/**
 If the plant is alive and it is dispersal time, the function returns
 the number of seeds produced during the last weeks.
 Subsequently the allocated resources are reset to zero.
 */
int CPlant::GetNSeeds() {
	int NSeeds = 0;
	double prop_seed = 1.0;  //proportion of reproductive biomass that are seeds

	if (!dead) {
		if ((mRepro > 0) && (CEnvir::week > Traits->DispWeek)) {
			NSeeds = floor(mRepro * prop_seed / Traits->SeedMass);
			mRepro = 0;
			if (Age > Traits->MaxAge - 1)
				this->dead = true; // kill senescent plants after they reproduced the last time
		}
	}

	yearlyFecundity += NSeeds;

	return NSeeds;
}

//------------------------------------------------
/**
 returns the number of new spacer to set: currently
 - 1 if there are clonal-growth-resources and spacer-lisdt is empty, and
 - 0 otherwise
 \return the number of new spacer to set
 Unlike CPlant::GetNSeeds() no resources are reset due to ongoing growth
 */
int CPlant::GetNRamets() {
	if ((mReproRamets > 0) && (!dead) && (growingSpacerList.size() == 0))
		return 1;
	return 0;
}

//-----------------------------------------------------------------------------
/**
 Remove half shoot mass and seed mass from a plant.

 \return mass that was removed
 */
double CPlant::RemoveMass() {
	double mass_removed = 0;
	//proportion of mass removed (0.5)
	const double prop_remove = SRunPara::RunPara.BitSize;
	if (mshoot + mRepro > 1) {   //only remove mass if shoot mas > 1mg
		mass_removed = prop_remove * mshoot + mRepro;
		mshoot *= 1 - prop_remove;
		mRepro = 0;
	}
	return mass_removed;
}

//-----------------------------------------------------------------------------
/**
 Remove root mass from a plant.
 \param prop_remove   proportion of mass to be removed
 \return mass that was removed
 \since belowground herbivory simulations
 */
double CPlant::RemoveRootMass(const double prop_remove) {
//	double mass_removed = 0;
//	if (mroot > 1) {   //only remove mass if root mass > 1mg
//		mass_removed = prop_remove * mroot;
//		mroot -= mass_removed;   //*=1-prop_remove;
//	}
//	return mass_removed;

	double mass_removed = prop_remove * mroot;
	assert(!(mass_removed > mroot));
	mroot -= mass_removed;

	if (mroot == 0) {
		dead = true;
	}

	return mass_removed;
}

//-----------------------------------------------------------------------------
/**
 * Winter dieback of aboveground biomass. Ageing of Plant.
 */
void CPlant::WinterLoss() {
	double prop_remove = SRunPara::RunPara.DiebackWinter;   //0.5;
	mshoot *= 1 - prop_remove;
	mRepro = 0;
	Age++;
}

//-----------------------------------------------------------------------------
double CPlant::Radius_shoot() {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC
	return sqrt(Traits->SLA * pow(Traits->LMR * mshoot, 2.0 / 3.0) / Pi);
}

//-----------------------------------------------------------------------------
double CPlant::Radius_root() {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC
	return sqrt(Traits->RAR * pow(mroot, 2.0 / 3.0) / Pi);
}

//-----------------------------------------------------------------------------
double CPlant::Area_shoot() {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC
	return Traits->SLA * pow(Traits->LMR * mshoot, 2.0 / 3.0);
}

//-----------------------------------------------------------------------------
double CPlant::Area_root() {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC
	return Traits->RAR * pow(mroot, 2.0 / 3.0);
}

//-----------------------------------------------------------------------------
/**
 * Competitive strength of plant.
 * @param layer above- (1) or belowground (2) ZOI
 * @param symmetry Symmetry of competition
 * (symmetric, partial asymmetric, complete asymmetric )
 * \sa SRunPara::RunPara.AboveCompMode
 * \sa SRunPara::RunPara.BelowCompMode
 *
 * @return competitive strength
 * \since revision
 */
double CPlant::comp_coef(const int layer, const int symmetry) const {
	if (SRunPara::RunPara.ITV == on)
		assert(Traits->myTraitType == SPftTraits::individualized); //MSC
	switch (symmetry) {
	case 1:
		if (layer == 1)
			return Traits->Gmax; //CompPowerA();
		if (layer == 2)
			return Traits->Gmax; //CompPowerB();
		break;
	case 2:
		if (layer == 1)
			return mshoot * Traits->CompPowerA();
		if (layer == 2)
			return mroot * Traits->CompPowerB();
		break;
	default:
		cerr << "CPlant::comp_coef() - wrong input";
		exit(3);
	}
	return -1;  //should not be reached
}

//-eof----------------------------------------------------------------------------

