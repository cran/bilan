\name{bil.get.values}
\alias{bil.get.data}
\alias{bil.get.dtm}
\alias{bil.get.values}
\title{Access to model data}
\usage{
bil.get.dtm(model)

bil.get.data(model, get_chars = FALSE)

bil.get.values(model, get_chars = FALSE)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{get_chars}{whether to get time-series of variables
  or monthly characteristics (min, mean, max)}
}
\value{
List of model results, for time-series:
\item{params}{parameters in a data frame: parameter names,
current value, lower and upper limits and initial values}
\item{crit}{resulting criterion value (after the second
part of optimization)} \item{vars}{time series of variables
in a data frame, the first column DTM means dates} For
characteristics: \item{P.min, P.mean, P.max,
RM.min...}{values of characteristics for all variables}
\item{params}{parameter names, values and limits}
\item{crit}{resulting criterion value}
}
\description{
Gets model data values: parameters and variables including
dates. For ensemble, gets data of last model in ensemble.
}
\details{
The most complex output is provided by
\code{bil.get.values}, the other functions get a subset of
it: \code{vars} (or characteristics) in case of
\code{bil.get.data} and \code{vars$DTM} in case of
\code{bil.get.dtm}.

An error occurs when requesting monthly characteristics for
daily time-series shorter than one complete month.
Additionally, monthly characteristics are set to 0 in case
of time-series shorter than one year.
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.pet(b, "latit")
resul = bil.get.values(b)
}
\seealso{
\code{\link{bil.get.params}}
}

