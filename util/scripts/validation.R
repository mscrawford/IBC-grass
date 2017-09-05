
###################################
### Re-validation

# data_dir = "/Users/Theodore/Documents/workspace/IBC-grass/data/out"
data_dir = "/Users/Theodore/Desktop/data.1/data/out"
# data_dir = "/Users/Theodore/Desktop/Validation/Remove_plant_iter_definition/data/out" # Pre-fix

param <- read_data(data_dir, "param")

pft <- read_data(data_dir, "PFT")

d <- combine_data(list(param, pft), c("SimID"))

d <- d %>%
    group_by(ComNr,
             BRes,
             GrazProb,
             SeedInput) %>%
    filter(Year == 100) %>%
    mutate(pi = Pop / sum(Pop),
           pi_ln_pi = pi * log(pi)) %>%
    filter(!is.na(pi_ln_pi))

d <- d %>%
    group_by(ComNr,
             BRes,
             GrazProb,
             SeedInput) %>%
    summarise(Shannon = -1 * sum(pi_ln_pi),
              Richness = n())

A <- ggplot(d,
            aes(x = GrazProb,
                y = Shannon,
                # y = Richness,
                group = GrazProb)) +
    geom_boxplot() +
    facet_grid(BRes ~ ., labeller = label_both, scales="free") +
    # ylim(c(0, 40)) +
    ylim(c(0.5, 3.5)) +
    theme_few() +
    theme(aspect.ratio = 1)

A

save_plot(A,
          filename = "Shannon.pdf",
          nrow = 3,
          ncol = 1,
          base_aspect_ratio = 1.3,
          path = "~/Desktop")

