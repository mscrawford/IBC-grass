#ifndef PlantH
#define PlantH

#include <math.h>
#include "CObject.h"
#include "CGenet.h"
#include "RunPara.h"
#include "SPftTraits.h"
//#include "Seed.h"
#include <vector>
using namespace std;


//---------------------------------------------------------------------------

const double Pi=3.14159265358979323846;

//---------------------------------------------------------------------------
//! Structure to store all PFT Parameters


class CSeed;class CCell;class CGenet;
//! Class that describes plant individuals
class CPlant : public CObject
{
protected:
   CCell* cell;      ///<cell where it sits
   virtual double ReproGrow(double uptake);
   virtual double ShootGrow(double shres);
   virtual double RootGrow(double rres);
//---
   ///helping function to set allocation to reproduction
//   double ReproGrow(double uptake);
   double mReproRamets;                         ///<resources for ramet growth
   CGenet* genet;                               ///<genet of the clonal plant

public:
   SPftTraits* Traits;///<PFT Traits

   int Age;  //!< Variable that takes track of plant age
   static int numPlants; // MSC

   int plantID;

   double xcoord;   //!< location of plant's central point
   double ycoord;   //!< location of plant's central point

   double mshoot;   //!< shoot mass
   double mroot;    //!< root mass
   double mRepro;    //!< reproductive mass (which is converted to seeds)
   int totalSeeds = 0; //!< The total accumulation of seeds. Note that this may be offset by one year, it's quite imperfect.
   int accumulatedSeeds = 0;

//   double alloc_shoot;     //!< root-shoot resource partition coefficient

//   double Ashoot;   //!< area of above-ground ZOI [cm^2]
//   double Aroot;    //!< area of below-ground ZOI [cm^2]

   double Ash_disc; //!< discrete above-ground ZOI area [number of cells covered * area of one cell]
   double Art_disc; //!< discrete below-ground ZOI area [number of cells covered * area of one cell]
   double Aroots_all;  ///<area of all species' roots in ZOI
   double Aroots_type; ///<area of all PFT's roots in ZOI
//   double rsh;      //!< radius of above-ground ZOI [cm]
//   double rrt;      //!< radius of below-ground ZOI [cm]

   double Auptake; //!< uptake of above-ground resource in one time step
   double Buptake; //!< uptake below-ground resource one time step

   bool dead;      //!< plant dead or alive?
   bool remove;    //!< trampled or not - should the plant be removed?

   int stress;     //!< counter for weeks with resource stress exposure
//--clonal..
   vector<CPlant*> growingSpacerList;     ///<List of growing Spacer
   double Spacerlength;                         ///<real spacer length
   double Spacerdirection;                      ///<spacer direction
   double SpacerlengthToGrow;                   ///<length to grow
   int Generation;                              ///<clonal generation

//   int LimitRes;   //!< limiting resource: 1->above, 2->below 0->equal (not used)

   //functions
//! constructor for plant objects
   CPlant(double x, double y,SPftTraits* Traits);
   CPlant(SPftTraits* Traits,CCell* cell,
     double mshoot=0, double mroot=0, double mrepro=0,
     int stress=0, bool dead=false);
   ///make a plant from a clonal-seed object
   CPlant(CSeed* seed);
   //! initalization of one plant
//   CPlant(SPftTraits* traits,CCell* cell,
//     double mshoot, double mroot, double mrepro, int stress, bool dead);
   virtual ~CPlant();  //!<destruktor
   //constructors for plant objects
   ///make a plant from a clonal-seed object
//   CclonalPlant(CclonalSeed* seed);
   ///clonal growth
   CPlant(double x, double y, CPlant* plant);
   ///keine vordefinierten Eigenschaften
//   CclonalPlant(double x, double y, SPftTraits* PlantTraits);//, SclonalTraits* clonalTraits
//   CclonalPlant(SPftTraits* PlantTraits, CCell* cell);//SclonalTraits* clonalTraits,
//   CclonalPlant(SPftTraits* Traits,CCell* cell,
//     double mshoot=0, double mroot=0, double mrepro=0,
//     int stress=0, bool dead=false);
//   virtual ~CclonalPlant();  //!<destructor

//---admin
   virtual string type();  ///<say what you are
   virtual string pft();   ///<say what a pft you are
   virtual string asString(); ///<report plant's status
   static string headerToString();
//   void Allometrics(); //!< calculates ZOI areas (above and below) from shoot and root mass

//-2nd order properties
   double Area_shoot();  //!<ZOI area aboveground
   double Area_root();   //!<ZOI area belowground
   double Radius_shoot(); //!<ZOI radius aboveground
   double Radius_root();  //!<ZOI radius belowground
   ///competition coefficient for a plant -needed for AboveComp and BelowComp
   double comp_coef(const int layer,const int symmetry)const;
   virtual bool stressed();///< return true if plant is stressed
/// lower threshold of aboveground resource uptake (light stress thresh.)
   virtual double minresA(){return Traits->mThres*Ash_disc*Traits->Gmax;}
/// lower threshold of belowground resource uptake (nutrient stress thresh.)
   virtual double minresB(){return Traits->mThres*Art_disc*Traits->Gmax;}

   //! shoot-root resource allocation and plant growth in two layers (one-layer option obsolete now)
   virtual void Grow2(); //!< shoot-root resource allocation and plant growth in two layers
   void Kill();  //!< Mortality due to resource shortage or at random
   void DecomposeDead();     //!< calculate mass shrinkage of dead plants
   void WinterLoss(); //!< removal of above-ground biomass in winter
   double RemoveMass();  //!< removal of above-ground biomass by grazing
   /// removal of belowground biomass by grazing
   double RemoveRootMass(const double prop_remove=0.5);
//   void Decompose();    //!< decomposition of dead plants (mass shrinkage)

//getters and setters...
   void setCell(CCell* cell); ///<define cell for plant
   inline CCell* getCell(){return cell;};///<returns central cell
   inline double GetMass(){return mshoot+mroot+mRepro;};//!< returns plant mass
/// \brief get plant's height
/// \param cheight mg vegetative plant mass per cm height
/// \note fkt meight be transfered to CPlant
/// \bug since mass relates to 3D-measurements as volume,
///   height has to be correlated with mass^(1/3)
///
  virtual double getHeight(double const cheight = 6.5){
    return pow(mshoot/(Traits->LMR),1/3.0)*cheight;};
   virtual int GetNSeeds();     //!< returns number of seeds of one plant individual

//-----clonal...
//   virtual string type();              ///<say what you are
//   virtual string pft();   ///<say what a pft you are
//   virtual string asString(); ///<report plant's status

   ///set genet and add ramet to its list
   void setGenet(CGenet* genet);
   CGenet* getGenet(){return genet;};
   void SpacerGrow();                  ///<spacer growth
   virtual int GetNRamets();           ///<return number of ramets


   //-----------------------------------------------------------------------------
   //functions that are used for STL algorithms (sort + partition)

   //! return if plant should be removed (necessary to apply algorithms from STL)
   bool GetPlantRemove(){return (!this->remove);};

   /// sort plant individuals descending after shoot size * palatability
   static bool ComparePalat(const CPlant* plant1, const CPlant* plant2)
   {
      return ((plant1->mshoot*plant1->Traits->GrazFac())
         > (plant2->mshoot*plant2->Traits->GrazFac()));
   };
   //-----------------------------------------------------------------------------
   /// sort plants descending after shoot size (mass*1/LMR)
   /**\bug '*' in secound line solved 100202*/
   static bool CompareShoot(const CPlant* plant1, const CPlant* plant2)
   {
      return ((plant1->mshoot/plant1->Traits->LMR)
          > (plant2->mshoot/plant2->Traits->LMR));
   };
   //-----------------------------------------------------------------------------
   /// sort plants descending after root mass
   static bool CompareRoot(const CPlant* plant1, const CPlant* plant2)
   {
      return ((plant1->mroot) > (plant2->mroot));
   };

private:

};

//---------------------------------------------------------------------------
// bool is_left( CPlant*& plant1)//const
// {
//   return (plant1->xcoord < SRunPara::RunPara.CellNum/2);
//}
//---------------------------------------------------------------------------
#endif
