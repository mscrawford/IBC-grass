//---------------------------------------------------------------------------

#ifndef CSeedH
#define CSeedH

#include "CObject.h"
#include "Plant.h" //for struct CPftTraits
//---------------------------------------------------------------------------
//struct CPftTraits;
//! class of seed individuals
class CSeed: public CObject
{
protected:
   CCell* cell;

public:
   SPftTraits* Traits;
   double mass;    //!< seed mass
   double estab;   ///< estab-probability (may differ from type-specific value)
   double xcoord;  //!< x position on the grid
   double ycoord;  //!< y position on the grid

   int Age;      //!< seed age [years]
   bool remove;  //!< should the seed be removed? (because it is dead)
   virtual std::string type();
   virtual std::string pft();

   CSeed(double x, double y, CPlant* plant);
   CSeed(CPlant* plant, CCell* cell);
   CSeed(double x, double y,double estab, SPftTraits* traits);
   CSeed(double estab, SPftTraits* traits, CCell* cell);
   CSeed(CSeed& seed);///<copy-constructor
   virtual ~CSeed(){};

   void setCell(CCell* cell);       ///<define cell (only if none defined yet)
   CCell* getCell(){return cell;}; ///<return address of cell

   //! return type affiliation(necessary to apply algorithms from STL)
   bool SeedOfType(string type){return (this->pft()==type);};

};

//-----------------------------------------------------------------------------
//! return seed removed -> necessary to use STL algorithm
bool GetSeedRemove(const CSeed* seed1);
//!< sort seeds ascending after PFT ID
int CompareTypeID(const CSeed* seed1, const CSeed* seed2);
//-----------------------------------------------------------------------------
//! return type affiliation(necessary to apply algorithms from STL) bool SeedOfType(CSeed* seed,string type){return (seed->pft()==type);};
// SeedOfType(CSeed* seed,string type){return (seed->pft()==type);};

#endif
