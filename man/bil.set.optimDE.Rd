\name{bil.set.optimDE}
\alias{bil.set.optimDE}
\alias{sbil.set.optimDE}
\title{Differential evolution optimization settings}
\usage{
bil.set.optimDE(object, DE_type = "best_one_bin", n_comp = 4,
  comp_size = 10, cross = 0.95, mutat_f = 0.95, mutat_k = 0.85,
  maxn_shuffles = 5, n_gen_comp = 10, ens_count = 5, seed = 0, ...)

sbil.set.optimDE(object, ...)
}
\arguments{
  \item{object}{pointer to model instance or to system of
  catchments instance}

  \item{DE_type}{name of differential evolution version:
  \code{"best_one_bin"}, \code{"best_two_bin"},
  \code{"rand_two_bin"} (means mutation of best/random
  parameter set and number of differences between
  parameters used for the mutation)}

  \item{n_comp}{number of complexes}

  \item{comp_size}{complex size (population size in one
  complex)}

  \item{cross}{crossover parameter}

  \item{mutat_f}{mutation parameter for all DE types}

  \item{mutat_k}{mutation parameter for two differences
  used for mutation}

  \item{maxn_shuffles}{number of shuffling}

  \item{n_gen_comp}{number of generations in one complex}

  \item{ens_count}{number of runs in ensemble}

  \item{seed}{seed to initialize random number generator
  (<= 0 for initialization based on time)}

  \item{\dots}{common optimization arguments described at
  \code{\link{bil.set.optim}}}
}
\description{
Sets optimization method to combined shuffled complex
evolution (SCE-UA) and differential evolution and sets its
parameters, either for the model or for the system of
catchments.
}
\examples{
b = bil.new("m")
input = data.frame(
  P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
bil.set.values(b, input, init_date = "1990-11-01")
bil.pet(b, "latit")
bil.set.optimDE(b, crit = "NS", comp_size = 20, cross = 0.8, mutat_f = 0.8)
bil.optimize(b)
s = sbil.new(b)
sbil.set.optimDE(s, crit = "NS", comp_size = 20, cross = 0.8, mutat_f = 0.8)
}
\references{
Rainer Storn and Kenneth Price. Differential evolution – a
simple and efficient heuristic for global optimization over
continuous spaces. J. of Global Optimization,
11(4):341–359, December 1997.

Qingyun Duan, Soroosh Sorooshian, and Vijai K. Gupta.
Optimal use of the SCE-UA global optimization method for
calibrating watershed models. Journal of Hydrology,
158(3-4):265–284, 1994.
}
\seealso{
\code{\link{bil.set.optim}}
}

