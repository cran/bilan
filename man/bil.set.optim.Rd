\name{bil.set.optim}
\alias{bil.set.optim}
\alias{sbil.set.optim}
\alias{set.optim.gen}
\title{Optimization setting}
\usage{
set.optim.gen(object, crit = NULL, crit_part1 = NULL, crit_part2 = NULL,
  weight_BF = 0, init_GS = 50, use_weights = FALSE, weights = NULL, ...)

bil.set.optim(object, method = "BS", from = NULL, ...)

sbil.set.optim(object, ...)
}
\arguments{
  \item{object}{pointer to model instance or to system of
  catchments instance}

  \item{method}{optimization method: gradient-based binary
  search (\code{"BS"}) or combined shuffled complex
  evolution (SCE-UA) and differential evolution
  (\code{"DE"})}

  \item{from}{for model only, pointer to another model
  instance; if specified, optimization settings is copied
  from that model and all other arguments are ignored}

  \item{\dots}{other common or specific arguments. The
  specific arguments are described at
  \code{\link{bil.set.optimBS}} or
  \code{\link{bil.set.optimDE}}, the common (passed to
  \code{set.optim.gen}) below.}

  \item{crit}{For BS method, sets criteria for both parts
  of optimization; if defined, \code{crit_part1} and
  \code{crit_part2} are ignored. For MSE and MAE, MAPE is
  used in the second part, for others, the criterion is the
  same for both parts.

  For DE method, name of optimization criterion (default
  MSE).

  Supported criteria: \code{"MSE"} (mean squared error),
  \code{"MAE"} (mean absolute error), \code{"NS"}
  (Nash-Sutcliffe efficiency), \code{"LNNS"}
  (Nash-Sutcliffe of log-transformed data), \code{"MAPE"}
  (mean absolute percentage error)}

  \item{crit_part1}{for BS method, name of criterion for
  the first part of optimization: MSE (mean squared error),
  MAE (mean absolute error), NS (Nash-Sutcliffe
  efficiency), LNNS (Nash-Sutcliffe of log-transformed
  data), MAPE (mean absolute percentage error); default MSE
  used if not defined}

  \item{crit_part2}{for BS method, name of criterion for
  the second part of optimization; default MAPE used if not
  defined}

  \item{weight_BF}{weight for criterion for observed and
  modelled baseflow (between 0 and 1, default 0), weight
  for runoff is (1 - weight_BF)}

  \item{init_GS}{initial groundwater storage in mm (initial
  condition, default 50)}

  \item{use_weights}{whether to use weights for criterion
  calculation in optimization}

  \item{weights}{a vector of weights for criterion
  calculation, length of weights must correspond with
  number of time steps of variable series.  If not given,
  the variable \code{WEI} is used (by default, its values
  are equal). The weights are considered as relative, e.g.
  \code{c(2,2,3)} has the same effect as \code{c(4,4,6)}.
  Parts of time series can be excluded from optimization by
  setting weights to zero.}
}
\description{
Sets optimization method and its parameters, either for the
model or for the system of catchments.
}
\examples{
b = bil.new("m")
input = data.frame(
  P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
bil.set.values(b, input, init_date = "1990-11-01")
bil.pet(b, "latit")
bil.set.optim(b, "DE", crit = "NS")
bil.optimize(b)
b2 = bil.new("d")
bil.set.optim(b2, from = b)
}
\seealso{
\code{\link{bil.optimize}}, \code{\link{bil.set.optimBS}},
\code{\link{bil.set.optimDE}}
}

