\name{bil.get.params}
\alias{bil.get.params}
\title{Access to model parameters}
\usage{
bil.get.params(model)
}
\arguments{
  \item{model}{pointer to model instance}
}
\value{
Parameters in a data frame: parameter names, current value,
lower and upper limits and initial values.
}
\description{
Gets current and initial values of parameters and their
limits.
}
\details{
This function is also part of \code{bil.get.values}.
}
\examples{
b = bil.new("m")
bil.set.params(b, list(Spa = 70, Grd = 0.2), "init")
bil.get.params(b)
}
\seealso{
\code{\link{bil.get.values}}
}

