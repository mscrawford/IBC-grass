/** \file
    \brief functions of class CGenet
*/
//---------------------------------------------------------------------------

#include "CGenet.h"

//---------------------------------------------------------------------------
int CGenet::staticID=0;

/// Calculate the mean of the uptakes of one genet and save this
/// as the uptake for each plant of this genet.
void CGenet::ResshareA()
{
  double sumAuptake=0;
  double MeanAuptake=0;
  const int listsize=this->AllRametList.size();
  for (int m=0; m<listsize;m++)//for all ramets of the genet
  {
       double AddtoSum=0;
       CPlant* Ramet = AllRametList[m];
       double minres= Ramet->Traits->mThres
                     *Ramet->Ash_disc
                     *Ramet->Traits->Gmax*2;
       //Uptake - the min resources that the plant need
       AddtoSum=std::max(0.0,Ramet->Auptake-minres);
       //if the plant has enought resources
       //new uptake is the min amount of resources
       if (AddtoSum>0)    Ramet->Auptake=minres;
       sumAuptake+=AddtoSum;
  }
  MeanAuptake=(sumAuptake/(double)listsize); //mean

  //Add the shared resourses (MeanAuptake) to the uptake
  for (int m=0; m<listsize;m++)
      AllRametList[m]->Auptake+=MeanAuptake;
} //end CGridclonal::ResshareA

//-----------------------------------------------------------------------------
/// Calculate the mean of the uptakes of one genet and save this
/// as the uptake for each plant of this genet.
void CGenet::ResshareB()
{
 double sumBuptake=0;
 double MeanBuptake=0;
    for (unsigned int m=0; m<AllRametList.size();m++)//for all ramets of the genet
    {
       double AddtoSum=0;
       CPlant* Ramet =AllRametList[m];
       double minres= Ramet->Traits->mThres*Ramet->Art_disc*Ramet->Traits->Gmax*2;
       //Uptake - the min resources that the plant need
       AddtoSum=std::max(0.0,Ramet->Buptake-minres);
       //if the plant has enought resources
       //new uptake is the min amount of resources
       if (AddtoSum>0)Ramet->Buptake=minres;
       sumBuptake+=AddtoSum;
    }

    MeanBuptake=(sumBuptake/(AllRametList.size())); //Mittelwert des �berschusses

    //Add the shared resourses to the uptake
     for (unsigned int m=0; m<AllRametList.size();m++)
       AllRametList[m]->Buptake+=MeanBuptake;
}//end CGridclonal::ResshareB
//eof---------------------------------------------------------------------
