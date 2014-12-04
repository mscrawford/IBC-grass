//---------------------------------------------------------------------------

#ifndef CObjectH
#define CObjectH
//#include "MMColorGrid.h"
#include <string>
//---------------------------------------------------------------------------
using namespace std;
///base class for seeds and plants
/**
  \since clonal version
*/
class CObject{
public:
   CObject(){};  //!< Constructor
   virtual ~CObject(){}; //!< Destructor
   virtual string type();  ///<say what you are
//   virtual string pft()=0;
};
#endif
