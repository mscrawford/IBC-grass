/**\file
   \brief definition of class CGenet
*/
//---------------------------------------------------------------------------
#ifndef CGenetH
#define CGenetH

#include <vector>

#include "Plant.h"

class CPlant;

//---------------------------------------------------------------------------
/**
 * Class organizing ramets of a genet.
 */
class CGenet
{
public:
   static int staticID;
   std::vector<CPlant*> AllRametList;  ///<list of ramets
   int number;                         ///<ID of genet

   CGenet():number(++staticID){};
   ~CGenet(){};
   void ResshareA();     ///< share above-ground resources
   void ResshareB();     ///< share below-ground resources
};

//---------------------------------------------------------------------------
#endif

