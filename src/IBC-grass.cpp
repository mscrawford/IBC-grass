#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <cassert>

#include "GridEnvir.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc == 3)
	{
		Parameters::NameSimFile  = argv[1];
		Parameters::outputPrefix = argv[2];
	}
	else
	{
        Parameters::NameSimFile = "data/in/SimFile.txt";
		Parameters::outputPrefix = "default";
	}

	ifstream SimFile(Parameters::NameSimFile.c_str()); // Open simulation parameterization file
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
			unique_ptr<GridEnvir> run = unique_ptr<GridEnvir>( new GridEnvir() );
			run->GetSim(data);
			run->RunNr = i;

			cout << Parameters::params.getSimID() << endl;
			cout << "Run " << run->RunNr << " \n";

			run->InitRun();
			run->OneRun();
		}
	}

	SimFile.close();

	return 0;
}
