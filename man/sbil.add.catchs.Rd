\name{sbil.add.catchs}
\alias{sbil.add.catchs}
\title{Catchments addition to the system}
\usage{
sbil.add.catchs(system, catchs)
}
\arguments{
  \item{system}{pointer to system of catchments instance}

  \item{catchs}{a list of instances of class bilan
  representing catchments to be added, items of different
  classes will not be used}
}
\description{
Adds catchments to the system.
}
\details{
Instances of bilan are copied to the system, i.e. there is
no link between catchment in the system and in the original
model.

Catchments can be added also directly as a parameter of
\code{\link{sbil.new}} function.
}
\examples{
b = bil.new("m")
b2 = bil.new("m")
s = sbil.new()
sbil.add.catchs(s, c(b, b2))
}
\seealso{
\code{\link{sbil.new}}
}

