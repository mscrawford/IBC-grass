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
setwd("/Users/Theodore/Documents/workspace/IBC-grass/util/data_processing")

source("read_data.R")
source("utilities.R")

# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Broad_ITVsd"
# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Longterm"
# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Stress_and_Competition_largeSet"
# datadir = "/Volumes/Michael_Crawford_Research/IBC-grass/ITV/Stress_and_Competition_smallSet"

# datadir = "/Users/Theodore/Documents/workspace/IBC-grass/data/out"
datadir = "/Users/Theodore/Desktop/data/out/"

# # Loading raw data
srv_data <- read_data(datadir, "Srv", FALSE)
pft_data <- read_data(datadir, "Pft")
# spat_data <- read_data(datadir, "Spat", FALSE)
# comp_data <- read_files(datadir, "Comp")
grd_data <- read_data(datadir, "Grd", FALSE)
