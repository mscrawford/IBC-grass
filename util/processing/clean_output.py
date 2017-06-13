import os

os.chdir('/Users/Theodore/Desktop/data.1/data/out')

for f in os.listdir(os.getcwd()):
	simCount = 0
	with open(f, 'r') as i:
		with open("CLND_" + str(f), 'w') as o:
			for line in i:
				if (line.split(', ', 1)[0] == "SimID"):
					simCount = simCount + 1
				if (simCount == 2):
					o.write(line)
	if (simCount > 2):
		print("ERROR: SimID is written at least three times!")
	if (simCount == 2):
			os.remove(str(f))
	else:
		os.remove("CLND_" + str(f))