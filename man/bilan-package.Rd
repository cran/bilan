\name{bilan-package}
\alias{bilan-package}
\alias{bilan}
\docType{package}
\title{
Bilan water balance model
}
\description{The input data into the model are daily or monthly series of basin 
    average precipitation and air temperature. The structure of the model 
    is formed by a system of relationships describing basic principles of 
    water balance on ground, in the zone of aeration, including the effect 
    of vegetation cover, and in the saturated zone. Air temperature is used 
    as an indicator of energy conditions, which affect significantly the water 
    balance components, in particular the development and melting of snow cover. 
    The potential evapotranspiration is estimated either by a simple radiation
    -based method, which considers temperature, day of the year and latitude
    of the basin or by empirical relationships considering vegetation zone 
    and relative humidity.
}
\details{
\tabular{ll}{
Package: \tab bilan\cr
Type: \tab Package\cr
Version: \tab 2013.12\cr
Date: \tab 2013.12\cr
License: \tab GPL (>= 2)\cr
LazyLoad: \tab yes\cr
}
}
\author{
T. G. Masaryk Water Research Institute, p.r.i.:
Ladislav Kasparek, Martin Hanel, Stanislav Horacek, Petr Maca, Adam Vizina

Maintainer: Stanislav Horacek <stanislav.horacek@gmail.com>
}
\references{
Lena M. Tallaksen and Henny A. J. van Lanen (eds): Hydrological Drought – Processes and
Estimation Methods for Streamflow and Groundwater. Amsterdam, 2004.
}
\keyword{ package }
\examples{
\dontrun{
bil = bil.new("m")
input = data.frame(P = c(1, 0, 5), R = c(0.5, 0.7, 0.9), T = c(5, 4, 9), H = c(70, 80, 75))
bil.set.values(bil, input, "2011-01-01")
input2 = data.frame(P = c(4, 0, 5))
bil.set.values(bil, input2, "2011-01-01", append = TRUE)
x = bil.get.values(bil)

bil.read.file(bil, "1550m.dat")
bil.pet(bil, "tab")
bil.set.params.init(bil, list(Spa = 10, Grd = 0.05))
bil.set.params.lower(bil, list(Spa = 5))
bil.set.params.upper(bil, list(Spa = 15))
bil.set.optim(bil, method = "BS", crit = "MSE", weight_BF = 0, max_iter = 500, init_GS = 50)
bil.optimize(bil)
res = bil.get.values(bil)
bil.write.file(bil, "1550m.txt")

bil2 = bil.new("m", "0700m.dat")
bil.pet(bil2, "tab")
bil.read.params.file(bil2, "1550m.txt")
resul = bil.run(bil2)

# DE optim
bilDE = bil.new("m")
bil.read.file(bilDE, "0700m.dat")
bil.pet(bilDE, "tab")
bil.set.optim(bilDE, method = "DE", crit = "MAPE", DE_type = "best_one_bin", n_comp = 4,
   comp_size = 10, cross = 0.95, mutat_f = 0.95, mutat_k = 0.85, maxn_shuffles = 5,
   n_gen_comp = 10, ens_count = 5, seed = 446, weight_BF = 0, init_GS = 50)
bil.optimize(bilDE)
DEresul = bil.get.values(bilDE)
DEens = bil.get.ens.resul(bilDE)

# optimization of a system of catchments
marsov = bil.new("m", "../input/0170m.dat")
hronov = bil.new("m", "../input/0180m.dat")
bil.pet(marsov)
bil.pet(hronov)
bil.set.area(hronov, 247.75)
s = sbil.new(c(marsov, hronov))
sbil.optimize(s)
opt_hronov = sbil.get.catch(s, 2)
resul = bil.get.values(opt_hronov)
}
}
