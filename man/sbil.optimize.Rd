\name{sbil.optimize}
\alias{sbil.optimize}
\title{Optimization procedure for the catchment system}
\usage{
sbil.optimize(system)
}
\arguments{
  \item{system}{pointer to system of catchments instance}
}
\description{
Optimizes parameters collected from models for all
catchments according to observed data by using selected
optimization algorithm.
}
\details{
For simple model run, use \code{\link{sbil.run}} function
or choose the gradient method and set number of iterations
to zero (by \code{\link{bil.set.optim}}). An error occurs
if data needed for model run are missing.

If a catchment in the system has different time step or
data period than the first one or if its area has not been
set, it will be rejected from the catchments used for
optimization. Optimization will not be performed when
number of available catchments is zero.

Criterion value is calculated by using a penalty function
that increases the criterion value when difference in total
runoff between the second and the first catchment is
negative.
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.pet(b)
bil.set.area(b, 131.88)
s = sbil.new(b)
sbil.optimize(s)
}
\seealso{
sbil.run
}

