import sys, os, subprocess, itertools, csv, copy, math, re, random
import util

from util import *

PATH = "./tmp/"


####################################################################
#### Parameters
####################################################################

##########################################
### Hyperparameters

# For computing clusters
PARALLEL = False         # IF SERIES THIS -> FALSE
N_SLOTS  = 300          # Number of cores to split between
H_RT     = "08:00:00"   # Maximum runtime for simulations (08:00:00 = 8 hours)
H_VMEM   = "2G"         # Memory for each simulation run

# Frequency and type of output
weekly    = 1 # Print output yearly (0) or weekly (1)?

ind_out   = 0 # Print individual-level output?           (0) No; (1) Yes
pft_out   = 2 # Print PFT-level output:                  (0) No; (1) Yes, without repeating dead PFTs; (2) Yes, repeating dead PFTs
srv_out   = 0 # Print PFT-survival output:               (0) No; (1) Yes !!!-> NOT COMPATIBLE WITH SEED ADDITION
trait_out = 1 # Print trait-level output:                (0) No; (1) Yes
meta_out  = 1 # Print output about the environment, etc. (0) No; (1) Year

# Number of repetitions
N_REPS    = 1

# Number of communities and what kind of PFTs to use
N_COMS    = 1           # UNUSED WITH PAIRWISE INVASION CRITERION
PFT_type  = "EMPIRICAL" # "THEORETICAL" or "EMPIRICAL"

##########################################
### Environmental parameters

IC_vers = [1] # IBC-grass run mode -- Negative frequency dependence
MODE    = [2] # (0) Community Assembly; (1) Invasion criterion; (2) Catastrophic disturbance
N_PFTs  = [0] # UNUSED WITH PAIRWISE INVASION CRITERION
ITVsd   = [0]
Tmax    = [100]

# Custom environment time series --- PLEASE ONLY SINGLE VALUES
ENV   = [0] # (0) Static environment; (1) IBC-grass uses custom environmental time series
SIGMA = [0] # Variability with which the time series changes

# Resource levels
ARes  = [90]
BRes  = [90] # With belowground environmental variation, this MUST be NA

# Aboveground grazing 
GrazProb   = [0.2]
propRemove = [0.5]

# Belowground grazing
BelGrazProb = [1]
BelGrazPerc = [0.10, 0.20, 0.30, .40]

# Catastrophic disturbance
CatastrMort_Plant = [1]
CatastrMort_Seed  = [0]

# Seed introduction
SeedRainType = [1]
SeedInput    = [10]

##########################################
### Permutable list...

base_params =  [IC_vers,
                MODE,
                ITVsd,
                Tmax,
                ENV,
                SIGMA,
                ARes,
                BRes,
                GrazProb,
                propRemove,
                BelGrazProb,
                BelGrazPerc,
                CatastrMort_Plant,
                CatastrMort_Seed,
                SeedRainType,
                SeedInput]


####################################################################
#### Theoretical PFTs
####################################################################

# When creating the theoretical PFTs, each PFT is a permutation of the below nested list of traits
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


####################################################################
#### Header strings
####################################################################

SIM_HEADER = "NRep " + str(N_REPS) + "\n" + \
                "SimID ComNr IC_vers Mode ITVsd Tmax " + \
                "Env Sigma " + \
                "ARes Bres " + \
                "GrazProb PropRemove " + \
                "BelGrazProb BelGrazPerc " + \
                "CatastrophicPlantMortality CatastrophicSeedMortality " + \
                "SeedRainType SeedInput " + \
                "weekly ind_out pft_out srv_out trait_out NameInitFile\n"

PFT_HEADER = "ID Species MaxAge AllocSeed LMR m0 MaxMass mSeed Dist pEstab Gmax SLA palat memo RAR " + \
                "growth mThres clonal propSex meanSpacerLength sdSpacerlength Resshare AllocSpacer mSpacer\n"


####################################################################
#### Parallel setup helper-function
####################################################################

def buildBatchScripts(SimFile):
    sims_per_core = int( math.ceil( len(SimFile)/float(N_SLOTS) ) )
    sim_files = []
    for core in xrange(1, N_SLOTS+1): # one new SimFile for each core
        i = 0
        fn = "SimFile_" + str(core) + ".txt"
        with open(PATH + fn, 'w') as w:
            w.write(SIM_HEADER)
            while ( i < sims_per_core and len(SimFile) > 0 ):
                w.write(SimFile.pop())
                i += 1
        if (i == 0):
            os.remove(PATH + fn)
        else:
            sim_files.append(fn)

    with open('./resources/BatchTemplate.txt', 'r') as r:
        base = r.read()
        batch_number = 1
        base = re.sub('@RUNTIME@', H_RT, base)
        base = re.sub('@MEMORY@', H_VMEM, base)
        for s in sim_files:
            batch_name = "batch_" + str(batch_number)
            replace_string = s
            batch_text = re.sub('@SIMFILE@', replace_string, base)
            batch_text = re.sub('@PREFIX@', batch_name, batch_text)
            with open('./tmp/' + batch_name + ".sub", 'w') as w:
                w.write(batch_text)
            batch_number += 1


####################################################################
#### Generating environmental variation
#### Notes:
####    - Make sure that there is only one level of Tmax
####    - Make sure that "Bres" is set to 'NA'
####################################################################

def generateEnvironmentalVariation():
    assert(base_params[7][0] == 'NA') # Belowground resources = 'NA'
    assert(base_params[5][0] > 0) # Sigma is non-zero
    assert(len(base_params[3]) == 1) # Environmental variation will only work if the length of the simulation is uniform
    subprocess.call(['Rscript', '--vanilla', 'BrownianMotion.R', str(N_REPS), str(base_params[3][0]), str(SIGMA)])


####################################################################
#### Default setup
####################################################################

def buildPFTs():

    # Compose the superset of PFTs
    PFTs = []
    if (PFT_type == "THEORETICAL"):
        counter = 0
        for pft in itertools.product(*PFType_params):
            counter += 1
            pft = PFT(counter, *pft)
            PFTs.append(pft)

    elif (PFT_type == "EMPIRICAL"):
        with open("./resources/selectWeiss.txt", "r") as r:
            PFTs = r.read().splitlines()
            PFTs.pop(0)
            PFTs = [pft.split(' ') for pft in PFTs]
            [pft.pop(1) for pft in PFTs]
            PFTs = [" ".join(pft) for pft in PFTs]

    else:
        raise Exception("Wrong PFT type.")

    SimFile = [] # All the simulations go into one 'SimFile.' This is a list of strings.
    ComNr = 0 
    SimNr = random.randint(0, 2147483647) # Used to join datasets

    for s in xrange(1, N_COMS+1): # one SimNr per sample of PFTypes. 
        for n in N_PFTs:
            community = random.sample(PFTs, len(PFTs) if n == 0 else n)
            ComNr += 1
            Com_FN = "COM_" + str(ComNr) + ".txt"

            for base_param in itertools.product(*base_params):
                try:
                    base_param = Base_Parameter(*base_param)
                except:
                    continue

                SimNr += 1 # IBC-grass will barf if SimNr starts with 0.
                SimFile.append(" ".join([str(SimNr), str(ComNr), base_param.toString(), str(weekly), str(ind_out), str(pft_out), str(srv_out), str(trait_out), Com_FN, "\n"]))
                
                # community's PFT file    
            with open(PATH + Com_FN, 'w') as w: 
                w.write(PFT_HEADER)
                counter = 0
                for p in community:
                    w.write(str(counter) + " " + str(p))
                    counter += 1

                    # This is critical. There can be no trailing newline on the end of the PFT file.
                    if (community.index(p) != len(community)-1): 
                        w.write("\n")

    return (SimFile)


####################################################################
#### Pairwise setup
####################################################################

def buildPairs():

    PFTs = []
    counter = 0
    for p in itertools.product(*PFType_params):
        counter += 1
        pft = PFT(counter, *p)
        PFTs.append(pft)

    pairs = list(itertools.permutations(PFTs, 2))
    
    print("Total length: " + str(len(pairs)))

    SimFile = [] # all the simulations go into one 'SimFile.' This is a list of strings.
    SimNr = random.randint(0, 2147483647)
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
            SimNr += 1 # IBC-grass will barf if SimNr starts with 0.
            PFT_FN = "Pair_" + str(ComNr) + ".txt"

            SimFile.append(" ".join([str(SimNr), str(ComNr), base_param.toString(), str(weekly), str(ind_out), str(pft_out), str(srv_out), str(trait_out), PFT_FN, "\n"]))

        # PFT pair's PFT_file
        with open(PATH + PFT_FN, 'w') as w:
            w.write(PFT_HEADER)
            w.write("1 " + str(p) + "\n")
            w.write("2 " + str(q))

    return (SimFile)


####################################################################
#### MAIN
####################################################################

if __name__ == "__main__":
    
    SimFile = []
    if (MODE == 1):
        SimFile = buildPairs()
    else:
        SimFile = buildPFTs()

    if (ENV == 1):
        generateEnvironmentalVariation()

    if (PARALLEL):
        buildBatchScripts(SimFile)
        os.system('cp ./resources/queue.sh ./tmp')
    else:
        with open(PATH + "SimFile.txt", 'w') as w:
            w.write(SIM_HEADER)
            w.writelines(sim for sim in SimFile)

    with open('IBC-grass_parameterization.py', 'r') as r:
        words = r.readlines()
        with open(PATH + 'param_doc.txt', 'w') as w:
            w.write("*************************************************************************************\n")
            w.write("This is a copy of the parameterization file that this simulation set is derived from.\n")
            w.write("*************************************************************************************\n")
            w.write("\n\n___________________________________________________________________________________\n\n")
            w.writelines(words)
