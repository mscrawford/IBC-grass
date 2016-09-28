import random

class Plant():
	plantID = 0

	def __init__(self, belGrBiomass, palat):
		self.belGrBiomass = belGrBiomass
		self.palat = palat
		self.plantID = Plant.plantID
		Plant.plantID += 1 # increment the number of plants

	def removeRootMass(self, prop_remove):
		_mass_removed = 0
		if (self.belGrBiomass > 1):
			_mass_removed = prop_remove * self.belGrBiomass
			self.belGrBiomass = self.belGrBiomass - _mass_removed
		return (_mass_removed)

	def __str__(self):
		return(" ".join(map(str, [self.plantID, self.belGrBiomass, self.palat])))


def grazingBelGr(mode, plantList, prop_remove):
	if (mode == 0):
		for plant in plantList:
			plant.removeRootMass(prop_remove)
	else:
		_plantsToGraze = plantList
		_mass_removed = 0
		_mass_to_remove = sum(p.belGrBiomass for p in plantList) * prop_remove

		print("Mass to remove: " + str(_mass_to_remove) + "\n\n")

		while(_mass_removed < _mass_to_remove):
			_max_value = 0 # I think this is the most likely plant to be grazed...?
			_mass_removed_start = _mass_removed
			for p in _plantsToGraze:
				if (mode == 3):
					_max_value = max(_max_value, p.belGrBiomass * p.palat)
				else:
					Exception("Mode not recognized.")
			random.shuffle(plantList) # some stochasiticity, I guess
			c = 0
			while (c < len(plantList) and _mass_removed < _mass_to_remove):
				p = plantList[c]

				if (p.belGrBiomass) == 0:
					continue
				elif (p.belGrBiomass) < 0:
					Exception("Belowground biomass less than 0")

				if (mode == 3):
					_graz_prob = (p.belGrBiomass * p.palat) / _max_value
					print ("Plant: " + str(p.plantID) + " has a grazing probability of: " + str(_graz_prob))

				if (random.uniform(0, 1) < _graz_prob):
					_mass_removed += p.removeRootMass(prop_remove)

				c += 1

			if (_mass_removed_start >= _mass_removed):
				Exception("Shuffling has caused and a high prop_remove has killed most plants.")

			print ("This loop " + str(_mass_removed - _mass_removed_start) + " biomass was removed.\n\n")


if __name__ == "__main__":
	plantList = []
	for n in xrange(0, 5):
		_bgb = random.uniform(0, 1000)
		_p = random.uniform(0, 1)
		new_plant = Plant(_bgb, _p)
		plantList.append(new_plant)

	print("\n\n")
	print("Before grazing!")
	print("PlantID BelGrBiomass Palat")
	for plant in plantList:
		print(plant)
	print("\n\n")

	grazingBelGr(3, plantList, 0.5)

	print("After grazing!")
	print("PlantID BelGrBiomass Palat")
	for plant in plantList:
		print(plant)
	print("\n\n")

