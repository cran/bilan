#' Getting the optimization settings
#'
#' For the model or for the system of catchments, the function gets criterion value and optimization settings: type of criteria, weight for baseflow, initial groundwater storage
#'   and specific settings for applied optimization type.
#'
#' @param object pointer to model instance or to system of catchments instance
#' @return List of criterion value and optimization settings, for two-step gradient algorithm:
#'   \item{crit_part1}{criterion for the first part}
#'   \item{crit_part2}{criterion for the second part}
#'   \item{crit_value}{resulting criterion value (after the second part)}
#'   \item{init_GS}{initial groundwater storage}
#'   \item{max_iter}{maximum number of iterations}
#'   \item{weight_BF}{weight for baseflow}
#'   For shuffled complex evolution combined with differential evolution:
#'   \item{DE_type}{differential evolution version}
#'   \item{comp_size}{complex size}
#'   \item{crit}{optimization criterion}
#'   \item{crit_value}{resulting criterion value (after the second part)}  
#'   \item{cross}{crossover parameter}
#'   \item{ens_count}{number of runs in ensemble}
#'   \item{init_GS}{initial groundwater storage}
#'   \item{maxn_shuffles}{number of shuffling}
#'   \item{mutat_f}{mutation parameter}
#'   \item{mutat_k}{mutation parameter}
#'   \item{n_comp}{number of complexes}
#'   \item{n_gen_comp}{number of generations in one complex}
#'   \item{seed}{seed used for random number generator}
#'   \item{weight_BF}{weight for baseflow}
#' @seealso \code{\link{bil.set.optim}}
#' @aliases sbil.get.optim
#' @export
#' @examples
#' b = bil.new("m")
#' s = sbil.new(b)
#' bil.set.optim(b, "DE")
#' bil.get.optim(b)
#' sbil.get.optim(s)
bil.get.optim <- function (object) {
    if (class(object) == "bil_system") {
        func = "sbil_get_optim"
        check_func = check.system
    }
    else {
        func = "get_optim"
        check_func = check.model
    }
    optim = .Call(func, check_func(object), PACKAGE = "bilan")
    num_pos = substr(names(optim), 0, 4) != "crit" | names(optim) == "crit_value"
    optim[num_pos] = lapply(optim[num_pos], as.numeric) # not criterion name to numeric
    return(optim)
}

#' @name bil.get.optim
#' @rdname bil.get.optim
sbil.get.optim <- function (object) {
    bil.get.optim(object)
}

# general optimization settings for all optimization methods
#' @name bil.set.optim
#' @rdname bil.set.optim
set.optim.gen <- function (object, crit = NULL, crit_part1 = NULL, crit_part2 = NULL, weight_BF = 0, init_GS = 50, use_weights = FALSE, weights = NULL, ...) {
    crit_names = c("MSE", "MAE", "NS", "LNNS", "MAPE")
    pos = pmatch(crit, crit_names) - 1 # to C index
    if (is.null(crit)) { #crit not defined, crits for parts used if available
        if (is.null(crit_part1)) #default values
            crit_part1 = "MSE"
        if (is.null(crit_part2))
            crit_part2 = "MAPE"
    }
    else if (is.na(pos))
        stop("Unknown name of optimization criterion.")
    else { #crit is set, crits for parts are ignored
        crit_part1 = crit
        if (crit == "MSE" || crit == "MAE")
            crit_part2 = "MAPE"
        else
            crit_part2 = crit
    }
    pos_crit1 = pmatch(crit_part1, crit_names) - 1
    pos_crit2 = pmatch(crit_part2, crit_names) - 1

    if (is.na(pos_crit1) || is.na(pos_crit2))
        stop("Unknown name of optimization criterion.")

    if (!is.null(weights)) {
        if (class(object) == "bilan")
            bil.set.values(object, data.frame(WEI = weights), append = TRUE)
        else
            stop("Optimization using weights is not supported for system of catchments.")
    }
        
    return(list(pos_crit1 = pos_crit1, pos_crit2 = pos_crit2, weight_BF = weight_BF, init_GS = init_GS, use_weights = use_weights))
}

#' Binary search optimization settings
#'
#' Sets parameters of optimization method based on binary search, either for the model or for the system of catchments.
#'
#' @param object pointer to model instance or to system of catchments instance
#' @param max_iter maximum number of iterations
#' @param \dots common optimization arguments described at \code{\link{bil.set.optim}}
#' @seealso \code{\link{bil.set.optim}}
#' @aliases sbil.set.optimBS
#' @export
#' @examples
#' b = bil.new("m")
#' input = data.frame(
#'   P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
#' bil.set.values(b, input, init_date = "1990-11-01")
#' bil.pet(b, "latit")
#' bil.set.optimBS(b, crit = "NS", max_iter = 1e3)
#' bil.optimize(b)
#' s = sbil.new(b)
#' sbil.set.optimBS(s, crit = "NS", max_iter = 1e3)
bil.set.optimBS <- function (object, max_iter = 500, ...) {
    optim_par = set.optim.gen(object, ...)
    if (max_iter < 0)
        stop("Number of iterations cannot be negative.")
        
    if (class(object) == "bil_system") {
        func = "sbil_set_optim"
        check_func = check.system
    }
    else {
        func = "set_optim"
        check_func = check.model
    }
    err = .Call(func, check_func(object), optim_par[["pos_crit1"]], optim_par[["pos_crit2"]], optim_par[["weight_BF"]], max_iter, optim_par[["init_GS"]], optim_par[["use_weights"]], PACKAGE = "bilan")
    
    if (err != "")  
      stop(err)
}

#' @name bil.set.optimBS
#' @rdname bil.set.optimBS
sbil.set.optimBS <- function (object, ...) {
    bil.set.optimBS(object, ...)
}

#' Differential evolution optimization settings
#'
#' Sets optimization method to combined shuffled complex evolution (SCE-UA) and differential evolution and sets its parameters, either for the model or for the system of catchments.
#'
#' @param object pointer to model instance or to system of catchments instance
#' @param DE_type name of differential evolution version: \code{"best_one_bin"}, \code{"best_two_bin"}, \code{"rand_two_bin"} 
#'   (means mutation of best/random parameter set and number of differences between parameters used for the mutation)
#' @param n_comp number of complexes
#' @param comp_size complex size (population size in one complex)
#' @param cross crossover parameter
#' @param mutat_f mutation parameter for all DE types
#' @param mutat_k mutation parameter for two differences used for mutation
#' @param maxn_shuffles number of shuffling
#' @param n_gen_comp number of generations in one complex
#' @param ens_count number of runs in ensemble
#' @param seed seed to initialize random number generator (<= 0 for initialization based on time)
#' @param \dots common optimization arguments described at \code{\link{bil.set.optim}}
#' @seealso \code{\link{bil.set.optim}}
#' @aliases sbil.set.optimDE
#' @references Rainer Storn and Kenneth Price. Differential evolution – a simple and efficient heuristic for global optimization over 
#'   continuous spaces. J. of Global Optimization, 11(4):341–359, December 1997.
#'
#'   Qingyun Duan, Soroosh Sorooshian, and Vijai K. Gupta. Optimal use of the SCE-UA global optimization method for calibrating 
#'   watershed models. Journal of Hydrology, 158(3-4):265–284, 1994.
#' @export
#' @examples
#' b = bil.new("m")
#' input = data.frame(
#'   P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
#' bil.set.values(b, input, init_date = "1990-11-01")
#' bil.pet(b, "latit")
#' bil.set.optimDE(b, crit = "NS", comp_size = 20, cross = 0.8, mutat_f = 0.8)
#' bil.optimize(b)
#' s = sbil.new(b)
#' sbil.set.optimDE(s, crit = "NS", comp_size = 20, cross = 0.8, mutat_f = 0.8)
bil.set.optimDE <- function (object, DE_type = "best_one_bin", n_comp = 4, comp_size = 10, cross = 0.95, mutat_f = 0.95, mutat_k = 0.85,
                             maxn_shuffles = 5, n_gen_comp = 10, ens_count = 5, seed = 0, ...) {
    if (is.list(DE_type))
        stop("Passing list to bil.set.optimDE is obsolete. Use function arguments instead.")

    optim_par = set.optim.gen(object, ...)

    pos_DE_type = pmatch(DE_type, c("best_one_bin", "best_two_bin", "rand_two_bin")) - 1
    if (is.na(pos_DE_type))
        stop("Unknown name of DE type.")

    if (class(object) == "bil_system") {
        func = "sbil_set_DE_optim"
        check_func = check.system
    }
    else {
        func = "set_DE_optim"
        check_func = check.model
    }
    err = .Call(func, check_func(object), optim_par[["pos_crit1"]], pos_DE_type, n_comp, comp_size, cross, mutat_f, mutat_k, maxn_shuffles,
                n_gen_comp, ens_count, seed, optim_par[["weight_BF"]], optim_par[["init_GS"]], optim_par[["use_weights"]], PACKAGE = "bilan")

    if (err != "")  
        stop(err)
}

#' @name bil.set.optimDE
#' @rdname bil.set.optimDE
sbil.set.optimDE <- function (object, ...) {
    bil.set.optimDE(object, ...)
}

#' Optimization setting
#'
#' Sets optimization method and its parameters, either for the model or for the system of catchments.
#'
#' @param object pointer to model instance or to system of catchments instance
#' @param method optimization method: gradient-based binary search (\code{"BS"}) or combined shuffled complex evolution (SCE-UA) and differential evolution (\code{"DE"})
#' @param from for model only, pointer to another model instance; if specified, optimization settings is copied from that model and all other arguments are ignored
#' @param \dots other common or specific arguments. The specific arguments are described at \code{\link{bil.set.optimBS}} or \code{\link{bil.set.optimDE}}, the common (passed to \code{set.optim.gen}) below.
#' @param crit For BS method, sets criteria for both parts of optimization; if defined, \code{crit_part1} and \code{crit_part2} are ignored. For MSE and MAE, MAPE is used
#'   in the second part, for others, the criterion is the same for both parts.
#'
#'   For DE method, name of optimization criterion (default MSE).
#'
#'   Supported criteria: \code{"MSE"} (mean squared error), \code{"MAE"} (mean absolute error), \code{"NS"} (Nash-Sutcliffe efficiency),
#'   \code{"LNNS"} (Nash-Sutcliffe of log-transformed data), \code{"MAPE"} (mean absolute percentage error)
#' @param crit_part1 for BS method, name of criterion for the first part of optimization: MSE (mean squared error), MAE (mean absolute error), NS (Nash-Sutcliffe efficiency),
#'   LNNS (Nash-Sutcliffe of log-transformed data), MAPE (mean absolute percentage error); default MSE used if not defined
#' @param crit_part2 for BS method, name of criterion for the second part of optimization; default MAPE used if not defined
#' @param weight_BF weight for criterion for observed and modelled baseflow (between 0 and 1, default 0), weight for runoff is (1 - weight_BF)
#' @param init_GS initial groundwater storage in mm (initial condition, default 50)
#' @param use_weights whether to use weights for criterion calculation in optimization
#' @param weights a vector of weights for criterion calculation, length of weights must correspond with number of time steps of variable series. 
#'   If not given, the variable \code{WEI} is used (by default, its values are equal). The weights are considered as relative, e.g. \code{c(2,2,3)} has the same effect as \code{c(4,4,6)}.
#'   Parts of time series can be excluded from optimization by setting weights to zero.
#' @seealso \code{\link{bil.optimize}}, \code{\link{bil.set.optimBS}}, \code{\link{bil.set.optimDE}}
#' @aliases set.optim.gen sbil.set.optim
#' @export
#' @examples
#' b = bil.new("m")
#' input = data.frame(
#'   P = c(42, 48, 53, 66, 46, 26, 149, 50, 75, 33, 55, 36),
#'   R = c(23, 16, 28, 26, 40, 78, 62, 27, 16, 11, 12, 18),
#'   T = c(1.7, -4.6, -2.9, -3.7, -2.9, 7.3, 8.7, 12.4, 13.8, 15.7, 10.8, 6.9))
#' bil.set.values(b, input, init_date = "1990-11-01")
#' bil.pet(b, "latit")
#' bil.set.optim(b, "DE", crit = "NS")
#' bil.optimize(b)
#' b2 = bil.new("d")
#' bil.set.optim(b2, from = b)
bil.set.optim <- function(object, method = "BS", from = NULL, ...) {
    if (!is.null(from) && class(object) != "bil_system") {
        err = .Call("copy_optim", check.model(object), check.model(from), PACKAGE = "bilan")
        if (err != "")
            stop(err)
    }
    else {
        if (is.list(method))
            stop("Passing list to bil.set.optim is obsolete. Use function arguments instead.")
        
        if (method == "BS")
            bil.set.optimBS(object, ...)
        else if (method == "DE")
            bil.set.optimDE(object, ...)
        else
            stop("Unknown optimization method.")
    }
}

#' @name bil.set.optim
#' @rdname bil.set.optim
sbil.set.optim <- function (object, ...) {
    bil.set.optim(object, ...)
}
