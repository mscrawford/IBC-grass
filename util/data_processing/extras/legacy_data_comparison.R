# Tuesday, December 9, 2014

library(dplyr)
library(ggplot2)

legacy_data_dir = "/Users/Theodore/Documents/Google_Drive/Research/IBC-grass/data/Legacy_community_data"

compare_to_legacy_data <- function(contemporary_data, legacy_data = 0, save = FALSE)
{
    # There is a default legacy dataset that contains one community with 16 different
    # PFTs. Printed are all of the usual things.
    # The PFT community is:

    if (legacy_data == 0) {
        legacy_data = read_legacy_data(legacy_data_dir, "Pft")
    }

    # Note that the legacy code has an error in it:
    # "Shootmass" -> cover
    # "Rootmass" -> shootmass
    # and no such thing as rootmass
    t_ld <- legacy_data %>%
        group_by(PFT, Time) %>%
        summarise_each(funs(mean, sd), shootmass)

    t_cd <- contemporary_data %>%
        group_by(SimNr, PFT, Year) %>%
        summarise_each(funs(mean, sd), cover)

    plot <- ggplot()

    plot <- plot +
            geom_line(data=t_ld,
                        aes(x=Time,
                            y=mean,
                            color="Legacy data"))

    plot <- plot +
            geom_line(data=t_cd,
                aes(x=Year,
                    y=mean,
                    color="Contemporary data"))

    plot <- plot +
            facet_wrap(~PFT) +
            ggtitle("Does ITV with an SD of 0 mimic the general version?") +
            labs(fill="Data set") +
            ylab("Number of Individuals") +
            theme(legend.title=element_blank())

    plot

    if (save) {
        ggsave(plot,
               filename="comparing_ITVwithSD_of0_to_legacy_data.pdf")
    }

}