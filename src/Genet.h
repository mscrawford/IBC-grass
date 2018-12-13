
#ifndef SRC_GENET_H_
#define SRC_GENET_H_

#include <vector>
#include <memory>

class Plant;

/*
 * The super-individual that is one clonal plant.
 * Some clonal species are able to share resources.
 */
class Genet
{

public:
   static int staticID;
   int genetID;
   std::vector< std::weak_ptr<Plant> > RametList;

   Genet():genetID(++staticID) { }

   void ResshareA();     // share above-ground resources
   void ResshareB();     // share below-ground resources

};

#endif
