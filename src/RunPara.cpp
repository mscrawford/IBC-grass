/**\file
\brief constructor of struct SRunPara and Initialization of static Variables
*/
//---------------------------------------------------------------------------
//#pragma hdrstop

#include "RunPara.h"
#include "CEnvir.h"
#include "SPftTraits.h"

//#include <iostream>
#include <cstdlib>
#include <sstream>
//---------------------------------------------------------------------------
//#pragma package(smart_init)
//Input Files
std::string SRunPara::NamePftFile; // trait file for experiment species
std::string SRunPara::NameSimFile = "data/in/SimFile.txt"; //file with simulation scenarios

SRunPara SRunPara::RunPara=SRunPara();
//-------------------------------------------------------------------
SRunPara::SRunPara() :
		Version(version1), ITV(off), ITVsd(0),
		AboveCompMode(sym), BelowCompMode(sym), PFT(1), SPAT(0), SPATyear(0), COMP(0),
		mort_base(0.007), LitterDecomp(0.5), DiebackWinter(0.5), EstabRamet(1),
		GridSize(128), CellNum(128), Tmax(100), GrazProb(0.2), PropRemove(0.5),
		BitSize(0.5), MassUngraz(15300), BelGrazProb(0), BelGrazStartYear(0),
		BelGrazWindow(0), BelPropRemove(0), BelGrazMode(0),
		BGThres(1), NCut(0), CutHeight(0), catastrophicDistYear(0), torus(true), DistAreaYear(0),
		AreaEvent(0), mort_seeds(0.5), meanARes(100), meanBRes(100),
		Aampl(0), Bampl(0), SeedInput(0), SeedRainType(0)
{
	;
}

/**
 * PFT, SPATyear, ITVsd, SPAT, COMP
\author MSC
\date  19-01-2015
*/
std::string SRunPara::toString(){
	std::stringstream mystream;
	mystream
			<< Version << "\t"
			<< ITVsd << "\t"
			<< Tmax << "\t"
			<< meanARes << "\t"
			<< meanBRes << "\t"
			<< CutHeight << "\t"
			<< GrazProb << "\t"
			<< PropRemove << "\t"
			<< BelGrazProb << "\t"
			<< BelGrazStartYear << "\t"
			<< BelGrazWindow << "\t"
			<< BelGrazMode << "\t"
			<< BelPropRemove << "\t"
			<< catastrophicDistYear << "\t"
			;

	return mystream.str();
}

/*
 * MSC
 * 19-01-2015
 */
std::string SRunPara::headerToString() {
	std::stringstream mystream;
	mystream
			<< "IC_vers" << "\t"
			<< "ITVsd" << "\t"
			<< "Tmax" << "\t"
			<< "ARes" << "\t"
			<< "BRes" << "\t"
			<< "CutHeight" << "\t"
			<< "GrazProb" << "\t"
			<< "PropRemove" << "\t"
			<< "BelGrazProb" << "\t"
			<< "BelGrazStartYear" << "\t"
			<< "BelGrazWindow" << "\t"
			<< "BelGrazMode" << "\t"
			<< "BelPropRemove" << "\t"
			<< "catastrophicDistYear" << "\t"
			;

	return mystream.str();
}

std::string SRunPara::getFileID() {

	string t = to_string(CEnvir::SimNr) + "_" + to_string(CEnvir::ComNr) + "_" + to_string(CEnvir::RunNr);
	return t;

}
//eof  ---------------------------------------------------------------------
