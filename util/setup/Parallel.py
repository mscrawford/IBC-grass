import sys, os, subprocess, itertools, csv, copy, math, re, random

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

    with open('BatchTemplate.txt', 'r') as r:
        base = r.read()
        batch_number = 1
        for s in sim_files:
        	batch_name = "batch_" + str(batch_number)
        	replace_string = s
        	with open('./tmp/' + batch_name + ".sub", 'w') as w:
        		w.write(re.sub('@SIMFILE@', replace_string, base))
        	batch_number += 1