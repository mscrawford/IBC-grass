FindNextInhabitant <- function(SimNr_, ComNr_, RunNr_, ITVsd_, plantID_, year_)
{

    t <- spat_data %>% filter(SimNr == SimNr_,
                              ComNr == ComNr_,
                              RunNr == RunNr_,
                              ITVsd == ITVsd_)

    XY <- t %>%
        filter(plantID == plantID_) %>%
        select(xcoord, ycoord)

    X <- XY[[1]]
    Y <- XY[[2]]

    t <- t %>% filter(abs(xcoord - X) < 2,
                      abs(ycoord - Y) < 2,
                      year > year_) # Revise this later on

    t <- t %>%
        filter(min(year))

    # print(t$PFT)

    return(t$PFT)
}

## Categorize which species are rare and which are common
originally_rare <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, year, PFT) %>%
    summarise(PFT_tPop_perYear = n())

originally_rare <- originally_rare %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    summarise(PFT_avgPop = mean(PFT_tPop_perYear))

originally_rare <- originally_rare %>%
    filter(ITVsd == 0) %>%
    mutate(originally_rare = ifelse(PFT_avgPop < 50, T, F)) %>%
    ungroup() %>%
    select(ComNr, PFT, originally_rare)
## Done categorizing rarity

tdata <- spat_data

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, plantID) %>%
    filter(Age == max(Age))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, PFT, ITVsd) %>%
    filter(n() > 10) %>%
    sample_n(2)

turnover <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, plantID) %>%
    mutate(nextSpecies = FindNextInhabitant(SimNr, ComNr, RunNr, ITVsd, plantID, year)) %>%
    select(SimNr, ComNr, RunNr, ITVsd, PFT, plantID, nextSpecies)

turnover <- turnover %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, plantID) %>%
    mutate(replacedBySelf = (PFT == nextSpecies))

turnover <- turnover %>%
    ungroup() %>%
    filter(!is.na(replacedBySelf))

turnover <- turnover %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    mutate(replacedBySelf_count = sum(replacedBySelf == T),
           total_count = n())

turnover <- turnover %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    summarise(ratio = replacedBySelf_count / total_count)

turnover <- inner_join(turnover, originally_rare, copy=T)

p <- ggplot(turnover)

p <- p + geom_point(aes(x = PFT,
                        y = ratio))

p <- p + facet_grid(originally_rare ~ ITVsd)

p

summary_statistics <- turnover

summary_statistics <- summary_statistics %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(avg_ratio = mean(ratio))

p <- ggplot(summary_statistics)

p <- p + geom_point(aes(x = ITVsd,
                        y = avg_ratio))

p <- p + facet_wrap(~ originally_rare)

p
