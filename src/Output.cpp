#include "Output.h"
#include "CEnvir.h"

#include <iostream>
#include <sstream>
#include <iterator>
#include <cassert>

using namespace std;

const vector<string> Output::param_header
	({
			"SimID", "ComNr", "RunNr",
			"IC_vers", "ITVsd", "Tmax",
			"Invader", "Resident",
			"ARes", "BRes",
			"GrazProb", "PropRemove",
			"BelGrazProb", "BelGrazResidualPerc", "BelGrazPerc",
			"CatastrophicPlantMortality", "CatastrophicSeedMortality",
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

const vector<string> Output::ind_header
	({
			"SimID", "plantID", "PFT", "Year", "Week",
			"i_X", "i_Y",
			"i_LMR",
			"i_m0", "i_MaxMass", "i_SeedMass", "i_Dist",
			"i_SLA", "i_palat",
			"i_Gmax", "i_memory",
			"i_clonal", "i_meanSpacerlength", "i_sdSpacerlength",
			"i_Age",
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
		srv_fn("data/out/trait.txt"),
		PFT_fn("data/out/PFT.txt"),
		ind_fn("data/out/ind.txt")
{
	;
}

Output::~Output()
{
	Output::param_stream.close();
	Output::trait_stream.close();
	Output::srv_stream.close();
	Output::PFT_stream.close();
	Output::ind_stream.close();
}

void Output::setupOutput(string _param_fn, string _trait_fn, string _srv_fn, string _PFT_fn, string _ind_fn)
{
	Output::param_fn = _param_fn;
	Output::trait_fn = _trait_fn;
	Output::srv_fn = _srv_fn;
	Output::PFT_fn = _PFT_fn;
	Output::ind_fn = _ind_fn;

	bool mid_batch = is_file_exist(param_fn.c_str());

	param_stream.open(param_fn.c_str(), ios_base::app);
	assert(param_stream.good());
	if (!mid_batch) print_row(param_header, param_stream);

	if (SRunPara::RunPara.trait_out)
	{
		trait_stream.open(trait_fn.c_str(), ios_base::app);
		assert(trait_stream.good());
		if (!mid_batch) print_row(trait_header, trait_stream);
	}

	if (SRunPara::RunPara.PFT_out)
	{
		PFT_stream.open(PFT_fn.c_str(), ios_base::app);
		assert(PFT_stream.good());
		if (!mid_batch) print_row(PFT_header, PFT_stream);
	}

	if (SRunPara::RunPara.ind_out)
	{
		ind_stream.open(ind_fn.c_str(), ios_base::app);
		assert(ind_stream.good());
		if (!mid_batch) print_row(ind_header, ind_stream);
	}

	if (SRunPara::RunPara.srv_out)
	{
		srv_stream.open(srv_fn.c_str(), ios_base::app);
		assert(srv_stream.good());
		if (!mid_batch) print_row(srv_header, srv_stream);
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
}

void Output::print_param()
{
	std::ostringstream ss;

	ss << SRunPara::RunPara.getSimID()					<< ", ";
	ss << CEnvir::ComNr 								<< ", ";
	ss << CEnvir::RunNr 								<< ", ";
	ss << SRunPara::RunPara.Version 					<< ", ";
	ss << SRunPara::RunPara.ITVsd 						<< ", ";
	ss << SRunPara::RunPara.Tmax 						<< ", ";

	if (SRunPara::RunPara.mode == invasionCriterion)
	{
		std::string invader 	= SPftTraits::pftInsertionOrder[0];
		std::string resident 	= SPftTraits::pftInsertionOrder[1];

		ss << invader 									<< ", ";
		ss << resident 									<< ", ";
	}
	else
	{
		ss << "NA"	 									<< ", ";
		ss << "NA"	 									<< ", ";
	}

	ss << SRunPara::RunPara.meanARes 					<< ", ";
	ss << SRunPara::RunPara.meanBRes 					<< ", ";
	ss << SRunPara::RunPara.GrazProb 					<< ", ";
	ss << SRunPara::RunPara.PropRemove 					<< ", ";
	ss << SRunPara::RunPara.BelGrazProb 				<< ", ";
	ss << SRunPara::RunPara.BelGrazResidualPerc			<< ", ";
	ss << SRunPara::RunPara.BelGrazPerc 				<< ", ";
	ss << SRunPara::RunPara.CatastrophicPlantMortality 	<< ", ";
	ss << SRunPara::RunPara.CatastrophicSeedMortality 	<< ", ";
	ss << SRunPara::RunPara.SeedRainType 				<< ", ";
	ss << SRunPara::RunPara.SeedInput						   ;

	print_row(ss, param_stream);
}

void Output::print_trait()
{

	for (auto it : SPftTraits::PftLinkList)
	{
		std::ostringstream ss;

		ss << SRunPara::RunPara.getSimID()	<< ", ";
		ss << it.first 						<< ", ";
		ss << it.second->LMR 				<< ", ";
		ss << it.second->m0 				<< ", ";
		ss << it.second->MaxMass 			<< ", ";
		ss << it.second->SeedMass 			<< ", ";
		ss << it.second->Dist 				<< ", ";
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

void Output::print_srv_and_PFT(vector<CPlant*> & PlantList)
{
	// Create the data structure necessary to aggregate individuals
	map<string, PFT_struct> PFT_map;

	for (auto it : SPftTraits::PftLinkList) {
		PFT_map[it.first] = PFT_struct();
	}

	// Aggregate individuals
	for (auto it : PlantList)
	{
		CPlant* p = it;

		if (SRunPara::RunPara.PFT_out != 2 && p->dead) continue; // If PFT_out is 2, it will print "dead" PFTs

		PFT_struct* s = &(PFT_map[p->pft()]);

		s->Pop = s->Pop + 1;
		s->Rootmass = s->Rootmass + p->mroot;
		s->Shootmass = s->Shootmass + p->mshoot;
		s->Repro = s->Repro + p->mRepro;
	}

	// If any PFT went extinct, record it in "srv" stream
	if (SRunPara::RunPara.srv_out != 0)
	{
		for (auto it : PFT_map)
		{
			if ((CEnvir::PftSurvTime[it.first] == 0 && it.second.Pop == 0) ||
					(CEnvir::PftSurvTime[it.first] == 0 && CEnvir::year == SRunPara::RunPara.Tmax))
			{
				CEnvir::PftSurvTime[it.first] = CEnvir::year;

				std::ostringstream s_ss;

				s_ss << SRunPara::RunPara.getSimID()	<< ", ";
				s_ss << it.first 						<< ", "; // PFT name
				s_ss << CEnvir::year					<< ", ";
				s_ss << it.second.Pop 					<< ", ";
				s_ss << it.second.Shootmass 			<< ", ";
				s_ss << it.second.Rootmass 					   ;

				print_row(s_ss, srv_stream);
			}
		}
	}

	// If one should print PFTs, do so.
	if (SRunPara::RunPara.PFT_out != 0)
	{
		// print each PFT
		for (auto it : PFT_map)
		{
			if (SRunPara::RunPara.PFT_out == 1 &&
					it.second.Pop == 0 &&
					CEnvir::PftSurvTime[it.first] != CEnvir::year)
				continue;

			std::ostringstream p_ss;

			p_ss << SRunPara::RunPara.getSimID()	<< ", ";
			p_ss << it.first 						<< ", "; // PFT name
			p_ss << CEnvir::year 					<< ", ";
			p_ss << CEnvir::week 					<< ", ";
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

void Output::print_ind(vector<CPlant*> & PlantList)
{
	for (auto it : PlantList)
	{
		CPlant* p = it;

		if (p->dead) continue;

		shared_ptr<SPftTraits> s = p->Traits;

		std::ostringstream ss;

		ss << SRunPara::RunPara.getSimID()	<< ", ";
		ss << p->plantID 					<< ", ";
		ss << p->pft() 						<< ", ";
		ss << CEnvir::year 					<< ", ";
		ss << CEnvir::week 					<< ", ";
		ss << p->xcoord 					<< ", ";
		ss << p->ycoord 					<< ", ";
		ss << s->LMR 						<< ", ";
		ss << s->m0 						<< ", ";
		ss << s->MaxMass 					<< ", ";
		ss << s->SeedMass 					<< ", ";
		ss << s->Dist 						<< ", ";
		ss << s->SLA 						<< ", ";
		ss << s->palat 						<< ", ";
		ss << s->Gmax 						<< ", ";
		ss << s->memory 					<< ", ";
		ss << s->clonal 					<< ", ";
		ss << s->meanSpacerlength 			<< ", ";
		ss << s->sdSpacerlength 			<< ", ";
		ss << p->Age 						<< ", ";
		ss << p->mshoot						<< ", ";
		ss << p->mroot 						<< ", ";
		ss << p->Radius_shoot() 			<< ", ";
		ss << p->Radius_root() 				<< ", ";
		ss << p->mRepro 					<< ", ";
		ss << p->lifetimeFecundity 			<< ", ";
		ss << p->stress							   ;

		print_row(ss, ind_stream);
	}
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
