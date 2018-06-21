#include <cassert>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>

#include "Traits.h"
#include "Environment.h"

using namespace std;

map< string, unique_ptr<Traits> > Traits::pftTraitTemplates = map< string, unique_ptr<Traits> >();

vector<string> Traits::pftInsertionOrder = vector<string>();

/*
 * Default constructor
 */
Traits::Traits() :
        myTraitType(Traits::species), PFT_ID("EMPTY"),
        LMR(-1), SLA(-1), RAR(1), m0(-1), maxMass(-1),
        allocSeed(0.05), seedMass(-1), dispersalDist(-1), pEstab(0.5),
        Gmax(-1), palat(-1), memory(-1),
        mThres(0.2), growth(0.25), flowerWeek(16), dispersalWeek(20),
        clonal(false), meanSpacerlength(0), sdSpacerlength(0),
        allocSpacer(0), resourceShare(false), mSpacer(0)
{

}

/*
 * Copy constructor
 */
Traits::Traits(const Traits& s) :
        myTraitType(s.myTraitType), PFT_ID(s.PFT_ID),
        LMR(s.LMR), SLA(s.SLA), RAR(s.RAR), m0(s.m0), maxMass(s.maxMass),
        allocSeed(s.allocSeed), seedMass(s.seedMass), dispersalDist(s.dispersalDist), pEstab(s.pEstab),
        Gmax(s.Gmax), palat(s.palat), memory(s.memory),
        mThres(s.mThres), growth(s.growth), flowerWeek(s.flowerWeek), dispersalWeek(s.dispersalWeek),
        clonal(s.clonal), meanSpacerlength(s.meanSpacerlength), sdSpacerlength(s.sdSpacerlength),
        allocSpacer(s.allocSpacer), resourceShare(s.resourceShare), mSpacer(s.mSpacer)
{

}

/**
 * Retrieve a deep-copy of that PFT's basic trait set
 */
unique_ptr<Traits> Traits::createTraitSetFromPftType(string type)
{
    const auto pos = pftTraitTemplates.find(type);

    assert(pos != pftTraitTemplates.end() && "Trait type not found");

    return (make_unique<Traits>(*pos->second));
}

/**
 * Retrieve a deep-copy some arbitrary trait set (for plants dropping seeds)
 */
unique_ptr<Traits> Traits::copyTraitSet(const unique_ptr<Traits> & t)
{
    return (make_unique<Traits>(*t));
}

//-----------------------------------------------------------------------------
/**
 * Read definition of PFTs used in the simulation
 * @param file file containing PFT definitions
 */
void Traits::ReadPFTDef(const string& file)
{
    //Open InitFile
    ifstream InitFile(file.c_str());

    string line;
    getline(InitFile, line); // skip header line
    while (getline(InitFile, line))
    {
        std::stringstream ss(line);

        unique_ptr<Traits> traits(new Traits);

        ss >> traits->PFT_ID
                >> traits->allocSeed >> traits->LMR >> traits->m0
                >> traits->maxMass >> traits->seedMass >> traits->dispersalDist
                >> traits->pEstab >> traits->Gmax >> traits->SLA
                >> traits->palat >> traits->memory >> traits->RAR
                >> traits->growth >> traits->mThres >> traits->clonal
                >> traits->meanSpacerlength >> traits->sdSpacerlength >> traits->resourceShare
                >> traits->allocSpacer >> traits->mSpacer;

        Traits::pftInsertionOrder.push_back(traits->PFT_ID);

        Traits::pftTraitTemplates.insert(std::make_pair(traits->PFT_ID, std::move(traits)));
    }
}

/* MSC
 * Vary the current individual's traits, based on a Gaussian distribution with a
 * standard deviation of "ITVsd". Sub-traits that are tied will vary accordingly.
 * Bounds on 1 and -1 ensure that no trait garners a negative value and keep the resulting
 * distribution balanced. Other, trait-specific, requirements are checked as well. (e.g.,
 * LMR cannot be greater than 1, memory cannot be less than 1).
 */
void Traits::varyTraits()
{

    assert(myTraitType == Traits::species);
    assert(Parameters::parameters.ITV == on);

    myTraitType = Traits::individualized;
    double dev;

    double LMR_;
    do
    {
        dev = Environment::rng.getGaussian(0, Parameters::parameters.ITVsd);
        LMR_ = LMR + (LMR * dev);
    } while (dev < -1.0 || dev > 1.0 || LMR_ < 0 || LMR_ > 1);
    LMR = LMR_;

    double m0_, MaxMass_, SeedMass_, Dist_;
    do
    {
        dev = Environment::rng.getGaussian(0, Parameters::parameters.ITVsd);
        m0_ = m0 + (m0 * dev);
        MaxMass_ = maxMass + (maxMass * dev);
        SeedMass_ = seedMass + (seedMass * dev);
        Dist_ = dispersalDist - (dispersalDist * dev);
    } while (dev < -1.0 || dev > 1.0 || m0_ < 0 || MaxMass_ < 0 || SeedMass_ < 0 || Dist_ < 0);
    m0 = m0_;
    maxMass = MaxMass_;
    seedMass = SeedMass_;
    dispersalDist = Dist_;

    double Gmax_;
    int memory_;
    do
    {
        dev = Environment::rng.getGaussian(0, Parameters::parameters.ITVsd);
        Gmax_ = Gmax + (Gmax * dev);
        memory_ = memory - (memory * dev);
    } while (dev < -1.0 || dev > 1.0 || Gmax_ < 0 || memory_ < 1);
    Gmax = Gmax_;
    memory = memory_;

    double palat_, SLA_;
    do
    {
        dev = Environment::rng.getGaussian(0, Parameters::parameters.ITVsd);
        palat_ = palat + (palat * dev);
        SLA_ = SLA + (SLA * dev);
    } while (dev < -1.0 || dev > 1.0 || palat_ < 0 || SLA_ < 0 || palat_ > 1 || SLA_ > 1);
    palat = palat_;
    SLA = SLA_;

    double meanSpacerlength_, sdSpacerlength_;
    do
    {
        dev = Environment::rng.getGaussian(0, Parameters::parameters.ITVsd);
        meanSpacerlength_ = meanSpacerlength + (meanSpacerlength * dev);
        sdSpacerlength_ = sdSpacerlength + (sdSpacerlength * dev);
    } while (dev < -1.0 || dev > 1.0 || meanSpacerlength_ < 0 || sdSpacerlength_ < 0);
    meanSpacerlength = meanSpacerlength_;
    sdSpacerlength = sdSpacerlength_;

}
