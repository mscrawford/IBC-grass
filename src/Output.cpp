#include <iostream>
#include <sstream>
#include <iterator>
#include <cassert>
#include <math.h>

#include "Output.h"
#include "Environment.h"

using namespace std;

const vector<string> Output::parameter_header
    ({
         "SimID", "ComNr", "RunNr", "nPFTs",
         "Stabilization", "ITVsd", "Tmax",
         "Invader", "Resident",
         "ARes", "BRes",
         "AbvGrazProb", "AbvGrazPerc",
         "BelGrazProb", "BelGrazPerc", "BelGrazThreshold", "BelGrazAlpha", "BelGrazWindow",
         "DisturbanceMortality", "DisturbanceWeek",
         "SeedLongevity",
         "SeedRainType", "SeedInput"
    });

const vector<string> Output::trait_header
    ({
         "SimID", "PFT",
         "LMR",
         "m0", "MaxMass", "SeedMass", "Dist",
         "SLA", "palat",
         "Gmax", "memory",
         "clonal", "meanSpacerlength", "sdSpacerlength"
    });

const vector<string> Output::populationSurvival_header
    ({
         "SimID", "PFT", "Extinction_Year", "Final_Pop", "Final_Shootmass", "Final_Rootmass"
    });

const vector<string> Output::population_header
    ({
         "SimID", "PFT", "Year", "Week", "Pop", "Shootmass", "Rootmass", "Repro"
    });

const vector<string> Output::community_header
    ({
         "SimID", "Year", "Week",
         "FeedingPressure", "ContemporaneousRootmass",
         "Shannon", "PIE", "Richness", "BrayCurtisDissimilarity",
         "TotalAboveComp", "TotalBelowComp",
         "TotalShootmass", "TotalRootmass",
         "TotalNonClonalPlants", "TotalClonalPlants",
         "wm_LMR", "wm_MaxMass", "wm_Gmax", "wm_SLA"
    });

const vector<string> Output::individual_header
    ({
         "SimID", "plantID", "PFT", "Year", "Week",
         "i_X", "i_Y",
         "i_LMR",
         "i_m0", "i_MaxMass", "i_SeedMass", "i_Dist",
         "i_SLA", "i_palat",
         "i_Gmax", "i_memory",
         "i_clonal", "i_meanSpacerlength", "i_sdSpacerlength",
         "i_genetID", "i_Age",
         "i_mShoot", "i_mRoot", "i_rShoot", "i_rRoot",
         "i_mRepro", "i_lifetimeFecundity",
         "i_stress"
    });

struct Output::PFT_struct
{
        double Shootmass;
        double Rootmass;
        double Repro;
        int Pop;

        PFT_struct() {
            Shootmass = 0;
            Rootmass = 0;
            Repro = 0;
            Pop = 0;
        }

        ~PFT_struct(){}
};

Output::Output() :
        parameter_fn("data/out/parameter.txt"),
        trait_fn("data/out/trait.txt"),
        populationSurvival_fn("data/out/populationSurvival.txt"),
        population_fn("data/out/population.txt"),
        individual_fn("data/out/individual.txt"),
        community_fn("data/out/community.txt")
{
    BlwgrdGrazingPressure = { 0 };
    ContemporaneousRootmassHistory = { 0 };
    TotalShootmass = { 0 };
    TotalRootmass = { 0 };
    TotalAboveComp = { 0 };
    TotalBelowComp = { 0 };
    TotalNonClonalPlants = { 0 };
    TotalClonalPlants = { 0 };
}

Output::~Output()
{
    cleanup();
}

void Output::setupOutput(string _parameter_fn, string _trait_fn, string _populationSurvival_fn,
                         string _population_fn, string _individual_fn, string _community_fn)
{
    Output::parameter_fn = _parameter_fn;
    Output::trait_fn = _trait_fn;
    Output::populationSurvival_fn = _populationSurvival_fn;
    Output::population_fn = _population_fn;
    Output::individual_fn = _individual_fn;
    Output::community_fn = _community_fn;

    bool mid_batch = is_file_exist(parameter_fn.c_str());

    parameter_stream.open(parameter_fn.c_str(), ios_base::app);
    assert(parameter_stream.good());
    if (!mid_batch) print_row(parameter_header, parameter_stream);

    if (Parameters::parameters.trait_out)
    {
        trait_stream.open(trait_fn.c_str(), ios_base::app);
        assert(trait_stream.good());
        if (!mid_batch) print_row(trait_header, trait_stream);
    }

    if (Parameters::parameters.population_out)
    {
        population_stream.open(population_fn.c_str(), ios_base::app);
        assert(population_stream.good());
        if (!mid_batch) print_row(population_header, population_stream);
    }

    if (Parameters::parameters.individual_out)
    {
        individual_stream.open(individual_fn.c_str(), ios_base::app);
        assert(individual_stream.good());
        if (!mid_batch) print_row(individual_header, individual_stream);
    }

    if (Parameters::parameters.populationSurvival_out)
    {
        populationSurvival_stream.open(populationSurvival_fn.c_str(), ios_base::app);
        assert(populationSurvival_stream.good());
        if (!mid_batch) print_row(populationSurvival_header, populationSurvival_stream);
    }

    if (Parameters::parameters.community_out)
    {
        community_stream.open(community_fn.c_str(), ios_base::app);
        assert(community_stream.good());
        if (!mid_batch) print_row(community_header, community_stream);
    }
}

bool Output::is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Output::cleanup()
{
    if (Output::parameter_stream.is_open()) {
        Output::parameter_stream.close();
        Output::parameter_stream.clear();
    }

    if (Output::trait_stream.is_open()) {
        Output::trait_stream.close();
        Output::trait_stream.clear();
    }

    if (Output::populationSurvival_stream.is_open()) {
        Output::populationSurvival_stream.close();
        Output::populationSurvival_stream.clear();
    }

    if (Output::population_stream.is_open()) {
        Output::population_stream.close();
        Output::population_stream.clear();
    }

    if (Output::individual_stream.is_open()) {
        Output::individual_stream.close();
        Output::individual_stream.clear();
    }

    if (Output::community_stream.is_open()) {
        Output::community_stream.close();
        Output::community_stream.clear();
    }
}

void Output::print_parameter()
{
    std::ostringstream ss;

    ss << Parameters::parameters.getSimID()					<< ", ";
    ss << Environment::ComNr 							<< ", ";
    ss << Environment::RunNr 							<< ", ";
    ss << Traits::pftTraitTemplates.size()				<< ", ";
    ss << Parameters::parameters.stabilization 				<< ", ";
    ss << Parameters::parameters.ITVsd 						<< ", ";
    ss << Parameters::parameters.Tmax 						<< ", ";

    if (Parameters::parameters.mode == invasionCriterion)
    {
        std::string invader 	= Traits::pftInsertionOrder[0];
        std::string resident 	= Traits::pftInsertionOrder[1];

        ss << invader 									<< ", ";
        ss << resident 									<< ", ";
    }
    else
    {
        ss << "NA"	 									<< ", ";
        ss << "NA"	 									<< ", ";
    }

    ss << Parameters::parameters.meanARes 					<< ", ";
    ss << Parameters::parameters.meanBRes 					<< ", ";
    ss << Parameters::parameters.AbvGrazProb 				<< ", ";
    ss << Parameters::parameters.AbvGrazPerc                << ", ";
    ss << Parameters::parameters.BelGrazProb 				<< ", ";
    ss << Parameters::parameters.BelGrazPerc 				<< ", ";
    ss << Parameters::parameters.BelGrazThreshold           << ", ";
    ss << Parameters::parameters.BelGrazAlpha				<< ", ";
    ss << Parameters::parameters.BelGrazWindow              << ", ";
    ss << Parameters::parameters.DisturbanceMortality       << ", ";
    ss << Parameters::parameters.DisturbanceWeek            << ", ";
    ss << Parameters::parameters.SeedLongevity              << ", ";
    ss << Parameters::parameters.SeedRainType               << ", ";
    ss << Parameters::parameters.SeedInput						   ;

    print_row(ss, parameter_stream);
}

void Output::print_trait()
{

    for (auto const& it : Traits::pftTraitTemplates)
    {
        std::ostringstream ss;

        ss << Parameters::parameters.getSimID()	<< ", ";
        ss << it.first 						<< ", ";
        ss << it.second->LMR 				<< ", ";
        ss << it.second->m0 				<< ", ";
        ss << it.second->maxMass 			<< ", ";
        ss << it.second->seedMass 			<< ", ";
        ss << it.second->dispersalDist 		<< ", ";
        ss << it.second->SLA 				<< ", ";
        ss << it.second->palat 				<< ", ";
        ss << it.second->Gmax 				<< ", ";
        ss << it.second->memory 			<< ", ";
        ss << it.second->clonal 			<< ", ";
        ss << it.second->meanSpacerlength 	<< ", ";
        ss << it.second->sdSpacerlength 	       ;

        print_row(ss, trait_stream);
    }

}

map<string, Output::PFT_struct> Output::buildPFT_map(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
    map<string, PFT_struct> PFT_map;

    for (auto const& it : Traits::pftTraitTemplates)
    {
        PFT_map[it.first] = PFT_struct();
    }

    // Aggregate individuals
    for (auto const& p : PlantList)
    {
        if (p->isDead)
            continue;

        PFT_struct* s = &(PFT_map[p->pft()]);

        s->Pop = s->Pop + 1;
        s->Rootmass = s->Rootmass + p->mRoot;
        s->Shootmass = s->Shootmass + p->mShoot;
        s->Repro = s->Repro + p->mRepro;
    }

    return PFT_map;
}

void Output::print_populationSurvival_and_population(const std::vector< std::shared_ptr<Plant> > & PlantList)
{

    // Create the data structure necessary to aggregate individuals
    auto PFT_map = buildPFT_map(PlantList);

    // If any PFT went extinct, record it in "srv" stream
    if (Parameters::parameters.populationSurvival_out != 0)
    {
        for (auto it : PFT_map)
        {
            if ((Environment::PftSurvTime[it.first] == 0 && it.second.Pop == 0) ||
                    (Environment::PftSurvTime[it.first] == 0 && Environment::year == Parameters::parameters.Tmax))
            {
                Environment::PftSurvTime[it.first] = Environment::year;

                std::ostringstream s_ss;

                s_ss << Parameters::parameters.getSimID()	<< ", ";
                s_ss << it.first 						<< ", "; // PFT name
                s_ss << Environment::year				<< ", ";
                s_ss << it.second.Pop 					<< ", ";
                s_ss << it.second.Shootmass 			<< ", ";
                s_ss << it.second.Rootmass 					   ;

                print_row(s_ss, populationSurvival_stream);
            }
        }
    }

    // If one should print PFTs, do so.
    if (Parameters::parameters.population_out != 0 && (Environment::year == 99 ||
                                                       Environment::year == 101 ||
                                                       Environment::year == 105 ||
                                                       Environment::year == 125 ||
                                                       Environment::year == 150))
//    if (Parameters::params.PFT_out != 0)
    {
        // print each PFT
        for (auto it : PFT_map)
        {
            if (Parameters::parameters.population_out == 1 &&
                    it.second.Pop == 0 &&
                    Environment::PftSurvTime[it.first] != Environment::year)
            {
                continue;
            }

            std::ostringstream p_ss;

            p_ss << Parameters::parameters.getSimID()	<< ", ";
            p_ss << it.first 						<< ", "; // PFT name
            p_ss << Environment::year 				<< ", ";
            p_ss << Environment::week 				<< ", ";
            p_ss << it.second.Pop 					<< ", ";
            p_ss << it.second.Shootmass 			<< ", ";
            p_ss << it.second.Rootmass 				<< ", ";
            p_ss << it.second.Repro 					   ;

            print_row(p_ss, population_stream);
        }
    }

    // delete PFT_map
    PFT_map.clear();
}

void Output::print_individual(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
    for (auto const& p : PlantList)
    {
        if (p->isDead) continue;

        std::ostringstream ss;

        ss << Parameters::parameters.getSimID()	<< ", ";
        ss << p->plantID 					<< ", ";
        ss << p->pft() 						<< ", ";
        ss << Environment::year 			<< ", ";
        ss << Environment::week 			<< ", ";
        ss << p->y 							<< ", ";
        ss << p->x 							<< ", ";
        ss << p->traits->LMR 				<< ", ";
        ss << p->traits->m0 				<< ", ";
        ss << p->traits->maxMass 			<< ", ";
        ss << p->traits->seedMass 			<< ", ";
        ss << p->traits->dispersalDist 		<< ", ";
        ss << p->traits->SLA 				<< ", ";
        ss << p->traits->palat 				<< ", ";
        ss << p->traits->Gmax 				<< ", ";
        ss << p->traits->memory 			<< ", ";
        ss << p->traits->clonal 			<< ", ";
        ss << p->traits->meanSpacerlength 	<< ", ";
        ss << p->traits->sdSpacerlength 	<< ", ";
        ss << p->genet.lock()->genetID		<< ", ";
        ss << p->age 						<< ", ";
        ss << p->mShoot						<< ", ";
        ss << p->mRoot 						<< ", ";
        ss << p->Radius_shoot() 			<< ", ";
        ss << p->Radius_root() 				<< ", ";
        ss << p->mRepro 					<< ", ";
        ss << p->lifetimeFecundity 			<< ", ";
        ss << p->isStressed						   ;

        print_row(ss, individual_stream);
    }
}

void Output::print_community(const std::vector< std::shared_ptr<Plant> > & PlantList)
{

    auto PFT_map = buildPFT_map(PlantList);

    std::map<std::string, double> meanTraits = calculateMeanTraits(PlantList);

    std::ostringstream ss;

    ss << Parameters::parameters.getSimID() 										<< ", ";
    ss << Environment::year															<< ", ";
    ss << Environment::week 														<< ", ";
    ss << BlwgrdGrazingPressure.back()                                              << ", ";
    ss << ContemporaneousRootmassHistory.back()                                 	<< ", ";
    ss << calculateShannon(PFT_map) 												<< ", ";
    ss << calculatePIE(PFT_map)                                                     << ", ";
    ss << calculateRichness(PFT_map)												<< ", ";

    double brayCurtis = calculateBrayCurtis(PFT_map, Parameters::parameters.DisturbanceYear - 1);
    if (!Environment::AreSame(brayCurtis, -1))
    {
        ss << brayCurtis 															<< ", ";
    }
    else
    {
        ss << "NA"																	<< ", ";
    }

    ss << TotalAboveComp.back()                                                     << ", ";
    ss << TotalBelowComp.back()                                                     << ", ";
    ss << TotalShootmass.back()                                                     << ", ";
    ss << TotalRootmass.back()                                                      << ", ";
    ss << TotalNonClonalPlants.back()                                               << ", ";
    ss << TotalClonalPlants.back()                                                  << ", ";
    ss << meanTraits["LMR"] 														<< ", ";
    ss << meanTraits["MaxMass"] 													<< ", ";
    ss << meanTraits["Gmax"] 														<< ", ";
    ss << meanTraits["SLA"] 													           ;

    print_row(ss, community_stream);

}

// Prints a row of data out a string, as a comma separated list with a newline at the end.
void Output::print_row(std::ostringstream & ss, ofstream & stream)
{
    assert(stream.good());

    stream << ss.str() << endl;

    stream.flush();
}

void Output::print_row(vector<string> row, ofstream & stream)
{
    assert(stream.good());

    std::ostringstream ss;

    std::copy(row.begin(), row.end() - 1, std::ostream_iterator<string>(ss, ", "));

    ss << row.back();

    stream << ss.str() << endl;

    stream.flush();
}

double Output::calculateShannon(const std::map<std::string, Output::PFT_struct> & _PFT_map)
{
    int totalPop = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                        [] (int s, const std::map<string, PFT_struct>::value_type& p)
                        {
                            return s + p.second.Pop;
                        });

    map<string, double> pi_map;

    for (auto pft : _PFT_map)
    {
        if (pft.second.Pop > 0)
        {
            double propPFT = pft.second.Pop / (double) totalPop;
            pi_map[pft.first] = propPFT * log(propPFT);
        }
    }

    double total_Pi_ln_Pi = std::accumulate(pi_map.begin(), pi_map.end(), 0.0,
                                [] (double s, const std::map<string, double>::value_type& p)
                                {
                                    return s + p.second;
                                });

    if (Environment::AreSame(total_Pi_ln_Pi, 0))
    {
        return 0;
    }

    return (-1.0 * total_Pi_ln_Pi);
}


double Output::calculatePIE(const std::map<std::string, Output::PFT_struct> & _PFT_map)
{
    int totalPop = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                        [] (int s, const std::map<string, PFT_struct>::value_type& p)
                        {
                            return s + p.second.Pop;
                        });

    map<string, double> pi_map;

    for (auto pft : _PFT_map)
    {
        if (pft.second.Pop > 0)
        {
            double propPFT = pft.second.Pop / (double) totalPop;
            pi_map[pft.first] = pow(propPFT, 2.0);
        }
    }

    double PIE_term_1 = totalPop / (double) (totalPop - 1);

    double PIE_term_2 = 1 - std::accumulate(pi_map.begin(), pi_map.end(), 0.0,
                                 [] (double s, const std::map<string, double>::value_type& p)
                                 {
                                    return s + p.second;
                                 });

    double PIE = PIE_term_1 * PIE_term_2;

    return(PIE);
}

double Output::calculateRichness(const std::map<std::string, Output::PFT_struct> & _PFT_map)
{
    int richness = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                        [] (int s, const std::map<string, PFT_struct>::value_type& p)
                        {
                            if (p.second.Pop > 0)
                            {
                                return s + 1;
                            }
                            return s;
                        });

    return richness;
}

/*
 * benchmarkYear is generally the year to prior to disturbance
 * BC_window is the length of the time period (years) in which PFT populations are averaged to arrive at a stable mean for comparison
 */
double Output::calculateBrayCurtis(const std::map<std::string, Output::PFT_struct> & _PFT_map, int benchmarkYear)
{
    static const int BC_window = 10;

    // Preparing the "average population counts" in the years preceding the catastrophic disturbance
    if (Environment::year > benchmarkYear - BC_window && Environment::year <= benchmarkYear)
    {
        // Add this year's population to the PFT's abundance sum over the window
        for (auto& pft : _PFT_map)
        {
            BC_predisturbance_Pop[pft.first] += pft.second.Pop;
        }

        // If it's the last year before disturbance, divide the population count by the window
        if (Environment::year == benchmarkYear)
        {
            for (auto& pft_total : BC_predisturbance_Pop)
            {
                pft_total.second = pft_total.second / BC_window;
            }
        }
    }

    if (Environment::year <= benchmarkYear)
    {
        return -1;
    }

    std::vector<int> popDistance;
    for (auto pft : _PFT_map)
    {
        popDistance.push_back( abs( BC_predisturbance_Pop[pft.first] - pft.second.Pop ) );
    }

    int BC_distance_sum = std::accumulate(popDistance.begin(), popDistance.end(), 0);

    int present_totalAbundance = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                                [] (int s, const std::map<string, PFT_struct>::value_type& p)
                                {
                                    return s + p.second.Pop;
                                });

    int past_totalAbundance = std::accumulate(BC_predisturbance_Pop.begin(), BC_predisturbance_Pop.end(), 0,
                                [] (int s, const std::map<string, int>::value_type& p)
                                {
                                    return s + p.second;
                                });

    int BC_abundance_sum = present_totalAbundance + past_totalAbundance;

    return BC_distance_sum / (double) BC_abundance_sum;
}

std::map<std::string, double> Output::calculateMeanTraits(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
    std::map<std::string, double> weightedMeanTraits;
    int pop = 0;

    for (auto const& p : PlantList)
    {
        if (p->isDead)
        {
            continue;
        }

        weightedMeanTraits["LMR"] += p->traits->LMR;
        weightedMeanTraits["MaxMass"] += p->traits->maxMass;
        weightedMeanTraits["Gmax"] += p->traits->Gmax;
        weightedMeanTraits["SLA"] += p->traits->SLA;

        ++pop;
    }

    for (auto& trait : weightedMeanTraits)
    {
        trait.second = trait.second / pop;
    }

    return weightedMeanTraits;
}
