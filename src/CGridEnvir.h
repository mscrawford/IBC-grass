
#ifndef CGRIDENVIR_H_
#define CGRIDENVIR_H_

#include "CEnvir.h"

#include <string>

class CGridEnvir: public CEnvir, public CGrid
{

public:

	CGridEnvir();

	void InitRun();
	void OneYear();   // runs one year in default mode
	void OneRun();    // runs one simulation run in default mode
	void OneWeek();   // calls all weekly processes

	void InitInds();

	bool exitConditions();

	void SeedRain();  // distribute seeds on the grid each year

};

#endif /* CGRIDENVIR_H_ */
