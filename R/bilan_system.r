check.system <- function (system) {
    if (class(system) != "bil_system" || typeof(system) != "externalptr")
        stop("System must be created by sbil.new() function.")
    else
        return(system)
}

#' System of catchments instance creation
#'
#' Creates a new instance of system of catchments, optionally adds Bilan instances to it.
#'
#' @param catchs a vector of Bilan instances created by \code{\link{bil.new}}
#' @return Object of class \dQuote{bil_system} which is a pointer to system of catchments instance in C++ that cannot be directly accessed.
#' @details If \code{catchs} is specified, given catchments are added by using \code{\link{sbil.add.catchs}}.
#' @seealso \code{\link{sbil.add.catchs}}
#' @export
#' @examples
#' b = bil.new("m")
#' s = sbil.new()
#' s2 = sbil.new(b)
sbil.new <- function (catchs = NULL) {
    sbil = .Call("new_sbil", PACKAGE = "bilan")
    class(sbil) = "bil_system"
    if (!is.null(catchs))
        sbil.add.catchs(sbil, catchs)
    return(sbil)
}

#' Catchments addition to the system
#'
#' Adds catchments to the system.
#'
#' @param system pointer to system of catchments instance
#' @param catchs a list of instances of class bilan representing catchments to be added, items of different classes will not be used
#' @details Instances of bilan are copied to the system, i.e. there is no link between catchment in the system and in the original model.
#' 
#'   Catchments can be added also directly as a parameter of \code{\link{sbil.new}} function.
#' @seealso \code{\link{sbil.new}}
#' @export
#' @examples
#' b = bil.new("m")
#' b2 = bil.new("m")
#' s = sbil.new()
#' sbil.add.catchs(s, c(b, b2))
sbil.add.catchs <- function (system, catchs) {
    catchs = c(catchs) # for single catchment could not be used sapply
    catchs[sapply(catchs, class) != "bilan"] = NULL
    err = .Call("add_catchs", check.system(system), catchs, PACKAGE = "bilan")
    if (err != "")
        stop(err)
}

#' Catchments removal from the system
#'
#' Removes catchments from the system.
#'
#' @param system pointer to system of catchments instance
#' @param catch_ns serial numbers of catchments to be removed, counted from 1
#' @details An error occurs if the required serial number is greater than number of catchments in the system; all catchments from the list in front of this number are removed.
#' @export
#' @examples
#' b = bil.new("m")
#' b2 = bil.new("m")
#' s = sbil.new()
#' sbil.add.catchs(s, c(b, b2))
#' sbil.remove.catchs(s, 2)
sbil.remove.catchs <- function (system, catch_ns) {
    catch_ns = c(as.integer(catch_ns) - 1) # to C index
    err = .Call("remove_catchs", check.system(system), catch_ns, PACKAGE = "bilan")
    if (err != "")
        stop(err)
}

#' Getting a catchment copy from the system
#'
#' Gets a copy of a specified catchment from all of the catchments in the system.
#'
#' @param system pointer to system of catchments instance
#' @param catch_n serial number of a catchment, counted from 1 
#' @return Catchment model as an object of class \dQuote{bilan} which is a pointer to Bilan model instance.
#' @details An error occurs if the required serial number is greater than number of catchments in the system.
#' @export
#' @examples
#' b = bil.new("m")
#' b2 = bil.new("m")
#' s = sbil.new(c(b, b2))
#' bil = sbil.get.catch(s, 2)
sbil.get.catch <- function (system, catch_n) {
    catch_n = as.integer(catch_n) - 1 # to C index
    err = .Call("get_catch", check.system(system), catch_n, PACKAGE = "bilan")
    if (typeof(err) == "externalptr") {
        class(err) = "bilan"
        return(err)
    }
    else
        stop(err)
}

#' Simple run of model of catchment system
#'
#' Runs model of the catchment system, i.e. runs model for each catchment in the system.
#'
#' @param system pointer to system of catchments instance
#' @param init_GS initial groundwater storage, same for all catchments; if not defined, it is tried to be taken from optimization settings
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.pet(b)
#' s = sbil.new(b)
#' sbil.run(s, 100)
sbil.run <- function (system, init_GS = NULL) {
    if (is.null(init_GS)) {
        if (is.null(sbil.get.optim(system)$init_GS))
            stop("Initial groundwater storage is not defined.")
        else
            init_GS = sbil.get.optim(system)$init_GS
    }
    err = .Call("sbil_run", check.system(system), init_GS, PACKAGE = "bilan")
    if (err != "")
        stop(err)
}

#' Optimization procedure for the catchment system
#'
#' Optimizes parameters collected from models for all catchments according to observed data by using selected optimization algorithm.
#'
#' @param system pointer to system of catchments instance
#' @details For simple model run, use \code{\link{sbil.run}} function or choose the gradient method and set number of iterations to zero
#'   (by \code{\link{bil.set.optim}}). An error occurs if data needed for model run are missing.
#'
#'   If a catchment in the system has different time step or data period than the first one or if its area has not been set,
#'   it will be rejected from the catchments used for optimization. Optimization will not be performed when number of available catchments is zero.
#'
#'   Criterion value is calculated by using a penalty function that increases the criterion value when difference in total runoff between the second and the first catchment is negative.
#' @seealso sbil.run
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.pet(b)
#' bil.set.area(b, 131.88)
#' s = sbil.new(b)
#' sbil.optimize(s)
sbil.optimize <- function (system) {
    err = .Call("sbil_optimize", check.system(system), PACKAGE = "bilan")

    if (err != "")  
        stop(err)
}
