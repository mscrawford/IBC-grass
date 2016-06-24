# Michael Crawford
# Wednesday, November 26, 2014

library(dplyr)
library(stringr)
library(ggplot2)

pick_communities <- function(data)
{
    data <- data %>%
        filter(invasion_ver == 0)
    return(data)
}

pick_pairs <- function(data)
{
    data <- data %>%
        filter(invasion_ver == 1)
    return(data)
}

# params:
## data:
## - Survival data (the "Srv" files). This will break on "Pft" data.
## - Be sure to compose the required dataset with the "read_data(dir, file_type)" function.
pairwise_heatmap <- function(srv_data)
{
    tdata <- pick_pairs(srv_data)

    tdata <- tdata %>%
        group_by(SimNr, RunNr, PFT, ITVsd) %>%
        filter(PFT == invader, ComNr == 5) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    tdata <- tdata %>%
        group_by(SimNr, ITVsd, invader, monoculture) %>%
        summarise(invasion_success = mean(lives))

    p <- ggplot(tdata) +
            geom_tile(
                aes(
                    x=factor(monoculture),
                    y=factor(invader),
                    fill=invasion_success*100)) +
            labs(title="Pairwise invasion heatmap of a random, 16 PFT, community. 20 trials per pair.",
                 x="PFT monoculture being invaded",
                 y="Invading PFT") +
            scale_fill_continuous(name="% successful\ninvasion")

    p <- p + facet_grid(. ~ ITVsd)

    p
}

community_invasibility <- function(srv_data)
{
    community_filter <- function(x) {
        com = cdata %>% filter(ComNr == x$ComNr, ITVsd == x$ITVsd)
        if (x$monoculture %in% com$PFT) {
            return(TRUE)
        }
        return (FALSE)
    }

    pdata <- srv_data %>%
        group_by(SimNr, ComNr, RunNr, ITVsd, PFT, invader) %>%
        filter(invasion_ver == 1, PFT == invader) %>%
        mutate(lives = ifelse(cPop > 0, 1, 0))

    pdata <- pdata %>%
        group_by(SimNr, ComNr, ITVsd, PFT, invader, monoculture) %>%
        summarise(invasion_success = mean(lives))

    cdata <- srv_data %>%
        group_by(SimNr, ComNr, RunNr, ITVsd, PFT) %>%
        filter(invasion_ver == 0) %>%
        mutate(survives = ifelse(cPop > 0, 1, 0))

    cdata <- cdata %>%
        group_by(SimNr, ComNr, ITVsd, PFT) %>%
        summarise(survives = mean(survives)) %>%
        filter(survives > 0.8) # This should eventually change...

    tdata <- inner_join(cdata, pdata)

    tmp <- NULL
    for (i in 1:nrow(tdata)) {
        if (community_filter(tdata[i,])) {
            tmp <- rbind(tmp, tdata[i,])
        }
    }

    cdata <- tmp %>%
        group_by(ITVsd) %>%
        summarise_each(funs(mean, sd), invasion_success)

    pdata <- pdata %>%
        group_by(ITVsd) %>%
        summarise_each(funs(mean, sd), invasion_success)

    p <- ggplot()

    p <- p + geom_line(data = cdata,
                       aes(x = ITVsd,
                           y = mean,
                           color = "Existing communities")) +
        geom_errorbar(data = cdata,
                      aes(x = ITVsd,
                          ymin=mean-sd,
                          ymax=mean+sd),
                      width=.01)

    p <- p + geom_line(data = pdata,
                       aes(x = ITVsd,
                           y = mean,
                           color = "Global pairs")) +
        geom_errorbar(data = pdata,
                      aes(x = ITVsd,
                          ymin=mean-sd,
                          ymax=mean+sd),
                      width=.01)

#     p <- p + facet_grid(. ~ ComNr)

    p <- p + labs(
        title = "Pairwise coexistence rates",
        x = "ITVsd",
        y = "Probability of invasion survival",
        colour = "Superset of pairs")

    p
}
