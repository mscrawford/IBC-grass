
#ifndef CSeedH
#define CSeedH

#include <memory>

#include "CObject.h"

class CCell;
class CPlant;
class SPftTraits;

class CSeed: public CObject
{
	protected:
	   CCell* cell;

	public:
	   std::shared_ptr<SPftTraits> Traits;

	   double mass;
	   double estab;
	   int Age;
	   bool remove;
	   virtual std::string type();
	   virtual std::string pft();

	   CSeed(CPlant* plant, CCell* cell);
	   CSeed(double estab, std::shared_ptr<SPftTraits> traits, CCell* cell);
	   virtual ~CSeed();

	   void setCell(CCell* cell);
	   CCell* getCell(){ return cell; };

	   //! return type affiliation (necessary to apply algorithms from STL)
	   bool SeedOfType(std::string type) { return (this->pft()==type); };
};

//-----------------------------------------------------------------------------
//! return seed removed -> necessary to use STL algorithm
bool GetSeedRemove(const CSeed* seed1);

#endif
