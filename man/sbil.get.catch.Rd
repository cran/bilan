\name{sbil.get.catch}
\alias{sbil.get.catch}
\title{Getting a catchment copy from the system}
\usage{
sbil.get.catch(system, catch_n)
}
\arguments{
  \item{system}{pointer to system of catchments instance}

  \item{catch_n}{serial number of a catchment, counted from
  1}
}
\value{
Catchment model as an object of class \dQuote{bilan} which
is a pointer to Bilan model instance.
}
\description{
Gets a copy of a specified catchment from all of the
catchments in the system.
}
\details{
An error occurs if the required serial number is greater
than number of catchments in the system.
}
\examples{
b = bil.new("m")
b2 = bil.new("m")
s = sbil.new(c(b, b2))
bil = sbil.get.catch(s, 2)
}

