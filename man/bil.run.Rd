\name{bil.run}
\alias{bil.run}
\title{Simple model run and access to model state}
\usage{
bil.run(model, file = NULL, var_names = NULL, data = NULL,
  init_GS = NULL, get_state = NULL, from_state = NULL,
  update_pet = FALSE, ...)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{file}{file containing input variables}

  \item{var_names}{a vector of names of input variables}

  \item{data}{a data frame of time-series of input
  variables (as in \code{\link{bil.set.values}})}

  \item{init_GS}{initial groundwater storage; if not
  defined, it is tried to be taken from optimization
  settings}

  \item{get_state}{a date for which state variables will be
  returned; must be part of the data period}

  \item{from_state}{a list of state variables (result of
  run with \code{get_state}): \itemize{ \item \code{date}
  is date of state, the run will start at time following
  this date (hence cannot equal to the date of the last
  time step) \item \code{season} season mode \item
  \code{SS, SW, GS, DS} represent snow, soil, groundwater
  and direct runoff storage (DS used in daily type only)
  \item \code{params} a named vector of parameter (names
  must match with model parameters); if not defined,
  initial parameter values will be used }}

  \item{update_pet}{whether to (re)calculate potential
  evapotranspiration (potentially based on new time-series
  given by \code{data})}

  \item{\dots}{further arguments for PET calculation passed
  to \code{\link{bil.pet}}}
}
\value{
A data frame of time-series of input and output variables
including dates. For \code{from_state} specified, the
returned time-series will start with time step following
the date of state.

In case of \code{get_state}, a list containing date of the
state, season mode, state reservoir volumes and current
parameter values.
}
\description{
Runs model, possibly with data given in an input file (has
priority) or in a data frame. A model state for given date
can be also obtained or the model can be run starting from
specified state.
}
\details{
If both \code{get_state} and \code{from_state} specified,
\code{get_state} will be applied.
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.run(b, update_pet = TRUE)
st = bil.run(b, get_state = "1990-12-01")
st$date = as.Date("1991-07-01")
bil.run(b, from_state = st)
}
\seealso{
\code{\link{bil.set.values}}, \code{\link{bil.read.file}},
\code{\link{bil.get.values}}
}

