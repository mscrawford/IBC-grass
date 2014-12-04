//---------------------------------------------------------------------------
//#pragma hdrstop

#include "CSeed.h"
#include "CGrid.h"//for CGrid::PftList[]
//---------------------------------------------------------------------------

//#pragma package(smart_init)


//---------------------------------------------------------------------------
/*
CSeed::CSeed(double x, double y, int ID, double m, double estab, int maxage)
:xcoord(x),ycoord(y),mass(m),estab(estab)
{
//   TypeID=ID;
   Traits=*CPftTraits::PftList[ID-1];
   cell=NULL;

   Age=1;
   remove=false;
}
*/

//---------------------------------------------------------------------------
///not used
///
CSeed::CSeed(CSeed& seed)
  :xcoord(seed.xcoord),ycoord(seed.ycoord),Age(seed.Age),cell(seed.cell),
  remove(seed.remove),Traits(seed.Traits),estab(seed.estab),mass(seed.mass)
{
}//end copy-constructor
//---------------------------------------------------------------------------
///not used
///
CSeed::CSeed(double x, double y, CPlant* plant)
  :xcoord(x),ycoord(y),Age(1),cell(NULL),remove(false),
  Traits(plant->Traits),estab(Traits->pEstab),mass(Traits->SeedMass)
{}
//---------------------------------------------------------------------------
CSeed::CSeed(CPlant* plant,CCell* cell)
  :xcoord(plant->xcoord),ycoord(plant->ycoord),Age(1),cell(NULL),remove(false)
{
   Traits=plant->Traits;
   estab=Traits->pEstab;
   mass=Traits->SeedMass;
   setCell(cell);
}

//---------------------------------------------------------------------------
CSeed::CSeed(double estab, SPftTraits* traits,CCell* cell)
  :xcoord(0),ycoord(0),Age(1),cell(NULL),remove(false),estab(estab),
  Traits(traits),mass(traits->SeedMass)
{
   setCell(cell);
   if (cell){
     xcoord=(cell->x*SRunPara::RunPara.CellScale());
     ycoord=(cell->y*SRunPara::RunPara.CellScale());
   }
}

//---------------------------------------------------------------------------
///not used
///
CSeed::CSeed(double x, double y,double estab, SPftTraits* traits)
  :xcoord(x),ycoord(y),estab(estab){
   Traits=traits;
   mass=Traits->SeedMass;
   Age=1;
   remove=false;
   cell=NULL;
}
//-----------------------------------------------------------------------------
void CSeed::setCell(CCell* cell){
   if (this->cell==NULL){
     this->cell=cell;
     //add to seed bank
     this->cell->SeedBankList.push_back(this);
   }
}//end setCell
//---------------------------------------------------------------------------
/*
///not used
///
bool CSeed::Survive()
{
   if (Age<Traits->Dorm) return true;
   else return false;
}

//---------------------------------------------------------------------------
///not used
///
void CSeed::SetAge(int age)
{
   Age=age;
}
*/
//---------------------------------------------------------------------------
bool GetSeedRemove(const CSeed* seed1)
{
   return (!seed1->remove);
}

//-----------------------------------------------------------------------------
///sort plants ascending after TypeID
///\warning  compiler complains about temporal state of seed1 and seed2
///
int CompareTypeID(const CSeed* seed1, const CSeed* seed2)
{
  return (seed1->Traits->TypeID < seed2->Traits->TypeID);
}
//-----------------------------------------------------------------------------
std::string CSeed::type(){
        return "CSeed";
}
std::string CSeed::pft(){
        return this->Traits->name;
}   //say what a pft you are

//-eof----------------------------------------------------------------------------
