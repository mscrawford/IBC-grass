#include <iostream>
#include <string>
#include <sstream>
#include <memory>

#include "CGridEnvir.h"

using namespace std;

/**
 * Program launch - Model Design is defined.
 * Here the model design is read from file.
 * Grid size is constant.
 * @param argc number of program parameters (>1 if parameters are given)
 * @param argv list of program parameters (SimFileName is expected)
 * \sa SRunPara::NameSimFile
 * @return zero if simulations are done
 * \sa CGridEnvir
 * \sa CGridEnvir::OneRun()
 */
int main(int argc, char* argv[])
{

	if (argc >= 2) {
		SRunPara::NameSimFile = argv[1];
		SRunPara::outputPrefix = argv[2];
	} else {
		SRunPara::NameSimFile = "data/in/SimFile.txt";
		SRunPara::outputPrefix = "default";
	}

	ifstream SimFile(SRunPara::NameSimFile.c_str()); // Open Simulation Parameterization file
	int _NRep;

	// Temporary strings
	string trash;
	string data;

	getline(SimFile, data);
	std::stringstream ss(data);
	ss >> trash >> _NRep; 		// Remove "NRep" header, set NRep
	getline(SimFile, trash); 	// Remove parameterization header file

	while (getline(SimFile, data))
	{

		unique_ptr<CGridEnvir> Envir = unique_ptr<CGridEnvir>( new CGridEnvir() );

		Envir->NRep = _NRep;

		Envir->GetSim(data);

		for (Envir->RunNr = 0; Envir->RunNr < Envir->NRep; Envir->RunNr++)
		{
			cout << SRunPara::RunPara.getSimID() << endl;

			if (SRunPara::RunPara.verbose)
			{
				cout << "Run " << Envir->RunNr + 1 << " \n";
			}

			Envir->InitRun();

			Envir->OneRun();
		}

		SRunPara::RunPara.cleanRunPara();

		Envir->output.cleanup();

	}

	SimFile.close();

	return 0;
}
