\name{bil.read.file}
\alias{bil.read.file}
\title{Data input from a file}
\usage{
bil.read.file(model, file_name, input_vars = c("P", "R", "T", "H", "B"))
}
\arguments{
  \item{model}{pointer to model instance}

  \item{file_name}{input file name}

  \item{input_vars}{a vector of names of input variables
  (described at \code{\link{bil.set.values}})}
}
\description{
Loads initial date, catchment area and observed
meteorological and hydrological data from a file.
}
\details{
The file must contain the initial date of data at first
line (in yyyy mm dd format separated by white characters).
Optionally catchment area in square kilometers can be set
at this line; a number will be interpreted as catchment
area in case it is fourth at the line or it is last and
contains decimal point.

The first line is followed by columns with variables as
defined by input_vars. Number of variables cannot be
greater than number of columns in file. If number of
variables is less than number of columns, the last
exceeding columns are ignored.

Old-style formatted output files are also supported. These
files begin with three lines with number of rows, number of
columns and initial water year.

An error occurs if input variables include unknown name or
if number of columns is less than number of variables.
}
\examples{
\dontrun{
b = bil.new("m")
bil.read.file(b, "1550m.dat", c("P", "R", "T", "POD"))}
}
\seealso{
\code{\link{bil.write.file}} for writing to a file,
\code{\link{bil.set.values}} for information about variable
names
}

