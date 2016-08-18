# Michael Crawford
# Wednesday, November 26, 2014

# params:
## dir -> directory (full path) of the data
## file_type -> regular "snippet" in the names of the files that you want to read from.
##              Example: "Srv" or "Pft"

library(dplyr)
library(data.table)

read_data <- function(dir, file_type)
{
    data <- read_files(dir, file_type)
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

    files <- list.files(full.names = T)
    files <- files[which(grepl(file_type, files))]

    if (file_type == "Grd")
    {
        data <- rbindlist(lapply(files,
                                 FUN = function(files) {
                                     fread(files,
                                           stringsAsFactors = T,
                                           na.strings = "NA",
                                           header = F,
                                           verbose = T)
                                 }))
        setnames(data, strsplit(readLines(files[1], n = 1), split = "\t")[[1]])
    } else if (file_type == "Spat") {
        data <- rbindlist(lapply(files,
                                 FUN = function(files) {
                                     fread(files,
                                           stringsAsFactors = T,
                                           na.strings = "NA",
                                           header = T,
                                           verbose = T,
                                           select = c("SimNr", "ComNr", "RunNr",
                                                      "year", "IC_vers", "ITVsd", "PFT", "plantID",
                                                      "Age", "yearlyFecundity", "lifetimeFecundity",
                                                      "stress", "memory", "SeedMass"))
                                 }))
    } else {
        data <- rbindlist(lapply(files,
                                 FUN = function(files) {
                                     fread(files,
                                           stringsAsFactors = T,
                                           na.strings = "NA",
                                           header = T,
                                           verbose = T)
                                 }))
    }

    setwd(main_dir)
    return(data)
}
