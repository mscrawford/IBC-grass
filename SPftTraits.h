/*
 * SPftTraits.h
 *
 *  Created on: 21.04.2014
 *      Author: KatrinK
 */

#ifndef SPFTTRAITS_H_
#define SPFTTRAITS_H_

#include <map>
#include <string>
#include <vector>
using namespace std;

/**
 * Structure to store all PFT Parameters
 *
 */
class SPftTraits {
public:
	   static map<string,SPftTraits*> PftLinkList;  //!< links of Pfts(SPftTrais) used
	   static vector<string> pftInsertionOrder; // Do I need to explicitly call the constructor here?
//	   static vector<SPftTraits*> PftList;        //!< List for Pft parameters
//general
	   int TypeID;     //!< PFT ID same number for all individuals of one PFT
	   string name;   ///< name of functional type
//	   int N0;         ///< number of initial individuals
	   int MaxAge;     ///< maximum age of plants

//morphology
	   double LMR;     //!< leaf mass ratio (LMR) (leaf mass per shoot mass) [0;1] 1 -> only leafs, 0 -> only stem
	   double SLA;     //!< specific leaf area (SLA) equal to cshoot in the model description (leaf area per leaf mass)
	   double RAR;     //!< root area ratio (root area per root mass) equal to croot in the model description
	   double m0;      ///< initial masses of root and shoot
	   double MaxMass; //!< maximum individual mass

//seed reproduction
	   double AllocSeed;  //!< constant proportion that is allocated to seeds between FlowerWeek and DispWeek
	   double SeedMass;   //!< Seed mass (mass of ONE seed)
	   double Dist;    //!< mean dispersal distance (and standard deviation of the dispersal kernel
	   int    Dorm;    //!< maximum seed longevity
	   double pEstab;  //!< annual probability of establishment

//competitive strength
	   //!< maximal resource utilization per ZOI area per time step
	   /*!< (optimum uptake for two layers : LimRes=2*Gmax)
	   (optimum uptake for one layer : LimRes=Gmax)
	   */
	   double Gmax;
	   //! above-ground competitive ability
	   inline double CompPowerA()const {return 1.0/LMR*Gmax;};
	   //! below-ground competitive ability
	   inline double CompPowerB()const {return Gmax;};

//grazing response
	   //! fraction of above-ground biomass removal if a plant is grazed
	   inline double GrazFac()const {return 1.0/LMR*palat;};
	   double palat;   //!< Palatability -> susceptability towards grazing

//stress tolerance
	   int    memory;    //!< equal to surv_max in the model description -> maximal time of survival under stress
	   double mThres;  //!< Fraction of maximum uptake that is considered as resource stress
	   double growth;  //!< concersion rate  resource -> biomass [mass/resource unit]

//phenology
	   int    FlowerWeek; //!< week of start of seed oroduction
	   int    DispWeek;   //!< week of seed dispersal (and end of seed production)
//clonality...
	   bool clonal;///< is this plant clonal at all?
	   ///allocation to sexual reproduction during time of seed production
	   double PropSex;
	   double meanSpacerlength;  ///<mean spacer length [cm]
	   double sdSpacerlength;    ///<sd spacer length [cm]
	   double AllocSpacer;       ///<proportion of ressource invested in ramet growth -> for annual and biannual species this should not be=AllocSeed, because this is then way to high
	   bool Resshare;            ///<do established ramets share their ressources?
	   double mSpacer;  ///<resources for 1 cm spacer (default=70)
//functions..
	   SPftTraits();
	virtual ~SPftTraits();
	static void ReadPFTDef(const string& file, int n=-1);
	   ///get basic type according to string
	   static SPftTraits* getPftLink(string type);//{return PftLinkList.find(type)->second;};
	   static void addPftLink(string type,SPftTraits* link){PftLinkList[type]=link;};
       static int getNPFTInit(){return PftLinkList.size();};
};

#endif /* SPFTTRAITS_H_ */
