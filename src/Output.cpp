#include <iostream>
#include <sstream>
#include <iterator>
#include <cassert>
#include <math.h>

#include "Output.h"
#include "Environment.h"

using namespace std;

const vector<string> Output::param_header
	({
			"SimID", "ComNr", "RunNr", "nPFTs",
			"IC_vers", "ITVsd", "Tmax",
			"Invader", "Resident",
			"ARes", "BRes",
			"GrazProb", "PropRemove",
			"BelGrazProb", "BelGrazPerc",
			"BelGrazAlpha", "BelGrazHistorySize",
			"CatastrophicMortality", "CatastrophicDistWeek",
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

const vector<string> Output::srv_header
	({
			"SimID", "PFT", "Extinction_Year", "Final_Pop", "Final_Shootmass", "Final_Rootmass"
	});

const vector<string> Output::PFT_header
	({
			"SimID", "PFT", "Year", "Week", "Pop", "Shootmass", "Rootmass", "Repro"
	});

const vector<string> Output::aggregated_header
	({
			"SimID", "Year", "Week", "FeedingPressure", "ContemporaneousRootmass", "Shannon", "Richness",
			"TotalShootmass", "TotalRootmass", "TotalNonClonalPlants", "TotalClonalPlants"
	});

const vector<string> Output::ind_header
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
		};

		~PFT_struct(){};
};

Output::Output() :
		param_fn("data/out/param.txt"),
		trait_fn("data/out/trait.txt"),
		srv_fn("data/out/srv.txt"),
		PFT_fn("data/out/PFT.txt"),
		ind_fn("data/out/ind.txt"),
		aggregated_fn("data/out/aggregated.txt")
{
	;
}

Output::~Output()
{
	cleanup();
}

void Output::setupOutput(string _param_fn, string _trait_fn, string _srv_fn,
						 string _PFT_fn, string _ind_fn, string _agg_fn)
{
	Output::param_fn = _param_fn;
	Output::trait_fn = _trait_fn;
	Output::srv_fn = _srv_fn;
	Output::PFT_fn = _PFT_fn;
	Output::ind_fn = _ind_fn;
	Output::aggregated_fn = _agg_fn;

	bool mid_batch = is_file_exist(param_fn.c_str());

	param_stream.open(param_fn.c_str(), ios_base::app);
	assert(param_stream.good());
	if (!mid_batch) print_row(param_header, param_stream);

	if (Parameters::params.trait_out)
	{
		trait_stream.open(trait_fn.c_str(), ios_base::app);
		assert(trait_stream.good());
		if (!mid_batch) print_row(trait_header, trait_stream);
	}

	if (Parameters::params.PFT_out)
	{
		PFT_stream.open(PFT_fn.c_str(), ios_base::app);
		assert(PFT_stream.good());
		if (!mid_batch) print_row(PFT_header, PFT_stream);
	}

	if (Parameters::params.ind_out)
	{
		ind_stream.open(ind_fn.c_str(), ios_base::app);
		assert(ind_stream.good());
		if (!mid_batch) print_row(ind_header, ind_stream);
	}

	if (Parameters::params.srv_out)
	{
		srv_stream.open(srv_fn.c_str(), ios_base::app);
		assert(srv_stream.good());
		if (!mid_batch) print_row(srv_header, srv_stream);
	}

	if (Parameters::params.aggregated_out)
	{
		aggregated_stream.open(aggregated_fn.c_str(), ios_base::app);
		assert(aggregated_stream.good());
		if (!mid_batch) print_row(aggregated_header, aggregated_stream);
	}
}

bool Output::is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Output::cleanup()
{
	if (Output::param_stream.is_open()) {
		Output::param_stream.close();
		Output::param_stream.clear();
	}

	if (Output::trait_stream.is_open()) {
		Output::trait_stream.close();
		Output::trait_stream.clear();
	}

	if (Output::srv_stream.is_open()) {
		Output::srv_stream.close();
		Output::srv_stream.clear();
	}

	if (Output::PFT_stream.is_open()) {
		Output::PFT_stream.close();
		Output::PFT_stream.clear();
	}

	if (Output::ind_stream.is_open()) {
		Output::ind_stream.close();
		Output::ind_stream.clear();
	}

	if (Output::aggregated_stream.is_open()) {
		Output::aggregated_stream.close();
		Output::aggregated_stream.clear();
	}
}

void Output::print_param()
{
	std::ostringstream ss;

	ss << Parameters::params.getSimID()					<< ", ";
	ss << Environment::ComNr 							<< ", ";
	ss << Environment::RunNr 							<< ", ";
	ss << Traits::pftTraitTemplates.size()				<< ", ";
	ss << Parameters::params.stabilization 				<< ", ";
	ss << Parameters::params.ITVsd 						<< ", ";
	ss << Parameters::params.Tmax 						<< ", ";

	if (Parameters::params.mode == invasionCriterion)
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

	ss << Parameters::params.meanARes 					<< ", ";
	ss << Parameters::params.meanBRes 					<< ", ";
	ss << Parameters::params.AbvGrazProb 				<< ", ";
	ss << Parameters::params.AbvPropRemoved 			<< ", ";
	ss << Parameters::params.BelGrazProb 				<< ", ";
	ss << Parameters::params.BelGrazPerc 				<< ", ";
	ss << Parameters::params.BelGrazAlpha				<< ", ";
	ss << Parameters::params.BelGrazHistorySize			<< ", ";
	ss << Parameters::params.CatastrophicPlantMortality << ", ";
	ss << Parameters::params.CatastrophicDistWeek 		<< ", ";
	ss << Parameters::params.SeedRainType 				<< ", ";
	ss << Parameters::params.SeedInput						   ;

	print_row(ss, param_stream);
}

void Output::print_trait()
{

	for (auto const& it : Traits::pftTraitTemplates)
	{
		std::ostringstream ss;

		ss << Parameters::params.getSimID()	<< ", ";
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
		if (Parameters::params.PFT_out != 2 && p->isDead) continue; // If PFT_out is 2, it will print "dead" PFTs

		PFT_struct* s = &(PFT_map[p->pft()]);

		if (!p->isDead)
		{
			s->Pop = s->Pop + 1;
			s->Rootmass = s->Rootmass + p->mRoot;
			s->Shootmass = s->Shootmass + p->mShoot;
			s->Repro = s->Repro + p->mRepro;
		}
	}

	return PFT_map;
}

void Output::print_srv_and_PFT(const std::vector< std::shared_ptr<Plant> > & PlantList)
{

	// Create the data structure necessary to aggregate individuals
	auto PFT_map = buildPFT_map(PlantList);

	// If any PFT went extinct, record it in "srv" stream
	if (Parameters::params.srv_out != 0)
	{
		for (auto it : PFT_map)
		{
			if ((Environment::PftSurvTime[it.first] == 0 && it.second.Pop == 0) ||
					(Environment::PftSurvTime[it.first] == 0 && Environment::year == Parameters::params.Tmax))
			{
				Environment::PftSurvTime[it.first] = Environment::year;

				std::ostringstream s_ss;

				s_ss << Parameters::params.getSimID()	<< ", ";
				s_ss << it.first 						<< ", "; // PFT name
				s_ss << Environment::year				<< ", ";
				s_ss << it.second.Pop 					<< ", ";
				s_ss << it.second.Shootmass 			<< ", ";
				s_ss << it.second.Rootmass 					   ;

				print_row(s_ss, srv_stream);
			}
		}
	}

	// If one should print PFTs, do so.
	if (Parameters::params.PFT_out != 0)
	{
		// print each PFT
		for (auto it : PFT_map)
		{
			if (Parameters::params.PFT_out == 1 &&
					it.second.Pop == 0 &&
					Environment::PftSurvTime[it.first] != Environment::year)
				continue;

			std::ostringstream p_ss;

			p_ss << Parameters::params.getSimID()	<< ", ";
			p_ss << it.first 						<< ", "; // PFT name
			p_ss << Environment::year 				<< ", ";
			p_ss << Environment::week 				<< ", ";
			p_ss << it.second.Pop 					<< ", ";
			p_ss << it.second.Shootmass 			<< ", ";
			p_ss << it.second.Rootmass 				<< ", ";
			p_ss << it.second.Repro 					   ;

			print_row(p_ss, PFT_stream);
		}
	}

	// delete PFT_map
	PFT_map.clear();
}

void Output::print_ind(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
	for (auto const& p : PlantList)
	{
		if (p->isDead) continue;

		std::ostringstream ss;

		ss << Parameters::params.getSimID()	<< ", ";
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

		print_row(ss, ind_stream);
	}
}

void Output::print_aggregated(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
	auto PFT_map = buildPFT_map(PlantList);

	auto calculateShannon = [PFT_map]()
	{
		int totalPop = std::accumulate(PFT_map.begin(), PFT_map.end(), 0,
							[] (int s, const std::map<string, PFT_struct>::value_type& p)
							{
								return s + p.second.Pop;
							});

		map<string, double> pi_map;
		for (auto pft : PFT_map)
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

		return (-1.0 * total_Pi_ln_Pi);
	};

	auto calculateRichness = [PFT_map]()
	{
		int richness = std::accumulate(PFT_map.begin(), PFT_map.end(), 0,
							[] (int s, const std::map<string, PFT_struct>::value_type& p)
							{
								if (p.second.Pop > 0)
								{
									return s + 1;
								}
								return s;
							});

		return richness;
	};

	std::ostringstream ss;

	ss << Parameters::params.getSimID() 						<< ", ";
	ss << Environment::year										<< ", ";
	ss << Environment::week 									<< ", ";
	ss << yearlyBlwgrdGrazingPressure.back() 					<< ", ";
	ss << yearlyContemporaneousRootmassHistory.back() 			<< ", ";
	ss << calculateShannon() 									<< ", ";
	ss << calculateRichness() 									<< ", ";
	ss << yearlyTotalShootmass.back()							<< ", ";
	ss << yearlyTotalRootmass.back() 							<< ", ";
	ss << yearlyTotalNonClonalPlants.back() 					<< ", ";
	ss << yearlyTotalClonalPlants.back() 							   ;
	print_row(ss, aggregated_stream);

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
