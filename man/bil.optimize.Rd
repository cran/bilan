\name{bil.optimize}
\alias{bil.optimize}
\title{Optimization procedure}
\usage{
bil.optimize(model)
}
\arguments{
  \item{model}{pointer to model instance}
}
\value{
Data frame of time-series of input and output variables
including dates.
}
\description{
Optimizes model parameters according to observed data by
using selected algorithm (a two-step gradient procedure or
combined shuffled complex evolution and differential
evolution).
}
\details{
For simple model run, use \code{\link{bil.run}} function or
choose the gradient method and set number of iterations to
zero (by \code{\link{bil.set.optim}}). An error occurs if
data needed for model run are missing.
}
\examples{
b = bil.new("m")
input = data.frame(
  P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
bil.set.values(b, input, init_date = "1990-11-01")
bil.pet(b, "latit")
bil.set.optim(b, "DE")
bil.optimize(b)
}

