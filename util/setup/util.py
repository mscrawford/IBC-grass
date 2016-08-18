import sys, os, subprocess, itertools, csv, copy, math, re, random

class Base_Parameter():
    def __init__(self, IC_version, ITVsd, Tmax, ARes, Bres, 
        GrazProb, PropRemove, BelGrazMode, BelGrazStartYear, BelGrazWindow, BelGrazProb, BelPropRemove, 
        DistAreaYear, AreaEvent, NCut, CutMass):
        self.IC_version = IC_version
        self.ITVsd = ITVsd
        self.Tmax = Tmax
        self.ARes = ARes
        self.Bres = Bres
        self.GrazProb = GrazProb
        self.PropRemove = PropRemove
        self.BelGrazMode = BelGrazMode
        self.BelGrazStartYear = BelGrazStartYear
        self.BelGrazWindow = BelGrazWindow
        self.BelGrazProb = BelGrazProb
        self.BelPropRemove = BelPropRemove
        self.DistAreaYear = DistAreaYear
        self.AreaEvent = AreaEvent
        self.NCut = NCut
        self.CutMass = CutMass

        if (self.GrazProb == 0 and self.PropRemove > 0 or self.GrazProb > 0 and self.PropRemove == 0):
            print "Nonsensical or redundant parameterization -- GrazProb and PropRemove."
            raise Exception("Nonsensical or redundant parameterization")

        if (self.BelGrazMode > 0 and (self.BelGrazProb == 0 or self.BelPropRemove == 0)):
            print "Nonsensical or redundant parameterization -- BelGrazMode and (BelGrazProb or BelPropRemove)."
            raise Exception("Nonsensical or redundant parameterization")

        if (self.BelGrazStartYear > 0 and (self.BelGrazProb == 0 or self.BelPropRemove == 0)):
            print "Nonsensical or redundant parameterization -- BelGrazStartYear and (BelGrazProb or BelPropRemove)."
            raise Exception("Nonsensical or redundant parameterization")            

        if (self.BelGrazWindow > 0 and (self.BelGrazProb == 0 or self.BelPropRemove == 0)):
            print "Nonsensical or redundant parameterization -- BelGrazWindow and (BelGrazProb or BelPropRemove)."
            raise Exception("Nonsensical or redundant parameterization")   

        if (self.BelGrazProb == 0 and self.BelPropRemove > 0 or self.BelGrazProb > 0 and BelPropRemove == 0):
            print "Nonsensical or redundant parameterization -- BelGrazProb and BelPropRemove."
            raise Exception("Nonsensical or redundant parameterization")

        if (self.DistAreaYear == 0 and self.AreaEvent > 0 or self.DistAreaYear > 0 and self.AreaEvent == 0):
            print "Nonsensical or redundant parameterization -- DistAreaYear and AreaEvent."
            raise Exception("Nonsensical or redundant parameterization")

        # If there's no cutting, cutmass must be 0. If there is cutting, there must be some mass cut.
        if (self.NCut == 0 and self.CutMass > 0 or self.NCut > 0 and self.CutMass == 0):
            print "Nonsensical or redundant parameterization -- NCut and CutMass"
            raise Exception("Nonsensical or redundant parameterization")

    def toString(self):
        return " ".join(map(str, [self.IC_version, self.ITVsd, self.Tmax, self.ARes, self.Bres, 
            self.GrazProb, self.PropRemove, self.BelGrazMode, self.BelGrazStartYear, self.BelGrazWindow,
            self.BelGrazProb, self.BelPropRemove, self.DistAreaYear, self.AreaEvent, self.NCut, self.CutMass]))






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