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






















p <- ggplot(tdata)

p <- p + geom_histogram(tdata, ind_lifetime_fecundity)

p




























#FIG4 Keystone Individuals Alternative Graph ------------------

# Determine rarity
t <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year) %>%
    mutate(Nind = n())

t <- t %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    summarise(avg_population = mean(Nind))

t <- t %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    mutate(Class = ifelse(avg_population > 50,
                          "Common",
                          "Rare"))

originally_rare <- t %>%
    group_by(ComNr, RunNr, PFT) %>%
    filter(ITVsd == 0) %>%
    summarise(originally_rare = ifelse(Class == "Rare",
                                       "Originally rare",
                                       "Originally common")) %>%
    select(ComNr, PFT, originally_rare)

t <- inner_join(t, originally_rare)
# End determine rarity

tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, year, PFT) %>%
    mutate(PFT_nTotalSeeds = sum(accumulatedSeeds))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, year, PFT, plantID) %>%
    summarise(proportionTotalSeeds = ifelse(accumulatedSeeds > 0,
                                            accumulatedSeeds / PFT_nTotalSeeds,
                                            0))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year) %>%
    summarise(maxProportionSeeds = max(proportionTotalSeeds))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    filter(maxProportionSeeds > .5) %>%
    summarise(nYearsOverThreshold = n())

tdata <- inner_join(tdata, t)

tdata %>% group_by(ITVsd, originally_rare) %>% summarise(mean(nYearsOverThreshold))

p <- ggplot(tdata)

p <- p + geom_histogram(aes(x = nYearsOverThreshold),
                        color = "black",
                        fill = "white",
                        binwidth = 2)

means <- tdata %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(yearsSaved_mean = mean(nYearsOverThreshold))

p <- p + geom_vline(data = means,
                    aes(xintercept = yearsSaved_mean),
                    linetype = "dashed",
                    size = 1)

p <- p + facet_grid(originally_rare ~ ITVsd,
                    labeller = labeller(ITVsd = label_both))

p <- p + geom_text(data = means,
                   aes(label = paste("µ:", sprintf("%.02f", yearsSaved_mean)),
                       x = 67,
                       y = 225),
                   size = 8)

p <- p + labs(x = "# of years one plant produces >50% of its PFT's reproductive mass",
              y = "Frequency of PFT-years")

# p <- p + labs(x = "Proportion of reproduction done by the most fit individual per PFT-year",
#               y = "Density of PFT-years")

p <- p + theme_few(26)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    panel.margin = unit(0.5, "lines"),
    axis.ticks = element_line(colour = "black"),
    axis.title.x = element_text(vjust=-2),
    axis.title.y = element_text(vjust=2),
    plot.margin = unit(c(0,1,0,1), "lines"),
    aspect.ratio = 1)

p

ggsave(filename = "MostFit.pdf",
       path = "~/Desktop")














#FIG4 Keystone Individuals Top 10% Graph ------------------

# Determine rarity
t <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year) %>%
    mutate(Nind = n())

t <- t %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    summarise(avg_population = mean(Nind))

t <- t %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    mutate(Class = ifelse(avg_population > 50,
                          "Common",
                          "Rare"))

originally_rare <- t %>%
    group_by(ComNr, PFT) %>%
    filter(ITVsd == 0) %>%
    summarise(originally_rare = (Class == "Rare")) %>%
    select(ComNr, PFT, originally_rare)

t <- inner_join(t, originally_rare)
# End determine rarity

tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, plantID) %>%
    filter(Age = max(Age)) %>%
    summarise(measure = totalSeeds) %>%
    filter(measure > 0)

tdata <- inner_join(tdata, t)

means <- tdata %>%
    group_by(ITVsd, Class) %>%
    summarise(avg_measure = mean(measure))

means

p <- ggplot(tdata)

p <- p + geom_histogram(aes(x = measure,
                            y = ..density..))

p <- p + scale_x_log10()

# p <- p + scale_y_log10()

p <- p + facet_grid(originally_rare ~ ITVsd, labeller = label_both)

p















tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, plantID) %>%
    filter(Age = max(Age)) %>%
    summarise(average_yearly_fitness = totalSeeds/Age)

tdata <- inner_join(tdata, t)

zeros <- tdata %>%
    group_by(ITVsd, Class) %>%
    filter(average_yearly_fitness == 0) %>%
    summarise(n_zero = n())

total <- tdata %>%
    group_by(ITVsd, Class) %>%
    summarise(n = n())

zero_ratio <- full_join(zeros, total)












#Keystone Individuals Single Plant Records Graph ------------------

# Determine rarity
t <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, year, PFT) %>%
    mutate(Nind = n())

t <- t %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    summarise(avg_population = mean(Nind))

t <- t %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    mutate(Class = ifelse(avg_population > 50,
                          "Common",
                          "Rare"))
# End determine rarity

tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, year, PFT) %>%
    mutate(PFT_nTotalSeeds = sum(accumulatedSeeds))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year, plantID) %>%
    summarise(proportionTotalSeeds = ifelse(accumulatedSeeds > 0,
                                            accumulatedSeeds / PFT_nTotalSeeds,
                                            0))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year, plantID) %>%
    filter(proportionTotalSeeds > .5)

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, plantID) %>%
    summarise(n_PersonalSaves = n())

tdata <- inner_join(tdata, t)

tmp <- tdata

tdata %>% group_by(ITVsd, Class) %>% summarise(mean(n_PersonalSaves))

p <- ggplot(tdata)

p <- p + geom_histogram(aes(x = n_PersonalSaves),
                        color = "black",
                        fill = "white")

p <- p + facet_grid(Class ~ ITVsd, labeller = label_both)

p <- p + theme_few(26)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    panel.margin = unit(0.5, "lines"),
    axis.ticks = element_line(colour = "black"),
    axis.title.x = element_text(vjust=-2),
    axis.title.y = element_text(vjust=2),
    plot.margin = unit(c(0,1,0,1), "lines"),
    aspect.ratio = 1)

p

