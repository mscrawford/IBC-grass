# Michael Crawford
# Wednesday, November 26, 2014

# params:
## dir -> directory (full path) of the data
## file_type -> regular "snippet" in the names of the files that you want to read from.
##              Example: "Srv" or "Pft"

library(dplyr)
library(data.table)

read_data <- function(dir, file_type, classify = TRUE)
{
    data <- read_files(dir, file_type)
    data <- clean_data(data, classify)
    if (file_type != "Grd") {
        data$PFT = as.factor(data$PFT)
    }
    return(data)
}

read_legacy_data <- function(dir, file_type)
{
    data <- read_files(dir, file_type)
    if (file_type != "Grd") {
        data$PFT = as.factor(data$PFT)
    }
    return(data)
}

read_files <- function(dir, file_type)
{
    main_dir = getwd()
    setwd(dir)

    files <- list.files(full.names=T)
    files <- files[which(grepl(file_type, files))]

    if (file_type == "Grd")
    {
        data <- rbindlist(lapply(files,
                               FUN = function(files) {
                                   fread(files,
                                         stringsAsFactors = T,
                                         na.strings = "NA",
                                         header = T)
                               }))
        setnames(data, strsplit(readLines(files[1], n = 1), split = "\t")[[1]])
    } else {
        data <- rbindlist(lapply(files,
                               FUN = function(files) {
                                   fread(files,
                                         stringsAsFactors = T,
                                         na.strings = "NA",
                                         header = T)
                               }))
    }

    setwd(main_dir)
    return(data)
}

clean_data <- function(data, classify = TRUE)
{
    # Transforming the "Sim" such that the pairwise and community runs are consistent. Each set of sims will
    # now have the same "Sim" number.
    data <- data %>%
        mutate(SimNr = ifelse(invasion_ver == 1,
                            as.integer(
                                substr(
                                    as.character(SimNr),
                                    1,
                                    str_length(as.character(SimNr)) - str_length(as.character(
                                               paste(as.character(monoculture),
                                                     as.character(invader), sep=""))))),
                            SimNr))

    # Classify the pairs by their attributes:
    if (classify)
    {
        # Growth form:
        data <- data %>%
            mutate(growthForm = ifelse(LMR == 1.0,
                                       "rosette",
                                       ifelse(LMR == 0.75,
                                              "intermediate",
                                              ifelse(LMR == 0.5,
                                                     "erect",
                                                     "ERROR")))) %>%
            select(-LMR)

        # Maximum plant size:
        data <- data %>%
            mutate(plantSize = ifelse(MaxMass == 5000,
                                      "large",
                                      ifelse(MaxMass == 2000,
                                             "medium",
                                             ifelse(MaxMass == 1000,
                                                    "small",
                                                    "ERROR")))) %>%
            select(-MaxMass, -SeedMass, -m0, -Dist)

        # Resource response
        data <- data %>%
            mutate(resourceResponse = ifelse(Gmax == 60,
                                             "competitor",
                                             ifelse(Gmax == 40,
                                                    "intermediate",
                                                    ifelse(Gmax == 20,
                                                           "stress-tolerator",
                                                           "ERROR")))) %>%
            select(-Gmax, -memory)

        # Grazing response
        data <- data %>%
            mutate(grazingResponse = ifelse(palat == 1.0,
                                            "tolerator",
                                            ifelse(palat == 0.5,
                                                   "intermediate",
                                                   ifelse(palat == 0.25,
                                                          "avoider",
                                                          "ERROR")))) %>%
            select(-palat, -SLA)

        # Crap we don't need
        data <- data %>%
            select(-AllocSeed, -pEstab, -RAR, -growth, -PropSex,
                   -meanSpacerlength, -sdSpacerlength, -Resshare,
                   -AllocSpacer, -mSpacer, -mThres, -clonal)
    }

    # Remove RunParameters that we don't use.
    data <- data %>%
        select(-PropRemove, -NCut, -CutMass, -DistAreaYear,
               -AreaEvent, -SeedRainType, -SeedInput)

    return (data)
}
