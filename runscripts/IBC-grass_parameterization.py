## TODO:
# 1. It would be a good move to split up the "runs" so that multiple processors can work on the same simulation.
# 2. This script could be broken into smaller pieces and made less fragile.
# 3. It would be cool to safeguard the skipping parameterizations that are redundant 
#       functionality so that you don't screw up later without realizing it
# 4. Get rid of "IndividualVariationVers" and replace it with only SD.

import sys, os, subprocess, itertools, csv, copy, math, re, random
import PFT, Base_Parameter, Parallel

from PFT import *
from Base_Parameter import *
from Parallel import *

PARALLEL = True
SPAT_out = 0 # print spat_out grid
SPAT_out_year = 0 # only print out the spatial grid in year 100, if 0 every year
PFT_out = 1 # print PFT output
COMP_out = 0
N_SLOTS = 400

path = "./tmp/"
N_SIMS = 20
N_REPS = 1
n_PFTs = 16

Sim_header = "SimNrMax  NRep  OutFile\n" + \
                "18\t" + str(N_REPS) + "\tUPD2-SR\nSimNr ComNr ICvers InvasionVers IndividualVariationVers IndivVariationSD Tmax ARes Bres " + \
                "GrazProb PropRemove DistAreaYear AreaEvent NCut CutMass SeedRainType SeedInput SPATout SPATOutYear PFTout COMPout NameInitFile\n"

PFT_header = "ID Species MaxAge AllocSeed LMR m0 MaxMass mSeed Dist pEstab Gmax SLA palat memo RAR " + \
                        "growth mThres clonal propSex meanSpacerLength sdSpacerlength Resshare AllocSpacer mSpacer \n"

## These parameters are specific to the environment and "type" of the simulation, e.g.
## whether or not it is an "invasion" type, or an "individual variation" type.
base_params =  [[0, 1], # interspecific competition version
                [0], # invasionVers
                [0, 1], # IndividualVariationVers
                [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7], # indivVariationSD
                [100], # CTmax
                [10], # PftTmax
                [100], # ARes
                [100], # Bres
                [0.2], # GrazProb
                [0.5], # propRemove
                [0], # DistAreaYear
                [0], # AreaEvent
                [0], # NCut
                [0], # CutMass
                [0], # SeedRain
                [0]] # SeedInput

## These parameters are specific to each plant functional type. That is, this details the composition
## of functional traits.
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

def makePFTs(parallel = PARALLEL, N_SLOTS = N_SLOTS):

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
    sim_number = random.randint(0, 33554432) # This is so that you can run multiple simulations at once. 
    for s in xrange(1, N_SIMS+1): # one sim_number per sample of PFTypes. 
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
            SimFile.append(" ".join([base_simID, ComNr, base_param.toString(True), str(SPAT_out), str(SPAT_out_year), str(PFT_out), str(COMP_out), sim_filename, "\n"]))
            
            # community's PFT file    
            with open(path + sim_filename, 'w') as w: 
                w.write(PFT_header)
                counter = 0
                for p in community:
                    w.write(str(counter) + " " + str(p))
                    counter += 1

                    if (community.index(p) != len(community)-1): # This is critical. There can be no trailing newline on the end of the PFT file.
                        w.write("\n")

            # pft pairs
            if (base_param.invasionVers == 1):
                pairs = list(itertools.permutations(community, 2))
                for pair in pairs:
                    p, q = pair[0], pair[1]
                    if p.Species == q.Species:
                        continue
                    
                    # PFT pair's SimFile entry 
                    pft_simID = base_simID + str(p.Species) + str(q.Species)
                    PFT_filename = base_simID + "_" + "PFT" + "_" + pft_simID + ".txt"
                    SimFile.append(" ".join([pft_simID, ComNr, base_param.toString(False), str(SPAT_out), str(SPAT_out_year), str(PFT_out), str(COMP_out), PFT_filename, "\n"]))

                    # PFT pair's PFT file
                    with open(path + PFT_filename, 'w') as w:
                        w.write(PFT_header)

                        PFT_id = pft_simID + "1"
                        w.write(PFT_id + " " + str(p) + "\n")

                        PFT_id = pft_simID + "2"
                        w.write(PFT_id + " " + str(q))

    if (PARALLEL):
        buildBatchScripts(SimFile, N_SLOTS, path, Sim_header)
        os.system('cp ./queue.sh ./tmp')
    else:
        with open(path + "SimFile.txt", 'w') as w:
            w.write(Sim_header)
            w.writelines(sim for sim in SimFile)

if __name__ == "__main__":
    makePFTs()
