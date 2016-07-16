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


#SUPL3_ITV_IC_Tmax_StDev----------

#USE "BROADITV" DATASET

## Species Richness
# tdata <- srv_data %>%
#     group_by(ComNr, RunNr, IC_ver, ITVsd, PFT, Tmax) %>%
#     summarise(lives = ifelse(cPop > 0, 1, 0))
#
# tdata <- tdata %>%
#     group_by(ComNr, RunNr, IC_ver, ITVsd, Tmax) %>%
#     summarise(richness = sum(lives), start = n())
#
# tdata <- tdata %>%
#     group_by(ComNr, IC_ver, ITVsd, Tmax) %>%
#     summarise(richness = mean(richness), start = start)
#
# tdata <- tdata %>%
#     group_by(IC_ver, ITVsd, Tmax) %>%
#     summarise_each(funs(mean, sd), richness)

## Shannon diversity
tdata <- grd_data %>%
    group_by(ComNr, RunNr, IC_ver, ITVsd, Tmax) %>%
    filter(Year == max(Year))

tdata <- tdata %>%
    group_by(ComNr, IC_ver, ITVsd, Tmax) %>%
    summarise(mean_shannon = mean(shannon), sem_shannon = sd(shannon)/sqrt(n()))

# Average standard error of the replicates
mean(tdata$sem_shannon)
# 95% confidence intervals of the mean of the replicates
c(mean(tdata$mean_shannon)-2*mean(tdata$sem_shannon),
  mean(tdata$mean_shannon)+2*mean(tdata$sem_shannon))

tdata <- tdata %>%
    group_by(IC_ver, ITVsd, Tmax) %>%
    summarise_each(funs(mean, sd), mean_shannon)

## Continue as usual
tdata <- tdata %>%
    dplyr::rename(Timespan = Tmax)

tdata <- tdata %>%
    filter(IC_ver == 1)

p <- ggplot()

# p <- p + geom_line(data = tdata,
#                    aes(x = ITVsd,
#                        y = mean,
#                        linetype = factor(IC_ver)))

p <- p + geom_line(data = tdata,
                   aes(x = ITVsd,
                       y = mean,
                       linetype = factor(Timespan)))

p <- p + geom_errorbar(data = tdata,
                       aes(x = ITVsd,
                           ymin = mean-sd,
                           ymax = mean+sd,
                           group = factor(Timespan)),
                       position = position_dodge(0.01),
                       width = 0.02)

p <- p + geom_point(data = tdata,
                    aes(x = ITVsd,
                        y = mean,
                        group = factor(Timespan)),
                    position = position_dodge(0.01),
                    size = 3)

p <- p + labs(x = "ITVsd",
              y = "Shannon diversity")
              # y = "Species richness")

# p <- p + scale_linetype_discrete(name="Intraspecific negative\ndensity dependence",
#                                  breaks=c(1, 0),
#                                  labels=c("Present", "Absent"))

p <- p + scale_linetype_discrete(name = "Timespan (yrs)")

p <- p + scale_x_continuous(breaks = c(0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7),
                            labels = c("0", "", "0.2", "", "0.4", "", "0.6", ""))

# p <- p + facet_grid(. ~ Timespan, labeller = label_both)

p <- p + theme_few(26)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    panel.margin = unit(0.5, "lines"),
    axis.ticks = element_line(colour = "black"),
    axis.title.x = element_text(vjust=-2),
    axis.title.y = element_text(vjust=2),
    legend.key.height=unit(1, "cm"),
    legend.key.width=unit(1, "cm"),
    plot.margin = unit(c(1,1,1,1), "lines"),
    aspect.ratio = 1)

p

ggsave(filename = "SUPL3.pdf",
       path = "~/Desktop")













#FIG1_ITV_IC_Tmax_CI----------

#USE "BROADITV" DATASET

## Species Richness
# tdata <- srv_data %>%
#     group_by(ComNr, RunNr, IC_ver, ITVsd, PFT, Tmax) %>%
#     summarise(lives = ifelse(cPop > 0, 1, 0))
#
# tdata <- tdata %>%
#     group_by(ComNr, RunNr, IC_ver, ITVsd, Tmax) %>%
#     summarise(richness = sum(lives), start = n())
#
# tdata <- tdata %>%
#     group_by(ComNr, IC_ver, ITVsd, Tmax) %>%
#     summarise(richness = mean(richness), start = start)
#
# tdata <- tdata %>%
#     group_by(IC_ver, ITVsd, Tmax) %>%
#     summarise_each(funs(mean, sd), richness)

## Shannon diversity
tdata <- grd_data %>%
    group_by(ComNr, RunNr, IC_ver, ITVsd, Tmax) %>%
    filter(Year == max(Year))

tdata <- tdata %>%
    group_by(ComNr, IC_ver, ITVsd, Tmax) %>%
    summarise(mean_shannon = mean(shannon))

# Average standard error of the replicates
# mean(tdata$sem_shannon)
# 95% confidence intervals of the mean of the replicates
# c(mean(tdata$mean_shannon)-2*mean(tdata$sem_shannon),
  # mean(tdata$mean_shannon)+2*mean(tdata$sem_shannon))

tdata <- tdata %>%
    group_by(IC_ver, ITVsd, Tmax) %>%
    summarise(mean = mean(mean_shannon),
              sd = sd(mean_shannon),
              CI = 2 * sd(mean_shannon)/sqrt(n()))

## Continue as usual
tdata <- tdata %>%
    dplyr::rename(Timespan = Tmax)

tdata <- tdata %>%
    filter(IC_ver == 1)

p <- ggplot()

# p <- p + geom_line(data = tdata,
#                    aes(x = ITVsd,
#                        y = mean,
#                        linetype = factor(IC_ver)))

p <- p + geom_line(data = tdata,
                   aes(x = ITVsd,
                       y = mean,
                       linetype = factor(Timespan)))

p <- p + geom_errorbar(data = tdata,
                       aes(x = ITVsd,
                           ymin = mean - CI,
                           ymax = mean + CI,
                           group = factor(Timespan)),
                       position = position_dodge(0.01),
                       width = 0.02)

p <- p + geom_point(data = tdata,
                    aes(x = ITVsd,
                        y = mean,
                        group = factor(Timespan)),
                    position = position_dodge(0.01),
                    size = 3)

p <- p + labs(x = "ITVsd",
              y = "Shannon diversity")
# y = "Species richness")

# p <- p + scale_linetype_discrete(name="Intraspecific negative\ndensity dependence",
#                                  breaks=c(1, 0),
#                                  labels=c("Present", "Absent"))

p <- p + scale_linetype_discrete(name = "Timespan (yrs)")

p <- p + scale_x_continuous(breaks = c(0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7),
                            labels = c("0", "", "0.2", "", "0.4", "", "0.6", ""))

# p <- p + facet_grid(. ~ Timespan, labeller = label_both)

p <- p + theme_few(26)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    panel.margin = unit(0.5, "lines"),
    axis.ticks = element_line(colour = "black"),
    axis.title.x = element_text(vjust=-2),
    axis.title.y = element_text(vjust=2),
    legend.key.height=unit(1, "cm"),
    legend.key.width=unit(1, "cm"),
    plot.margin = unit(c(1,1,1,1), "lines"),
    aspect.ratio = 1)

p

ggsave(filename = "FIG1.pdf",
       path = "~/Desktop")












#FIG2_5000Years-----------
# USE "LONGTERM" DATASET
tdata <- grd_data %>%
    group_by(ComNr, RunNr, IC_ver, ITVsd, Tmax) %>%
    filter(Year == max(Year))

tdata <- tdata %>%
    group_by(ComNr, IC_ver, ITVsd, Tmax) %>%
    summarise(shannon = mean(shannon))

tdata <- tdata %>%
    group_by(IC_ver, ITVsd, Tmax) %>%
    summarise_each(funs(mean, sd, se), shannon)

p <- ggplot()

p <- p + geom_line(data = tdata,
                   aes(x = Tmax,
                       y = mean,
                       linetype = factor(IC_ver),
                       group = factor(IC_ver)))

p <- p + geom_errorbar(data = tdata,
                       aes(x = Tmax,
                           ymin = mean-sd,
                           ymax = mean+sd,
                           group = factor(IC_ver)),
                       width=50)

p <- p + geom_point(data = tdata,
                    aes(x = Tmax,
                        y = mean),
                    size=4)

p <- p + labs(x="Year",
              y="Shannon diversity")

p <- p + scale_linetype_discrete(name="Intraspecific negative\ndensity dependence",
                                 breaks=c(1, 0),
                                 labels=c("Present", "Absent"))

p <- p + theme_few(26)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    # panel.margin = unit(0.5, "lines"),
    axis.ticks = element_line(colour = "black"),
    # axis.title.x = element_text(vjust=-2),
    # axis.title.y = element_text(vjust=2),
    # plot.margin = unit(c(2,2,2,2), "lines"),
    legend.key.height=unit(1, "cm"),
    legend.key.width=unit(1, "cm"),
    aspect.ratio = 1)

p

ggsave(filename = "FIG2.pdf",
       path = "~/Desktop")





#FIG3,4 Four Chart Setup---------------
# USE A "STRESS_AND_COMPETITION" DATASET
# Among individuals
tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, plantID) %>%
    summarise(ind_lifetime_stress = sum(stress),
              ind_memory = mean(memory),
              ind_max_age = max(Age),
              ind_lifetime_fecundity = max(totalSeeds) * mean(SeedMass))

# Among PFTs
tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    summarise(PFT_total_stress = sum(ind_lifetime_stress),
              PFT_total_memory = sum(ind_memory),
              PFT_total_population = n(),
              PFT_total_plantYears = sum(ind_max_age),
              PFT_total_fecundity = sum(ind_lifetime_fecundity))

# Among runs
tdata <- tdata %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    summarise(PFT_avg_mort = (sum(PFT_total_stress) / sum(PFT_total_plantYears)) /
                  (sum(PFT_total_memory) / sum(PFT_total_population)),
              PFT_total_fecundity = sum(PFT_total_fecundity),
              PFT_total_population = sum(PFT_total_population))

## Add a measure of whether the PFT lives and how often
t <- srv_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    mutate(tExtinction = ifelse(TE != 0, as.numeric(TE), 100),
           lives = ifelse(cPop > 0, 1, 0))

t <- t %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    summarise(PFT_total_tExtinction = sum(tExtinction),
              lives = sum(lives),
              total = n())

tdata <- tbl_df(inner_join(tdata, t))

tdata <- tdata %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    summarise(PFT_aPop_norm = PFT_total_population/PFT_total_tExtinction,
              PFT_aFec_norm = PFT_total_fecundity/PFT_total_tExtinction,
              PFT_avg_mort = PFT_avg_mort,
              PFT_avg_pers = lives / total)

t <- tdata %>%
    filter(ITVsd == 0) %>%
    mutate(originally_rare = ifelse(PFT_aPop_norm < 50, T, F)) %>%
    ungroup() %>%
    select(ComNr, PFT, originally_rare)

tdata <- inner_join(tdata, t)

## Persistence
tmp <- tdata %>%
    group_by(ITVsd) %>%
    summarise(PFT_avg_pers = mean(PFT_avg_pers))

ITVsd_0 = (tmp %>% filter(ITVsd == 0))$PFT_avg_pers
ITVsd_.2 = (tmp %>% filter(ITVsd == 0.2))$PFT_avg_pers
ITVsd_.5 = (tmp %>% filter(ITVsd == 0.5))$PFT_avg_pers
cat("Persistence, total (0 - 0.2): ", (ITVsd_.2 - ITVsd_0) / ITVsd_0 * 100)
cat("Persistence, total (0 - 0.5): ", (ITVsd_.5 - ITVsd_0) / ITVsd_0 * 100)

## How many PFTs are negatively influenced by ITV?
ITVsd_0 = (tdata %>% filter(ITVsd == 0, originally_rare==F))$PFT_avg_pers
ITVsd_.2 = (tdata %>% filter(ITVsd == 0.2, originally_rare==F))$PFT_avg_pers
ITVsd_.5 = (tdata %>% filter(ITVsd == 0.5, originally_rare==F))$PFT_avg_pers
cat("Persistence, total (0 - 0.2): ",
    length(which(((ITVsd_.2 - ITVsd_0) / ITVsd_0 * 100) < 0))/length(ITVsd_0))
cat("Persistence, total (0 - 0.5): ",
    length(which(((ITVsd_.5 - ITVsd_0) / ITVsd_0 * 100) < 0))/length(ITVsd_0))



## Persistence categorized
tmp <- tdata %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(PFT_avg_pers = mean(PFT_avg_pers))

ITVsd_0_Common = (tmp %>% filter(ITVsd == 0, originally_rare == F))$PFT_avg_pers
ITVsd_0.2_Common = (tmp %>% filter(ITVsd == 0.2, originally_rare == F))$PFT_avg_pers
ITVsd_0.5_Common = (tmp %>% filter(ITVsd == 0.5, originally_rare == F))$PFT_avg_pers
cat("Persistence, common (0 - 0.2): ", (ITVsd_0.2_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)
cat("Persistence, common (0 - 0.5): ", (ITVsd_0.5_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)

ITVsd_0_Rare = (tmp %>% filter(ITVsd == 0, originally_rare == T))$PFT_avg_pers
ITVsd_0.2_Rare = (tmp %>% filter(ITVsd == 0.2, originally_rare == T))$PFT_avg_pers
ITVsd_0.5_Rare = (tmp %>% filter(ITVsd == 0.5, originally_rare == T))$PFT_avg_pers
cat("Persistence, rare (0 - 0.2): ", (ITVsd_0.2_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)
cat("Persistence, rare (0 - 0.5): ", (ITVsd_0.5_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)






## Fecundity total
tmp <- tdata %>%
    group_by(ITVsd) %>%
    summarise(PFT_avg_fec = mean(PFT_aFec_norm))

ITVsd_0 = (tmp %>% filter(ITVsd == 0))$PFT_avg_fec
ITVsd_.2 = (tmp %>% filter(ITVsd == 0.2))$PFT_avg_fec
ITVsd_.5 = (tmp %>% filter(ITVsd == 0.5))$PFT_avg_fec
cat("Fecundity, total (0 - 0.2): ", (ITVsd_.2 - ITVsd_0) / ITVsd_0 * 100)
cat("Fecundity, total (0 - 0.5): ", (ITVsd_.5 - ITVsd_0) / ITVsd_0 * 100)

## Fecundity categorized
tmp <- tdata %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(PFT_avg_fec = mean(PFT_aFec_norm))

ITVsd_0_Common = (tmp %>% filter(ITVsd == 0, originally_rare == F))$PFT_avg_fec
ITVsd_0.2_Common = (tmp %>% filter(ITVsd == 0.2, originally_rare == F))$PFT_avg_fec
ITVsd_0.5_Common = (tmp %>% filter(ITVsd == 0.5, originally_rare == F))$PFT_avg_fec
cat("Fecundity, common (0 - 0.2): ", (ITVsd_0.2_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)
cat("Fecundity, common (0 - 0.5): ", (ITVsd_0.5_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)

ITVsd_0_Rare = (tmp %>% filter(ITVsd == 0, originally_rare == T))$PFT_avg_fec
ITVsd_0.2_Rare = (tmp %>% filter(ITVsd == 0.2, originally_rare == T))$PFT_avg_fec
ITVsd_0.5_Rare = (tmp %>% filter(ITVsd == 0.5, originally_rare == T))$PFT_avg_fec
cat("Fecundity, rare (0 - 0.2): ", (ITVsd_0.2_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)
cat("Fecundity, rare (0 - 0.5): ", (ITVsd_0.5_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)

## Mortality total
tmp <- tdata %>%
    group_by(ITVsd) %>%
    summarise(PFT_avg_mort = mean(PFT_avg_mort))

ITVsd_0 = (tmp %>% filter(ITVsd == 0))$PFT_avg_mort
ITVsd_.2 = (tmp %>% filter(ITVsd == 0.2))$PFT_avg_mort
ITVsd_.5 = (tmp %>% filter(ITVsd == 0.5))$PFT_avg_mort
cat("Mortality, total (0 - 0.2): ", (ITVsd_.2 - ITVsd_0) / ITVsd_0 * 100)
cat("Mortality, total (0 - 0.5): ", (ITVsd_.5 - ITVsd_0) / ITVsd_0 * 100)

## Mortality categorized
tmp <- tdata %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(PFT_avg_mort = mean(PFT_avg_mort))

ITVsd_0_Common = (tmp %>% filter(ITVsd == 0, originally_rare == F))$PFT_avg_mort
ITVsd_0.2_Common = (tmp %>% filter(ITVsd == 0.2, originally_rare == F))$PFT_avg_mort
ITVsd_0.5_Common = (tmp %>% filter(ITVsd == 0.5, originally_rare == F))$PFT_avg_mort
cat("Mortality, common (0 - 0.2): ", (ITVsd_0.2_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)
cat("Mortality, common (0 - 0.5): ", (ITVsd_0.5_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)

ITVsd_0_Rare = (tmp %>% filter(ITVsd == 0, originally_rare == T))$PFT_avg_mort
ITVsd_0.2_Rare = (tmp %>% filter(ITVsd == 0.2, originally_rare == T))$PFT_avg_mort
ITVsd_0.5_Rare = (tmp %>% filter(ITVsd == 0.5, originally_rare == T))$PFT_avg_mort
cat("Mortality, rare (0 - 0.2): ", (ITVsd_0.2_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)
cat("Mortality, rare (0 - 0.5): ", (ITVsd_0.5_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)

## Average population total
tmp <- tdata %>%
    group_by(ITVsd) %>%
    summarise(PFT_aPop_norm = mean(PFT_aPop_norm))

ITVsd_0 = (tmp %>% filter(ITVsd == 0))$PFT_aPop_norm
ITVsd_.2 = (tmp %>% filter(ITVsd == 0.2))$PFT_aPop_norm
ITVsd_.5 = (tmp %>% filter(ITVsd == 0.5))$PFT_aPop_norm
cat("Population, total (0 - 0.2): ", (ITVsd_.2 - ITVsd_0) / ITVsd_0 * 100)
cat("Population, total (0 - 0.5): ", (ITVsd_.5 - ITVsd_0) / ITVsd_0 * 100)

##  Average population categorized
tmp <- tdata %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(PFT_aPop_norm = mean(PFT_aPop_norm))

ITVsd_0_Common = (tmp %>% filter(ITVsd == 0, originally_rare == F))$PFT_aPop_norm
ITVsd_0.2_Common = (tmp %>% filter(ITVsd == 0.2, originally_rare == F))$PFT_aPop_norm
ITVsd_0.5_Common = (tmp %>% filter(ITVsd == 0.5, originally_rare == F))$PFT_aPop_norm
cat("Population, common (0 - 0.2): ", (ITVsd_0.2_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)
cat("Population, common (0 - 0.5): ", (ITVsd_0.5_Common - ITVsd_0_Common) / ITVsd_0_Common * 100)

ITVsd_0_Rare = (tmp %>% filter(ITVsd == 0, originally_rare == T))$PFT_aPop_norm
ITVsd_0.2_Rare = (tmp %>% filter(ITVsd == 0.2, originally_rare == T))$PFT_aPop_norm
ITVsd_0.5_Rare = (tmp %>% filter(ITVsd == 0.5, originally_rare == T))$PFT_aPop_norm
cat("Population, rare (0 - 0.2): ", (ITVsd_0.2_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)
cat("Population, rare (0 - 0.5): ", (ITVsd_0.5_Rare - ITVsd_0_Rare) / ITVsd_0_Rare * 100)





#FIG3,4 FourFacetChart GRAPHIC --------------------

d <- read_csv("/Users/Theodore/Documents/Google_Drive/Research/Projects/ITV/R/four_facet_data.txt")

d <- d %>% mutate(fitness_measure = ifelse(fitness_measure == "persistence",
                                           "Persistence Probability",
                                           fitness_measure))

d <- d %>% mutate(fitness_measure = ifelse(fitness_measure == "mortality_risk",
                                           "Mortality Risk",
                                           fitness_measure))

d <- d %>% mutate(fitness_measure = ifelse(fitness_measure == "avg_population",
                                           "Mean Population Count",
                                           fitness_measure))

d <- d %>% mutate(fitness_measure = ifelse(fitness_measure == "total_fecundity",
                                           "Total Reproductive Mass",
                                           fitness_measure))

d <- d %>% mutate(ITV = ifelse(ITV == 0.2, "0 - 0.2", ITV))
d <- d %>% mutate(ITV = ifelse(ITV == 0.5, "0 - 0.5", ITV))
d <- d %>% mutate(ITV = ifelse(ITV == 0.5, "0 - 0.5", ITV))

d <- d %>% filter(class != "total")

d <- d %>% mutate(class = ifelse(class == "common", "Common PFTs     ", class))
d <- d %>% mutate(class = ifelse(class == "rare", "Rare PFTs     ", class))
d <- d %>% mutate(class = ifelse(class == "total", "Total     ", class))

d <- d %>% filter(class != "Total")

d <- d %>% mutate(percent_change = round(percent_change, 0))

p <- ggplot()

p <- p + geom_bar(data = d %>% filter(fitness_measure %in% c("Persistence Probability", "Mean Population Count")),
                  aes(x = ITV,
                      y = percent_change,
                      group = class,
                      fill = class),
                  stat="identity",
                  position = "dodge")

p <- p + geom_text(data = d %>% filter(fitness_measure %in% c("Persistence Probability", "Mean Population Count")),
                   aes(x = ITV,
                       y = ifelse(percent_change > 0,
                                  percent_change + 5,
                                  5),
                       label = paste(percent_change, "%", sep=""),
                       group = class),
                   vjust = 0,
                   size = 7,
                   position = position_dodge(0.9))

p <- p + facet_wrap(~ fitness_measure)

p <- p + scale_fill_grey("")

p <- p + xlab("ITVsd")

p <- p + ylab("Percent change from no ITV")

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
    legend.key.height=unit(2, "line"),
    legend.key.width=unit(2, "line"),
    legend.position="top",
    aspect.ratio = 1)

p

ggsave(path=outdir,
       filename="FIG3.pdf")

####

p <- ggplot()

p <- p + geom_bar(data = d %>% filter(fitness_measure %in% c("Mortality Risk", "Total Reproductive Mass")),
                  aes(x = ITV,
                      y = percent_change,
                      group = class,
                      fill = class),
                  stat="identity",
                  position = "dodge")

p <- p + geom_text(data = d %>% filter(fitness_measure %in% c("Mortality Risk", "Total Reproductive Mass")),
                   aes(x = ITV,
                       y = ifelse(percent_change > 0,
                                  percent_change + 1,
                                  1),
                       label = paste(percent_change, "%", sep=""),
                       group = class),
                   vjust = 0,
                   size = 7,
                   position = position_dodge(0.9))

p <- p + facet_wrap(~ fitness_measure)

p <- p + scale_fill_grey("")

p <- p + xlab("ITVsd")

p <- p + ylab("Percent change from no ITV")

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
    legend.key.height=unit(2, "line"),
    legend.key.width=unit(2, "line"),
    legend.position="top",
    aspect.ratio = 1)

p

ggsave(path=outdir,
       filename="FIG4.pdf")





#FIG4 Keystone Individuals ------------------
tdata <- srv_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    mutate(lives = ifelse(cPop > 0, 1, 0),
           tExtinction = ifelse(TE != 0, as.numeric(TE), 100))

tdata <- tdata %>%
    select(SimNr, ComNr, RunNr, PFT, lives, tExtinction)

tdata <- tbl_df(inner_join(spat_data, tdata))

tdata <- tdata %>%
    filter(lives == 1)

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year) %>%
    mutate(PFT_nTotalSeeds = sum(accumulatedSeeds))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year, plantID) %>%
    mutate(proportionTotalSeeds = ifelse(accumulatedSeeds > 0,
                                         accumulatedSeeds / PFT_nTotalSeeds,
                                         0))

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
    mutate(Nind = n())

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, year, tExtinction, Nind) %>%
    summarise(maxProportionSeeds = max(proportionTotalSeeds)) %>%
    filter(maxProportionSeeds > 0.5)

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, tExtinction, Nind) %>%
    summarise(saved = n())

tdata <- tdata %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    summarise(percentSaved = sum(saved)/sum(tExtinction),
              avg_population = sum(Nind)/sum(tExtinction))

tdata <- tdata %>%
    group_by(SimNr, ComNr, ITVsd, PFT) %>%
    mutate(Class = ifelse(avg_population > 50,
                          "Common",
                          "Rare"))

means <- tdata %>%
    group_by(ITVsd, Class) %>%
    summarise(percentSaved_mean = mean(percentSaved),
              percentSave_sd = sd(percentSaved))

p <- ggplot(tdata)

p <- p + geom_histogram(aes(x = percentSaved,
                            y = ..density..),
                        binwidth = 0.05,
                        color = "black",
                        fill = "white")

p <- p + geom_vline(data = means,
                    aes(xintercept = percentSaved_mean),
                    linetype = "dashed",
                    size = 1)

p <- p + facet_grid(Class ~ ITVsd, labeller = label_both)

p <- p + geom_text(data = means,
                   aes(label = paste("µ:", sprintf("%.02f", percentSaved_mean)),
                       x = 0.71,
                       y = 15),
                   size = 8)

p <- p + labs(x = "Proportion of years when one plant produces over 50% of its PFT's seeds",
              y = "Density of PFTs")

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

ggsave(filename = "FIG4.pdf",
       path = "~/Desktop")





#SUPL TraitVariation ----------------
tdata <- spat_data %>%
    group_by(SimNr, ITVsd, ComNr, RunNr, PFT, plantID,
             LMR, MaxMass, Dist, SLA, palat, memory, Gmax) %>%
    summarise(cumsum_lifetime_stress = sum(stress),
              maximum_age = max(Age),
              realized_lifetime_fecundity = max(totalSeeds),
              max_realized_shootMass = max(mshoot),
              max_realized_rootMass = max(mroot))

tdata <- tdata %>%
    ungroup() %>%
    # filter(ComNr == 3) %>%
    filter(ITVsd %in% c(0, 0.2))

tdata <- tdata %>%
    group_by(PFT) %>%
    sample_n(100, replace=T)

p <- ggplot(tdata)

p <- p + geom_histogram(aes(x = LMR,
                            fill = PFT))

p <- p + scale_colour_brewer(palette = "Greens")

p <- p + facet_wrap(~ ITVsd, ncol = 2, labeller = "label_both")

p <- p + xlab("Leaf mass ratio (LMR)")

p <- p + ylab("Frequency")

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
    legend.key.height=unit(2, "line"),
    legend.key.width=unit(2, "line"),
    aspect.ratio = 1)

p

ggsave(filename = "SUPL1.pdf",
       path = "~/Desktop")





#SUPL_AbundanceTimeSeries ---------------
tdata <- pft_data %>%
    filter(IC_ver == 1, Tmax == 100, ITVsd %in% c(0, 0.2, 0.5)) %>%
    select(SimNr, ComNr, RunNr, Year, IC_ver, ITVsd, PFT, Nind, rootmass, shootmass)

tdata <- tdata %>%
    filter(ComNr == sample(ComNr, 1), RunNr == sample(RunNr, 1))

p <- ggplot(tdata)

p <- p + geom_line(aes(x = Year,
                       # y = rootmass + shootmass,
                       y = Nind,
                       # linetype = factor(ITVsd)))
                       col = factor(ITVsd)))

p <- p + facet_wrap( ~ PFT,
                     labeller = label_both)

p <- p + theme_few(18)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    panel.margin = unit(0.2, "lines"),
    axis.ticks = element_line(colour = "black"),
    axis.title.x = element_text(vjust=-2),
    axis.title.y = element_text(vjust=2),
    plot.margin = unit(c(2,2,2,2), "lines"))

p <- p + labs(x = "Year",
              # y = "Total biomass")
              y = "Population count")

p <- p + scale_color_discrete("ITVsd")

p

ggsave(filename = "SUPL_AbundanceTimeSeries.pdf",
       path = "~/Desktop")





#SUPL_RankAbundance ----------------
# FOR SUPPLEMENT -- Rank abundance curves. USE BROAD DATASET
tdata <- srv_data

tdata <- tdata %>%
    group_by(SimNr, RunNr, PFT, ITVsd, IC_ver) %>%
    filter(year == max(year),
           ITVsd %in% c(0, 0.2, 0.5)) %>%
    summarise(abundance = cPop)

tdata <- tdata %>%
    group_by(SimNr, RunNr, ITVsd, IC_ver) %>%
    mutate(rank = n() - row_number(abundance))

tdata <- tdata %>%
    group_by(rank, ITVsd, IC_ver) %>%
    summarise(mean_abundance = mean(abundance))

## START -- Unrelated to the graph
tdata <- tdata %>%
    group_by(ITVsd, IC_ver) %>%
    mutate(total_abundance = sum(mean_abundance))

tdata <- tdata %>%
    group_by(rank, ITVsd, IC_ver) %>%
    mutate(perc_total_abundance = mean_abundance / total_abundance)

tdata <- tdata %>%
    ungroup() %>%
    arrange(ITVsd, IC_ver, rank)
## END -- Unrelated to the graph

tdata <- tdata %>%
    dplyr::rename(Stabilizing_mechanism = IC_ver) %>%
    mutate(Stabilizing_mechanism = ifelse(Stabilizing_mechanism == 0, "Off", "On"))

tdata <- tdata %>%
    filter(Stabilizing_mechanism == "On")

p <- ggplot(tdata)

p <- p + geom_line(aes(x = rank,
                       y = mean_abundance,
                       linetype = factor(ITVsd)))
#
#     p <- p + facet_wrap(~ Stabilizing_mechanism,
#                         scales = "free",
#                         labeller = label_both)

p <- p + theme_few(26)

p <- p + theme(
    panel.border = element_rect(fill = NA,
                                colour = "black",
                                size = 1),
    panel.margin = unit(0.5, "lines"),
    axis.ticks = element_line(colour = "black"),
    axis.title.x = element_text(vjust=-2),
    axis.title.y = element_text(vjust=2),
    plot.margin = unit(c(2,2,2,2), "lines"),
    aspect.ratio = 1)

p <- p + labs(x = "Rank",
              y = "Abundance")

p <- p + scale_linetype_discrete("ITVsd")

p

ggsave(filename = "SUPL2.pdf",
       path = "~/Desktop")





#SUPL_NMDS--------------
# Do this with each ITV level.
tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, PFT, plantID,
             LMR, MaxMass, Dist, SLA, palat, memory, Gmax) %>%
    filter(ITVsd == 0.2) %>%
    summarise(cumsum_lifetime_stress = sum(stress),
              maximum_age = max(Age),
              realized_lifetime_fecundity = max(totalSeeds),
              max_realized_shootMass = max(mshoot),
              max_realized_rootMass = max(mroot))

tdata <- tdata %>%
    ungroup() %>%
    select(-SimNr, -ComNr, -RunNr, -PFT, -plantID)

# rename fixed traits
names(tdata)[names(tdata) == 'LMR'] <- 'Fixed growth form'
names(tdata)[names(tdata) == 'MaxMass'] <- 'Fixed maximum plant mass'
names(tdata)[names(tdata) == 'Dist'] <- 'Fixed dispersal distance'
names(tdata)[names(tdata) == 'palat'] <- 'Fixed palatability'
names(tdata)[names(tdata) == 'memory'] <- 'Fixed stress tolerance'
names(tdata)[names(tdata) == 'Gmax'] <- 'Fixed maximum resource uptake rate'

names(tdata)[names(tdata) == 'cumsum_lifetime_stress'] <- 'Realized cumulative lifetime stress'
names(tdata)[names(tdata) == 'maximum_age'] <- 'Realized maximum age'
names(tdata)[names(tdata) == 'realized_lifetime_fecundity'] <- 'Realized cumulative lifetime fecundity'
names(tdata)[names(tdata) == 'max_realized_shootMass'] <- 'Realized maximum shoot mass'
names(tdata)[names(tdata) == 'max_realized_rootMass'] <- 'Realized maximum root mass'

tdata <- tdata %>% sample_n(1000)

t <- melt(tdata)
p <- ggplot(t)
p <- p + geom_density(aes(value))
p <- p + facet_wrap(~ variable, scales="free")
p

tdata.MDS <- metaMDS(tdata, trymax = 100)

stressplot(tdata.MDS)

plot(tdata.MDS)

ef <- envfit(tdata.MDS, tdata)

plot(ef, add=T)





#SUPL_PCA ---------------
tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, PFT, plantID, MaxMass, SLA, Gmax, LMR) %>%
    filter(ITVsd == 0.5) %>%
    summarise(sum_lifetime_stress = sum(stress),
              ind_maximum_age = max(Age),
              sum_lifetime_fecundity = max(totalSeeds),
              sum_realized_shootMass = sum(mshoot),
              sum_realized_rootMass = sum(mroot))

tdata <- tdata %>%
    ungroup() %>%
    select(-SimNr, -ComNr, -RunNr, -PFT, -plantID)
# -sum_lifetime_stress,
# -ind_maximum_age,
# -sum_realized_shootMass,
# -sum_realized_rootMass)

tdata <- tdata %>%
    sample_n(1000)

tdata <- tdata %>%
    summarise_each(funs(scale(log( . + 1))))

# Visualize
t <- melt(tdata)
p <- ggplot(t)
p <- p + geom_density(aes(value))
p <- p + facet_wrap(~ variable, scale="free")
p

# Shapiro
mult.norm <- t(tdata)
mshapiro.test(mult.norm)

tdata.pca <- rda(tdata, scaling=TRUE)

screeplot(tdata.pca)

biplot(tdata.pca, display="species")





#NeighborhoodStatistics -------------
FindNeighborhoodAverage <- function(SimNr_, ComNr_, RunNr_, ITVsd_, plantID_, year_)
{

    t <- spat_data %>% filter(SimNr == SimNr_,
                              ComNr == ComNr_,
                              RunNr == RunNr_,
                              ITVsd == ITVsd_)

    ## Average over the plant's lifespan
    # years <- (t %>% filter(plantID == plantID_) %>% select(year))[[1]]
    #
    # t <- t %>% filter(year %in% years)

    ## Only the year in question
    t <- t %>% filter(year == year_)

    XY <- t %>% filter(plantID == plantID_) %>% select(xcoord, ycoord) %>% distinct()
    X <- XY[[1]]
    Y <- XY[[2]]

    t <- t %>% filter(abs(xcoord - X) < 5,
                      abs(ycoord - Y) < 5,
                      xcoord != X & ycoord != Y)

    # print(t)
    if (nrow(t) == 0) {
        neighbors = 0
    } else {
        neighbors = mean(t$accumulatedSeeds * t$SeedMass)
    }

    return(neighbors)
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

tdata <- spat_data %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, PFT, plantID) %>%
    mutate(ind_lifetime_stress = sum(stress),
           ind_lifetime_fecundity = max(totalSeeds) * SeedMass,
           ind_avg_fecundity = mean(accumulatedSeeds) * SeedMass) %>%
    ungroup()

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd) %>%
    sample_n(2)

tdata <- tdata %>%
    group_by(SimNr, ComNr, RunNr, ITVsd, plantID) %>%
    mutate(neighbors_avg_fecundity = FindNeighborhoodAverage(SimNr, ComNr, RunNr,
                                                             ITVsd, plantID, year))

tdata <- inner_join(tdata, originally_rare, copy = T)

tdata <- tdata %>% select(ComNr, RunNr, year, PFT, plantID, ind_lifetime_stress,
                          ind_lifetime_fecundity, ind_avg_fecundity, neighbors_avg_fecundity,
                          originally_rare)


## Point plot
p <- ggplot(tdata)

p <- p + geom_point(aes(x = ind_avg_fecundity,
                        y = neighbors_avg_fecundity,
                        color = originally_rare))

p <- p + facet_grid(~ ITVsd)

p


## Histogram plot
t <- tdata %>%
    group_by(ITVsd, originally_rare) %>%
    summarise(r = (ind_avg_fecundity+1) / (neighbors_avg_fecundity+1))

t

p <- ggplot(t)

p <- p + geom_histogram(aes(x = log(r)))

p <- p + facet_grid(originally_rare ~ ITVsd)

p



