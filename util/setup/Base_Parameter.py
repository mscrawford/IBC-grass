import sys, os, subprocess, itertools, csv, copy, math, re, random

class Base_Parameter():
    def __init__(self, ICvers, invasionVers, IndividualVariationVers, indivVariationSD, 
                CTmax, PftTmax, ARes, Bres, GrazProb, propRemove, 
                DistAreaYear, AreaEvent, NCut, CutMass, SeedRain, SeedInput):
        self.ICvers = ICvers
        self.invasionVers = invasionVers
        self.IndividualVariationVers = IndividualVariationVers
        self.indivVariationSD = indivVariationSD
        self.CTmax = CTmax
        self.PftTmax = PftTmax
        self.ARes = ARes
        self.Bres = Bres
        self.GrazProb = GrazProb
        self.propRemove = propRemove
        self.DistAreaYear = DistAreaYear
        self.AreaEvent = AreaEvent
        self.NCut = NCut
        self.CutMass = CutMass
        self.SeedRain = SeedRain
        self.SeedInput = SeedInput

        # If there's no individual variation on this run, set the SD to 0 for consistency.
        if (self.IndividualVariationVers == 0 and self.indivVariationSD > 0 or 
            self.IndividualVariationVers == 1 and self.indivVariationSD == 0):
            print "Redundant parameterization -- IndividualVariationVers and indivVariationSD"
            raise Exception("Redundant parameterization")

        # I AM UNCERTAIN ABOUT THIS.
        if (self.DistAreaYear == 0 and self.AreaEvent > 0 or 
            self.DistAreaYear > 0 and self.AreaEvent == 0):
            print "Redundant parameterization -- DistAreaYear and AreaEvent."
            raise Exception("Redundant parameterization")

        # If there's no cutting, cutmass must be 0. If there is cutting, there must be some mass cut.
        if (self.NCut == 0 and self.CutMass > 0 or 
            self.NCut > 0 and self.CutMass == 0):
            print "Redundant parameterization -- NCut and CutMass"
            raise Exception("Redundant parameterization")

        # If there's no seed rain, there can't be any seed input. If there is seed rain, there must be input.
        if (self.SeedRain == 0 and self.SeedInput > 0 or
            self.SeedRain > 0 and self.SeedInput == 0):
            print "Redundant parameterization -- SeedRain and SeedInput"
            raise Exception("Redundant parameterization")

    def toString(self, community):
        if (community):
            return " ".join(map(str, [self.ICvers, 0, self.IndividualVariationVers, 
                            self.indivVariationSD, self.CTmax, self.ARes, self.Bres, 
                            self.GrazProb, self.propRemove, self.DistAreaYear, self.AreaEvent, self.NCut, 
                            self.CutMass, self.SeedRain, self.SeedInput]))
        else:
            return " ".join(map(str, [self.ICvers, 1, self.IndividualVariationVers, 
                            self.indivVariationSD, self.PftTmax, self.ARes, self.Bres, 
                            self.GrazProb, self.propRemove, self.DistAreaYear, self.AreaEvent, self.NCut, 
                            self.CutMass, self.SeedRain, self.SeedInput]))