/*
 * SPftTraits.cpp
 *
 *  Created on: 21.04.2014
 *      Author: KatrinK
 */

#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <sstream>

#include "SPftTraits.h"
#include "CEnvir.h"

SPftTraits::SPftTraits() :
		TypeID(999), name("default"), MaxAge(100), AllocSeed(0.05), LMR(0), m0(
				0), MaxMass(0), SeedMass(0), Dist(0), pEstab(0.5), Gmax(0), memory(
				0), SLA(0), palat(0), RAR(1), growth(0.25), mThres(0.2), Dorm(
				1), FlowerWeek(16), DispWeek(20), PropSex(0.1), meanSpacerlength(
				17.5), sdSpacerlength(12.5), Resshare(true), mSpacer(70), AllocSpacer(
				0.05), clonal(true), myTraitType(SPftTraits::species)
{

} //end constructor

SPftTraits::SPftTraits(const SPftTraits& s) :
		TypeID(s.TypeID), name(s.name), MaxAge(s.MaxAge), AllocSeed(s.AllocSeed), LMR(s.LMR),
		m0(s.m0), MaxMass(s.MaxMass), SeedMass(s.SeedMass), Dist(s.Dist), pEstab(s.pEstab),
		Gmax(s.Gmax), memory(s.memory), SLA(s.SLA), palat(s.palat), RAR(s.RAR), growth(s.growth),
		mThres(s.mThres), Dorm(s.Dorm), FlowerWeek(s.FlowerWeek), DispWeek(s.DispWeek),
		PropSex(s.PropSex), meanSpacerlength(s.meanSpacerlength),sdSpacerlength(s.sdSpacerlength),
		Resshare(s.Resshare), mSpacer(s.mSpacer), AllocSpacer(s.AllocSpacer), clonal(s.clonal),
		myTraitType(s.myTraitType)
{
	if (SRunPara::RunPara.indivVariationVer == off) {
		assert(s.myTraitType == SPftTraits::species);
	}

	if (s.myTraitType == SPftTraits::individualized) {
		assert(SRunPara::RunPara.indivVariationVer == on);
	}
}

SPftTraits::~SPftTraits() {
	// TODO Auto-generated destructor stub
}

//std::vector<SPftTraits*> SPftTraits::PftList;//(CRunPara::RunPara.NPft,new SPftTraits);
//std::vector<SclonalTraits*> SclonalTraits::clonalTraits;//(8,new SclonalTraits());
map<string, SPftTraits*> SPftTraits::PftLinkList = map<string, SPftTraits*>();

// MSC : This vector is for remembering the order of the PFTs if this is an InvasionCriterion experiment.
vector<string> SPftTraits::pftInsertionOrder = vector<string>();

/** MSC
 * Records every "individualized" trait syndrome (or rather, the pointer to it).
 * This is used to remove them after each run. By the end of a 100 year run with 128cm grid cells,
 * there will be about 2GB of memory used. To improve this algorithm (though complicating the code),
 * one could delete the trait syndromes after their final use (i.e. a seed does not germinate, or a
 * plant dies). One must be cognizant to ensure that
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
SPftTraits* SPftTraits::getPftLink(string type) {
	SPftTraits* traits = NULL;
	map<string, SPftTraits*>::iterator pos = PftLinkList.find(type);
	if (pos == (PftLinkList.end()))
		cerr << "type not found:" << type << endl;
	else
		traits = pos->second;
	if (traits == NULL)
		cerr << "NULL-pointer error\n";
	return traits;
}

/**
 * Get - the instance (pass by value) of a specific PFT (as defined by its name)
 * @param type PFT asked for
 * @return Object instance defining a PFT.
 */
SPftTraits* SPftTraits::createPftInstanceFromPftType(string type) {
	SPftTraits* traits = NULL;
	map<string, SPftTraits*>::iterator pos = PftLinkList.find(type);
	if (pos == (PftLinkList.end()))
		cerr << "type not found:" << type << endl;
	else
		traits = new SPftTraits(*pos->second);
	return traits;
}

SPftTraits* SPftTraits::createPftInstanceFromPftLink(SPftTraits* traits) {
	assert(traits != NULL);
	traits = new SPftTraits(*traits);
	return traits;
}

//-----------------------------------------------------------------------------
/**
 * Read definition of PFTs used in the simulation
 * @param file file containing PFT definitions
 * @param n default=-1; in case of monoculture runs, nb of PFT to test
 */
void SPftTraits::ReadPFTDef(const string& file, int n) {
	//delete old definitions
	for (map<string, SPftTraits*>::iterator i = SPftTraits::PftLinkList.begin();
			i != SPftTraits::PftLinkList.end();
			++i)
	{
		delete i->second;
	}
	//delete static pointer vectors
	SPftTraits::PftLinkList.clear();
	SPftTraits::pftInsertionOrder.clear(); // MSC

	//Open InitFile
	ifstream InitFile(file.c_str());
	if (!InitFile.good()) {
		cerr << ("Fehler beim �ffnen InitFile");
		cerr << file.c_str();
		exit(3);
	}
	cout << "InitFile: " << file << endl;

	string line;
	getline(InitFile, line); //skip header line
	//skip first lines if only one Types should be initiated
	if (n > -1) // MSC: This is a broken window.
		for (int x = 0; x < n; x++)
			getline(InitFile, line);

	int dummi1;
	string dummi2; // int PFTtype; string Cltype;

	do {
		// erstelle neue traits
		SPftTraits* traits = new SPftTraits();
//		SclonalTraits* cltraits=new SclonalTraits();
		// get type definitions from file
		InitFile >> dummi1;
		InitFile >> dummi2;
		InitFile >> traits->MaxAge >> traits->AllocSeed >> traits->LMR
				>> traits->m0 >> traits->MaxMass >> traits->SeedMass
				>> traits->Dist >> traits->pEstab >> traits->Gmax >> traits->SLA
				>> traits->palat >> traits->memory >> traits->RAR
				>> traits->growth >> traits->mThres >> traits->clonal
				>> traits->PropSex >> traits->meanSpacerlength
				>> traits->sdSpacerlength >> traits->Resshare >>
				traits->AllocSpacer >> traits->mSpacer;
		traits->name = dummi2; //=cltraits->name
		traits->TypeID = dummi1;

		SPftTraits::addPftLink(dummi2, traits);
		pftInsertionOrder.push_back(dummi2); // MSC
//	    addClLink(dummi2,cltraits);

		if (!InitFile.good() || n > -1) {
			return;
		}
	} while (!InitFile.eof());
} //read PFT defs

/* MSC
 * Unfortunately, because the traits (i.e. LMR, m0, ...) are stored as variables
 * rather than inside a map, or some such, there is no simple way to parameterize
 * which values get varied. The reason is that one can't get a variable (i.e. LMR)
 * from a string (which would be the input value).
 *
 * Don't forget that you'll have to deliver the new traits to the seeds (and they'll in turn
 * create a plant which will then fecund new seeds).
 */
void SPftTraits::varyTraits() {
	assert(myTraitType == SPftTraits::species);
	assert(SRunPara::RunPara.indivVariationVer == on);

	myTraitType = SPftTraits::individualized;
	double dev;

	dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	while (dev < -1.0 || dev > 1.0 || LMR + (LMR * dev) > 1)
		dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
 	LMR = LMR + (LMR * dev);

	dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	while (dev < -1.0 || dev > 1.0)
		dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	m0 = m0 + (m0 * dev);
	MaxMass = MaxMass + (MaxMass * dev);
	SeedMass = SeedMass + (SeedMass * dev);
	Dist = Dist - (Dist * dev);

	dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	while (dev < -1.0 || dev > 1.0)
		dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	Gmax = Gmax + (Gmax * dev);
	memory = (int) round((double) memory - ((double) memory * dev));

	dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	while (dev < -1.0 || dev > 1.0 || palat + (palat * dev) > 1)
		dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	palat = palat + (palat * dev);
	SLA = SLA + (SLA * dev);

	dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	 while (dev < -1.0 || dev > 1.0)
		 dev = CEnvir::normrand(0, SRunPara::RunPara.indivVariationSD);
	 meanSpacerlength = meanSpacerlength + (meanSpacerlength * dev);
	 sdSpacerlength = sdSpacerlength + (sdSpacerlength  * dev);
}

string SPftTraits::toString()
{
	std::stringstream mystream;
	mystream << name << '\t'
			<< AllocSeed << "\t"
			<< LMR << "\t"
			<< m0 << "\t"
			<< MaxMass << "\t"
			<< SeedMass << "\t"
			<< Dist << "\t"
			<< pEstab << "\t"
			<< Gmax << "\t"
			<< SLA << "\t"
			<< palat << "\t"
			<< memory << "\t"
			<< RAR << "\t"
			<< growth << "\t"
			<< mThres << "\t"
			<< clonal << "\t"
			<< PropSex << "\t"
			<< meanSpacerlength << "\t"
			<< sdSpacerlength << "\t"
			<< Resshare << "\t"
			<< AllocSpacer << "\t"
			<< mSpacer << "\t"
			;

	return mystream.str();
}

string SPftTraits::headerToString()
{
	std::stringstream mystream;
	mystream << "PFT" << "\t"
			<< "AllocSeed" << "\t"
			<< "LMR" << "\t"
			<< "m0" << "\t"
			<< "MaxMass" << "\t"
			<< "SeedMass" << "\t"
			<< "Dist" << "\t"
			<< "pEstab" << "\t"
			<< "Gmax" << "\t"
			<< "SLA" << "\t"
			<< "palat" << "\t"
			<< "memory" << "\t"
			<< "RAR" << "\t"
			<< "growth" << "\t"
			<< "mThres" << "\t"
			<< "clonal" << "\t"
			<< "PropSex" << "\t"
			<< "meanSpacerlength" << "\t"
			<< "sdSpacerlength" << "\t"
			<< "Resshare" << "\t"
			<< "AllocSpacer" << "\t"
			<< "mSpacer" << "\t"
		;

	return mystream.str();
}

/**
 initialize clonal traits with default values
 * /
 SclonalTraits::SclonalTraits()  //clonal plant with specifc traits
 :name("default")//,PropSex(0.1),meanSpacerlength(17.5),sdSpacerlength(12.5),
 //Resshare(true),mSpacer(70),AllocSpacer(0.05),clonal(true)
 {
 }
 /* /---------------------------------------------------------------------------
 void SclonalTraits::ReadclonalTraits(char* file)    // not needed anymore -> new initialisation
 {
 std::string sfile=file;
 std::ifstream clonalFile;
 //open parameter file
 if (sfile=="") sfile=(CClonalGridEnvir::NameClonalPftFile);
 clonalFile.open(sfile.c_str());
 if (!clonalFile.good()) {std::cerr<<("Fehler beim �ffnen clonalFile");exit(3); }
 string line1;
 getline(clonalFile,line1);
 //*******************

 //loop for all clonal types
 for (int type=0; type<8; type++)
 {   int num; string name;SclonalTraits* temp=new SclonalTraits;
 //read plant parameter from inputfile
 clonalFile>>num
 //             >>(char*)clonalTraits[type].name
 >>name
 >>temp->PropSex
 >>temp->meanSpacerlength
 >>temp->sdSpacerlength
 >>temp->Resshare
 ;
 temp->name=name;
 clonalTraits.push_back(temp);
 }
 clonalFile.close();
 }
 */
//eof
