check.model <- function (model) {
    if (class(model) != "bilan" || typeof(model) != "externalptr")
        stop("Model must be created by bil.new() function.")
    else
        return(model)
}

#' Bilan model instance creation
#'
#' Creates a new instance of Bilan model, optionally loads data to it.
#'
#' @param type type of model: daily (\dQuote{d} or \dQuote{D}) or monthly (\dQuote{m} or \dQuote{M})
#' @param file name of input or output file to be used for loading data
#' @param use_data output files only, whether to use output time-series of variables
#' @param data a data frame of model variables (described at \code{\link{bil.set.values}})
#' @param \dots further arguments passed to \code{\link{bil.read.file}} (names of variables) or \code{\link{bil.set.values}} (initial date)
#' @return Object of class \dQuote{bilan} which is a pointer to Bilan model instance in C++ that cannot be directly accessed.
#' @details If \code{file} is specified, loads data from Bilan input or output files. Input data and initial date are loaded from input file
#'   (by \code{\link{bil.read.file}}), whereas parameter set (by \code{\link{bil.read.params.file}}) or optionally (controlled by \code{use_data})
#'   all variables and initial date are loaded from output file.
#'
#'   If both \code{file} and \code{data} are specified, \code{data} are used and \code{file} is ignored.
#'
#'   Old-style formatted input and output files are also supported.
#' @seealso \code{\link{bil.read.file}}, \code{\link{bil.read.params.file}}, \code{\link{bil.set.values}}
#' @export
#' @examples
#' b = bil.new("m")
bil.new <- function (type, file = NULL, use_data = TRUE, data = NULL, ...) {
    bil = .Call("new_bil", as.character(type), PACKAGE = "bilan")
    if (class(bil) == "character")
        stop(bil)
    class(bil) = "bilan"
    
    if (!is.null(data)) {
        bil.set.values(bil, data, ...)
    }
    else if (!is.null(file)) {
        if (scan(file, character(), 1, quiet = TRUE) == "Initial") { # output file
            bil.read.params.file(bil, file)
            if (use_data) {
                init_date = as.Date(scan(file, what = character(), n = 2, quiet = TRUE)[2])
                output_vars = read.table(file, header = TRUE, skip = 6 + nrow(bil.get.params(bil)))
                bil.set.values(bil, output_vars, init_date)
            }
        }
        else if (substr(scan(file, character(), 1, quiet = TRUE), 1, 5) == "BILAN") { # old-style output file
            bil.read.params.file(bil, file)
            if (use_data) {
                output_vars = read.table(file, header = TRUE, skip = 23, sep = ",", check.names = FALSE, fileEncoding = "latin1")
                init_date = as.Date(paste(output_vars$Year[1] - 1, "11", "01", sep = "-")) # old file always begins with 11-01
                output_vars[["Rank number"]] = output_vars[["Year"]] = NULL
                var_names = unlist(lapply(strsplit(names(output_vars), "\\["), function(x) x[1]))
                var_names[which(var_names == "PE")] = "PET" # old names to new
                var_names[which(var_names == "E")] = "ET"
                names(output_vars) = var_names
                bil.set.values(bil, output_vars, init_date)
            }
        }
        else
            bil.read.file(bil, file, ...)
    }
    return(bil)
}

#' Cloning an existing model
#'
#' Creates a new instance of Bilan model that is identical to a given existing instance.
#'
#' @param model pointer to model instance to be cloned
#' @return Result of cloning: object of class \dQuote{bilan} which is a pointer to Bilan model instance.
#' @export
#' @examples
#' b = bil.new("m")
#' b2 = bil.clone(b)
bil.clone <- function (model) {
    bil = .Call("clone_model", check.model(model), PACKAGE = "bilan")
    class(bil) = "bilan"
    return(bil)
}

#' Data input from a file
#'
#' Loads initial date, catchment area and observed meteorological and hydrological data from a file.
#'
#' @param model pointer to model instance
#' @param file_name input file name
#' @param input_vars a vector of names of input variables (described at \code{\link{bil.set.values}})
#' @details The file must contain the initial date of data at first line (in yyyy mm dd format separated by white characters).
#'   Optionally catchment area in square kilometers can be set at this line;
#'   a number will be interpreted as catchment area in case it is fourth at the line or it is last and contains decimal point.
#'
#'   The first line is followed by columns with variables as defined by input_vars. Number of variables 
#'   cannot be greater than number of columns in file. If number of variables is less than number of columns, the last exceeding columns are ignored.
#' 
#'   Old-style formatted output files are also supported. These files begin with three lines with number of rows, number of columns and initial water year.
#'
#'   An error occurs if input variables include unknown name or if number of columns is less than number of variables.
#' @seealso \code{\link{bil.write.file}} for writing to a file, \code{\link{bil.set.values}} for information about variable names
#' @export
#' @examples
#' \dontrun{
#' b = bil.new("m")
#' bil.read.file(b, "1550m.dat", c("P", "R", "T", "POD"))}
bil.read.file <- function (model, file_name, input_vars = c("P", "R", "T", "H", "B")) {
    err = .Call("read_file", check.model(model), as.character(file_name), as.character(input_vars), PACKAGE = "bilan")

    if (err != "")  
        stop(err)
}

#' Input variable settings
#'
#' Sets time-series of (input) variables and initial date.
#'
#' @param model pointer to model instance
#' @param input_vars data frame of time-series of input variables, their names must match with names in Bilan (columns with unknown names are omitted).
#'   The model is automatically extended to a version with water use when one of the names \code{POD}, \code{POV}, \code{PVN} or \code{VYP} is specified.
#' @param init_date initial date of time serie, should be of Date or character (formatted as \dQuote{YYYY-MM-DD}) class. 
#'   If not defined, the first value of data column \code{DTM} or the first row name of \code{input_vars} is tried to be used. 
#'   Initial date is not needed for appending (\code{append = TRUE}).
#' @param append if false, all old time-series are deleted and replaced; otherwise only given variables are replaced – 
#'   in that case number of time steps needs to be equal to time steps of data to be replaced
#' @details An error occurs if numbers of time steps do not match (only for appending) or if initial date is not given or found (in varible DTM or row names).
#' @export
#' @examples
#' bil = bil.new("m")
#' input = data.frame(P = c(1, 0, 5), R = c(0.5, 0.7, 0.9), T = c(5, 4, 9), H = c(70, 80, 75))
#' bil.set.values(bil, input, "2011-01-01")
#' input2 = data.frame(P = c(4, 0, 5), POD = c(0.5, 0.5, 0.5))
#' bil.set.values(bil, input2, "2011-01-01", append = TRUE)
#' x = bil.get.values(bil)
#' print(x)
bil.set.values <- function (model, input_vars, init_date = NULL, append = FALSE) {
    if (is.null(init_date)) {
        if (append) {
            init_date = "1918-10-28" # dummy, date is not used for append
        }
        else if (!is.null(input_vars$DTM)) {
            init_date = input_vars$DTM[1]
            input_vars$DTM = NULL
        }
        else if (!is.null(rownames(input_vars))) {
            init_date = try(as.Date(rownames(input_vars)[1]), silent = TRUE)
            if (class(init_date) == "try-error")
                stop("Initial date is not defined and cannot be found as the first row name or the first item of DTM.\n")
        }
        else
            stop("Initial date is not defined.")
    }

    err = .Call("set_input_vars", check.model(model), as.data.frame(input_vars), as.Date(init_date), append, PACKAGE = "bilan")

    if (err != "")  
        stop(err)
}

#' Area setting
#'
#' Sets a catchment area for the model.
#'
#' @param model pointer to model instance
#' @param area catchment area in square kilomters
#' @export
#' @examples
#' bil = bil.new("m")
#' bil.set.area(bil, 131.88)
bil.set.area <- function (model, area) {
    area = as.numeric(area)
    invisible(.Call("set_area", check.model(model), area, PACKAGE = "bilan")) # not to return
}

#' Data output to a file
#'
#' Writes resulting parameters and time series of variables into a file.
#'
#' @param model pointer to model instance
#' @param file_name output file name
#' @param type type of output file: \dQuote{series} for default time-series, \dQuote{series_monthly} for monthly time-series, \dQuote{series_daily} for daily time-series
#'   (not available for monthly model type), \dQuote{chars} for monthly characteristics calculated from complete hydrological years
#' @seealso \code{\link{bil.read.file}}
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.pet(b, "latit")
#' bil.run(b)
#' bil.write.file(b, "0446.txt")
bil.write.file <- function (model, file_name, type = "series") {
    type_int = pmatch(type, c("series_daily", "series_monthly", "chars"))
    if (is.na(type_int))
        type_int = 0  # series default
    
    err = .Call("write_file", check.model(model), as.character(file_name), type_int, PACKAGE = "bilan")
    
    if (err != "")  
        stop(err)
}

#' @name bil.set.params
#' @rdname bil.set.params
bil.set.params.init <- function (model, params) {
    bil.set.params(model, params, 0)
}

#' @name bil.set.params
#' @rdname bil.set.params
bil.set.params.curr <- function (model, params) {
    bil.set.params(model, params, 1)
}

#' @name bil.set.params
#' @rdname bil.set.params
bil.set.params.lower <- function (model, params) {
    bil.set.params(model, params, 2)
}

#' @name bil.set.params
#' @rdname bil.set.params
bil.set.params.upper <- function (model, params) {
    bil.set.params(model, params, 3)
}

#' Parameter settings
#'
#' Sets values of parameters: initial, current, lower or upper limit or all of them.
#'
#' @param model pointer to model instance
#' @param params list or vector of parameter values with appropriate names, or a data frame with \dQuote{name} column (preferably a result from \code{\link{bil.get.params}}),
#'   or another model instance including parameter values to be set
#' @param type type of settings: initial values for gradient optimization (\dQuote{init} or 0), current values -- result of optimization
#'   or values used for simple run (\dQuote{curr} or 1), lower limits (\dQuote{lower} or 2), upper limits (\dQuote{upper} or 3) or all values (\dQuote{all} or 4)
#' @aliases bil.set.params.init bil.set.params.curr bil.set.params.lower bil.set.params.upper
#' @details If no names are given and number of parameters is equal to the number for current model type, names in default order are assigned.
#'
#'   An error occurs if another model instance is used and numbers of parameters in models do not match.
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.params(b, list(Spa = 70, Grd = 0.2), "init")
#' bil.set.params(b, list(Spa = 300), "upper")
#' pars = bil.get.params(b)
#' b2 = bil.new("m")
#' bil.set.params(b2, pars)
#' bil.set.params(b2, b)
bil.set.params <- function (model, params, type = "all") {
    if (class(params) == "bilan") {
        err = .Call("copy_params", check.model(model), check.model(params), PACKAGE = "bilan")
        if (err != "")  
            stop(err)
        else
            return(invisible(NULL))
    }
    if (type == "all") {
        types = colnames(params)[colnames(params) != "name"]
        all_params = params
    }
    else {
        types = type
        all_params = NULL
    }
        
    for (type in types) {
        # for all params take data frame column
        if (!is.null(all_params)) {
            params = all_params[[type]]
            names(params) = all_params$name
        }
        # convert type given by a word
        if (typeof(type) == "character")
            type_int = pmatch(type, c("initial", "current", "lower", "upper")) - 1
        else if (mode(type) == "numeric" && floor(type) >= 0 && floor(type) <= 3)
            type_int = floor(type)
        else
            type_int = NA

        if (is.na(type_int))
            stop(paste("Unknown type '", type, "' of parameters to set.", sep = ""))

        par_names = names(params)
        par_values = as.numeric(params)
        if (length(par_names) == 0) {
            # default names if number of parameters corresponds to the model type
            if (length(par_values) == nrow(bil.get.params(check.model(model))))
                par_names = as.character(bil.get.params(model)$name)
            else
                stop("Parameter names are not specified.")
        }        
        else if (any(par_names == ""))
            stop("Parameter names are not complete.")
            
        err = .Call("set_params", check.model(model), par_names, par_values, type_int, PACKAGE = "bilan")
            
        if (err != "")
            stop(err)
    }
}

#' Load of parameters from file
#'
#' Loads (optimized) model parameters from output file to initial and current values.
#'
#' @param model pointer to model instance
#' @param file_name output file with saved parameters
#' @details Old-style formatted output files are also supported.
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.pet(b, "latit")
bil.read.params.file <- function (model, file_name) {
    err = .Call("read_params_file", check.model(model), file_name, PACKAGE = "bilan")
        
    if (err != "")  
        stop(err)
}

#' Potential evapotranspiration estimation
#'
#' Estimates values of potential evapotranspiration, two methods are available.
#'
#' @param model pointer to model instance
#' @param pet_type the method to be used: based on vegetation zone (\dQuote{tab}) or based on latitude (\dQuote{latit}) 
#' @param latitude latitude (required only by the second method)
#' @details An error occurs if required input data are missing (temperature or humidity).
#' @references Gidrometeoizdat. Rekomendatsii po roschotu ispareniia s poverhnosti suchi. Gidrometeoizdat, St. Peterburg, 1976.
#'
#'   Ludovic Oudin, Lætitia Moulin, Hocine Bendjoudi, and Pierre Ribstein. Estimating potential evapotranspiration without 
#'   continuous daily data: possible errors and impact on water balance simulations. Hydrological Sciences Journal, 55(2):209, 2010.
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.pet(b, "latit")
bil.pet <- function (model, pet_type = "latit", latitude = 50) {
    err = .Call("pet", check.model(model), pet_type, latitude, PACKAGE = "bilan")

    if (err != "")
        stop(err)
}

#' Simple model run and access to model state
#'
#' Runs model, possibly with data given in an input file (has priority) or in a data frame. A model state for given date can be also obtained
#'   or the model can be run starting from specified state.
#'
#' @param model pointer to model instance
#' @param file file containing input variables
#' @param var_names a vector of names of input variables
#' @param data a data frame of time-series of input variables (as in \code{\link{bil.set.values}})
#' @param init_GS initial groundwater storage; if not defined, it is tried to be taken from optimization settings
#' @param get_state a date for which state variables will be returned; must be part of the data period
#' @param from_state a list of state variables (result of run with \code{get_state}):
#' \itemize{
#'  \item \code{date} is date of state, the run will start at time following this date (hence cannot equal to the date of the last time step)
#'  \item \code{season} season mode
#'  \item \code{SS, SW, GS, DS} represent snow, soil, groundwater and direct runoff storage (DS used in daily type only)
#'  \item \code{params} a named vector of parameter (names must match with model parameters); if not defined, initial parameter values will be used
#' }
#' @param update_pet whether to (re)calculate potential evapotranspiration (potentially based on new time-series given by \code{data})
#' @param \dots further arguments for PET calculation passed to \code{\link{bil.pet}}
#' @return A data frame of time-series of input and output variables including dates. For \code{from_state} specified,
#'   the returned time-series will start with time step following the date of state.
#' 
#'   In case of \code{get_state}, a list containing date of the state, season mode, state reservoir volumes and current parameter values.
#' @details If both \code{get_state} and \code{from_state} specified, \code{get_state} will be applied.
#' @seealso \code{\link{bil.set.values}}, \code{\link{bil.read.file}}, \code{\link{bil.get.values}}
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.run(b, update_pet = TRUE)
#' st = bil.run(b, get_state = "1990-12-01")
#' st$date = as.Date("1991-07-01")
#' bil.run(b, from_state = st)
bil.run <- function (model, file = NULL, var_names = NULL, data = NULL, init_GS = NULL, get_state = NULL, from_state = NULL, update_pet = FALSE, ...) {
    if (!is.null(file)) {
        if (is.null(var_names))
            bil.read.file(model, file)
        else
            bil.read.file(model, file, var_names)
    }
    else if (!is.null(data)) {
        bil.set.values(model, data)
    }
    if (update_pet)
        bil.pet(model, ...)
    
    if (is.null(init_GS)) {
        if (is.null(bil.get.optim(model)$init_GS))
            stop("Initial groundwater storage is not defined.")
        else
            init_GS = bil.get.optim(model)$init_GS
    }
    if (!is.null(get_state)) {
        state = .Call("get_state", check.model(model), as.Date(get_state), init_GS, PACKAGE = "bilan")
        if (class(state) == "character")
            stop(state)
        else {
            state$date = as.Date(state$date)
            pars = bil.get.params(model)
            state$params = pars$current
            names(state$params) = pars$name
            return(state)
        }
    }
    else if (!is.null(from_state)) {
        if (!is.null(from_state$params))
            bil.set.params.init(model, from_state$params)
        err = .Call("run_from_state", check.model(model), from_state, PACKAGE = "bilan")
        if (err != "")
            stop(err)
        else {
            resul = bil.get.data(model)
            from_row = which(from_state$date == resul$DTM) + 1
            return(resul[from_row:nrow(resul), ])
        }        
    }
    else {
        err = .Call("run", check.model(model), init_GS, PACKAGE = "bilan")
        if (err != "")
            stop(err)
        else
            return(bil.get.data(model))
    }
}

#' Optimization procedure
#'
#' Optimizes model parameters according to observed data by using selected algorithm (a two-step gradient procedure or combined
#'   shuffled complex evolution and differential evolution).
#'
#' @param model pointer to model instance
#' @return Data frame of time-series of input and output variables including dates.
#' @details For simple model run, use \code{\link{bil.run}} function or choose the gradient method and set number of iterations to zero
#'   (by \code{\link{bil.set.optim}}). An error occurs if data needed for model run are missing.
#' @export
#' @examples
#' b = bil.new("m")
#' input = data.frame(
#'   P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
#' bil.set.values(b, input, init_date = "1990-11-01")
#' bil.pet(b, "latit")
#' bil.set.optim(b, "DE")
#' bil.optimize(b)
bil.optimize <- function (model) {
    err = .Call("optimize", check.model(model), PACKAGE = "bilan")

    if (err != "")  
        stop(err)
    else
        return(bil.get.values(model)$vars)        
}

#' @name bil.get.values
#' @rdname bil.get.values
bil.get.dtm <- function (model) {
    as.Date(.Call("get_dates", check.model(model), PACKAGE = "bilan"))
}

#' @name bil.get.values
#' @rdname bil.get.values
bil.get.data <- function (model, get_chars = FALSE) {
    vars = .Call("get_vars", check.model(model), get_chars, PACKAGE = "bilan")
    if (class(vars) == "character")
        stop(vars)
    
    if (!get_chars) {
        dates = bil.get.dtm(model)
        return(cbind(DTM = dates, as.data.frame(vars)))
    }
    else {
        month = list(c(11:12, 1:10))
        chars = as.list(c(vars, month = month))
        return(chars)
    }
}

#' Access to model data
#'
#' Gets model data values: parameters and variables including dates. For ensemble, gets data of last model in ensemble.
#'
#' @param model pointer to model instance
#' @param get_chars whether to get time-series of variables or monthly characteristics (min, mean, max)
#' @return List of model results, for time-series:
#'   \item{params}{parameters in a data frame: parameter names, current value, lower and upper limits and initial values}
#'   \item{crit}{resulting criterion value (after the second part of optimization)}
#'   \item{vars}{time series of variables in a data frame, the first column DTM means dates}
#'   For characteristics:
#'   \item{P.min, P.mean, P.max, RM.min...}{values of characteristics for all variables}
#'   \item{params}{parameter names, values and limits}
#'   \item{crit}{resulting criterion value}
#' @details The most complex output is provided by \code{bil.get.values}, the other functions get a subset of it:
#'   \code{vars} (or characteristics) in case of \code{bil.get.data} and \code{vars$DTM} in case of \code{bil.get.dtm}.
#'
#'   An error occurs when requesting monthly characteristics for daily time-series shorter than one complete month.
#'   Additionally, monthly characteristics are set to 0 in case of time-series shorter than one year.
#' @seealso \code{\link{bil.get.params}}
#' @aliases bil.get.dtm bil.get.data
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.values(b, init_date = "1990-11-01", input_vars = 
#'   data.frame(P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9)))
#' bil.pet(b, "latit")
#' resul = bil.get.values(b)
bil.get.values <- function (model, get_chars = FALSE) {
    params = bil.get.params(model)
    optim = bil.get.optim(model)
    vars = bil.get.data(model, get_chars)

    if (!get_chars) {
        return(list(params = params, vars = vars, crit = optim[["crit_value"]]))
    }
    else {
        return(c(vars, params = list(params), crit = optim[["crit_value"]]))
    }
}

#' Access to model parameters
#'
#' Gets current and initial values of parameters and their limits.
#'
#' @param model pointer to model instance
#' @return Parameters in a data frame: parameter names, current value, lower and upper limits and initial values.
#' @details This function is also part of \code{bil.get.values}.
#' @seealso \code{\link{bil.get.values}}
#' @export
#' @examples
#' b = bil.new("m")
#' bil.set.params(b, list(Spa = 70, Grd = 0.2), "init")
#' bil.get.params(b)
bil.get.params <- function (model) {
    params = .Call("get_params", check.model(model), PACKAGE = "bilan")
    return(params)
}

#' Information about model
#'
#' Gets information about model type, number of observations, time period and used variables.
#'
#' @param model pointer to model instance
#' @param print whether to print the summary
#' @return A list containing model type, number of time steps, begin and end of time period and vector of model variables.
#' @export
#' @examples
#' b = bil.new("m")
#' input = data.frame(
#'   P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
#' bil.set.values(b, input, init_date = "1990-11-01")
#' info = bil.info(b)
bil.info <- function (model, print = TRUE) {
    info = .Call("get_info", check.model(model), PACKAGE = "bilan")
    info[c("period_begin", "period_end")] = lapply(info[c("period_begin", "period_end")], as.Date)
    if (print) {
        cat("\n====== Bilan model ======\n")
        cat(paste("time step: ", info$time_step, "\n", sep = ""))
        cat(paste("period: ", info$period_begin, " -- ", info$period_end, "\n", sep = ""))
        cat(paste("number of observations: ", info$time_steps, "\n", sep = ""))
        cat(paste("variables: ", paste(info$var_names, ", ", sep = "", collapse = ""), "\n", sep = ""))
        cat("=========================\n")
    }
    invisible(info)
}

#' Access to results of model ensemble
#'
#' For differential evolution optimization, gets results of model ensemble: parameters and criterion values.
#'
#' @param model pointer to model instance
#' @return Data frame whose named columns represents model parameters, criterion value and number of model evaluations, the ensemble is represented by rows.
#' @export
#' @examples
#' b = bil.new("m")
#' input = data.frame(
#'   P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
#' bil.set.values(b, input, init_date = "1990-11-01")
#' bil.pet(b, "latit")
#' bil.set.optim(b, "DE")
#' bil.optimize(b)
#' bil.get.ens.resul(b)
bil.get.ens.resul <- function(model) {
  resul = .Call("get_ens_resul", check.model(model), PACKAGE = "bilan")
  if (is.null(nrow(resul))) # gradient
    return(NA)
  else { # differential evolution
    crit_names = c("MSE", "MAE", "NS", "LNNS", "MAPE")
    names(resul)[ncol(resul) - 1] = crit_names[as.numeric(sub("OF", "", names(resul)[ncol(resul) - 1], fixed = TRUE)) + 1]
    return(resul)
  }
}
