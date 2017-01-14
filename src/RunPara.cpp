/**\file
\brief constructor of struct SRunPara and Initialization of static Variables
*/
//---------------------------------------------------------------------------
//#pragma hdrstop

#include "RunPara.h"
#include "CEnvir.h"

//Input Files
std::string SRunPara::NamePftFile; // trait file for experiment species
std::string SRunPara::NameSimFile = "data/in/SimFile.txt"; //file with simulation scenarios

SRunPara SRunPara::RunPara = SRunPara();

//-------------------------------------------------------------------
SRunPara::SRunPara() :
		Version(version1), ITV(off), ITVsd(0),
		weekly(0), ind_out(0),
		AboveCompMode(sym), BelowCompMode(sym),
		mort_base(0.007), LitterDecomp(0.5), DiebackWinter(0.5), EstabRamet(1),
		GridSize(128), CellNum(128), Tmax(100), GrazProb(0), PropRemove(0),
		BitSize(0.5), MassUngraz(15300), BelGrazProb(0), BelGrazStartYear(0),
		BelGrazWindow(0), BelGrazGrams(0), BelGrazMode(0),
		NCut(0), CutHeight(0), catastrophicDistYear(0), torus(true), DistAreaYear(0),
		AreaEvent(0), mort_seeds(0.5), meanARes(100), meanBRes(100),
		Aampl(0), Bampl(0), SeedInput(0), SeedRainType(0),
		CatastrophicPlantMortality(0), CatastrophicSeedMortality(0)
{
	;
}

std::string SRunPara::getFileID()
{
	std::string s =
			std::to_string(CEnvir::SimNr) + "_" +
			std::to_string(CEnvir::ComNr);
	return s;
}

std::string SRunPara::getSimID()
{
	std::string s =
			std::to_string(CEnvir::SimNr) + "_" +
			std::to_string(CEnvir::ComNr) + "_" +
			std::to_string(CEnvir::RunNr);

	return s;
}
//eof  ---------------------------------------------------------------------
