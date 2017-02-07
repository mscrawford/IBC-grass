import sys, os, subprocess, itertools, csv, copy, math, re, random

class Base_Parameter():
    def __init__(self, 
        IC_version, Mode, ITVsd, Tmax, ARes, Bres, 
        GrazProb, PropRemove, 
        BelGrazProb, BelGrazResidualPerc, BelGrazPerc, 
        CatastrophicPlantMortality, CatastrophicSeedMortality,
        SeedRainType, SeedInput):
        self.IC_version = IC_version
        self.Mode = Mode
        self.ITVsd = ITVsd
        self.Tmax = Tmax
        self.ARes = ARes
        self.Bres = Bres
        self.GrazProb = GrazProb
        self.PropRemove = PropRemove
        self.BelGrazProb = BelGrazProb
        self.BelGrazResidualPerc = BelGrazResidualPerc
        self.BelGrazPerc = BelGrazPerc
        self.CatastrophicPlantMortality = CatastrophicPlantMortality
        self.CatastrophicSeedMortality = CatastrophicSeedMortality
        self.SeedRainType = SeedRainType
        self.SeedInput = SeedInput

        if (self.Mode != 2 and (self.CatastrophicSeedMortality > 0 or self.CatastrophicPlantMortality > 0)):
            raise Exception("Nonsensical or redundant parameterization")

        if (self.Mode == 2 and (self.CatastrophicSeedMortality == 0 and self.CatastrophicPlantMortality == 0)):
            raise Exception("Nonsensical or redundant parameterization")

        if (self.SeedRainType == 0 and self.SeedInput > 0 or 
            self.SeedRainType > 0 and self.SeedInput == 0):
            raise Exception("Nonsensical or redundant parameterization")

        if (self.GrazProb == 0 and self.PropRemove > 0 or self.GrazProb > 0 and self.PropRemove == 0):
            raise Exception("Nonsensical or redundant parameterization")

        if (self.BelGrazProb == 0 and (self.BelGrazResidualPerc > 0 or self.BelGrazPerc > 0)):
            raise Exception("Nonsensical or redundant parameterization")

        if (self.BelGrazProb > 0 and (self.BelGrazPerc == 0 or self.BelGrazResidualPerc == 0)):
            raise Exception("Nonsensical or redundant parameterization")

    def toString(self):
        return " ".join(map(str, [self.IC_version, self.Mode, self.ITVsd, 
            self.Tmax, 
            self.ARes, self.Bres, 
            self.GrazProb, self.PropRemove, 
            self.BelGrazProb, self.BelGrazResidualPerc, self.BelGrazPerc, 
            self.CatastrophicPlantMortality, self.CatastrophicSeedMortality,
            self.SeedRainType, self.SeedInput]))


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

