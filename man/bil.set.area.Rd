\name{bil.set.area}
\alias{bil.set.area}
\title{Area setting}
\usage{
bil.set.area(model, area)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{area}{catchment area in square kilomters}
}
\description{
Sets a catchment area for the model.
}
\examples{
bil = bil.new("m")
bil.set.area(bil, 131.88)
}

