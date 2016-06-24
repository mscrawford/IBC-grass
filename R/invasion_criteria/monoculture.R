# Michael Crawford
# Wednesday, November 26, 2014

biomass_facet_wrap <- function(data)
{
    tdata <- tbl_df(data)

    tdata <- tdata %>%
        group_by(PFT, Time) %>%
        summarise_each(funs(mean, sd), Inds, cover, shootmass, rootmass)

    p <- ggplot()
    
    p <- p + geom_line(data=tdata, 
                       aes(x=Time, 
                           y=shootmass_mean, 
                           color="shoot mass")) 
    p <- p + geom_errorbar(data=tdata, 
                           aes(x=Time, 
                               ymin=shootmass_mean-shootmass_sd, 
                               ymax=shootmass_mean+shootmass_sd, 
                               width=0.25))
    
    p <- p + geom_line(data=tdata, 
                aes(x=Time, 
                    y=rootmass_mean, 
                    color="root mass")) 

    p <- p + geom_errorbar(data=tdata, 
                aes(x=Time, 
                    ymin=rootmass_mean-rootmass_sd, 
                    ymax=rootmass_mean+rootmass_sd, 
                    width=0.25))

    p <- p + facet_wrap(~PFT)

    p <- p + ggtitle("Monocultures") + ylab("Biomass") + scale_color_discrete(name = "Biomass")
    
    p
}