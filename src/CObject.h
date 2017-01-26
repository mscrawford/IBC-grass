
#ifndef CObjectH
#define CObjectH

#include <string>

///base class for seeds and plants


class CObject
{

public:

   CObject(){};

   virtual ~CObject(){};

   virtual std::string type();

};
#endif
