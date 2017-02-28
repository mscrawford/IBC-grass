import sys, os, subprocess, itertools, csv, copy, math, re, random
import util

from util import *

path = "./tmp/"

PARALLEL = True
N_SLOTS = 500

weekly = 0 # print yearly (0) or weekly (1)?
ind_out = 0 # individual-level output (1)? 
pft_out = 0 # PFT-level output? 0: No, 1: Yes, no dead PFTs, 2: Yes, even dead PFTs
srv_out = 1 # Print survival statistics (1)? Bad idea with seed addition...
trait_out = 0 # Print trait-level output

N_COMS = 500
N_REPS = 50
n_PFTs = 16 # Doesn't matter with pairwise invasion criterion... (FIX THIS...)

MODE = 0 # Community Assembly (0), Invasion criterion (1), Catastrophic disturbance (2)
PFT_type = 0 # Theoretical (0) or Empirical (1) PFTs

Sim_header = "NRep " + str(N_REPS) + "\n" + \
                "SimNr ComNr IC_vers Mode ITVsd Tmax ARes Bres " + \
                "GrazProb PropRemove " + \
                "BelGrazProb BelGrazResidualPerc BelGrazPerc " + \
                "CatastrophicPlantMortality CatastrophicSeedMortality " + \
                "SeedRainType SeedInput " + \
                "weekly ind_out pft_out srv_out trait_out NameInitFile\n"

PFT_header = "ID Species MaxAge AllocSeed LMR m0 MaxMass mSeed Dist pEstab Gmax SLA palat memo RAR " + \
                "growth mThres clonal propSex meanSpacerLength sdSpacerlength Resshare AllocSpacer mSpacer\n"

base_params =  [[1], # IC version
                [MODE],
                [0, 0.0125, 0.025, 0.05, 0.1, 0.2, 0.5], # ITVsd
                [100], # Tmax
                [100], # ARes
                [90], # Bres
                [0.2], # GrazProb
                [0.5], # propRemove
                [0], # BelGrazProb
                [0], # BelGrazResidualPerc
                [0], # BelGrazPerc
                [0], # CatastrophicPlantMortality
                [0], # CatastrophicSeedMortality
                [0], # SeedRainType
                [0]] # SeedInput

# These parameters are specific to each plant functional type. That is, this details the composition
# of functional traits.
PFType_params = [[100], # MaxAge
                [0.05], # AllocSeed
                [1.0, 0.75, 0.50], # LMR
                [[1.0, 5000, 1.0, 0.1], # maxPlantSizeSet is a linked trait set
                 [0.3, 2000, 0.3, 0.3],# maxPlantSizeSet. Maximum plant size -- large
                 [0.1, 1000, 0.1, 0.6]], # Maximum plant size -- small
                [0.5], # pEstab
                [[60, 2],
                 [40, 4], # resourceCompetitionSet. Resource response -- competitor
                 [20, 6]], # Resource response -- tolerator
                [[1.00, 1.00],
                 [0.50, 0.75],# grazingResponseSet. Grazing response -- tolerator
                 [0.25, 0.50]], # Grazing response -- avoider
                [1], # RAR
                [0.25], # growth
                [0.2], # mThres
                [0], # clonal
                [0], # propSex
                [0], # meanSpacerLength
                [0], # sdSpacerLength
                [0], # Resshare
                [0], # AllocSpacer
                [0]] # mSpacer

def buildBatchScripts(SimFile, n_cores, path, Sim_header):
    sims_per_core = int(math.ceil(len(SimFile)/float(n_cores)))
    sim_files = []
    for core in xrange(1, n_cores+1): # one new SimFile for each core
        i = 0
        fn = "SimFile_" + str(core) + ".txt"
        with open(path + fn, 'w') as w:
            w.write(Sim_header)
            while (i < sims_per_core and len(SimFile) > 0):
                w.write(SimFile.pop())
                i+=1
        if (i == 0):
            os.remove(path + fn)
        else:
            sim_files.append(fn)

    with open('./resources/BatchTemplate.txt', 'r') as r:
        base = r.read()
        batch_number = 1
        for s in sim_files:
            batch_name = "batch_" + str(batch_number)
            replace_string = s
            with open('./tmp/' + batch_name + ".sub", 'w') as w:
                w.write(re.sub('@SIMFILE@', replace_string, base))
            batch_number += 1


def makeTheoreticalPFTs(parallel = PARALLEL, N_SLOTS = N_SLOTS):
    assert(n_PFTs <= 81)

    # compose the superset of PFTs
    pfts = []
    counter = 0
    for p in itertools.product(*PFType_params):
        counter += 1
        pft = PFT(counter, *p)
        pfts.append(pft)

    SimFile = [] # all the simulations go into one 'SimFile.' This is a list of strings.

    community_number = 0
    sim_number = random.randint(0, 2147483647) # This is so that you can run multiple simulations at once. 
    for s in xrange(1, N_COMS+1): # one sim_number per sample of PFTypes. 
        community = random.sample(pfts, n_PFTs)
        community_number += 1
        
        for base_param in itertools.product(*base_params):
            
            try:
                base_param = Base_Parameter(*base_param)
            except:
                continue

            sim_number += 1 # IBC-grass will barf if sim_number starts with 0.
            base_simID = str(sim_number)
            ComNr = str(community_number)
            sim_filename = base_simID + "_" + "COM" + ".txt"

            # community's SimFile entry
            SimFile.append(" ".join([base_simID, ComNr, base_param.toString(), 
                                    str(weekly), str(ind_out), str(pft_out), str(srv_out), str(trait_out), sim_filename, "\n"]))
            
            # community's PFT file    
            with open(path + sim_filename, 'w') as w: 
                w.write(PFT_header)
                counter = 0
                for p in community:
                    w.write(str(counter) + " " + str(p))
                    counter += 1

                    # This is critical. There can be no trailing newline on the end of the PFT file.
                    if (community.index(p) != len(community)-1): 
                        w.write("\n")

    if (PARALLEL):
        buildBatchScripts(SimFile, N_SLOTS, path, Sim_header)
        os.system('cp ./resources/queue.sh ./tmp')
    else:
        with open(path + "SimFile.txt", 'w') as w:
            w.write(Sim_header)
            w.writelines(sim for sim in SimFile)

def makeEmpiricalPFTs():

    # Read in Lina's superset of PFTs
    with open("./resources/selectWeiss.txt", "r") as r:
        pfts = r.read().splitlines()
        pfts.pop(0)
        pfts = [pft.split(' ') for pft in pfts]
        [pft.pop(1) for pft in pfts]
        pfts = [" ".join(pft) for pft in pfts]

    SimFile = [] # all the simulations go into one 'SimFile.' This is a list of strings.
    community_number = 0
    sim_number = random.randint(0, 2147483647) # This is so that you can run multiple simulations at once. 

    for s in xrange(1, N_COMS+1): # one sim_number per sample of PFTypes. 
        community = random.sample(pfts, len(pfts) if n_PFTs == 0 else n_PFTs)
        community_number += 1

        for base_param in itertools.product(*base_params):
            
            try:
                base_param = Base_Parameter(*base_param)
            except:
                continue

            sim_number += 1 # IBC-grass will barf if sim_number starts with 0.
            base_simID = str(sim_number)
            ComNr = str(community_number)
            sim_filename = base_simID + "_" + "COM" + ".txt"

            # community's SimFile entry
            SimFile.append(" ".join([base_simID, ComNr, base_param.toString(),
                                    str(weekly), str(ind_out), str(pft_out), str(srv_out), str(trait_out), sim_filename, "\n"]))
            
            # community's PFT file    
            with open(path + sim_filename, 'w') as w: 
                w.write(PFT_header)
                counter = 0
                for p in community:
                    w.write(str(counter) + " " + str(p))
                    counter += 1

                    if (community.index(p) != len(community)-1): # This is critical. There can be no trailing newline on the end of the PFT file.
                        w.write("\n")

    if (PARALLEL):
        buildBatchScripts(SimFile, N_SLOTS, path, Sim_header)
        os.system('cp ./resources/queue.sh ./tmp')

    else:
        with open(path + "SimFile.txt", 'w') as w:
            w.write(Sim_header)
            w.writelines(sim for sim in SimFile)


def makePFTPairs():

    pfts = []
    counter = 0
    for p in itertools.product(*PFType_params):
        counter += 1
        pft = PFT(counter, *p)
        pfts.append(pft)

    for p in pfts:
        print("PFT:" + str(p))

    pairs = list(itertools.permutations(pfts, 2))
    for pair in pairs:
        print ("A: " + str(pair[0].Species) + "; B: " + str(pair[1].Species))
    print("Total length: " + str(len(pairs)))

    SimFile = [] # all the simulations go into one 'SimFile.' This is a list of strings.
    sim_number = random.randint(0, 2147483647)
    ComNr = 0

    for pair in pairs:
        p, q = pair[0], pair[1]
        ComNr += 1

        for base_param in itertools.product(*base_params):
            
            try:
                base_param = Base_Parameter(*base_param)
            except:
                continue

            # PFT pair's SimFile entry
            sim_number += 1 # IBC-grass will barf if sim_number starts with 0.
            base_simID = str(sim_number) 
            
            PFT_filename = base_simID + ".txt"
            SimFile.append(" ".join([base_simID, str(ComNr), base_param.toString(),
                                    str(weekly), str(ind_out), str(pft_out), str(srv_out), str(trait_out), PFT_filename, "\n"]))
                       

            # PFT pair's PFT file
            with open(path + PFT_filename, 'w') as w:
                w.write(PFT_header)
                w.write("1 " + str(p) + "\n")
                w.write("2 " + str(q))

    if (PARALLEL):
        buildBatchScripts(SimFile, N_SLOTS, path, Sim_header)
        os.system('cp ./resources/queue.sh ./tmp')
    else:
        with open(path + "SimFile.txt", 'w') as w:
            w.write(Sim_header)
            w.writelines(sim for sim in SimFile)


if __name__ == "__main__":
    if (MODE == 1):
        makePFTPairs()
    else:
        if (PFT_type == 0):
            makeTheoreticalPFTs()
        elif (PFT_type == 1):
            makeEmpiricalPFTs()

    with open('IBC-grass_parameterization.py', 'r') as r:
        words = r.readlines()
        with open(path + 'param_doc.txt', 'w') as w:
            w.write("*************************************************************************************\n")
            w.write("This is a copy of the parameterization file that this simulation set is derived from.\n")
            w.write("*************************************************************************************\n")
            w.write("\n\n___________________________________________________________________________________\n\n")
            w.writelines(words)
