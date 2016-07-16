library(mvnormtest)
library(vegan)
library(dplyr)
library(purrr)
library(reshape)
library(ggplot2)
library(stringr)
library(logspline)
library(animation)
library(ggthemes)
library("RColorBrewer")
library(readr)

## Set working directory to the root of the project's folder

#SECTION 1----------
setwd("/Users/Theodore/Documents/Google_Drive/Research/Projects/ITV/R")

source("read_data.R")
source("utilities.R")

indir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/"
outdir = "/Users/Theodore/Desktop/"

# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Broad_ITVsd"
# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Longterm"
# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Stress_and_Competition_largeSet"
# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Stress_and_Competition_smallSet"

datadir = "/Users/Theodore/Documents/workspace/IBC-grass/Output"

# Loading raw data
srv_data <- read_data(datadir, "Srv", FALSE)
pft_data <- read_data(datadir, "Pft")
spat_data <- read_data(datadir, "Spat", FALSE)
comp_data <- read_files(datadir, "Comp")
grd_data <- read_data(datadir, "Grd", FALSE)
