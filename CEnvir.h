/**\file
   \brief definition of environmental classes and result structures PftOut and GridOut
*/
//---------------------------------------------------------------------------
#ifndef environmentH
#define environmentH

#include "CGrid.h"
#include "OutStructs.h"

#include <vector>
#include <fstream>
#include "LCG.h"

struct SSR {
	   static map<string,double> PftSeedRainList; ///<seed rain list
	      static string NameLDDFile1;   ///< Filename of Seed-Output
	      static string NameLDDFile2;   ///< Filename of Seed-Output
	      static string NameLDDFile3;   ///< Filename of Seed-Output
	      static string NameLDDFile4;   ///< Filename of Seed-Output
	      static string NameLDDFile5;   ///< Filename of Seed-Output
//	protected:
	   int NPftSeedSize[3];                  //!< number of PFTs with small, medium, large seeds
	   int NPftClonal[2];                  //!< number of non-clonal and clonal PFTs
	   void GetNPftSeedsize(); //!< count number of PFTs with small, medium an large seeds
	   void GetNPftSeedClonal(); //!< count number of nonclonal and clonal PFTs


   };

//---------------------------------------------------------------------------
/// virtual Basic Results Class with general static simulation parameters
/** The class contains
    - simulation-wide (static) information on
      - Names of in- and output-files,
      - an Random Number Generator (plus some service functions), and
      - a template for above-and belowground resources as well as
      - current simulation status (current year, week etc.)
    - variables storing result information on grid and single pfts
    - functions
      - collecting and writing results to output-files
      - reading-in Resource data
      - core function OneWeek(), running a week of the simulation
   \par time scales of the simulations:
      - 1 step = 1 week
      - 1 year = 30 weeks
*/
class CEnvir{
protected:
   map<string,int> PftSurvTime;    //!< array for survival times of PFTs [years];
   static map<string,long> PftInitList;  //!< list of Pfts used
public:
///seed rain struct
SSR SeedRainGr;
   //   static string NameInitFile; ///< Filename of initial PFT file
//   static string NameClonalPftFile; ///< Filename of clonal Pft-File

   //Output Files
   static string NamePftOutFile;   ///< Filename of Pft-Output
   static string NameGridOutFile;  ///< Filename of Grid-Output
   static string NameSurvOutFile;   ///< Filename of Survival-Output
   static string NameLogFile;      ///< Filename of Log-Entries
   static string NameClonalOutFile; ///< Filename of clonal Output-File

   static vector<double> AResMuster;     //!< mean above-ground resource availability [resource units per cm^2]
   static vector<double> BResMuster;     //!< mean below-ground resource availability [resource units per cm^2]

   static int week;        ///< current week (0-30)
   static int year;        ///< current year
   static int WeeksPerYear;///< nb of weeks per year (constantly at value 30)
   static int NRep;        //!< number of replications -> read from SimFile;
   static int SimNr;       ///< simulation-ID
   static int ComNr;	   ///< Community identifier for multiple parameter settings of the same community.
   static int RunNr;       ///< repitition number

   bool endofrun;			///<end of simulation reached? (flag)
   int init;               ///< flag for simulation issue (init time)
   vector<SPftOut*> PftOutData;   //!< Vector for Pft output data
   vector<SGridOut*> GridOutData;  //!< Vector for Grid output data
   //annual Results variables -clonal
 //  vector<SClonOut*>  ClonOutData; ///< Vector of clonal Output data

  //result variables - non-clonal
   vector<int> ACover;     //!< mean above-ground resource availability [resource units per cm^2]
   vector<int> BCover;     //!< mean below-ground resource availability [resource units per cm^2]
   map<string,double> PftCover;  //!< current Grid-cover of Pfts used
   double NCellsAcover;    ///< Number of Cells shaded by plants on ground

   //Functions
   CEnvir();
   CEnvir(string id);  ///<load saved parameter set and state info
   virtual ~CEnvir();

   //! read in fractal below-ground resource distribution (not used)
   static void ReadLandscape();
   ///reads simulation environment from file
   int GetSim(const int pos=0,string file=SRunPara::NameSimFile);
   /// returns absolute time horizon
   static int GetT(){return (year-1)*WeeksPerYear+week;};
   /// reset time
   static void ResetT(){ year=1;week=0;};
   /// set new week
   static void NewWeek(){week++;if (week>WeeksPerYear){week=1;year++;};};

   /**
    * \name math and random help functions
    */
   ///@{
   ///round a double value
   inline static int Round(const double& a){return (int)floor(a+0.5);};
   ///get a uniformly distributed random number (0-n)
   inline static int nrand(int n){return combinedLCG()*n;};
   ///get a uniformly distributed random number (0-1)
   inline static double rand01(){return combinedLCG();};//RandNumGen.rand_halfclosed01()
   ///get a uniformly distributed random number (0-1)
   inline static double normrand(double mean,double sd){return normcLCG(mean,sd);};//RandNumGen.rand_halfclosed01()
   ///@}
/**
 * \name core simulation functions (virtual)
 * Functions needed for simulation runs.
 * To be defined in inheriting classes.
 */
///@{
   virtual void InitRun();   ///<init a new run
//  virtual void clearResults(); ///<reset result storage
   virtual void OneWeek()=0;  //!< calls all weekly processes
   virtual void OneYear()=0; ///<runs one year in default mode
   virtual void OneRun()=0;  ///<runs one simulation run in default mode
   ///collect and write Output to an output-file
   virtual void GetOutput()=0;//PftOut& PftData, GridOut& GridData)=0;
   virtual void Save(string ID)=0;//<save current grid state
   //! returns number of surviving PFTs
   /*! a PFT is condsidered as a survivor if individuals or
        at least seeds are still there
   */
   virtual int PftSurvival()=0;
///@}
/**
 * \name File Output
 */
///@{
   void WriteOFiles();///<collect file output
   //! write suvival time of each PFT to the output file
   /*! For each scenario the year in which each PFT went extinct is written
     to the output file. If the year is equal to the simulation time the PFT survived
     In addition the number or surviving PFTs and the mean Shannon index of the last
     quarter of the simulation is saved to the file.
   */
   void WriteSurvival();
   ///write survival data while adding an index to the file name
   void WriteSurvival(string str);
   void WriteSurvival(int runnr, int simnr, int comnr);
   //! writes detailed data for the modeled community to output file
   void WriteGridComplete(bool allYears=true);
   //! writes detailed data for each PFT to output file
   void WritePftComplete(bool allYears=true);
   void WriteclonalOutput();   ///< write clonal results collected last
   void WritePftSeedOutput();  ///< write ldd-seed information
   ///add string1 to file - for logging
   void AddLogEntry(string,string);
   ///add string1 to file - for logging
   void AddLogEntry(float,string);
///@}

   /// get mean Shannon diversity over several years
   double GetMeanShannon(int years);
   /// get mean number of types
   double GetMeanNPFT(int years);
          /// get mean Pop size of type (last x years)
   double GetMeanPopSize(string pft,int x);
      ///get current PopSize of type pft
   double GetCurrPopSize(string pft);

   static string toString();
   static string headerToString();
};
//---------------------------------------------------------------------------
#endif
