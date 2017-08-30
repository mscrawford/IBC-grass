import os
import re

os.chdir('/Users/Theodore/Desktop/data/data/out')
# os.chdir('/Users/Theodore/Desktop')

for f in os.listdir(os.getcwd()):
	if re.match(".*PFT.csv", f):
		with open(f, 'r') as i:
			with open("CLND_" + str(f), 'w') as o:
				for line in i:
		
					toPrint = True
					
					splitLine = line.split(", ")

					if (splitLine[0] == "SimID"):
						o.write(line)
						continue
						
					if ( int(splitLine[0].split("_")[1]) > 30 ): # Replicate is larger than 30
						toPrint = False
						
					elif (splitLine[4] == "0"): # this PFT is dead (this year)
						toPrint = False

					if (toPrint):
						o.write(line)

			os.remove(str(f))