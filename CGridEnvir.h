/*
 * CGridEnvir.h
 *
 *  Created on: 24.04.2014
 *      Author: KatrinK
 */

#ifndef CGRIDENVIR_H_
#define CGRIDENVIR_H_

#include "CEnvir.h"
#include <vector>
using namespace std;
//---------------------------------------------------------------------------
/// simulation service class including grid-, result- and environmental information
/**
   The class collects simulation environment with clonal properties.
   CGridclonal and CEnvir are connected, and some Clonal-specific
   result-variables added.
*/
class CGridEnvir: public CEnvir, public CGrid{
protected:
public:

  //Constructors, Destructors ...
  CGridEnvir();
  CGridEnvir(string id); ///< load from file(s)
  virtual ~CGridEnvir();///<delete clonalTraits;

  ///\name core simulating Functions
  ///@{
  void InitRun();   ///< from CEnvir
  void OneYear();   ///< runs one year in default mode
  void OneRun();    ///< runs one simulation run in default mode
  void OneWeek();   //!< calls all weekly processes
  virtual void Save(string ID);//<save current grid state
  virtual void Load(string ID){};//<load previosly saved grid
  int PftSurvival();    ///< from CEnvir
  ///@}

  /// \name collect general results
  ///@{
    void GetOutput();    //run in 20th week of year
  void GetClonOutput(SGridOut& GridData); //run in 30th week of year
  void GetOutputCutted(); ///<get anually cutted biomass (after week 22)
  ///@}
  ///\name init new Individuals/Seeds
  ///@{
  void InitInds();///<Initialization of individuals on grid
  void InitInds(string file,int n=-1);///< new way of initializing clonal and other traits at the same time from one file
  bool InitInd(string def);///<init of one ind based on saved data
  void InitSeeds(int); ///<Initialization of seeds on grid
  void InitSeeds(string, int);
  ///@}


  int exitConditions(); ///< get exit conditions //first implemented by Ines
   ///\name Functions to get Acover and Bcover of cells.
   /** It is assumed that coordinates/indices match grid size.
       Functions have to be called after function CGrid::CoverCells and before
       first function calling delete for established plants in the same week.
       else undefined behavior including access violation is possible.

     \note depends (at least) on an inherited subclass of CGrid
   */
   ///@{
   int getACover(int x, int y);
   int getBCover(int x, int y);
   double getTypeCover(const string type)const;
   double getTypeCover(const int i, const string type)const;

   void SeedRain(); //!< distribute seeds on the grid each year
private:
   int getGridACover(int i);
   int getGridBCover(int i);
   ///set cell state information
   void setCover();
   ///@}

};

#endif /* CGRIDENVIR_H_ */
