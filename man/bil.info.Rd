\name{bil.info}
\alias{bil.info}
\title{Information about model}
\usage{
bil.info(model, print = TRUE)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{print}{whether to print the summary}
}
\value{
A list containing model type, number of time steps, begin
and end of time period and vector of model variables.
}
\description{
Gets information about model type, number of observations,
time period and used variables.
}
\examples{
b = bil.new("m")
input = data.frame(
  P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
bil.set.values(b, input, init_date = "1990-11-01")
info = bil.info(b)
}

