\name{sbil.new}
\alias{sbil.new}
\title{System of catchments instance creation}
\usage{
sbil.new(catchs = NULL)
}
\arguments{
  \item{catchs}{a vector of Bilan instances created by
  \code{\link{bil.new}}}
}
\value{
Object of class \dQuote{bil_system} which is a pointer to
system of catchments instance in C++ that cannot be
directly accessed.
}
\description{
Creates a new instance of system of catchments, optionally
adds Bilan instances to it.
}
\details{
If \code{catchs} is specified, given catchments are added
by using \code{\link{sbil.add.catchs}}.
}
\examples{
b = bil.new("m")
s = sbil.new()
s2 = sbil.new(b)
}
\seealso{
\code{\link{sbil.add.catchs}}
}

