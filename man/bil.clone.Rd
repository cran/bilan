\name{bil.clone}
\alias{bil.clone}
\title{Cloning an existing model}
\usage{
bil.clone(model)
}
\arguments{
  \item{model}{pointer to model instance to be cloned}
}
\value{
Result of cloning: object of class \dQuote{bilan} which is
a pointer to Bilan model instance.
}
\description{
Creates a new instance of Bilan model that is identical to
a given existing instance.
}
\examples{
b = bil.new("m")
b2 = bil.clone(b)
}

