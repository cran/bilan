\name{sbil.run}
\alias{sbil.run}
\title{Simple run of model of catchment system}
\usage{
sbil.run(system, init_GS = NULL)
}
\arguments{
  \item{system}{pointer to system of catchments instance}

  \item{init_GS}{initial groundwater storage, same for all
  catchments; if not defined, it is tried to be taken from
  optimization settings}
}
\description{
Runs model of the catchment system, i.e. runs model for
each catchment in the system.
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.pet(b)
s = sbil.new(b)
sbil.run(s, 100)
}

