\name{bil.new}
\alias{bil.new}
\title{Bilan model instance creation}
\usage{
bil.new(type, file = NULL, use_data = TRUE, data = NULL, ...)
}
\arguments{
  \item{type}{type of model: daily (\dQuote{d} or
  \dQuote{D}) or monthly (\dQuote{m} or \dQuote{M})}

  \item{file}{name of input or output file to be used for
  loading data}

  \item{use_data}{output files only, whether to use output
  time-series of variables}

  \item{data}{a data frame of model variables (described at
  \code{\link{bil.set.values}})}

  \item{\dots}{further arguments passed to
  \code{\link{bil.read.file}} (names of variables) or
  \code{\link{bil.set.values}} (initial date)}
}
\value{
Object of class \dQuote{bilan} which is a pointer to Bilan
model instance in C++ that cannot be directly accessed.
}
\description{
Creates a new instance of Bilan model, optionally loads
data to it.
}
\details{
If \code{file} is specified, loads data from Bilan input or
output files. Input data and initial date are loaded from
input file (by \code{\link{bil.read.file}}), whereas
parameter set (by \code{\link{bil.read.params.file}}) or
optionally (controlled by \code{use_data}) all variables
and initial date are loaded from output file.

If both \code{file} and \code{data} are specified,
\code{data} are used and \code{file} is ignored.

Old-style formatted input and output files are also
supported.
}
\examples{
b = bil.new("m")
}
\seealso{
\code{\link{bil.read.file}},
\code{\link{bil.read.params.file}},
\code{\link{bil.set.values}}
}

