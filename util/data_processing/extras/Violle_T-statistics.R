library(dplyr)
library(stringr)
library(logspline)
library(ggplot2)
library(animation)
library("RColorBrewer")

t_ip_ic <- function(spat_data, srv_data)
{
    coexistence <- srv_data %>%
        group_by(ComNr, RunNr, IC_vers, ITVsd, PFT, GrazProb, ARes, BRes) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    coexistence <- coexistence %>%
        group_by(ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(numStartingPFTs = n(), numLivingPFTs = sum(lives))

    coexistence <- coexistence %>%
        group_by(ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(totalLivingPFTs = sum(numLivingPFTs),
                  totalStartingPFTs = sum(numStartingPFTs))

    coexistence <- coexistence %>%
        group_by(ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(coexistence = totalLivingPFTs/totalStartingPFTs)

    coexistence <- coexistence %>%
        group_by(ComNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(coexistence = mean(coexistence))

    tdata <- spat_data %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, PFT, GrazProb, ARes, BRes) %>%
        filter(year == 99) %>%
        filter(n() >= 5)

    population_variance <- tdata %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, PFT, GrazProb, ARes, BRes) %>%
        summarise(ip = var(SLA))

    community_variance <- tdata %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(ic = var(SLA))

    tdata <- inner_join(population_variance, community_variance)

    tdata <- inner_join(tdata, coexistence)

    tdata <- tdata %>%
        mutate(t_ip_ic = ip/ic)

    tdata <- tdata %>%
        filter(ITVsd == 0.5)

    p <- ggplot(tdata)

    p <- p + geom_point(aes(x = t_ip_ic,
                            y = coexistence))

#     p <- p + stat_smooth(aes(x = t_ip_ic,
#                              y = coexistence))

    p <- p + theme_minimal(16)

    p <- p + facet_wrap(~ PFT)

    p
}


t_ic_ir <- function(spat_data, srv_data, trait)
{
    coexistence <- srv_data %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, PFT, GrazProb, ARes, BRes) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    coexistence <- coexistence %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(numStartingPFTs = n(), numLivingPFTs = sum(lives))

    coexistence <- coexistence %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(totalLivingPFTs = sum(numLivingPFTs),
                  totalStartingPFTs = sum(numStartingPFTs))

    coexistence <- coexistence %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(coexistence = totalLivingPFTs/totalStartingPFTs)

    coexistence <- coexistence %>%
        group_by(SimNr, ComNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(coexistence = mean(coexistence))

    tdata <- spat_data %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes, PFT) %>%
        filter(year == 100) %>%
        filter(n() >= 5) %>%
        filter(ITVsd == 0.5)

    community_variance <- tdata %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, GrazProb, ARes, BRes) %>%
        summarise(ic = var(LMR))

    regional_variance <- tdata %>%
        ungroup() %>%
        summarise(ir = var(LMR))

    tdata <- community_variance %>%
        mutate(ir = regional_variance$ir)

    tdata <- inner_join(tdata, coexistence)

    tdata <- tdata %>%
        mutate(t_ic_ir = ic/ir)

    p <- ggplot(tdata)

    p <- p + geom_point(aes(x = t_ic_ir,
                            y = coexistence,
                            color = ARes + BRes))

    p <- p + stat_smooth(aes(x = t_ic_ir,
                             y = coexistence))

    p <- p + facet_grid(~ ITVsd)

    p

    m <- lm(data = tdata, formula = coexistence ~ t_ic_ir)
    summary(m)
    plot(m)
}


ic_ir_trait_distributions <- function(spat_data, srv_data, attribute = "LMR")
{
    tdata <- spat_data %>%
        filter(ITVsd == 0) %>%
        filter(year == 100) %>%
        filter(n() >= 10)

    lives <- srv_data %>%
        group_by(SimNr, ComNr, RunNr, IC_vers, ITVsd, PFT, GrazProb, ARes, BRes) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata %>% inner_join(tdata, lives)

    # regional trait distributions
    ir_trait_dist <- tdata

    # community trait distributions
    ic_trait_dist <- tdata
        group_by(SimNr, ComNr, RunNr, GrazProb, PFT, ARes, BRes, get(attribute)) %>%
        filter(lives == 1) %>%
        filter(SimNr == 21746137) %>%
        filter(SimNr == 21746132)
        ungroup() %>%
        summarise(var = var(get(attribute)))

    p <- ggplot()
    p <- p + geom_line(data = ir_trait_dist,
                       stat = "density",
                       aes(x = get(attribute),
                           color = "Regional"))
    p

    p <- ggplot()
    p <- p + geom_line(data = ic_trait_dist,
                       stat = "density",
                       aes(x = get(attribute),
                           color = PFT))
    p
}
