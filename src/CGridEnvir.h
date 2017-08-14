
#ifndef CGRIDENVIR_H_
#define CGRIDENVIR_H_

#include "CEnvir.h"

#include <string>

//---------------------------------------------------------------------------
// simulation service class including grid-, result- and environmental information

/**
 The class collects simulation environment with clonal properties.
 CGridclonal and CEnvir are connected, and some Clonal-specific
 result-variables added.
 */
class CGridEnvir: public CEnvir, public CGrid
{

public:

	CGridEnvir();

	void InitRun();   // from CEnvir
	void OneYear();   // runs one year in default mode
	void OneRun();    // runs one simulation run in default mode
	void OneWeek();   // calls all weekly processes

	void InitInds();

	bool exitConditions();

	void SeedRain(); // distribute seeds on the grid each year
};

#endif /* CGRIDENVIR_H_ */
