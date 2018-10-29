import sys, os, subprocess, itertools, csv, copy, math, re, random

class Base_Parameter():
    def __init__(self, 
        IC_version, Mode, ITVsd, Tmax, 
        Env, Sigma, ARes, BRes, 
        AbvGrazProb, AbvGrazPerc, 
        BelGrazProb, BelGrazPerc, BelGrazThreshold,
        BelGrazAlpha, BelGrazWindow,
        DisturbanceMortality, DisturbanceWeek,
        ExperimentDuration, EutrophicationIntensity,
        AbvHerbExclusion, BelHerbExclusion,
        SeedLongevity, 
        SeedRainType, SeedInput):

        self.IC_version = IC_version
        self.Mode = Mode
        self.ITVsd = ITVsd
        self.Tmax = Tmax
        self.Env = Env
        self.Sigma = Sigma
        self.ARes = ARes
        self.BRes = BRes
        self.AbvGrazProb = AbvGrazProb
        self.AbvGrazPerc = AbvGrazPerc
        self.BelGrazProb = BelGrazProb
        self.BelGrazPerc = BelGrazPerc
        self.BelGrazThreshold = BelGrazThreshold
        self.BelGrazAlpha = BelGrazAlpha
        self.BelGrazWindow = BelGrazWindow
        self.DisturbanceMortality = DisturbanceMortality
        self.DisturbanceWeek = DisturbanceWeek
        self.ExperimentDuration = ExperimentDuration
        self.EutrophicationIntensity = EutrophicationIntensity
        self.AbvHerbExclusion = AbvHerbExclusion
        self.BelHerbExclusion = BelHerbExclusion
        self.SeedLongevity = SeedLongevity
        self.SeedRainType = SeedRainType
        self.SeedInput = SeedInput

        if (self.Mode != 2 and (self.DisturbanceWeek > 0 or self.DisturbanceMortality > 0)):
            raise Exception("Nonsensical parameterization")


        # Seed Senescence
        if (self.SeedLongevity == 0):
            raise Exception("Nonsensical parameterization")


        # Seed Rain
        if (self.SeedRainType == 0 and self.SeedInput > 0):
            raise Exception("Nonsensical parameterization")

        if(self.SeedRainType > 0 and self.SeedInput == 0):
            raise Exception("Nonsensical parameterization")


        # Grazing aboveground
        if (self.AbvGrazProb == 0 and self.AbvGrazPerc > 0):
            raise Exception("Nonsensical parameterization")

        if (self.AbvGrazProb > 0 and self.AbvGrazPerc == 0):
            raise Exception("Nonsensical parameterization")


        # Grazing belowground
        if (self.BelGrazProb == 0 and \
                (self.BelGrazPerc > 0 or self.BelGrazAlpha > 0 or self.BelGrazWindow > 0 or self.BelGrazThreshold > 0)):
            raise Exception("Nonsensical parameterization")

        if (self.BelGrazProb > 0 and \
                (self.BelGrazPerc == 0 or self.BelGrazAlpha == 0 or self.BelGrazWindow == 0 or self.BelGrazThreshold == 0)):
            raise Exception("Nonsensical parameterization")


        # Catastrophic disturbance
        if (self.DisturbanceMortality > 0 and self.DisturbanceWeek == 0):
            raise Exception("Nonsensical parameterization")

        if (self.DisturbanceMortality == 0 and self.DisturbanceWeek > 0):
            raise Exception("Nonsensical parameterization")


        # Borstroem experiment
        if (self.ExperimentDuration == 0 and self.EutrophicationIntensity > 0):
            raise Exception("Nonsensical parameterization")

        if (self.ExperimentDuration == 0 and self.AbvHerbExclusion > 0):
            raise Exception("Nonsensical parameterization")

        if (self.AbvGrazProb == 0 and self.AbvHerbExclusion > 0):
            raise Exception("Nonsensical parameterization")

        if (self.ExperimentDuration == 0 and self.BelHerbExclusion > 0):
            raise Exception("Nonsensical parameterization")

        if (self.BelGrazProb == 0 and self.BelHerbExclusion > 0):
            raise Exception("Nonsensical parameterization")


    def toString(self):
        return " ".join(map(str, [self.IC_version, self.Mode, self.ITVsd, 
            self.Tmax, 
            #self.Env, self.Sigma, 
            self.ARes, self.BRes, 
            self.AbvGrazProb, self.AbvGrazPerc, 
            self.BelGrazProb, self.BelGrazPerc, self.BelGrazThreshold, self.BelGrazAlpha, self.BelGrazWindow,
            self.DisturbanceMortality, self.DisturbanceWeek,
            self.ExperimentDuration, 
            self.EutrophicationIntensity, 
            self.AbvHerbExclusion, self.BelHerbExclusion,
            self.SeedLongevity, self.SeedRainType, self.SeedInput]))


class PFT():
    def __init__(self, counter, AllocSeed, LMR, maxPlantSizeSet, pEstab,
                 resourceCompetitionSet, grazingResponseSet, RAR, growth, mThres,
                 clonal, meanSpacerLength, sdSpacerlength, Resshare,
                 AllocSpacer, mSpacer):

        self.Species = counter
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
        self.meanSpacerLength = meanSpacerLength
        self.sdSpacerlength = sdSpacerlength
        self.Resshare = Resshare
        self.AllocSpacer = AllocSpacer
        self.mSpacer = mSpacer

    def __str__(self):
        return " ".join(map(str, [self.Species, self.AllocSeed, self.LMR, self.m0,
                                    self.MaxMass, self.mSeed, self.Dist, self.pEstab, self.Gmax, self.SLA,
                                    self.palat, self.memo, self.RAR, self.growth, self.mThres, self.clonal,
                                    self.meanSpacerLength, self.sdSpacerlength, self.Resshare,
                                    self.AllocSpacer, self.mSpacer])) 

