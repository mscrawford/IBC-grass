#ifndef SRC_OUTPUT_H_
#define SRC_OUTPUT_H_

#include <fstream>
#include <string>
#include <vector>

#include "CGrid.h"

class Output
{

private:
	// Describes simulation parameters. These are static throughout and provided at initialization.
	static const std::vector<std::string> param_header;

	// Describes the traits of each PFT. These are static and provided at initialization.
	static const std::vector<std::string> trait_header;

	// Describes the extinction times or final populations of PFTs.
	static const std::vector<std::string> srv_header;

	// Describes PFT's measured variables.
	static const std::vector<std::string> PFT_header;

	// Describes individual level variables over time.
	static const std::vector<std::string> ind_header;

	struct PFT_struct;

	std::ofstream param_stream;
	std::ofstream trait_stream;
	std::ofstream srv_stream;
	std::ofstream PFT_stream;
	std::ofstream ind_stream;

	// Filenames
	std::string param_fn;
	std::string trait_fn;
	std::string srv_fn;
	std::string PFT_fn;
	std::string ind_fn;

	void print_row(std::ostringstream &ss, std::ofstream &stream);
	void print_row(std::vector<std::string> row, std::ofstream &stream);

public:

	// constructors
	Output();
	~Output();

	void setupOutput(std::string param_fn, std::string trait_fn, std::string srv_fn, std::string PFT_fn, std::string ind_fn);
	void cleanup();

	void print_param(); // prints general parameterization data
	void print_trait(); // prints the traits of each PFT
	void print_srv_and_PFT(std::vector<CPlant*> & PlantList, CCell** & CellList); // prints PFT data
	void print_ind(std::vector<CPlant*> & PlantList); // prints individual data

};

#endif /* SRC_OUTPUT_H_ */
