#ifndef SRC_OUTPUT_H_
#define SRC_OUTPUT_H_

#include <fstream>
#include <string>
#include <vector>

#include "Grid.h"

class Output
{

private:
    // Describes simulation parameters. These are static throughout and provided at initialization.
    static const std::vector<std::string> parameter_header;

    // Describes the traits of each PFT. These are static and provided at initialization.
    static const std::vector<std::string> trait_header;

    // Describes the extinction times or final populations of PFTs.
    static const std::vector<std::string> populationSurvival_header;

    // Describes PFT's measured variables.
    static const std::vector<std::string> population_header;

    // Describes individual level variables over time.
    static const std::vector<std::string> individual_header;

    // Environmental and incidental data collection
    static const std::vector<std::string> community_header;

    struct PFT_struct;

    std::ofstream parameter_stream;
    std::ofstream trait_stream;
    std::ofstream populationSurvival_stream;
    std::ofstream population_stream;
    std::ofstream individual_stream;
    std::ofstream community_stream;

    // Filenames
    std::string parameter_fn;
    std::string trait_fn;
    std::string populationSurvival_fn;
    std::string population_fn;
    std::string individual_fn;
    std::string community_fn;

    bool is_file_exist(const char *fileName);
    void print_row(std::ostringstream &ss, std::ofstream &stream);
    void print_row(std::vector<std::string> row, std::ofstream &stream);

public:

    Output();
    ~Output();

    void setupOutput(std::string parameter_fn, std::string trait_fn, std::string populationSurvival_fn, std::string population_fn, std::string individual_fn, std::string agg_fn);
    void cleanup();

    void print_parameter();                                                                                 // prints general parameterization data
    void print_trait();                                                                                     // prints the traits of each PFT
    void print_populationSurvival_and_population(const std::vector< std::shared_ptr<Plant> > & PlantList); 	// prints PFT data
    void print_individual(const std::vector< std::shared_ptr<Plant> > & PlantList);                     	// prints individual data
    void print_community(const std::vector< std::shared_ptr<Plant> > & PlantList);                          // prints longitudinal data that's not just each PFT

    std::map<std::string, Output::PFT_struct> buildPFT_map(const std::vector< std::shared_ptr<Plant> > & PlantList);
    double calculateShannon(const std::map<std::string, Output::PFT_struct> & _PFT_map);
    double calculatePIE(const std::map<std::string, Output::PFT_struct> & _PFT_map);
    double calculateRichness(const std::map<std::string, Output::PFT_struct> & _PFT_map);
    double calculateBrayCurtis(const std::map<std::string, Output::PFT_struct> & _PFT_map, int benchmarkYear); // Bray-Curtis only makes sense with catastrophic disturbances
    std::map<std::string, double> calculateMeanTraits(const std::vector< std::shared_ptr<Plant> > & PlantList);

    // aggregated output
    std::vector<double> BlwgrdGrazingPressure;
    std::vector<double> ContemporaneousRootmassHistory;
    std::vector<double> TotalShootmass;
    std::vector<double> TotalRootmass;
    std::vector<double> TotalNonClonalPlants;
    std::vector<double> TotalClonalPlants;
    std::vector<double> TotalAboveComp;
    std::vector<double> TotalBelowComp;
    std::map<std::string, int> BC_predisturbance_Pop;

};

#endif /* SRC_OUTPUT_H_ */
