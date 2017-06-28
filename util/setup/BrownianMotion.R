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
# library(dplyr)
# library(sde)
# library(reshape2)
#
# WEEKS = 30
#
# YEARS <- 100
# SIGMA <- seq(from = 0.05, to = 0.20, by = 0.05)
#
# d = data.frame(cyclic = 50 +
#                    rep(sapply(seq(0,
#                                   pi,
#                                   length.out = WEEKS),
#                               sin) * 20,
#                        YEARS)
# )
#
# brownian.processes <- sapply(X = SIGMA,
#                              function(x) c(replicate(YEARS, GBM(N = WEEKS, sigma = x)[2:31])))
#
# colnames(brownian.processes) <- SIGMA
#
# d <- cbind(d, brownian.processes)
# d[(2:ncol(d))] <- d[[1]] * d[2:ncol(d)]
#
# lapply(d[2:ncol(d)],
#        function(x) ifelse(x > 100,
#                           100,
#                           ifelse(x < 10,
#                                  10,
#                                  x)))
#
# d = d %>% mutate(t = 1:n())
#
# d <- melt(d, id.vars = c("t", "cyclic"))
#
# p <- ggplot(d %>% filter(t < WEEKS * 10)) +
#     geom_line(aes(x = t / WEEKS,
#                   y = cyclic,
#                   group = "Cyclic",
#                   linetype = "Cyclic")) +
#     geom_line(aes(x = t / WEEKS,
#                   y = value,
#                   color = variable)) +
#     facet_wrap(~ variable, ncol = 2) +
#     labs(x = "Time (years)",
#          y = "Resources",
#          color = "Sigma",
#          linetype = "") +
#     ylim(c(10, 100)) +
#     theme_few() +
#     theme(aspect.ratio = 0.5)
#
# p
#
# ggsave(plot = p, filename = "Resource_variation.PDF")
