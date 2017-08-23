
#include <cassert>
#include <iostream>

#include "RunPara.h"
#include "CEnvir.h"

// Input Files
std::string SRunPara::NamePftFile; 							// trait file for experiment species
std::string SRunPara::NameSimFile = "data/in/SimFile.txt"; 	// file with simulation scenarios
std::string SRunPara::outputPrefix = "default";

SRunPara SRunPara::RunPara;

SRunPara::SRunPara() :
		weekly(0), ind_out(0), PFT_out(2), srv_out(1), trait_out(1), meta_out(1),
		AboveCompMode(asympart), BelowCompMode(sym), Version(version1), mode(communityAssembly),
		Tmax_monoculture(10),
		ITV(off), ITVsd(0),
		GridSize(100),
		Tmax(100),
		mort_seeds(0.5), DiebackWinter(0.5), mort_base(0.007), LitterDecomp(0.5),
		meanARes(100), meanBRes(100),
		EstabRamet(1),
		GrazProb(0), PropRemove(0), BitSize(0.5), MassUngraz(15300),
		BelGrazProb(0), BelGrazPerc(0), BelGrazResidualPerc(0),
		CutHeight(0), NCut(0),
		CatastrophicDistYear(50), CatastrophicDistWeek(21),
		CatastrophicPlantMortality(0),
		Aampl(0), Bampl(0),
		SeedInput(0), SeedRainType(0)
{

}

//-----------------------------------------------------------------------------

std::string SRunPara::getSimID()
{

	std::string s =
			std::to_string(CEnvir::SimNr) + "_" +
			std::to_string(CEnvir::ComNr) + "_" +
			std::to_string(CEnvir::RunNr);

	return s;
}
