\name{bil.pet}
\alias{bil.pet}
\title{Potential evapotranspiration estimation}
\usage{
bil.pet(model, pet_type = "latit", latitude = 50)
}
\arguments{
  \item{model}{pointer to model instance}

  \item{pet_type}{the method to be used: based on
  vegetation zone (\dQuote{tab}) or based on latitude
  (\dQuote{latit})}

  \item{latitude}{latitude (required only by the second
  method)}
}
\description{
Estimates values of potential evapotranspiration, two
methods are available.
}
\details{
An error occurs if required input data are missing
(temperature or humidity).
}
\examples{
b = bil.new("m")
bil.set.values(b, init_date = "1990-11-01", input_vars =
  data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
  R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
  T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
bil.pet(b, "latit")
}
\references{
Gidrometeoizdat. Rekomendatsii po roschotu ispareniia s
poverhnosti suchi. Gidrometeoizdat, St. Peterburg, 1976.

Ludovic Oudin, Lætitia Moulin, Hocine Bendjoudi, and Pierre
Ribstein. Estimating potential evapotranspiration without
continuous daily data: possible errors and impact on water
balance simulations. Hydrological Sciences Journal,
55(2):209, 2010.
}

