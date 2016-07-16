AverageBestPlantTraitSet <- function()
{
    # what about IC_version?
    s <- srv_data %>% select(SimNr, ComNr, RunNr, ITVsd, PFT, SeedMass)
    data <- tbl_df(inner_join(spat_data, s, c("SimNr", "ComNr", "RunNr", "ITVsd", "PFT")))

    t <- data %>%
        group_by(SimNr, ComNr) %>%
        filter(ComNr == 1, ITVsd %in% c(0.5)) # Only one community, with and without ITV

    t <- t %>%
        group_by(SimNr, ComNr, RunNr, PFT, ITVsd, plantID) %>%
        filter(Age == max(Age))

    t <- t %>%
        group_by(ComNr, ITVsd) %>%
        mutate(fitness = percent_rank(totalSeeds * SeedMass.y))

    t <- t %>%
        mutate(fitness_category = ifelse(fitness > 0.90, 1, 0))

    t_toMelt <- t %>%
        select(SimNr, ComNr, RunNr, ITVsd, PFT, plantID, LMR,
               MaxMass, SeedMass.x, Dist, Gmax, SLA, palat, memory, RAR)

    t_melted <- melt(t_toMelt, id=c("SimNr", "ComNr", "RunNr", "ITVsd", "PFT", "plantID"))

    t_melted <- tbl_df(inner_join(t, t_melted))

    p <- ggplot()

    p <- p + geom_line(data = t_melted,
                       stat = "density",
                       aes(x = value,
                           color = as.factor(fitness_category)))

    p <- p + facet_wrap(~ variable, scales="free")

    p <- p + theme_few(26)

    p <- p + scale_color_discrete(name="",
                                  breaks=c(1, 0),
                                  labels=c("Fit", "Unfit"))

    p <- p + labs(x = "Trait")

    p <- p + theme(
        panel.border = element_rect(fill = NA,
                                    colour = "black",
                                    size = 1),
        panel.margin = unit(1, "lines"),
        axis.ticks = element_line(colour = "black"),
        axis.title.x = element_text(vjust=-2),
        axis.title.y = element_text(vjust=2),
        plot.margin = unit(c(1,1,1,1), "cm"),
        aspect.ratio = 1)

    p

    ggsave(filename = "Overall_fitness.pdf",
           path = "~/Desktop")


    p <- ggplot()

    p <- p + geom_point(data = t,
                        aes(x = fitness,
                            y = totalSeeds))

    p <- p + facet_wrap(~ PFT)

    p
}










TheoreticalVSRealizedTraitDistributions <- function()
{
    s <- srv_data %>% select(SimNr, ComNr, RunNr, ITVsd, PFT, SeedMass)
    data <- tbl_df(inner_join(spat_data, s, c("SimNr", "ComNr", "RunNr", "ITVsd", "PFT")))

    t <- data %>%
        group_by(SimNr, ComNr) %>%
        filter(ComNr == 1, ITVsd %in% c(0.5)) # Only one community, with and without ITV

    t <- t %>%
        group_by(SimNr, ComNr, RunNr, PFT, ITVsd, plantID) %>%
        filter(Age == max(Age))

    t <- t %>%
        group_by(ComNr, ITVsd, PFT) %>%
        mutate(fitness = percent_rank(totalSeeds * SeedMass.y))

    t <- t %>%
        mutate(fitness_category = ifelse(fitness > 0.90, 1, 0))

    p <- ggplot()

    p <- p + geom_line(data = t,
                       stat = "density",
                       aes(x = Gmax,
                           color = as.factor(fitness_category)))

    p <- p + facet_wrap(~ PFT)

    p <- p + theme_few(26)

    p <- p + scale_color_discrete(name="",
                                  breaks=c(1, 0),
                                  labels=c("Fit", "Unfit"))

    p <- p + labs(x = "Maximum resource uptake per week (units)")

    p <- p + theme(
        panel.border = element_rect(fill = NA,
                                    colour = "black",
                                    size = 1),
        panel.margin = unit(1, "lines"),
        axis.ticks = element_line(colour = "black"),
        axis.title.x = element_text(vjust=-2),
        axis.title.y = element_text(vjust=2),
        plot.margin = unit(c(1,1,1,1), "cm"),
        aspect.ratio = 1)

    p

    ggsave(filename = "Fitness.pdf",
           path = "~/Desktop")


    p <- ggplot()

    p <- p + geom_point(data = t,
                        aes(x = fitness,
                            y = totalSeeds))

    p <- p + facet_wrap(~ PFT)

    p
}










SuperIndividualsOverTime <- function()
{
    tdata <- srv_data %>%
        filter(ComNr == 1, RunNr == 1, ITVsd == 0.2)
    
    tdata <- tdata %>%
        group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        select(SimNr, ComNr, RunNr, PFT, lives)

    tdata <- tbl_df(inner_join(spat_data, tdata))

    idata <- tdata %>%
        group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year) %>%
        summarise(PFT_nTotalSeeds = sum(accumulatedSeeds))

    tdata <- tbl_df(inner_join(tdata, idata))

    tdata <- tdata %>%
        group_by(SimNr, ComNr, RunNr, ITVsd, PFT, lives, year, plantID, accumulatedSeeds, PFT_nTotalSeeds) %>%
        mutate(proportionTotalSeeds = accumulatedSeeds / PFT_nTotalSeeds)

    tdata <- tdata %>%
        group_by(SimNr, ComNr, RunNr,  ITVsd, PFT, lives, year) %>%
        mutate(maxProportionSeeds = max(proportionTotalSeeds)) %>%
        filter(proportionTotalSeeds == maxProportionSeeds)

    t <- tdata %>%
        group_by(SimNr, ComNr, RunNr,  ITVsd, PFT, lives, year, plantID) %>%
        mutate(saved = ifelse(proportionTotalSeeds > 0.5, 1, 0))

    t <- t %>%
        group_by(SimNr, ComNr, RunNr, ITVsd, PFT, lives, plantID) %>%
        summarise(save_count = sum(saved))

    tdata <- inner_join(t, tdata)

    p <- ggplot(tdata)

    p <- p + geom_line(aes(x = year,
                           y = maxProportionSeeds,
                           group = ITVsd))

    p <- p + geom_point(aes(x = year,
                            y = ifelse(maxProportionSeeds > 0.5, maxProportionSeeds, NA),
                            group = ITVsd,
                            color = factor(save_count)),
                        size = 2)

    p <- p + facet_wrap(~ PFT)

    p <- p + theme_minimal(28)

    p <- p + labs(x = "Year",
                  y = "Ratio of the highest-fitness plant's fecundity to its population's")

    p <- p + scale_color_discrete("Number of times\nthe plant saves\nthe population")

    p
}










## Interactions of grazing pressure and resource levels with ITV.
ResourcesAndGrazing <- function()
{
    tdata <- srv_data %>%
        group_by(ComNr, RunNr, ITVsd, PFT, ARes, BRes, GrazProb) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        group_by(ComNr, RunNr, ITVsd, ARes, BRes, GrazProb) %>%
        summarise(numStartingPFTs = n(), numLivingPFTs = sum(lives))

    tdata <- tdata %>%
        group_by(ComNr, RunNr, ITVsd, ARes, BRes, GrazProb) %>%
        summarise(totalLivingPFTs = sum(numLivingPFTs),
                  totalStartingPFTs = sum(numStartingPFTs))

    tdata <- tdata %>%
        group_by(ComNr, RunNr, ITVsd, ARes, BRes, GrazProb) %>%
        summarise(coexistence = totalLivingPFTs/totalStartingPFTs)

    tdata <- tdata %>%
        group_by(ComNr, ITVsd, ARes, BRes, GrazProb) %>%
        summarise(coexistence = mean(coexistence))

    tdata <- tdata %>%
        group_by(ITVsd, ARes, BRes, GrazProb) %>%
        summarise_each(funs(mean, sd), coexistence)

    tdata$BRes <- factor(tdata$BRes, levels=c(100, 60, 30))

    p <- ggplot()

    p <- p + geom_line(data = tdata,
                       aes(x = ITVsd,
                           y = mean,
                           color = factor(GrazProb),
                           group = factor(GrazProb)))

    p <- p + geom_point(data = tdata,
                        aes(x = ITVsd,
                            y = mean),
                        size=2)

    p <- p + facet_grid(BRes ~ ARes, labeller=label_both)

    p <- p + labs(title = "Intraspecific trait variation affects coexistence in a varying environment",
                  x = "ITVsd",
                  y = "Coexistence (N surviving PFT ÷ N starting PFT)",
                  colour = "Probability of being\ngrazed per week")

    p <- p + theme_minimal(16)

    p
}










## How much does ITV impact coexistence under various resource regimes?
ResourcesContour <- function(srv_data)
{
    tdata <- srv_data %>%
        group_by(ComNr, RunNr, ITVsd, PFT, ARes, BRes) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        group_by(ComNr, RunNr, ITVsd, ARes, BRes) %>%
        summarise(numStartingPFTs = n(), numLivingPFTs = sum(lives))

    tdata <- tdata %>%
        group_by(ComNr, RunNr, ITVsd, ARes, BRes) %>%
        summarise(totalLivingPFTs = sum(numLivingPFTs),
                  totalStartingPFTs = sum(numStartingPFTs))

    tdata <- tdata %>%
        group_by(ComNr, RunNr, ITVsd, ARes, BRes) %>%
        summarise(coexistence = totalLivingPFTs/totalStartingPFTs)

    tdata <- tdata %>%
        group_by(ITVsd, ARes, BRes) %>%
        summarise(mean = mean(coexistence))

    tdata <- tdata %>%
        group_by(ARes, BRes) %>%
        arrange(ITVsd) %>%
        mutate(normalized_mean_delta = (mean - lag(mean))/(mean + lag(mean))) %>%
        filter(ITVsd == max(ITVsd)) %>%
        select(-ITVsd, -mean)

    tdata <- tdata %>%
        group_by(ARes, BRes) %>%
        summarise(norm_diff = mean(normalized_mean_delta)) %>%
        ungroup() %>%
        arrange(norm_diff)

    p <- ggplot(data = tdata,
                aes(x = ARes,
                    y = BRes,
                    z = norm_diff,
                    fill = norm_diff))

    p <- p + geom_tile()

    p <- p + theme_minimal(16)

    p <- p + labs(title = "The benefits to coexistence of high ITV (= 0.4) along a 2D resource gradient",
                  x = "Aboveground resources",
                  y = "Belowground resources")

    p <- p + scale_fill_gradient("% increase in\ncoexistence")

    p
}


timelapse_plot <- function(data, attribute = "rShoot", save = FALSE, gif = FALSE)
{
    attribute = "rShoot"
    save = T

    root_dir = getwd()
    setwd("~/Documents/Google_Drive/Research/IBC-grass/output")
    dir.create(attribute)
    setwd(attribute)

    PFT_community = levels(data$PFT)
    community_size = length(PFT_community)
    myPalette <- colorRampPalette(brewer.pal(
        ifelse(
            community_size > 12,
            12,
            community_size),
        "Set3"))
    myColors <- myPalette(community_size)
    names(myColors) <- PFT_community
    colScale <- scale_colour_manual(name="PFT", values=myColors) # get rid of guide = F if the legend will fit...

    years = sort(unique(data$year))
    for (y in years)
    {
        tdata <- data %>%
            group_by(SimNr, RunNr, PFT) %>%
            filter(year == y, week == w)
        plot <- ggplot() +
            geom_point(data=tdata,
                       aes(x=xcoord,
                           y=ycoord,
                           col=PFT,
                           size=get(attribute))) + # * 2 because radius...
            labs(title=paste("Time-lapse of IBC-grass. Year:",
                             y,
                             "| nPFTs:",
                             length(unique(tdata$PFT))),
                 x="X",
                 y="Y",
                 col = "PFT",
                 size = attribute) +
            colScale # +
        #                 scale_size_continuous(range = c(.05, 3))
        if (save)
        {
            ggsave(plot,
                   filename=paste(sprintf("%03s", y),
                                  ".png",
                                  sep=""),
                   width=5,
                   height=5)
        }
    }

    if (gif & save)
    {
        system("convert -delay 80 *.png animation.gif")
    }

    setwd(root_dir)
}








CompetitionOverTime <- function()
{
    tdata <- grd_data

    # Average the redundant runs
    tdata <- tdata %>%
        group_by(ComNr, ITVsd, IC_ver, Tmax, Year)

    tdata <- tdata %>%
        summarise(avg_AComp = mean(mean_aComp),
                  avg_BComp = mean(mean_bComp),
                  avg_abovemass = mean(abovemass),
                  avg_belowmass = mean(belowmass))

    p <- ggplot(tdata)

    p <- p + stat_smooth(aes(x = Year,
                             y = avg_AComp + avg_BComp,
                             # y = avg_abovemass + avg_belowmass,
                             col = as.factor(ITVsd)))

#     p <- p + geom_line(aes(x = Year,
#                            y = avg_abovemass + avg_belowmass,
#                            col = as.factor(ITVsd)))

    p <- p + facet_grid(Tmax ~ IC_ver, scales = "free")

#     p <- p + labs(x = "Year",
#                   y = "Mean global resource demand (above- and belowground)")

    # p <- p + scale_color_discrete("ITVsd")

    p <- p + theme_few(26)

    p <- p + theme(
        axis.title.x = element_text(vjust=-1.5),
        axis.title.y = element_text(vjust=1.5),
        plot.margin = unit(c(1, 1, 1, 1), "cm")
    )

    p
}










CompetitionCorrelatedToShannon <- function()
{
    tdata <- grd_data

    tdata <- tdata %>%
        group_by(ComNr, ITVsd, Year) %>%
        summarise(avg_AComp = mean(mean_aComp),
                  avg_BComp = mean(mean_bComp),
                  shannon = mean(shannon))

    tdata <- tdata %>%
        group_by(ComNr, ITVsd) %>%
        summarise(p = cor.test(shannon, avg_AComp + avg_BComp)[[3]],
                  cor = cor.test(shannon, avg_AComp + avg_BComp)[[4]])

    tdata <- tdata %>%
        group_by(ITVsd) %>%
        summarise(avg_cor = mean(cor))

    tdata

    p <- ggplot(tdata)

    p <- p + geom_point(aes(x = shannon,
                           y = avg_AComp))

    p

}










CompetitionTimelapse <- function(save = TRUE, gif = TRUE)
{
    root_dir = getwd()
    setwd("~/Documents/Google_Drive/Research/IBC-grass/output")
    dir.create("Comp")
    setwd("Comp")

    tdata <- comp_data

    years = sort(unique(tdata$year))
    max_comp = max(tdata$AComp)
    min_comp = min(tdata$AComp)
    for (y in years)
    {
        tdata <- tdata %>%
            group_by(SimNr, ITVsd) %>%
            filter(year == y)

        p <- ggplot(tdata)

        p <- p + geom_tile(aes(x = X,
                               y = Y,
                               fill = BComp))

        p <- p + labs(title = paste("Competition per square in year ", y),
                      x = "X",
                      y = "Y")

        p <- p + scale_fill_continuous(name = "Competition intensity",
#                                        limits = c(min_comp, max_comp),
                                       high = "red",
                                       low = "white")

        p <- p + theme_minimal(16)

        p <- p + facet_wrap(~ ITVsd)

        if (save)
        {
            ggsave(p,
                   filename=paste(sprintf("%03s", y),
                                  ".pdf",
                                  sep=""))
        }
    }

    if (gif & save) {
        system("convert -delay 80 *.pdf animation.gif")
    }

    setwd(root_dir)
}









yearling_deltaT <- function(spat_data, srv_data, save = FALSE)
{
    tdata <- tbl_df(inner_join(spat_data, srv_data, c("SimNr", "ComNr", "RunNr", "PFT")))

    tdata <- tdata %>%
        select(-contains(".y"))

    tdata <- tdata %>%
        group_by(SimNr, ComNr, IC_ver.x, ITVsd.x, PFT) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        group_by(SimNr, IC_ver.x, ITVsd.x, year.x, PFT, lives) %>%
        filter(Age == 1) %>%
        summarise(yearlings = n())

    p <- ggplot()

    p <- p + geom_line(data = tdata,
                       aes(x = year.x,
                           y = yearlings,
                           color = PFT))

    p <- p + facet_grid(ITVsd.x ~ IC_ver.x, labeller=label_both)

    p
}









MaxAge <- function(spat_data, srv_data, save = FALSE)
{
    tdata <- tbl_df(inner_join(spat_data, srv_data, c("SimNr", "ComNr", "RunNr", "PFT")))

    tdata <- tdata %>%
        select(-contains(".y"))

    tdata <- tdata %>%
        group_by(SimNr, ComNr, IC_ver.x, ITVsd.x, PFT) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        group_by(SimNr, ComNr, IC_ver.x, ITVsd.x, PFT, plantID) %>%
        filter(Age == max(Age))

    p <- ggplot()

    p <- p + geom_boxplot(data = tdata,
                          aes(x = factor(PFT),
                              y = Age,
                              color = as.factor(lives)))

    p <- p + facet_grid(ITVsd.x ~ IC_ver.x, labeller=label_both)

    p
}










age_class_mortality <- function(spat_data, srv_data)
{
    tdata <- srv_data %>%
        group_by(SimNr, ITVsd, PFT) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        select(SimNr, PFT, lives)

    tdata <- tbl_df(inner_join(spat_data, tdata))

    tdata <- tdata %>%
        group_by(SimNr, ComNr,ITVsd, PFT, plantID, lives) %>%
        filter(Age == max(Age))

    tdata <- tdata %>%
        group_by(SimNr, ComNr, ITVsd, PFT, lives) %>%
        mutate(total = n())

    tdata <- tdata %>%
        group_by(SimNr, ComNr, ITVsd, PFT, total, lives) %>%
        filter(totalSeeds == 0) %>%
        summarize(noRepro = n())

    tdata <- tdata %>%
        group_by(SimNr, ComNr, ITVsd, PFT, lives) %>%
        summarize(perc = noRepro/total)

    p <- ggplot(tdata)

    p <- p + geom_line(aes(x = ITVsd,
                           y = 1 - perc))

    p <- p + geom_point(aes(x = ITVsd,
                            y = 1 - perc,
                            color = as.factor(lives)),
                        size = 5)

    p <- p + facet_wrap(~ PFT)

    p <- p + theme_minimal(16)

    p <- p + labs(title = "Percentage of each PFT that reproduces",
                  x = "ITVsd",
                  y = "% of the population")

    p <- p + scale_color_discrete("Survives for 100 years",
                                  breaks=c(0, 1),
                                  labels=c("No", "Yes"))

    p
}
