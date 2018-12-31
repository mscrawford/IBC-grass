import sys, os, subprocess, itertools, csv, copy, math, re, random
import util

from util import *

####################################################################
#### Simulation setup parameters
#### If you want to have the setup script remove everything in
#### the input and output folders, and then write into the input
#### folder, set DESTRUCTIVE_SETUP to True. Otherwise, it will 
#### populate the tmp folder.
####################################################################

DESTRUCTIVE_SETUP = True

tmp_folder = "./tmp/"
in_folder  = "../../data/in/"
out_folder = "../../data/out/"

if (DESTRUCTIVE_SETUP):
    for f in os.listdir(in_folder):
        os.remove(in_folder + f)
    for f in os.listdir(out_folder):
        os.remove(out_folder + f)
    PATH = in_folder
else:
    for f in os.listdir(tmp_folder):
        os.remove(tmp_folder + f)
    PATH = tmp_folder

####################################################################
#### Parameters
####################################################################

##########################################
### Hyperparameters

# For computing clusters
PARALLEL = True         # IF SERIES THIS -> FALSE
N_SLOTS  = 400          # Number of cores to split between
H_RT     = "5:00:00"   # Maximum runtime for cluster simulations (08:00:00 = 8 hours)
H_VMEM   = "1G"         # Memory for each simulation run

# Frequency and type of output
weekly    = 0 # Print output yearly (0) or weekly (1)?

individual_out   = 1 # Print individual-level output?           (0) No; (1) Yes
population_out   = 1 # Print PFT-level output:                  (0) No; (1) Yes, without repeating dead PFTs; (2) Yes, repeating dead PFTs
populationSurvival_out   = 0 # Print PFT-survival output:       (0) No; (1) Yes !!!-> NOT COMPATIBLE WITH SEED ADDITION
trait_out = 1       # Print trait-level output:                 (0) No; (1) Yes
community_out   = 1 # Print output about the environment, etc.  (0) No; (1) Yes

# Number of repetitions 
N_REPS    = 1

# Number of communities and what kind of PFTs to use
N_COMS    = 50            # UNUSED WITH PAIRWISE INVASION CRITERION
PFT_type  = "EMPIRICAL"   # "THEORETICAL" or "EMPIRICAL"

##########################################
### Environmental parameters

IC_vers          = [1] # IBC-grass run mode -- Negative frequency dependence
MODE             = [3] # (0) Community Assembly; (1) Invasion criterion; (2) Catastrophic disturbance; (3) Eutrophication scenario
N_PFTs           = [0] # UNUSED WITH PAIRWISE INVASION CRITERION
ITVsd            = [0]
Tmax             = [300]

# Custom environment time series --- PLEASE ONLY SINGLE VALUES
ENV              = [0] # (0) Static environment; (1) IBC-grass uses custom environmental time series
SIGMA            = [0] # Variability with which the time series changes

# Resource levels
ARes             = [100]
BRes             = [60] # With belowground environmental variation, this MUST be NA

# Aboveground grazing 
AbvGrazProb      = [0.2]
AbvGrazPerc      = [0.5]

# Belowground grazing
BelGrazProb      = [1]
BelGrazPerc      = [0.1]
BelGrazThreshold = [0.0667616]
BelGrazAlpha     = [1, 1.25, 1.5, 1.75, 2]
BelGrazWindow    = [10]

# Seed bank
SeedLongevity          = [1] # Number of years a seed can theoretically persist within the seed bank

# Seed introduction
SeedRainType           = [1]
SeedInput              = [20]

# Experiment parameters
ExperimentDuration  = [100]

# Catastrophic disturbance
DisturbanceMortality   = [0]
DisturbanceWeek        = [0]

# Eutrophication
EutrophicationIntensity = [0, 30]

# Insecticide treatments
AbvHerbExclusion = [0, 1]
BelHerbExclusion = [0, 1]

# Sensitivity analyses

# BelGrazProb          = [0, 1, 0.90, 0.80, 0.70, 0.60, 0.50]
# BelGrazPerc          = [0, 0.2]
# BelGrazThreshold     = [0, 0.0667616]
# BelGrazAlpha         = [0, 1, 1.25]
# BelGrazWindow        = [0, 10]

# BelGrazProb          = [0, 1]
# BelGrazPerc          = [0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.30, 0.35, 0.40]
# BelGrazThreshold     = [0, 0.0667616]
# BelGrazAlpha         = [0, 1, 1.25]
# BelGrazWindow        = [0, 10]

# BelGrazProb          = [0, 1]
# BelGrazPerc          = [0, 0.2]
# BelGrazThreshold     = [0, 0.04673312, 0.05340928, 0.06008544, 0.06676160, 0.07343776, 0.08011392, 0.08679008]
# BelGrazAlpha         = [0, 1, 1.25]
# BelGrazWindow        = [0, 30]

# BelGrazProb          = [0, 1]
# BelGrazPerc          = [0, 0.2]
# BelGrazThreshold     = [0, 0.0667616]
# BelGrazAlpha         = [0, 1, 1.125, 1.25, 1.50, 2, 3]
# BelGrazWindow        = [0, 10]

# BelGrazProb          = [0, 1]
# BelGrazPerc          = [0, 0.2]
# BelGrazThreshold     = [0, 0.0667616]
# BelGrazAlpha         = [0, 1, 1.25]
# BelGrazWindow        = [0, 1, 5, 10, 20, 30, 60]

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
                AbvGrazProb,
                AbvGrazPerc,
                BelGrazProb,
                BelGrazPerc,
                BelGrazThreshold,
                BelGrazAlpha,
                BelGrazWindow,
                DisturbanceMortality,
                DisturbanceWeek,
                ExperimentDuration,
                EutrophicationIntensity,
                AbvHerbExclusion,
                BelHerbExclusion,
                SeedLongevity,
                SeedRainType,
                SeedInput]


####################################################################
#### Theoretical PFTs
####################################################################

# When creating the theoretical PFTs, each PFT is a permutation of the below nested list of traits
PFType_params = [[0.05], # AllocSeed
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
                [0], # meanSpacerLength
                [0], # sdSpacerLength
                [0], # Resshare
                [0], # AllocSpacer
                [0]] # mSpacer


####################################################################
#### Header strings
####################################################################

# SIM_HEADER = "NRep " + str(N_REPS) + "\n" + \
#                 "SimID ComNr IC_vers Mode ITVsd Tmax " + \
#                 "Env Sigma " + \
#                 "ARes BRes " + \
#                 "AbvGrazProb AbvGrazPerc " + \
#                 "BelGrazProb BelGrazPerc BelGrazThreshold " + \
#                 "BelGrazAlpha BelGrazWindow " + \
#                 "DisturbanceMortality DisturbanceWeek " + \
#                 "SeedLongevity SeedRainType SeedInput " + \
#                 "weekly individual_out population_out populationSurvival_out trait_out community_out NameInitFile\n"

SIM_HEADER = "NRep " + str(N_REPS) + "\n" + \
                "SimID ComNr IC_vers Mode ITVsd Tmax " + \
                "ARes BRes " + \
                "AbvGrazProb AbvGrazPerc " + \
                "BelGrazProb BelGrazPerc BelGrazThreshold " + \
                "BelGrazAlpha BelGrazWindow " + \
                "DisturbanceMortality DisturbanceWeek " + \
                "ExperimentDuration EutrophicationIntensity " + \
                "AbvHerbExclusion BelHerbExclusion " + \
                "SeedLongevity SeedRainType SeedInput " + \
                "weekly individual_out population_out populationSurvival_out trait_out community_out NameInitFile\n"

PFT_HEADER = "Species AllocSeed LMR m0 MaxMass mSeed Dist pEstab Gmax SLA palat memo RAR " + \
                "growth mThres clonal meanSpacerLength sdSpacerlength Resshare AllocSpacer mSpacer\n"


####################################################################
#### Parallel setup helper-function
####################################################################

def buildBatchScripts(SimFile):
    sims_per_core = int( math.ceil( len(SimFile)/float(N_SLOTS) ) )
    
    print("SimFile length: " + str(len(SimFile)))
    print("sims_per_core:  " + str(sims_per_core))

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
            with open(PATH + batch_name + ".sub", 'w') as w:
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
                SimFile.append(" ".join([str(SimNr), str(ComNr), base_param.toString(), \
                    str(weekly), str(individual_out), str(population_out), str(populationSurvival_out), str(trait_out), str(community_out), Com_FN, "\n"]))

                # community's PFT file    
            with open(PATH + Com_FN, 'w') as w: 
                w.write(PFT_HEADER)
                counter = 0
                for p in community:
                    w.write(str(p))
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

            SimFile.append(" ".join([str(SimNr), str(ComNr), base_param.toString(), \
                str(weekly), str(individual_out), str(population_out), str(populationSurvival_out), str(trait_out), str(community_out), PFT_FN, "\n"]))


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
        os.system('cp ./resources/queue.sh ' + PATH)
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
