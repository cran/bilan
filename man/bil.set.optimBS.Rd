\name{bil.set.optimBS}
\alias{bil.set.optimBS}
\alias{sbil.set.optimBS}
\title{Binary search optimization settings}
\usage{
bil.set.optimBS(object, max_iter = 500, ...)

sbil.set.optimBS(object, ...)
}
\arguments{
  \item{object}{pointer to model instance or to system of
  catchments instance}

  \item{max_iter}{maximum number of iterations}

  \item{\dots}{common optimization arguments described at
  \code{\link{bil.set.optim}}}
}
\description{
Sets parameters of optimization method based on binary
search, either for the model or for the system of
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
bil.set.optimBS(b, crit = "NS", max_iter = 1e3)
bil.optimize(b)
s = sbil.new(b)
sbil.set.optimBS(s, crit = "NS", max_iter = 1e3)
}
\seealso{
\code{\link{bil.set.optim}}
}

