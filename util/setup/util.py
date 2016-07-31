import sys, os, subprocess, itertools, csv, copy, math, re, random

class Base_Parameter():
    def __init__(self, IC_version, ITVsd, CTmax, ARes, Bres, GrazProb, propRemove, 
                DistAreaYear, AreaEvent, NCut, CutMass, SeedRain, SeedInput):
        self.IC_version = IC_version
        self.ITVsd = ITVsd
        self.CTmax = CTmax
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

    def toString(self):
        return " ".join(map(str, [self.IC_version, self.ITVsd, self.CTmax, self.ARes, 
                            self.Bres, self.GrazProb, self.propRemove, self.DistAreaYear, self.AreaEvent, 
                            self.NCut, self.CutMass, self.SeedRain, self.SeedInput]))






class PFT():
    def __init__(self, counter, MaxAge, AllocSeed, LMR, maxPlantSizeSet, pEstab,
                 resourceCompetitionSet, grazingResponseSet, RAR, growth, mThres,
                 clonal, propSex, meanSpacerLength, sdSpacerlength, Resshare,
                 AllocSpacer, mSpacer):
        self.Species = counter
        self.MaxAge = MaxAge
        self.AllocSeed = AllocSeed
        self.LMR = LMR # fLeaf
        self.m0 = maxPlantSizeSet[0] # m0 (initial seedling mass). Usually the same as mSeed.
        self.MaxMass = maxPlantSizeSet[1] # MaxMass
        self.mSeed = maxPlantSizeSet[2] # mSeed. This is the same as m0.
        self.Dist = maxPlantSizeSet[3] # Both meanDisp and stdDisp
        self.pEstab = pEstab
        self.Gmax = resourceCompetitionSet[0] # ruMax
        self.memo = resourceCompetitionSet[1] # survMax
        self.palat = grazingResponseSet[0] # palat
        self.SLA = grazingResponseSet[1] # cShoot
        self.RAR = RAR
        self.growth = growth
        self.mThres = mThres
        self.clonal = clonal
        self.propSex = propSex
        self.meanSpacerLength = meanSpacerLength
        self.sdSpacerlength = sdSpacerlength
        self.Resshare = Resshare
        self.AllocSpacer = AllocSpacer
        self.mSpacer = mSpacer

    def __str__(self):
        return " ".join(map(str, [self.Species, self.MaxAge, self.AllocSeed, self.LMR, self.m0,
                                    self.MaxMass, self.mSeed, self.Dist, self.pEstab, self.Gmax, self.SLA,
                                    self.palat, self.memo, self.RAR, self.growth, self.mThres, self.clonal,
                                    self.propSex, self.meanSpacerLength, self.sdSpacerlength, self.Resshare,
                                    self.AllocSpacer, self.mSpacer])) 