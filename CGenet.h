/**\file
   \brief definition of class CGenet
*/
//---------------------------------------------------------------------------
#ifndef CGenetH
#define CGenetH

#include "Plant.h"
#include <vector>
using namespace std;

class CPlant;
//---------------------------------------------------------------------------
/**
 * Class organizing ramets of a genet.
 */
class CGenet
{
public:
   static int staticID;
   vector<CPlant*> AllRametList;  ///<list of ramets
   int number;                         ///<ID of genet

   CGenet():number(++staticID){};
   ~CGenet(){};
   void ResshareA();     ///< share above-ground resources
   void ResshareB();     ///< share below-ground resources
};
class CPlant;
/**\brief a genet consists of several ramets

   \since clonal version
*/
//---------------------------------------------------------------------------
#endif

