\name{bil.set.values}
\alias{bil.set.values}
\title{Input variable settings}
\usage{
bil.set.values(model, input_vars, init_date = NULL, append = FALSE)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{input_vars}{data frame of time-series of input
  variables, their names must match with names in Bilan
  (columns with unknown names are omitted).  The model is
  automatically extended to a version with water use when
  one of the names \code{POD}, \code{POV}, \code{PVN} or
  \code{VYP} is specified.}

  \item{init_date}{initial date of time serie, should be of
  Date or character (formatted as \dQuote{YYYY-MM-DD})
  class.  If not defined, the first value of data column
  \code{DTM} or the first row name of \code{input_vars} is
  tried to be used.  Initial date is not needed for
  appending (\code{append = TRUE}).}

  \item{append}{if false, all old time-series are deleted
  and replaced; otherwise only given variables are replaced
  – in that case number of time steps needs to be equal to
  time steps of data to be replaced}
}
\description{
Sets time-series of (input) variables and initial date.
}
\details{
An error occurs if numbers of time steps do not match (only
for appending) or if initial date is not given or found (in
varible DTM or row names).
}
\examples{
bil = bil.new("m")
input = data.frame(P = c(1, 0, 5), R = c(0.5, 0.7, 0.9), T = c(5, 4, 9), H = c(70, 80, 75))
bil.set.values(bil, input, "2011-01-01")
input2 = data.frame(P = c(4, 0, 5), POD = c(0.5, 0.5, 0.5))
bil.set.values(bil, input2, "2011-01-01", append = TRUE)
x = bil.get.values(bil)
print(x)
}

