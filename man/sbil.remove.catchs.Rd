\name{sbil.remove.catchs}
\alias{sbil.remove.catchs}
\title{Catchments removal from the system}
\usage{
sbil.remove.catchs(system, catch_ns)
}
\arguments{
  \item{system}{pointer to system of catchments instance}

  \item{catch_ns}{serial numbers of catchments to be
  removed, counted from 1}
}
\description{
Removes catchments from the system.
}
\details{
An error occurs if the required serial number is greater
than number of catchments in the system; all catchments
from the list in front of this number are removed.
}
\examples{
b = bil.new("m")
b2 = bil.new("m")
s = sbil.new()
sbil.add.catchs(s, c(b, b2))
sbil.remove.catchs(s, 2)
}

