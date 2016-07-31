# General patterns between Grazing probability and belowground resources
# remain consistent with previous study.

data <- grd_data

tdata <- data %>%
    group_by(ComNr, IC_vers, ITVsd, BRes, GrazProb) %>%
    # filter(99 %in% Year) %>%
    summarise(shannon = mean(shannon))

p <- ggplot(tdata)

p <- p + geom_point(aes(x = GrazProb,
                        y = shannon))

p <- p + facet_grid(BRes ~ ITVsd)

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

p





data <- grd_data

# tdata <- data %>%
#     group_by(ComNr, RunNr, IC_vers, ITVsd, ARes, GrazProb) %>%
#     filter(99 %in% Year)

tdata <- data %>%
    group_by(ComNr, IC_vers, ITVsd, BRes, GrazProb, Year) %>%
    summarise(shannon = mean(shannon))

p <- ggplot(tdata)

p <- p + geom_point(aes(x = Year,
                        y = shannon,
                        color = factor(GrazProb)))

p <- p + facet_grid(ITVsd ~ BRes)

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
