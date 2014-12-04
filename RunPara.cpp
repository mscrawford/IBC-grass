/**\file
\brief constructor of struct SRunPara and Initialization of static Variables
*/
//---------------------------------------------------------------------------
//#pragma hdrstop

#include "RunPara.h"

//#include <iostream>
#include <cstdlib>
#include <sstream>
//---------------------------------------------------------------------------
//#pragma package(smart_init)
//Input Files
   std::string SRunPara::NamePftFile="Input/PftTraits.experiment.txt"; // trait file for experiment species
   std::string SRunPara::NameSimFile = "Input/SimFile.txt";  //file with simulation scenarios

SRunPara SRunPara::RunPara=SRunPara();
//-------------------------------------------------------------------
SRunPara::SRunPara():Version(version1),AboveCompMode(sym),BelowCompMode(sym),
  mort_base(0.007),LitterDecomp(0.5),DiebackWinter(0.5),EstabRamet(1),
  GridSize(128),CellNum(128),Tmax(10), invasionTmax(10), NPft(81),GrazProb(0),PropRemove(0.5),BitSize(0.5),MassUngraz(15300),
  BelGrazProb(0),BelPropRemove(0),BelGrazMode(0),BGThres(1),HetBG(false),
  CutMass(5000),NCut(0),torus(true),
  DistAreaYear(0),AreaEvent(0.1),mort_seeds(0.5),meanARes(100),meanBRes(100),Rootherb(false),
  Aampl(0),Bampl(0),PftFile("Input/PftTraits.experiment.txt"),SeedInput(0),SeedRainType(0){}

/**
\note  es fehlen: CellNum,NPft
\author KK
\date  120831
*/
std::string SRunPara::asString(){
  std::stringstream mystream;
      mystream
      <<"\n"<<Version<<"\t"<<AboveCompMode<<"\t"<<BelowCompMode
      <<"\t"<<GridSize <<"\t"<<Tmax        <<"\t"<<torus
      <<"\t"<<mort_seeds<<"\t"<<EstabRamet
      <<"\t"<<mort_base<<"\t"<<LitterDecomp<<"\t"<<DiebackWinter
      <<"\nRes: \t"<<meanARes     <<"\t"<<meanBRes
      <<"\nGrazing:\t"<<GrazProb    <<"\t"<<PropRemove <<"\t"<<BitSize
      <<"\nBGHerb:\t"<<BelGrazProb <<"\t"<<BelPropRemove
      <<"\t"<<BelGrazMode  <<"\t"<<BGThres   <<"\t"<<HetBG
      <<"\nCutting:\t"<<NCut         <<"\t"<<CutMass  //<<"\nCutLeave:\t"<<CutLeave
      <<"\nTrampling:\t"<<DistAreaYear<<"\t"<<AreaEvent
      <<"\t"<<DistAreaYear<<"\t"<<AreaEvent
//      <<"\t"<<mort_seeds
 //     <<"\nspecies:\t"<<species   <<"\nWaterLevel:\t"<<WaterLevel
 //     <<"\nWLseason:\t"<<WLseason <<"\nWLsigma:\t"<<WLsigma
 //     <<"\nchangeVal:\t"<<changeVal<<"\n Migration:\t"<<Migration
      <<"\n"<<SRunPara::NamePftFile;
 return mystream.str();
}//end print
void SRunPara::setRunPara(std::string def){
	using namespace std;
	stringstream dummi; dummi<<def;
	string d;

//  dummi.clear();
//  dummi>>Version;  //forum entries suggest BB6-Bug here
                     //probably BB can't cope with enums
  dummi>>d;
  switch(atoi(d.c_str())){
  case 0: Version=version1;break;
  case 1:Version=version2;break;
  case 2:Version=version3;break;
  default:break;
  }
  dummi>>d;//AboveCompMode=atoi(d.c_str());
  switch(atoi(d.c_str())){
  case 0: AboveCompMode=sym;break;
  case 1: AboveCompMode=asympart;break;
  case 2: AboveCompMode=asymtot;break;
  default:break;
  }
  dummi>>d;//BelowCompMode=atoi(d.c_str());
  switch(atoi(d.c_str())){
  case 0: BelowCompMode=sym;break;
  case 1: BelowCompMode=asympart;break;
  case 2: BelowCompMode=asymtot;break;
  default:break;
  }
  dummi>> GridSize; CellNum=GridSize;
  dummi>> Tmax>> torus;
  dummi>>mort_seeds>> EstabRamet>>mort_base>>LitterDecomp>>DiebackWinter;
  dummi>> GrazProb>> PropRemove>>BitSize>> BelGrazProb >>BelPropRemove>> BelGrazMode
       >> BGThres>> HetBG>> NCut>> CutMass>> meanARes>> meanBRes>>DistAreaYear
       >> AreaEvent  >>PftFile;
}

std::string SRunPara::getFileID() {
	std::stringstream mystream; unsigned pos = std::max(NamePftFile.find("/")+1,NamePftFile.find("\\")+2);
	// ares bres graz SR file
    mystream<<this->GridSize<<'_'<<this->meanARes<<'_'<<this->meanBRes<<'_'
    		<<this->GrazProb<<'_'<<this->SeedInput<<'_'<<this->NamePftFile.substr(pos,NamePftFile.find(".txt")-pos);
	return mystream.str();
}
//eof  ---------------------------------------------------------------------
