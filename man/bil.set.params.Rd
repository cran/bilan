\name{bil.set.params}
\alias{bil.set.params}
\alias{bil.set.params.curr}
\alias{bil.set.params.init}
\alias{bil.set.params.lower}
\alias{bil.set.params.upper}
\title{Parameter settings}
\usage{
bil.set.params.init(model, params)

bil.set.params.curr(model, params)

bil.set.params.lower(model, params)

bil.set.params.upper(model, params)

bil.set.params(model, params, type = "all")
}
\arguments{
  \item{model}{pointer to model instance}

  \item{params}{list or vector of parameter values with
  appropriate names, or a data frame with \dQuote{name}
  column (preferably a result from
  \code{\link{bil.get.params}}), or another model instance
  including parameter values to be set}

  \item{type}{type of settings: initial values for gradient
  optimization (\dQuote{init} or 0), current values --
  result of optimization or values used for simple run
  (\dQuote{curr} or 1), lower limits (\dQuote{lower} or 2),
  upper limits (\dQuote{upper} or 3) or all values
  (\dQuote{all} or 4)}
}
\description{
Sets values of parameters: initial, current, lower or upper
limit or all of them.
}
\details{
If no names are given and number of parameters is equal to
the number for current model type, names in default order
are assigned.

An error occurs if another model instance is used and
numbers of parameters in models do not match.
}
\examples{
b = bil.new("m")
bil.set.params(b, list(Spa = 70, Grd = 0.2), "init")
bil.set.params(b, list(Spa = 300), "upper")
pars = bil.get.params(b)
b2 = bil.new("m")
bil.set.params(b2, pars)
bil.set.params(b2, b)
}

