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
                                         header = F)
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

    return (data)
}
