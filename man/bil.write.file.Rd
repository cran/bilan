\name{bil.write.file}
\alias{bil.write.file}
\title{Data output to a file}
\usage{
bil.write.file(model, file_name, type = "series")
}
\arguments{
  \item{model}{pointer to model instance}

  \item{file_name}{output file name}

  \item{type}{type of output file: \dQuote{series} for
  default time-series, \dQuote{series_monthly} for monthly
  time-series, \dQuote{series_daily} for daily time-series
  (not available for monthly model type), \dQuote{chars}
  for monthly characteristics calculated from complete
  hydrological years}
}
\description{
Writes resulting parameters and time series of variables
into a file.
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.pet(b, "latit")
bil.run(b)
bil.write.file(b, "0446.txt")
}
\seealso{
\code{\link{bil.read.file}}
}

