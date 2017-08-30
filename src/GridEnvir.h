
#ifndef SRC_GRIDENVIR_H_
#define SRC_GRIDENVIR_H_

#include <string>

#include "Environment.h"

class GridEnvir: public Environment, public Grid
{

public:

	GridEnvir();

	void InitRun();
	void OneYear();   // runs one year in default mode
	void OneRun();    // runs one simulation run in default mode
	void OneWeek();   // calls all weekly processes

	void InitInds();

	bool exitConditions();

	void SeedRain();  // distribute seeds on the grid each year

};

#endif /* CGRIDENVIR_H_ */
