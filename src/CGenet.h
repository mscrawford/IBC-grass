
#ifndef CGenetH
#define CGenetH

#include <vector>

#include "Plant.h"

class CPlant;

/*
 * The super-individual that is one clonal plant.
 * Some clonal species are able to share resources.
 */
class CGenet
{

public:
   static int staticID;
   int genetID;
   std::vector< std::weak_ptr<CPlant> > AllRametList;

   CGenet():genetID(++staticID) { };
   ~CGenet() { };

   void ResshareA();     // share above-ground resources
   void ResshareB();     // share below-ground resources

};

#endif
