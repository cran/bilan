/**
 * @file
 * - model, state, parameter and date classes
 */

#ifndef BIL_MODEL_H_INCLUDED
#define BIL_MODEL_H_INCLUDED

#include <ctime>
#include <list>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cfloat>

#include <iostream>

#ifdef __sun
# undef GS
#endif

/**
 * - Bilan error exception
 */
class bil_err
{
  public:
    //!< creates an error from given text
    bil_err(std::string text) : descr(text) { };
    std::string descr; //!< description of the error
};

#include "bil_optim.h" //at least due to enum param_type in optimizer which cannot be forward declared

//to be uncommented for R interface
#include "bilan-r.h"

#ifndef BIL_OSTREAM
//! output of messages - std::cout by default, to be replaced by Rcout in R interface
#define BIL_OSTREAM std::cout
#endif

//! critical temperature for winter balance in monthly type
#define T_KRIT -8

//! epsilon related to machine precision
#define NUMERIC_EPS numeric_limits<long double>::epsilon()

/**
 * - day, month and year
 */
class date
{
  public:
    date(); //!< default date 2000-01-01
    date(unsigned y, unsigned m, unsigned d); //!< date from given values
    bool operator==(date& orig_date);
    bool operator!=(date& orig_date);
    bool operator<(date& orig_date);
    bool operator>(date& orig_date);

    //! daily or monthly
    enum step_type {DAY, MONTH};
    void increase(step_type step); //!< increases date by day or month
    void decrease(step_type step); //!< decreases date by day or month
    unsigned get_day_of_year(); //!< day of year for date
    bool is_leap(); //!< if year is leap
    std::string to_string(); //!< date as a string

    friend std::ostream& operator<<(std::ostream &os, date &d); //!< prints date in yyyy-mm-dd format

    //!@{
    /** @name
     *  parts of date
     */
    unsigned year;
    unsigned month;
    unsigned day;
    //!@}

    static const unsigned days_in_month[];
    static const unsigned days_in_shortest;
    static const unsigned day_of_year[];

  private:

};

/**
 * - model parameter
 */
class parameter
{
  public:
    parameter() : lower(0), upper(0), value(0), initial(0) {}; //!< defaults set to zero
    parameter(double low, double upp, double val, double init) : lower(low), upper(upp), value(val), initial(init) {}; //!< creates parameter from value, limits and initial value
    enum param_type {INIT, CURR, LOWER, UPPER}; //!< the same parameter types as in bilan class

    double lower; //!< lower limit
    double upper; //!< upper limit
    double value; //!< current value
    double initial; //!< initial value
};

/**
 * - properties of input file
 * - allows to separate reading of header
 */
class input_file_info
{
	public:
    input_file_info();
    ~input_file_info() {};

    unsigned nrow, nrow_blank, ncol; //!< number of rows, blank rows and columns
    bool old_style; //!< if old style format of file is used
    string rows[4]; //!< the first two or four (old style) header lines

    void read_header(string file_name); //!< reads header from given file

	private:

};

/**
 * - state variables of Bilan model for one time step
 * - used for getting and setting state for given date
 */
class bil_state
{
	public:
    bil_state();
    ~bil_state() {};

    bool is_active_get; //!< whether the state will be get in model run
    bool is_active_set; //!< or set in model run
    unsigned season; //!< seasonal mode
    date calen; //!< date of state (simulation starts from date + 1)
    unsigned ts; //!< time step matching with date
    //! names of state variables, DS unused for monthly type
    enum state_var_names {stSS, stSW, stGS, stDS};
    static const unsigned st_var_count = 4; //!< number of state variables
    long double st_var[st_var_count]; //!< state variables (reservoir storages)

	private:

};

class bilan;

/**
 * - functoid class for access to methods when optimizing
 * - intentionally separated from bilan class although only calls its methods
 */
class bilan_fcd
{
  public:
    bilan_fcd() { pbil = 0; };
    //! creation of functoid for bilan
    bilan_fcd(bilan *bil) { pbil = bil; };
    //! sets pointer to bilan
    void set(bilan *bil) { pbil = bil; };
    void calc_sum_weights(); //!< calculates sum of weights for optimization
    void check_vars_for_optim(bool is_weight_BF); //!< checks availability of variables for optimization
    double get_param(unsigned par_n, optimizer_gen<bilan_fcd*>::param_type par_type); //!< gets value of parameter
    void set_param(unsigned par_n, optimizer_gen<bilan_fcd*>::param_type par_type, double value); //!< sets value of parameter
    unsigned get_param_count(); //!< gets number of parameters
    unsigned get_param_fix_count(); //!< gets number of fixed parameters
    std::string get_param_name(unsigned par_n); //!< gets name of parameter
    void run(long double init_GS); //!< runs the model
    long double calc_crit(unsigned crit, double weight_BF, bool use_weights); //!< calculates optimization criterion value

  private:
    bilan* pbil; //!< pointer to a model
};

/**
 * - Bilan model with all information
 */
class bilan
{
  public:
    //! daily or monthly model type
    enum bilan_type {DAILY, MONTHLY};
    bilan(bilan_type type); //!< creates a model of given type
    bilan(const bilan& orig);
    bilan& operator=(const bilan& orig);
    ~bilan();
    //! type of parameter value
    enum param_type {INIT, CURR, LOWER, UPPER};

    void change_type(); //!< changes type of model (daily or monthly)
    void set_water_use(bool water_use); //!< reallocates for variables of water use
    //! area getter
    double get_area() { return area; };
    //! area setter
    void set_area(double new_area) { area = new_area; };
    //! water use getter
    bool get_water_use() { return water_use; };
    //! setter for is system optimization
    void set_system_optim(bool is_optim) { is_system_optim = is_optim; };
    //! system optimization getter
    bool get_system_optim() { return is_system_optim; };
    void check_vars_for_optim(bool is_weight_BF); //!< checks availability of variables for optimization
    void init_par(); //!< initializes values and limits of parameters
    void init_par(std::map<std::string, double>& par_inits, param_type par_type); //!< sets parameters inits by given values
    void init_var(unsigned time_steps); //!< initializes vectors of variables
    void calc_var_mon(); //!< calculates monthly variables for daily type
    void calc_char_mon(); //!< calculates monthly chars from monthly series
    void calc_chars(); //!< calculates monthly chars from monthly or daily series
    void read_file(std::string file_name, unsigned input_var[], unsigned input_var_count); //!< reads observed data from a file
    void read_file_header(unsigned& nrow, unsigned& ncol, bool& old_style, ifstream& in_stream, stringstream& st_stream); //!< reads header of input file
    void read_params_file(std::string file_name); //!< reads parameters from output file
    //! type of output file (series according to model type, daily series, monthly series, characteristics)
    enum output_type {SERIES, SERIES_DAILY, SERIES_MONTHLY, CHARS};
    void write_file(std::string file_name, output_type out_type); //!< writes results into a file

    bil_state get_state(long double init_GS, date st_date); //!< get state variables of given date
    void run_from_state(const bil_state& state); //!< run model starting from given state
    void run(long double init_GS); //!< runs daily or monthly Bilan model
    void winter_daily(double prev_snow); //!< winter surface balance - daily
    void winter_monthly(double prev_snow); //!< winter surface balance - monthly
    void melt_daily(double prev_snow); //!< snow melting - daily
    void melt_monthly(double prev_snow); //!< snow melting - monthly
    void winter_balance(double prev_W); //!< winter soil balance
    void summer_balance(double prev_W); //!< summer soil balance
    void divide_daily(int mode, double prev_DS, double prev_RB); //!< runoff divider - daily
    void divide_monthly(int mode, double prev_RB); //!< runoff divider - monthly
    void include_water_use(); //!< includes withdrawals and release

    void pet_estim_tab(); //!< PET using tables for vegetation zones
    void pet_estim_tab_zone(int veg_zone); //!< PET for chosen vegetation zone
    void pet_estim_latit(long double latitude); //!< PET by temperature and latitude

    optimizer_gen<bilan_fcd*> *optim; //!< optimization settings and variables (gradient or DE method)
    long double calc_crit(unsigned crit_type, unsigned var_obs, unsigned var_mod, bool use_weights); //!< calculates optimization criterion for given variable
    long double calc_crit_RM_BF(unsigned crit_type, double weight_BF, bool use_weights); //!< calculates optimization criterion for runoff and baseflow
    std::string get_param_name(unsigned par); //! returns name of given parameter
    std::string get_var_name(unsigned var); //!< returns name of given variable
    long double get_var_sum(unsigned var); //!< returns sum of given variable
    long double get_flow_m3s1(unsigned ts, unsigned var_n); //!< flow value to cubic meters per second
    long double get_optim_ok(); //!< gets optimization criterion value
    std::string get_init_date(); //!< gets initial date
    unsigned get_var_pos(const std::string& var_name); //!< gets index of variable
    //! gets model type
    bilan_type get_type() { return type; };
    void set_calendar(unsigned init_year, unsigned init_month, unsigned init_day); //!< sets calendar values according to the initial date
    void calc_years_count(date *calen, unsigned months); //!< find out init_m and number of years

    void set_var_na(unsigned var_n); //!< sets all variable values to NA
    bool is_var_na(unsigned var_n); //!< checks if any value in variable is NA
    bool is_value_na(unsigned ts, unsigned var_n); //!< checks if variable value is NA

    //!@{
    /** @name
     *  number of parameters (all and fixed after first optimization part) and variables (according to type of model)
     */
    unsigned par_count, par_fix_count, var_count;
    //!@}
    parameter *param; //!< model parameters
    long double **var; //!< observed and modelled variables (+time step, +variable)
    long double **var_mon; //!< monthly series of observed and modelled variables (daily version only) (+month, +variable)
    long double **char_mon; //!< monthly characteristics (+month, +variable - min/mean/max)
    bool *var_is_input; //!< if variable was loaded as input data - for check before run (+variable)
    unsigned months; //!< number of complete months (daily version only)
    date *calen; //!< calendar of dates of time-series (initialized and deleted together with variables)
    date *calen_mon; //!< calendar of dates of monthly series - daily version only (initialized and deleted together with var_mon)

    enum {P, R, RM, BF, B, DS, DR, PET, ET, SW, SS, GS, INF, PERC, RC, T, H, WEI, POD, POV, PVN, VYP};
    enum {Spa, Dgw, Alf, Dgm, Soc, Wic, Mec, Grd}; //!< Spa must be always the first - only common for daily and monthly
    enum {Spad, Alfd, Dgmd, Socd, Mecd, Grdd};
    enum {I = 5};
    enum {ZIMNI, TANI, LETNI}; //!< modes based on season

    unsigned time_steps; //!< number of time steps
    long double sum_weights; //!< auxiliary for optimization
    std::string input_file; //!< input file name

    bilan_fcd fcd; //!< functoid to be used in optimization

  private:
    bilan_type type; //!< daily or monthly
    bil_state prev_state; //!< previous state to be get or set
    bool water_use; //!< whether to use variables of water use
    double area; //!< catchment area in square kilometers
    bool is_system_optim; //!< whether this catchment will be used for optimization in system
    bool are_chars; //!< if monthly characteristics are calculated and up-to-date
    unsigned ts; //!< current time when running

    long double latitude; //!< latitude for PET estimation

    enum {param_count_daily = 6, param_count_monthly = 8, param_fix_daily = 3, param_fix_monthly = 4, var_count_daily = 18, var_count_monthly = 18, var_count_wat_use = 4}; //!< auxiliary for arrays initialization (without variables for water use)
    enum {veg_zones_count = 6};

    static const unsigned months_in_year = 12; //!< number of months in year (probably will not be changed)
    static const unsigned max_param = 8; //!< max. number of parameters (for daily or monthly)
    static const std::string param_names_daily[]; //!< names of parameters for daily type
    static const double param_init_daily[][param_count_daily]; //!< initial values and limits for daily type

    static const std::string param_names_monthly[]; //!< names of parameters for monthly type
    static const double param_init_monthly[][param_count_monthly]; //!< initial values and limits for monthly type

    static const std::string var_names_daily[]; //!< names of variables for daily type
    static const std::string var_names_monthly[]; //!< names of variables for monthly type

    static const double T_veg_zone[]; //!< temperatures for vegetation zones
    enum {TUNDRA, JEHL, SMIS, LIST, LESOSTEP, STEP}; //!< vegetation zones for PET estimation

    void write_series_file(std::ofstream& out_stream, long double **var, unsigned time_steps); //!< writes daily or monthly time-series into a stream
    void write_chars_file(std::ofstream& out_stream); //!< writes monthly characteristics into a stream

    unsigned init_m; //!< time step of monthly series for the first beginning of hydrological year
    unsigned years; //!< number of complete hydrological years in time-series
};

/**
 *  - returns name of given parameter
 */
inline std::string bilan::get_param_name(unsigned par)
{
  if (par >= par_count)
    throw bil_err("Bad parameter position.");
  if (type == MONTHLY)
    return param_names_monthly[par];
  else
    return param_names_daily[par];
}

/**
 * - returns name of given variable
 */
inline std::string bilan::get_var_name(unsigned var)
{
  if (var >= var_count)
    throw bil_err("Bad variable position.");
  if (type == MONTHLY)
    return var_names_monthly[var];
  else
    return var_names_daily[var];
}

/**
 * - sets given variable to NA
 */
inline void bilan::set_var_na(unsigned var_n)
{
  if (var_n >= var_count)
    throw bil_err("Bad variable position.");

  for (ts = 0; ts < time_steps; ts++)
    var[ts][var_n] = -999;
}

/**
 * - returns true if any of variable value is NA
 */
inline bool bilan::is_var_na(unsigned var_n)
{
  if (var_n >= var_count)
    throw bil_err("Bad variable position.");

  for (unsigned ts = 0; ts < time_steps; ts++) {
    if (var[ts][var_n] < -900)
      return true;
  }
  return false;
}

/**
 * - returns true if variable value is NA
 */
inline bool bilan::is_value_na(unsigned ts, unsigned var_n)
{
  if (var_n >= var_count || ts >= time_steps)
    throw bil_err("Bad variable or time step position.");

  if (var[ts][var_n] < -900)
    return true;
  else
    return false;
}

/**
 * - returns sum of given variable
 */
inline long double bilan::get_var_sum(unsigned var_n)
{
  if (var_n >= var_count)
    throw bil_err("Bad variable position.");

  long double sum = 0;
  for (ts = 0; ts < time_steps; ts++) {
    sum += var[ts][var_n];
  }
  return sum;
}

/**
 * - if given year is leap
 */
inline bool date::is_leap()
{
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

/**
 * - converts flow from mm to cubic meters by using catchment area
 * @param ts time step
 * @param var_n number of variable
 * @return flow in cubic meters per second
 */
inline long double bilan::get_flow_m3s1(unsigned ts, unsigned var_n)
{
  long double flow = var[ts][var_n] * area / 24 / 3.6;
  if (type == MONTHLY)
    flow /= 30;
  return flow;
}

/**
 * - gets optimization criterion value
 * @return criterion value
 */
inline long double bilan::get_optim_ok()
{
  return optim->get_ok();
}

#endif //BIL_MODEL_H_INCLUDED
