#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <cassert>

#include "CGridEnvir.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc == 3)
	{
		SRunPara::NameSimFile  = argv[1];
		SRunPara::outputPrefix = argv[2];
	}
	else
	{
		SRunPara::NameSimFile = "data/in/SimFile.txt";
		SRunPara::outputPrefix = "default";
	}

	ifstream SimFile(SRunPara::NameSimFile.c_str()); // Open simulation parameterization file
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
		for (int i = 0; i < _NRep; i++)
		{
			unique_ptr<CGridEnvir> Envir = unique_ptr<CGridEnvir>( new CGridEnvir() );
			Envir->GetSim(data);
			Envir->RunNr = i;

			if (SRunPara::RunPara.verbose)
			{
				cout << SRunPara::RunPara.getSimID() << endl;
				cout << "Run " << Envir->RunNr << " \n";
			}

			Envir->InitRun();
			Envir->OneRun();
		}
	}

	SimFile.close();

	return 0;
}
