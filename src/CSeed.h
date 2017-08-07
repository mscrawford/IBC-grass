
#ifndef CSeedH
#define CSeedH

#include <memory>

class CCell;
class CPlant;
class SPftTraits;

class CSeed
{
	protected:
	   CCell* cell;

	public:
	   std::unique_ptr<SPftTraits> Traits;

	   double mass;
	   double estab;
	   int Age;
	   bool remove;

	   CSeed(const std::shared_ptr<CPlant> & plant, CCell* cell);
	   CSeed(std::string PFT_ID, CCell* cell, double estab);
	   ~CSeed();

	   CCell* getCell(){ return cell; };

	   static bool GetSeedRemove(const std::unique_ptr<CSeed> & s) {
		   return s->remove;
	   };
};

#endif
