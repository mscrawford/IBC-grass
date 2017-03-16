#include "SPftTraits.h"
#include "CEnvir.h"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

map<string, SPftTraits*> SPftTraits::PftLinkList = map<string, SPftTraits*>();
vector<string> SPftTraits::pftInsertionOrder = vector<string>();

/*
 * Default constructor
 */
SPftTraits::SPftTraits() :
		myTraitType(SPftTraits::species), TypeID(999), name("default"),
		MaxAge(100),
		LMR(0), SLA(0), RAR(1), m0(0), MaxMass(0),
		AllocSeed(0.05), SeedMass(0), Dist(0), Dorm(1), pEstab(0.5),
		Gmax(0), palat(0), memory(0),
		mThres(0.2), growth(0.25), FlowerWeek(16), DispWeek(20),
		clonal(true), PropSex(0.1), meanSpacerlength(17.5), sdSpacerlength(12.5),
		AllocSpacer(0.05), Resshare(true), mSpacer(70)
{

}

/*
 * Constructor copying a given set of traits. This is necessary for IBC-grass.ITV.
 */
SPftTraits::SPftTraits(const SPftTraits& s) :
		myTraitType(s.myTraitType), TypeID(s.TypeID), name(s.name),
		MaxAge(s.MaxAge),
		LMR(s.LMR), SLA(s.SLA), RAR(s.RAR), m0(s.m0), MaxMass(s.MaxMass),
		AllocSeed(s.AllocSeed), SeedMass(s.SeedMass), Dist(s.Dist), Dorm(s.Dorm), pEstab(s.pEstab),
		Gmax(s.Gmax), palat(s.palat), memory(s.memory),
		mThres(s.mThres), growth(s.growth), FlowerWeek(s.FlowerWeek), DispWeek(s.DispWeek),
		clonal(s.clonal), PropSex(s.PropSex),
		meanSpacerlength(s.meanSpacerlength), sdSpacerlength(s.sdSpacerlength),
		AllocSpacer(s.AllocSpacer), Resshare(s.Resshare), mSpacer(s.mSpacer)
{
	if (SRunPara::RunPara.ITV == off)
	{
		assert(s.myTraitType == SPftTraits::species);
	}

	if (s.myTraitType == SPftTraits::individualized)
	{
		assert(SRunPara::RunPara.ITV == on);
	}
}

/** MSC
 * Records every "individualized" trait syndrome (or rather, the pointer to it).
 * This is used to remove them after each run. By the end of a 100 year run with 128cm grid cells,
 * there will be about 2GB of memory used. To improve this algorithm (though complicating the code),
 * one could delete the trait syndromes after their final use (i.e. a seed does not germinate, or a
 * plant dies). One must be cognizant to ensure that:
 *				0) all plants use individualized traits (though they do not necessarily have to vary) so that the
 *					destructor doesn't accidentally delete a species level trait. Or, if individualization is off (that is,
 *					no traits are actually being varied), all copies can just be deleted with their parent's (plant or seed) destructor.
 *				1) all seeds pass their traits to the plants they become (enabled by the use of SPftTrait's copy constructor) inside
 *					the plant constructor.
 *				2) all seeds and plants, when they die, remove their own traits.
 *				3) they got rid of the individualPftList (as all individualPfts have a very short life).
 */

//------------------------------------------------------------------------------
/**
 * Get - link for specific PFT
 * @param type PFT asked for
 * @return Object pointer to PFT definition
 */
SPftTraits* SPftTraits::getPftLink(string type)
{

	SPftTraits* traits = NULL;

	map<string, SPftTraits*>::iterator pos = PftLinkList.find(type);

	if (pos == PftLinkList.end())
	{
		cerr << "Type not found: " << type << endl;
		exit(1);
	}

	traits = pos->second;

	if (traits == NULL)
	{
		cerr << "NULL-pointer error\n";
		exit(1);
	}

	return traits;

}

/**
 * Get - the instance (pass by value) of a specific PFT (as defined by its name)
 * @param type PFT asked for
 * @return Object instance defining a PFT.
 */
SPftTraits* SPftTraits::createPftInstanceFromPftType(string type)
{
	SPftTraits* traits = NULL;
	map<string, SPftTraits*>::iterator pos = PftLinkList.find(type);

	if (pos == PftLinkList.end())
	{
		cerr << "Type not found:" << type << endl;
		exit(1);
	}

	traits = new SPftTraits(*pos->second);

	return traits;
}

//-----------------------------------------------------------------------------
/**
 * Read definition of PFTs used in the simulation
 * @param file file containing PFT definitions
 */
void SPftTraits::ReadPFTDef(const string& file)
{
	//Open InitFile
	ifstream InitFile(file.c_str());

	string line;
	getline(InitFile, line); //skip header line
	while (getline(InitFile, line))
	{
		std::stringstream ss(line);

		SPftTraits* traits = new SPftTraits();

		ss >> traits->TypeID >> traits->name >> traits->MaxAge
				>> traits->AllocSeed >> traits->LMR >> traits->m0
				>> traits->MaxMass >> traits->SeedMass >> traits->Dist
				>> traits->pEstab >> traits->Gmax >> traits->SLA
				>> traits->palat >> traits->memory >> traits->RAR
				>> traits->growth >> traits->mThres >> traits->clonal
				>> traits->PropSex >> traits->meanSpacerlength
				>> traits->sdSpacerlength >> traits->Resshare
				>> traits->AllocSpacer >> traits->mSpacer;

		SPftTraits::addPftLink(traits->name, traits);
		SPftTraits::pftInsertionOrder.push_back(traits->name); // MSC
	}
}

/* MSC
 * Vary the current individual's traits, based on a Gaussian distribution with a
 * standard deviation of "ITVsd". Sub-traits that are tied will vary accordingly.
 * Bounds on 1 and -1 ensure that no trait garners a negative value and keep the resulting
 * distribution balanced. Other, trait-specific, requirements are checked as well. (e.g.,
 * LMR cannot be greater than 1, memory cannot be less than 1).
 */
void SPftTraits::varyTraits()
{

	assert(myTraitType == SPftTraits::species);
	assert(SRunPara::RunPara.ITV == on);

	myTraitType = SPftTraits::individualized;
	double dev;

//	cout << "Varying traits! LMR..." << endl;
	double LMR_;
	do
	{
		dev = CEnvir::rng.getGaussian(0, SRunPara::RunPara.ITVsd);
		LMR_ = LMR + (LMR * dev);
	} while (dev < -1.0 || dev > 1.0 || LMR_ < 0 || LMR_ > 1);
	LMR = LMR_;
//	cout << "LMR: " << LMR << endl;

//	cout << "Varying traits! Mass..." << endl;
	double m0_, MaxMass_, SeedMass_, Dist_;
	do
	{
		dev = CEnvir::rng.getGaussian(0, SRunPara::RunPara.ITVsd);
		m0_ = m0 + (m0 * dev);
		MaxMass_ = MaxMass + (MaxMass * dev);
		SeedMass_ = SeedMass + (SeedMass * dev);
		Dist_ = Dist - (Dist * dev);
	} while (dev < -1.0 || dev > 1.0 || m0_ < 0 || MaxMass_ < 0 || SeedMass_ < 0
			|| Dist_ < 0);
	m0 = m0_;
	MaxMass = MaxMass_;
	SeedMass = SeedMass_;
	Dist = Dist_;
//	cout << "m0: " << m0 << endl;
//	cout << "MaxMass: " << MaxMass << endl;
//	cout << "SeedMass: " << SeedMass << endl;
//	cout << "Dist: " << Dist << endl;

	//	cout << "Varying traits! Gmax..." << endl;
	double Gmax_;
	int memory_;
	do
	{
//		cout << "Drawing variate..." << endl;
		dev = CEnvir::rng.getGaussian(0, SRunPara::RunPara.ITVsd);
//		cout << "Setting Gmax ..." << endl;
		Gmax_ = Gmax + (Gmax * dev);
//		cout << "Setting memory..." << endl;
		memory_ = memory - (memory * dev);
//		cout << "Testing values: dev: " << dev << " Gmax: " << Gmax_ << " memory: " << memory_  << endl;
	} while (dev < -1.0 || dev > 1.0 || Gmax_ < 0 || memory_ < 1);
	Gmax = Gmax_;
	memory = memory_;
//	cout << "Gmax: " << Gmax << endl;
//	cout << "Memory: " << memory << endl;

//	cout << "Varying traits! Palat..." << endl;
	double palat_, SLA_;
	do
	{
		dev = CEnvir::rng.getGaussian(0, SRunPara::RunPara.ITVsd);
		palat_ = palat + (palat * dev);
		SLA_ = SLA + (SLA * dev);
	} while (dev < -1.0 || dev > 1.0 || palat_ < 0 || palat_ > 1 || SLA_ < 0);
	palat = palat_;
	SLA = SLA_;
//	cout << "palat: " << palat << endl;
//	cout << "SLA: " << SLA << endl;

	//	cout << "Varying traits! Spacers..." << endl;
	double meanSpacerlength_, sdSpacerlength_;
	do
	{
		dev = CEnvir::rng.getGaussian(0, SRunPara::RunPara.ITVsd);
		meanSpacerlength_ = meanSpacerlength + (meanSpacerlength * dev);
		sdSpacerlength_ = sdSpacerlength + (sdSpacerlength * dev);
	} while (dev < -1.0 || dev > 1.0 || meanSpacerlength_ < 0
			|| sdSpacerlength_ < 0);
	meanSpacerlength = meanSpacerlength_;
	sdSpacerlength = sdSpacerlength_;
//	cout << "meanSpacerlength: " << meanSpacerlength << endl;
//	cout << "sdSpacerlength: " << sdSpacerlength << endl;

//	cout << "Done varying traits!" << endl;

}
