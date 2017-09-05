library(data.table)

read_data <- function(data_dir, file_type, verbose = FALSE) {
    main_dir = getwd()
    setwd(data_dir)

    files <- list.files(full.names = T)
    files <- files[which(grepl(file_type, files))]

    d <- rbindlist(lapply(files,
                             FUN = function(files) {
                                 fread(files,
                                       header = TRUE,
                                       stringsAsFactors = TRUE,
                                       na.strings = "NA",
                                       strip.white = TRUE,
                                       data.table = FALSE)
                             }))

    setwd(main_dir)
    return(tbl_df(d))
}

combine_data <- function(data_frames, key) {
    d <- Reduce(function(...) left_join(..., by = key), data_frames)
    return(d)
}

uv <- function(data) {
    r <- apply(data,
               2,
               function(x) (unique(x)))
    return(r)
}

gm_mean <- function(x, na.rm = TRUE, zero.propagate = FALSE) {
    if (any(x < 0, na.rm = TRUE)) {
        return(NaN)
    }
    if (zero.propagate) {
        if (any(x == 0, na.rm = TRUE)) {
            return(0)
        }
        exp(mean(log(x), na.rm = na.rm))
    } else {
        exp(sum(log(x[x > 0]), na.rm=na.rm) / length(x))
    }
}
