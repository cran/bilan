\name{bil.get.ens.resul}
\alias{bil.get.ens.resul}
\title{Access to results of model ensemble}
\usage{
bil.get.ens.resul(model)
}
\arguments{
  \item{model}{pointer to model instance}
}
\value{
Data frame whose named columns represents model parameters,
criterion value and number of model evaluations, the
ensemble is represented by rows.
}
\description{
For differential evolution optimization, gets results of
model ensemble: parameters and criterion values.
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
bil.get.ens.resul(b)
}

