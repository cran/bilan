\name{bil.read.params.file}
\alias{bil.read.params.file}
\title{Load of parameters from file}
\usage{
bil.read.params.file(model, file_name)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{file_name}{output file with saved parameters}
}
\description{
Loads (optimized) model parameters from output file to
initial and current values.
}
\details{
Old-style formatted output files are also supported.
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.pet(b, "latit")
}

