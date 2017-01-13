#include "Output.h"
#include "RunPara.h"
#include "CGridEnvir.h"
#include "CGrid.h"
#include "CEnvir.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <cassert>

using namespace std;

const vector<string> Output::param_header
	({
			"SimID", "ComNr", "RunNr",
			"IC_vers", "ITVsd", "Tmax",
			"ARes", "BRes",
			"GrazProb", "PropRemove",
			"BelGrazProb", "BelGrazStartYear", "BelGrazWindow", "BelGrazMode", "BelGrazGrams",
			"catastrophicDistYear", "CatastrophicPlantMortality", "CatastrophicSeedMortality",
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

const vector<string> Output::PFT_header
	({
			"SimID", "PFT", "Year", "Week", "Pop", "Shootmass", "Rootmass"
	});

const vector<string> Output::ind_header
	({
			"SimID", "plantID", "PFT", "Year", "Week", "xcoord", "ycoord",
			"i_LMR",
			"i_m0", "i_MaxMass", "i_SeedMass", "i_Dist",
			"i_SLA", "i_palat",
			"i_Gmax", "i_memory",
			"i_clonal", "i_meanSpacerlength", "i_sdSpacerlength",
			"Age",
			"mshoot", "mroot", "rShoot", "rRoot",
			"mRepro", "lifetimeFecundity",
			"stress"
	});

struct Output::PFT_struct {
		double Shootmass;
		double Rootmass;
		int Pop;
		int Nseeds;

		PFT_struct() {
			Shootmass = 0;
			Rootmass = 0;
			Pop = 0;
			Nseeds = 0;
		};

		~PFT_struct(){};
	};

Output::Output() :
		weekly(0),
		ind_out(0),
		param_fn("data/out/param.txt"),
		trait_fn("data/out/trait.txt"),
		PFT_fn("data/out/PFT.txt"),
		ind_fn("data/out/ind.txt")
{
	;
}

Output::~Output()
{
	Output::param_stream.close();
	Output::trait_stream.close();
	Output::PFT_stream.close();
	Output::ind_stream.close();
}

void Output::setupOutput(int weekly, int ind_out, string param_fn, string trait_fn, string PFT_fn, string ind_fn)
{

	Output::weekly = weekly;
	Output::ind_out = ind_out;

	Output::param_fn = param_fn;
	Output::trait_fn = trait_fn;
	Output::PFT_fn = PFT_fn;
	Output::ind_fn = ind_fn;

	param_stream.open(param_fn.c_str(), ios_base::app);
	trait_stream.open(trait_fn.c_str(), ios_base::app);
	PFT_stream.open(PFT_fn.c_str(), ios_base::app);
	ind_stream.open(ind_fn.c_str(), ios_base::app);

	assert(param_stream.good() && trait_stream.good() && PFT_stream.good() && ind_stream.good());

	// Write param_stream's header
	print_row(param_header, param_stream);
	print_row(trait_header, trait_stream);
	print_row(PFT_header, PFT_stream);
	print_row(ind_header, ind_stream);

}

void Output::print_param()
{
	vector<string> row = vector<string>();

	row.push_back(to_string(CEnvir::SimNr));
	row.push_back(to_string(CEnvir::ComNr));
	row.push_back(to_string(CEnvir::RunNr));
	row.push_back(to_string(SRunPara::RunPara.Version));
	row.push_back(to_string(SRunPara::RunPara.ITVsd));
	row.push_back(to_string(SRunPara::RunPara.Tmax));
	row.push_back(to_string(SRunPara::RunPara.meanARes));
	row.push_back(to_string(SRunPara::RunPara.meanBRes));
	row.push_back(to_string(SRunPara::RunPara.GrazProb));
	row.push_back(to_string(SRunPara::RunPara.PropRemove));
	row.push_back(to_string(SRunPara::RunPara.BelGrazProb));
	row.push_back(to_string(SRunPara::RunPara.BelGrazStartYear));
	row.push_back(to_string(SRunPara::RunPara.BelGrazWindow));
	row.push_back(to_string(SRunPara::RunPara.BelGrazMode));
	row.push_back(to_string(SRunPara::RunPara.BelGrazGrams));
	row.push_back(to_string(SRunPara::RunPara.catastrophicDistYear));
	row.push_back(to_string(SRunPara::RunPara.CatastrophicPlantMortality));
	row.push_back(to_string(SRunPara::RunPara.CatastrophicSeedMortality));
	row.push_back(to_string(SRunPara::RunPara.SeedRainType));
	row.push_back(to_string(SRunPara::RunPara.SeedInput));

	assert(row.size() == param_header.size());

	print_row(row, param_stream);
}

void Output::print_trait()
{

	for (auto it : SPftTraits::PftLinkList)
	{
		vector<string> row = vector<string>();
		row.push_back(to_string(CEnvir::SimNr));
		row.push_back(it.first);
		row.push_back(to_string(it.second->LMR));
		row.push_back(to_string(it.second->m0));
		row.push_back(to_string(it.second->MaxMass));
		row.push_back(to_string(it.second->SeedMass));
		row.push_back(to_string(it.second->Dist));
		row.push_back(to_string(it.second->SLA));
		row.push_back(to_string(it.second->palat));
		row.push_back(to_string(it.second->Gmax));
		row.push_back(to_string(it.second->memory));
		row.push_back(to_string(it.second->clonal));
		row.push_back(to_string(it.second->meanSpacerlength));
		row.push_back(to_string(it.second->sdSpacerlength));

		assert(row.size() == trait_header.size());

		print_row(row, trait_stream);
	}
}

void Output::print_PFT(vector<CPlant*> & PlantList, CCell** & CellList)
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

		if (p->dead) continue;

		PFT_struct* s = &(PFT_map[p->pft()]);

		s->Pop = s->Pop + 1;
		s->Rootmass = s->Rootmass + p->mroot;
		s->Shootmass = s->Shootmass + p->mshoot;
	}

	for (int i = 0; i < SRunPara::RunPara.GetSumCells(); ++i) {
		CCell* c = CellList[i];
		for (auto seed_it : c->SeedBankList)
		{
			PFT_struct pft = PFT_map[seed_it->pft()];
			pft.Nseeds++;
		}
	}

	// print each PFT
	for (auto it : PFT_map) {
		vector<string> row = vector<string>();

		row.push_back(to_string(CEnvir::SimNr));
		row.push_back(it.first); // PFT name
		row.push_back(to_string(CEnvir::year));
		row.push_back(to_string(CEnvir::week));
		row.push_back(to_string(it.second.Pop));
		row.push_back(to_string(it.second.Shootmass));
		row.push_back(to_string(it.second.Rootmass));

		assert(row.size() == PFT_header.size());

		print_row(row, PFT_stream);
	}

	//	 delete PFT_map
	PFT_map.clear();
}

void Output::print_ind(vector<CPlant*> & PlantList)
{
	for (auto it : PlantList)
	{
		CPlant* p = it;

		if (p->dead) continue;

		SPftTraits* s = p->Traits;

		vector<string> row = vector<string>();

		row.push_back(to_string(CEnvir::SimNr));
		row.push_back(to_string(p->plantID));
		row.push_back(p->pft());
		row.push_back(to_string(CEnvir::year));
		row.push_back(to_string(CEnvir::week));
		row.push_back(to_string(p->xcoord));
		row.push_back(to_string(p->ycoord));
		row.push_back(to_string(s->LMR));
		row.push_back(to_string(s->m0));
		row.push_back(to_string(s->MaxMass));
		row.push_back(to_string(s->SeedMass));
		row.push_back(to_string(s->Dist));
		row.push_back(to_string(s->SLA));
		row.push_back(to_string(s->palat));
		row.push_back(to_string(s->Gmax));
		row.push_back(to_string(s->memory));
		row.push_back(to_string(s->clonal));
		row.push_back(to_string(s->meanSpacerlength));
		row.push_back(to_string(s->sdSpacerlength));
		row.push_back(to_string(p->Age));
		row.push_back(to_string(p->mshoot));
		row.push_back(to_string(p->mroot));
		row.push_back(to_string(p->Radius_shoot()));
		row.push_back(to_string(p->Radius_root()));
		row.push_back(to_string(p->mRepro));
		row.push_back(to_string(p->lifetimeFecundity));
		row.push_back(to_string(p->stress));

		assert(row.size() == ind_header.size());

		print_row(row, ind_stream);
	}
}

// Prints a row of data out a string, as a comma separated list with a newline at the end.
void Output::print_row(vector<string> row, ofstream & stream)
{
	assert(row.size() > 0);
	assert(stream.good());

	std::ostringstream ss;
	std::copy(row.begin(), row.end() - 1, std::ostream_iterator<string>(ss, ", "));
	ss << row.back();

	stream << ss.str() << endl;

	stream.flush();
}
