suppressMessages(library(dplyr))
suppressMessages(library(sde))

WEEKS = 30

args <- commandArgs()
nRuns <- as.integer(args[7])
years <- as.integer(args[8])
sigma_ <- as.double(args[9])

generateVariation <- function()
{
    d_ = data.frame(b = 50 +
                        rep(sapply(seq(0,
                                       pi,
                                       length.out = WEEKS),
                                   sin) * 20,
                            years)
    )

    gbm <- c(replicate(years, GBM(N = WEEKS, sigma = sigma_)[2:31]))

    d_ = d_ %>%
        mutate(t = 1:n(),
               gbm = round(b * gbm, 5))

    d_ = d_ %>%
        mutate(gbm = ifelse(gbm > 100, 100, gbm)) %>%
        mutate(gbm = ifelse(gbm < 10, 10, gbm))

    return(d_$gbm)
}

d <- t(replicate(nRuns, generateVariation()))

setwd("./tmp/")

write.table(x = d, file = "Environment.csv", sep = ", ", col.names = F, row.names = F)

####################################################################################################
### Validation
####################################################################################################
# library(ggplot2)
# library(ggthemes)
# library(cowplot)
#
# p <- ggplot(d %>% filter(t < WEEKS * 10)) +
#     geom_line(aes(x = t,
#                   y = b,
#                   color = "Cyclic")) +
#     geom_line(aes(x = t,
#                   y = gbm,
#                   color = "Red noise")) +
#     labs(x = "Time (weeks)",
#          y = "Resources",
#          color = "Variation type") +
#     ylim(c(10, 100)) +
#     theme_few() +
#     theme(aspect.ratio = 0.5)
#
# p
#
# ggsave(plot = p, filename = "Resource_variation.PDF")

