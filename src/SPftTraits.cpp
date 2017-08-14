#include "SPftTraits.h"
#include "CEnvir.h"

#include <cassert>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>

using namespace std;

map< string, unique_ptr<SPftTraits> > SPftTraits::pftTraitTemplates = map< string, unique_ptr<SPftTraits> >();

vector<string> SPftTraits::pftInsertionOrder = vector<string>();

/*
 * Default constructor
 */
SPftTraits::SPftTraits() :
		myTraitType(SPftTraits::species), name("EMPTY"),
		LMR(-1), SLA(-1), RAR(1), m0(-1), MaxMass(-1),
		AllocSeed(0.05), SeedMass(-1), Dist(-1), Dorm(1), pEstab(0.5),
		Gmax(-1), palat(-1), memory(-1),
		mThres(0.2), growth(0.25), FlowerWeek(16), DispWeek(20),
		clonal(false), PropSex(0.1), meanSpacerlength(0), sdSpacerlength(0),
		AllocSpacer(0), Resshare(false), mSpacer(0)
{

}

/*
 * Copy constructor
 */
SPftTraits::SPftTraits(const SPftTraits& s) :
		myTraitType(s.myTraitType), name(s.name),
		LMR(s.LMR), SLA(s.SLA), RAR(s.RAR), m0(s.m0), MaxMass(s.MaxMass),
		AllocSeed(s.AllocSeed), SeedMass(s.SeedMass), Dist(s.Dist), Dorm(s.Dorm), pEstab(s.pEstab),
		Gmax(s.Gmax), palat(s.palat), memory(s.memory),
		mThres(s.mThres), growth(s.growth), FlowerWeek(s.FlowerWeek), DispWeek(s.DispWeek),
		clonal(s.clonal), PropSex(s.PropSex),
		meanSpacerlength(s.meanSpacerlength), sdSpacerlength(s.sdSpacerlength),
		AllocSpacer(s.AllocSpacer), Resshare(s.Resshare), mSpacer(s.mSpacer)
{

}

/**
 * Retrieve a deep-copy of that PFT's basic trait set
 */
unique_ptr<SPftTraits> SPftTraits::createTraitSetFromPftType(string type)
{
	const auto pos = pftTraitTemplates.find(type);

	assert(pos != pftTraitTemplates.end() && "Trait type not found");

	return (make_unique<SPftTraits>(*pos->second));
}

/**
 * Retrieve a deep-copy some arbitrary trait set (for plants dropping seeds)
 */
unique_ptr<SPftTraits> SPftTraits::copyTraitSet(const unique_ptr<SPftTraits> & t)
{
	return (make_unique<SPftTraits>(*t));
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

		unique_ptr<SPftTraits> traits(new SPftTraits);

		ss >> traits->name
				>> traits->AllocSeed >> traits->LMR >> traits->m0
				>> traits->MaxMass >> traits->SeedMass >> traits->Dist
				>> traits->pEstab >> traits->Gmax >> traits->SLA
				>> traits->palat >> traits->memory >> traits->RAR
				>> traits->growth >> traits->mThres >> traits->clonal
				>> traits->PropSex >> traits->meanSpacerlength
				>> traits->sdSpacerlength >> traits->Resshare
				>> traits->AllocSpacer >> traits->mSpacer;

		SPftTraits::pftInsertionOrder.push_back(traits->name);

		SPftTraits::pftTraitTemplates.insert(std::make_pair(traits->name, std::move(traits)));
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
	} while (dev < -1.0 || dev > 1.0 || m0_ < 0 || MaxMass_ < 0 || SeedMass_ < 0 || Dist_ < 0);
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
	} while (dev < -1.0 || dev > 1.0 || palat_ < 0 || SLA_ < 0);
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
	} while (dev < -1.0 || dev > 1.0 || meanSpacerlength_ < 0 || sdSpacerlength_ < 0);
	meanSpacerlength = meanSpacerlength_;
	sdSpacerlength = sdSpacerlength_;
//	cout << "meanSpacerlength: " << meanSpacerlength << endl;
//	cout << "sdSpacerlength: " << sdSpacerlength << endl;

//	cout << "Done varying traits!" << endl;

}
