#include "bil_model.h"

using namespace std;

const unsigned date::days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //!< number of days in months
const unsigned date::days_in_shortest = 28; //!< number of days in the shortest month
const unsigned date::day_of_year[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; //!< rank of days previous to the first days of months


/**
 * - creates default date: 2000-01-01
 */
date::date()
{
  year = 2000;
  month = 1;
  day = 1;
}

/**
 * - creates date from given values
 * - check if combination of month and day is possible (does not consider leap years)
 */
date::date(unsigned y, unsigned m, unsigned d) : year(y), month(m), day(d)
{
  if (month == 0 || month > 12)
    throw bil_err("Invalid month.");
  else if (day == 0 || day > days_in_month[month - 1])
    throw bil_err("Invalid day.");
}

/**
 * - increases date by 1 day or 1 month
 * - increase by month can produce nonsense dates
 * @param step day or month
 */
void date::increase(step_type step)
{
  switch (step) {
    case DAY:
      if (day == 28 && month == 2 && is_leap()) {
        day = 29;
      }
      else {
        day++;
        if (day > days_in_month[month - 1]) { //-1 due to zero index
          day = 1;
          month++;
          if (month > 12) {
            month = 1;
            year++;
          }
        }
      }
      break;
    case MONTH:
      month++;
      if (month > 12) {
        month = 1;
        year++;
      }
      break;
    default:
      break;
  }
}

/**
 * - decreases date by 1 day or 1 month
 * - decrease by month can produce nonsense dates
 * @param step day or month
 */
void date::decrease(step_type step)
{
  switch (step) {
    case DAY:
      if (day == 1 && month == 3 && is_leap()) {
        day = 29;
        month = 2;
      }
      else {
        day--;
        if (day == 0) {
          month--;
          if (month == 0) {
            month = 12;
            year--;
          }
          day = days_in_month[month - 1]; //-1 due to zero index
        }
      }
      break;
    case MONTH:
      month--;
      if (month == 0) {
        month = 12;
        year--;
      }
      break;
    default:
      break;
  }
}

/**
 * - finds rank of day within year
 * @return day of year
 */
unsigned date::get_day_of_year()
{
  unsigned doy;
  doy = day_of_year[month - 1] + day;

  if (month > 2 && is_leap()) {
    doy++;
  }
  return doy;

}

/**
 * - returns date as a string in yyyy-mm-dd format
 * @return date as a string
 */
string date::to_string()
{
  ostringstream os;
  os << *this;
  return os.str();
}

/**
 * - comparison operator overloaded
 * @param orig_date original date
 */
bool date::operator==(date& orig_date)
{
  if (year != orig_date.year || month != orig_date.month || day != orig_date.day)
    return false;
  else
    return true;
}

/**
 * - comparison operator negated
 * @param orig_date original date
 */
bool date::operator!=(date& orig_date)
{
  return !(*this == orig_date);
}

/**
 * - less than operator overloaded
 * @param orig_date date to be compared
 */
bool date::operator<(date& orig_date)
{
  if (year < orig_date.year)
    return true;
  else if (year == orig_date.year && month < orig_date.month)
    return true;
  else if (year == orig_date.year && month == orig_date.month && day < orig_date.day)
    return true;
  else
    return false;
}

/**
 * - greater than operator overloaded
 * @param orig_date date to be compared
 */
bool date::operator>(date& orig_date)
{
  if (!(*this < orig_date) && *this != orig_date)
    return true;
  else
    return false;
}

/**
 * - prints date in yyyy-mm-dd format
 */
ostream& operator<<(ostream &os, date &d)
{
  os << d.year << "-" << d.month << "-" << d.day;
  return os;
}

/**
 * - creates instance of Bilan state with default zeros
 */
bil_state::bil_state() : calen()
{
  is_active_get = is_active_set = false;
  season = bilan::LETNI;
  ts = 0;
  for (unsigned sv = 0; sv < st_var_count; sv++)
    st_var[sv] = 0;
}

/**
 * - returns init date of time-series in yyyy-mm-dd format
 * @return initial date or "NA" in case of no data
 */
string bilan::get_init_date()
{
  if (time_steps != 0)
    return calen[0].to_string();
  else
    return "NA";
}

/**
 * - returns position (index) of variable given by name in array
 * - an unknown name raises an exception
 * @param var_name variable name
 */
unsigned bilan::get_var_pos(const string& var_name)
{
  for (unsigned v = 0; v < var_count; v++) {
    if (var_name == get_var_name(v)) {
      return v;
    }
  }
  throw bil_err("Unknown variable '" + var_name + "'.");
}

/**
 * - assigns dates of current time-series to calendar
 * - monthly type: days are corrected if initial date is at the end of long month
 * @param init_year initial year
 * @param init_month initial month
 * @param init_day initial day of the serie
 */
void bilan::set_calendar(unsigned init_year, unsigned init_month, unsigned init_day)
{
  date tmp_date = date(init_year, init_month, init_day);
  for (ts = 0; ts < time_steps; ts++) {
    calen[ts] = tmp_date;
    switch (type) {
      case DAILY:
        tmp_date.increase(date::DAY);
        break;
      case MONTHLY:
        tmp_date.increase(date::MONTH);
        break;
      default:
        break;
    }
  }
  //correction for months shorter than month with init_day
  if (type == MONTHLY && init_day > date::days_in_shortest) {
    unsigned curr_days;
    for (ts = 0; ts < time_steps; ts++) {
      curr_days = date::days_in_month[calen[ts].month - 1];
      if (calen[ts].day > curr_days)
        calen[ts].day = curr_days;
    }
  }
}

//!< parameter names and variables for both daily and monthly version
const string bilan::param_names_daily[bilan::param_count_daily] = {"Spa", "Alf", "Dgm", "Soc", "Mec", "Grd"};
const double bilan::param_init_daily[3][bilan::param_count_daily] = {{20, 0.3, 5, 0.3, 0.05, 0.05}, {0, 0, 0, 0, 0, 0}, {200, 1, 200, 1, 1, 0.5}};
const string bilan::var_names_daily[bilan::var_count_daily + bilan::var_count_wat_use] = {"P", "R", "RM", "BF", "B", "DS", "DR", "PET", "ET", "SW","SS", "GS", "INF", "PERC", "RC", "T", "H", "WEI", "POD", "POV", "PVN", "VYP"};
const string bilan::param_names_monthly[bilan::param_count_monthly] = {"Spa", "Dgw", "Alf", "Dgm", "Soc", "Wic", "Mec", "Grd"};
const double bilan::param_init_monthly[3][bilan::param_count_monthly] = {{147.7, 13.8, 0.000779, 15.22, 0.699, 0.342, 0.799, 0.499}, {0, 0, 0, 0, 0, 0, 0, 0}, {200, 20, 0.003, 200, 1, 1, 1, 1}};
const string bilan::var_names_monthly[bilan::var_count_monthly + bilan::var_count_wat_use] = {"P", "R", "RM", "BF", "B", "I", "DR", "PET", "ET", "SW", "SS", "GS", "INF", "PERC", "RC", "T", "H", "WEI", "POD", "POV", "PVN", "VYP"};

/**
 * - creates an empty model instance of given type
 * - initializes its parameters and their limits
 * @param type daily or monthly time step version
 */
bilan::bilan(bilan_type type) : months(0), time_steps(0), ts(0), init_m(0), years(0)
{
  this->type = type;
  water_use = false;
  area = 0;
  is_system_optim = false;
  are_chars = false;
  latitude = 50;
  sum_weights = 0;
  fcd.set(this);

  switch (type) {
    case DAILY:
      par_count = param_count_daily;
      par_fix_count = param_fix_daily;
      var_count = var_count_daily;
      break;
    case MONTHLY:
      par_count = param_count_monthly;
      par_fix_count = param_fix_monthly;
      var_count = var_count_monthly;
      break;
    default:
      break;
  }
  var = 0;
  var_mon = 0;
  var_is_input = 0;
  char_mon = 0;
  calen_mon = 0;
  calen = 0;
  param = 0; //because of delete in init_par
  init_par();

  //default optimization
  optim = new optimizer<bilan_fcd*>();
  optim->set_functoid(&fcd);
}

/**
 * - copy constructor for adding to list of catchments
 */
bilan::bilan(const bilan& orig)
{
  optim = orig.optim->clone(); //calls "virtual copy constructor" depending on optim type

  par_count = orig.par_count;
  par_fix_count = orig.par_fix_count;
  var_count = orig.var_count;
  time_steps = orig.time_steps;
  months = orig.months;

  if (orig.param != 0) {
    param = new parameter[par_count];
    for (unsigned par = 0; par < par_count; par++)
      param[par] = orig.param[par];
  }
  else
    param = 0;

  unsigned v, m;
  if (orig.var != 0) {
    var = new long double*[time_steps];
    for (unsigned ts = 0; ts < time_steps; ts++) {
      var[ts] = new long double[var_count];
      for (v = 0; v < var_count; v++)
        var[ts][v] = orig.var[ts][v];
    }
  }
  else
    var = 0;
  if (orig.var_mon != 0) {
    var_mon = new long double*[months];
    for (m = 0; m < months; m++) {
      var_mon[m] = new long double[var_count];
      for (v = 0; v < var_count; v++)
        var_mon[m][v] = orig.var_mon[m][v];
    }
  }
  else
    var_mon = 0;
  if (orig.char_mon != 0) {
    char_mon = new long double*[months_in_year];
    for (m = 0; m < months_in_year; m++) {
      char_mon[m] = new long double[var_count * 3];
      for (v = 0; v < var_count * 3; v++)
        char_mon[m][v] = orig.char_mon[m][v];
    }
  }
  else
    char_mon = 0;

  if (orig.var_is_input != 0) {
    var_is_input = new bool[var_count];
    for (v = 0; v < var_count; v++)
      var_is_input[v] = orig.var_is_input[v];
  }
  else
    var_is_input = 0;

  if (orig.calen != 0) {
    calen = new date[time_steps];
    for (ts = 0; ts < time_steps; ts++)
      calen[ts] = orig.calen[ts];
  }
  else
    calen = 0;
  if (orig.calen_mon != 0) {
    calen_mon = new date[months];
    for (m = 0; m < months; m++)
      calen_mon[m] = orig.calen_mon[m];
  }
  else
    calen_mon = 0;

  fcd.set(this); //functoid always related to current model
  sum_weights = orig.sum_weights;
  input_file = orig.input_file;
  are_chars = orig.are_chars;
  type = orig.type;
  prev_state = orig.prev_state;
  water_use = orig.water_use;
  area = orig.area;
  is_system_optim = orig.is_system_optim;
  ts = orig.ts;
  latitude = orig.latitude;
  init_m = orig.init_m;
  years = orig.years;
}

/**
 * - assignment operator
 */
bilan& bilan::operator=(const bilan& orig)
{
  if (this != &orig) {
    if (var) {
      for (ts = 0; ts < time_steps; ts++) {
        delete[] var[ts];
      }
      delete[] var;
    }
    unsigned m;
    if (var_mon) {
      for (m = 0; m < months; m++) {
        delete[] var_mon[m];
      }
      delete[] var_mon;
    }
    if (char_mon) {
      for (m = 0; m < months_in_year; m++) {
        delete[] char_mon[m];
      }
      delete[] char_mon;
    }
    delete[] var_is_input;
    delete[] calen_mon;
    delete[] calen;
    delete[] param;
    delete optim;

    optim = orig.optim->clone();
    par_count = orig.par_count;
    par_fix_count = orig.par_fix_count;
    var_count = orig.var_count;
    time_steps = orig.time_steps;
    months = orig.months;

    if (orig.param != 0) {
      param = new parameter[par_count];
      for (unsigned par = 0; par < par_count; par++)
        param[par] = orig.param[par];
    }
    else
      param = 0;

    unsigned v;
    if (orig.var != 0) {
      var = new long double*[time_steps];
      for (unsigned ts = 0; ts < time_steps; ts++) {
        var[ts] = new long double[var_count];
        for (v = 0; v < var_count; v++)
          var[ts][v] = orig.var[ts][v];
      }
    }
    else
      var = 0;
    if (orig.var_mon != 0) {
      var_mon = new long double*[months];
      for (m = 0; m < months; m++) {
        var_mon[m] = new long double[var_count];
        for (v = 0; v < var_count; v++)
          var_mon[m][v] = orig.var_mon[m][v];
      }
    }
    else
      var_mon = 0;
    if (orig.char_mon != 0) {
      char_mon = new long double*[months_in_year];
      for (m = 0; m < months_in_year; m++) {
        char_mon[m] = new long double[var_count * 3];
        for (v = 0; v < var_count * 3; v++)
          char_mon[m][v] = orig.char_mon[m][v];
      }
    }
    else
      char_mon = 0;

    if (orig.var_is_input != 0) {
      var_is_input = new bool[var_count];
      for (v = 0; v < var_count; v++)
        var_is_input[v] = orig.var_is_input[v];
    }
    else
      var_is_input = 0;

    if (orig.calen != 0) {
      calen = new date[time_steps];
      for (ts = 0; ts < time_steps; ts++)
        calen[ts] = orig.calen[ts];
    }
    else
      calen = 0;
    if (orig.calen_mon != 0) {
      calen_mon = new date[months];
      for (m = 0; m < months; m++)
        calen_mon[m] = orig.calen_mon[m];
    }
    else
      calen_mon = 0;

    fcd.set(this);
    sum_weights = orig.sum_weights;
    input_file = orig.input_file;
    are_chars = orig.are_chars;
    type = orig.type;
    prev_state = orig.prev_state;
    water_use = orig.water_use;
    area = orig.area;
    is_system_optim = orig.is_system_optim;
    ts = orig.ts;
    latitude = orig.latitude;
    init_m = orig.init_m;
    years = orig.years;
  }
  return *this;
}

bilan::~bilan()
{
  if (var) {
    for (ts = 0; ts < time_steps; ts++) {
      delete[] var[ts];
    }
    delete[] var;
  }
  unsigned m;
  if (var_mon) {
    for (m = 0; m < months; m++) {
      delete[] var_mon[m];
    }
    delete[] var_mon;
  }
  if (char_mon) {
    for (m = 0; m < months_in_year; m++) {
      delete[] char_mon[m];
    }
    delete[] char_mon;
  }
  delete[] var_is_input;
  delete[] calen_mon;
  delete[] calen;
  delete[] param;
  delete optim;
}

/**
 * - changes model type to the second one and parameter initialization
 */
void bilan::change_type()
{
  if (var) {
    for (ts = 0; ts < time_steps; ts++) {
      delete[] var[ts];
    }
    delete[] var;
  }
  unsigned m;
  if (type == DAILY && var_mon) {
    for (m = 0; m < months; m++) {
      delete[] var_mon[m];
    }
    delete[] var_mon;
  }
  if (char_mon) {
    for (m = 0; m < months_in_year; m++) {
      delete[] char_mon[m];
    }
    delete[] char_mon;
  }
  if (type == DAILY)
    delete[] calen_mon;
  delete[] var_is_input;
  delete[] calen;
  delete[] param;

  var = 0;
  var_mon = 0;
  var_is_input = 0;
  char_mon = 0;
  calen_mon = 0;
  calen = 0;
  time_steps = months = 0;
  are_chars = false;

  switch (type) {
    case MONTHLY:
      par_count = param_count_daily;
      par_fix_count = param_fix_daily;
      if (water_use)
        var_count = var_count_daily + var_count_wat_use;
      else
        var_count = var_count_daily;
      type = DAILY;
      break;
    case DAILY:
      par_count = param_count_monthly;
      par_fix_count = param_fix_monthly;
      if (water_use)
        var_count = var_count_monthly + var_count_wat_use;
      else
        var_count = var_count_monthly;
      type = MONTHLY;
      break;
    default:
      break;
  }
  param = 0;
  init_par();
}

/**
 * - change number of variables according to water use option
 * - reallocates and preserves current values of variables
 * @param water_use whether to use variables of water use
 */
void bilan::set_water_use(bool water_use)
{
  if (this->water_use != water_use) {
    this->water_use = water_use;

    bool *tmp_is_input;
    long double **tmp_var;
    bool empty_var = true;
    unsigned var_count_orig = var_count, v;

    if (var) {
      empty_var = false;
      tmp_is_input = new bool[var_count];
      tmp_var = new long double*[time_steps];
      for (v = 0; v < var_count; v++) {
        tmp_is_input[v] = var_is_input[v];
      }
      for (ts = 0; ts < time_steps; ts++) {
        tmp_var[ts] = new long double[var_count];
        for (v = 0; v < var_count; v++)
          tmp_var[ts][v] = var[ts][v];
        delete[] var[ts];
      }
      delete[] var;
    }
    unsigned m;
    if (type == DAILY && var_mon) {
      for (m = 0; m < months; m++) {
        delete[] var_mon[m];
      }
      delete[] var_mon;
    }
    if (char_mon) {
      for (m = 0; m < months_in_year; m++) {
        delete[] char_mon[m];
      }
      delete[] char_mon;
    }
    delete[] var_is_input;

    var = 0;
    var_mon = 0;
    var_is_input = 0;
    char_mon = 0;

    switch (type) {
      case DAILY:
        par_count = param_count_daily;
        par_fix_count = param_fix_daily;
        if (water_use)
          var_count = var_count_daily + var_count_wat_use;
        else
          var_count = var_count_daily;
        break;
      case MONTHLY:
        par_count = param_count_monthly;
        par_fix_count = param_fix_monthly;
        if (water_use)
          var_count = var_count_monthly + var_count_wat_use;
        else
          var_count = var_count_monthly;
        break;
      default:
        break;
    }
    init_var(time_steps);

    if (!empty_var) {
      unsigned var_count_used;
      if (var_count_orig > var_count)
        var_count_used = var_count;
      else
        var_count_used = var_count_orig;

      for (v = 0; v < var_count_used; v++)
        var_is_input[v] = tmp_is_input[v];
      for (ts = 0; ts < time_steps; ts++) {
        for (v = 0; v < var_count_used; v++)
          var[ts][v] = tmp_var[ts][v];
      }
      for (ts = 0; ts < time_steps; ts++)
        delete[] tmp_var[ts];
      delete[] tmp_var;
      delete[] tmp_is_input;
    }
  }
}

/**
 * - initializes parameters according to model type
 */
void bilan::init_par()
{
  delete[] param;
  param = new parameter[par_count];

  switch (type) {
    case DAILY:
      for (unsigned p = 0; p < par_count; p++) {
        parameter tmp_par(param_init_daily[1][p], param_init_daily[2][p], param_init_daily[0][p], param_init_daily[0][p]);
        param[p] = tmp_par;
      }
      break;
    case MONTHLY:
      for (unsigned p = 0; p < par_count; p++) {
        parameter tmp_par(param_init_monthly[1][p], param_init_monthly[2][p], param_init_monthly[0][p], param_init_monthly[0][p]);
        param[p] = tmp_par;
      }
      break;
    default:
      break;
  }
}

/**
 * - initializes parameters by chosen values
 * - if parameters were not initialized before, not specified one gets initial value
 * @param par_inits initial value of parameters (not specified parameters are not affected)
 * @param par_type initial value, lower limit or upper limit
 */
void bilan::init_par(std::map<std::string, double>& par_inits, param_type par_type)
{
  for (map<string, double>::iterator pi = par_inits.begin(); pi != par_inits.end(); ++pi) {
    unsigned pos = 999;

    switch (type) {
      case DAILY:
        for (unsigned p = 0; p < par_count; p++) {
          if (param_names_daily[p] == pi->first)
            pos = p;
        }
        break;
      case MONTHLY:
        for (unsigned p = 0; p < par_count; p++) {
          if (param_names_monthly[p] == pi->first) {
            pos = p;
          }
        }
        break;
      default:
        break;
    }
    if (pos != 999) {
      switch (par_type) {
        case INIT:
          param[pos].initial = pi->second;
          break;
        case CURR:
          param[pos].value = pi->second;
          break;
        case LOWER:
          param[pos].lower = pi->second;
          break;
        case UPPER:
          param[pos].upper = pi->second;
          break;
        default:
          break;
      }
    }
    else
      BIL_OSTREAM << "Parameter '" << pi->first << "' does not exist in this model.\n";
  }
}

/**
 * - initializes variables and calendar by number of time steps
 * @param new_time_steps number of time steps (days or months) of data
 */
void bilan::init_var(unsigned new_time_steps)
{
  if (var) {
    for (ts = 0; ts < time_steps; ts++) {
      delete[] var[ts];
    }
    delete[] var;
  }

  this->time_steps = new_time_steps;

  var = new long double*[time_steps];
  for (ts = 0; ts < time_steps; ts++) {
    var[ts] = new long double[var_count];
    var[ts][WEI] = 1;
  }
  for (unsigned v = 0; v < var_count; v++) {
    if (v != WEI)
      set_var_na(v);
  }

  delete[] var_is_input;
  var_is_input = new bool[var_count];
  for (unsigned v = 0; v < var_count; v++)
    var_is_input[v] = false;

  delete[] calen;
  calen = new date[time_steps];
  for (ts = 0; ts < time_steps; ts++)
    calen[ts] = date(9999, 1, 1);
}

/**
 * - gets state variables from model run for given date
 * @param init_GS initial groundwater storage
 * @param st_date date for which state is returned
 * @return bil_state instance with volume of storages, season mode and date
 */
bil_state bilan::get_state(long double init_GS, date st_date)
{
  prev_state.calen = st_date;
  if (prev_state.calen < calen[0] || prev_state.calen > calen[time_steps - 1])
    throw bil_err("Date for getting state is out of data period.");

  bool ts_found = false;
  for (ts = 0; ts < time_steps; ts++) {
    if (calen[ts] == prev_state.calen) {
      prev_state.ts = ts;
      ts_found = true;
      break;
    }
  }
  if (!ts_found)
    throw bil_err("Date for getting state is not contained in time series.");

  prev_state.is_active_get = true;
  run(init_GS);
  prev_state.is_active_get = false;

  return prev_state;
}

/**
 * - runs model from given state
 * @param state bil_state with date, season mode and variables
 */
void bilan::run_from_state(const bil_state& state)
{
  prev_state = state;

  //state for last time (time_steps - 1) has no meaning - cannot be used for simulation
  if (prev_state.calen < calen[0] || prev_state.calen > calen[time_steps - 2])
    throw bil_err("Date for setting state is out of data period (or last time in the period).");

  bool ts_found = false;
  for (ts = 0; ts < time_steps - 1; ts++) {
    if (calen[ts] == prev_state.calen) {
      prev_state.ts = ts;
      ts_found = true;
      break;
    }
  }
  if (!ts_found)
    throw bil_err("Date for setting state is not contained in time series.");

  prev_state.is_active_set = true;
  run(0);
  prev_state.is_active_set = false;
}

/**
 * - checks if input variables needed for optimization are loaded
 * @param is_weight_BF whether baseflow will be used for optimization
 */
void bilan::check_vars_for_optim(bool is_weight_BF)
{
  if (!param)
    throw bil_err("Parameters are not initialized for optimization.");
  if (!var_is_input)
    throw bil_err("Input variables for optimization are missing.");
  if (!var_is_input[R])
    throw bil_err("Observed runoff needed for optimization is missing.");
  if (is_weight_BF) {
    if (!var_is_input[B])
      throw bil_err("Observed baseflow needed for optimization is missing.");
  }
}

/**
 * - runs the Bilan model in daily or monthly step
 * @param init_GS initial groundwater storage
 */
void bilan::run(long double init_GS)
{
  if (!var)
    throw bil_err("Variables are not initialized for model run.");
  if (!param)
    throw bil_err("Parameters are not initialized for model run.");
  if (!var_is_input[P] || !var_is_input[T] || !var_is_input[PET])
    throw bil_err("Variables needed for model run are not complete (P, T, PET required).");
  if (water_use && !(var_is_input[POD] && var_is_input[POV] && var_is_input[PVN] && var_is_input[VYP]))
    throw bil_err("Variables of water use needed for model run are not complete (POD, POV, PVN, VYP required).");
  are_chars = false;

  int akt_typ = LETNI; //seasonal mode for the current and previous time step
  int predch_typ;

  double predch_snih, predch_W, predch_DS, predch_RB; //hodnoty pro den - 1, takhle zvlášť kvůli ošetření prvního řádku

  unsigned ts_begin;
  if (prev_state.is_active_set)
    ts_begin = prev_state.ts + 1;
  else
    ts_begin = 0;

  for (ts = ts_begin; ts < time_steps; ts++) {
    //previous state variables and dealing with both the first time and specified state
    if (ts == ts_begin) {
      if (prev_state.is_active_set) {
        predch_typ = prev_state.season;
        predch_snih = prev_state.st_var[bil_state::stSS];
        predch_W = prev_state.st_var[bil_state::stSW];
        predch_DS = prev_state.st_var[bil_state::stDS];
        predch_RB = prev_state.st_var[bil_state::stGS];
      }
      else {
        predch_typ = LETNI;
        predch_snih = 0;
        predch_W = param[Spa].value;
        predch_DS = 0;
        predch_RB = init_GS;
      }
    }
    else {
      predch_typ = akt_typ;
      predch_snih = var[ts - 1][SS];
      predch_W = var[ts - 1][SW];
      predch_RB = var[ts - 1][GS];
      if (type == DAILY)
        predch_DS = var[ts - 1][DS];
      else
        predch_DS = 0;
    }
    //seasonal model, first previous is assumed to be summer
    if (var[ts][T] >= 0) {
      if (predch_typ == ZIMNI || (predch_typ == TANI && predch_snih > 0)) //previous winter => melting, previous melting and snow available => melting
        akt_typ = TANI;
      else //previous summer => summer, previous melting and no snow => summer
        akt_typ = LETNI;
    }
    else
      akt_typ = ZIMNI; //winter for negative temperature

    switch (type) {
      case DAILY:
        switch (akt_typ) {
          case TANI:
            melt_daily(predch_snih);
            winter_balance(predch_W);
            divide_daily(akt_typ, predch_DS, predch_RB);
            break;
          case LETNI:
            summer_balance(predch_W);
            divide_daily(akt_typ, predch_DS, predch_RB);
            break;
          case ZIMNI:
            winter_daily(predch_snih);
            winter_balance(predch_W);
            divide_daily(akt_typ, predch_DS, predch_RB);
            break;
          default:
            break;
        }
        break;
      case MONTHLY:
        switch (akt_typ) {
          case TANI:
            melt_monthly(predch_snih);
            winter_balance(predch_W);
            divide_monthly(akt_typ, predch_RB);
            break;
          case LETNI:
            summer_balance(predch_W);
            divide_monthly(akt_typ, predch_RB);
            break;
          case ZIMNI:
            winter_monthly(predch_snih);
            winter_balance(predch_W);
            divide_monthly(akt_typ, predch_RB);
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    if (prev_state.is_active_get && ts == prev_state.ts) {
      prev_state.season = akt_typ;
      prev_state.st_var[bil_state::stSS] = var[ts][SS];
      prev_state.st_var[bil_state::stSW] = var[ts][SW];
      prev_state.st_var[bil_state::stGS] = var[ts][GS];
      if (type == DAILY) {
        prev_state.st_var[bil_state::stDS] = var[ts][DS];
      }
    }
  }
}

/**
 * - daily Bilan - winter surface balance
 * @param prev_snow snow in previous time step
 */
void bilan::winter_daily(double prev_snow)
{
  var[ts][INF] = 0;
  var[ts][SS] = prev_snow + var[ts][P] - var[ts][PET];
  if (var[ts][SS] < 0) {
    var[ts][SS] = 0;
    var[ts][ET] = prev_snow + var[ts][P];
  }
  else {
    var[ts][ET] = var[ts][PET];
  }
}

/**
 * - monthly Bilan - winter surface balance
 * @param prev_snow snow in previous time step
 */
void bilan::winter_monthly(double prev_snow)
{
  var[ts][DR] = 0;
  var[ts][ET] = var[ts][PET];

  if (var[ts][T] > T_KRIT) {
    double pom_pot = (var[ts][T] - T_KRIT) * param[Dgw].value;
    double pom_akt = prev_snow + var[ts][P] - var[ts][PET];
    if (pom_akt > pom_pot) {
      var[ts][INF] = pom_pot;
      var[ts][SS] = pom_akt - var[ts][INF];
    }
    else {
      var[ts][SS] = 0;
      if (pom_akt > 0)
        var[ts][INF] = pom_akt;
      else {
        var[ts][INF] = 0;
        var[ts][ET] = var[ts][P] + prev_snow;
      }
    }
  }
  else {
    var[ts][SS] = prev_snow + var[ts][P] - var[ts][PET];
    var[ts][INF] = 0;
  }
}

/**
 * - daily Bilan - surface melting
 * @param prev_snow snow in previous time step
 */
void bilan::melt_daily(double prev_snow)
{
  double pom, melt_snow;

  /*co roztaje*/
  pom = var[ts][T] * param[Dgmd].value;
  if (pom >= prev_snow) { /*roztaje vsechno*/
    melt_snow = prev_snow;
    var[ts][SS] = 0;
  }
  else { /*roztaje jen co muze*/
    melt_snow = pom;
    var[ts][SS] = prev_snow - melt_snow;
  }

  /*co se vypaří a infiltruje*/
  if (var[ts][P] > var[ts][PET]) {
    var[ts][INF] = melt_snow + var[ts][P] - var[ts][PET]; /*infiltruje vsechno roztaly a zbytek srazky*/
    var[ts][ET] = var[ts][PET]; /*vypar co to jde, jen ze srazek*/
  }
  else {
    var[ts][INF] = melt_snow; /*vsechen roztaly snih na infiltraci*/
    var[ts][ET] = var[ts][P]; /*veskera srazka na vypar - proc nejde na vypar i snih???*/
  }
}

/**
 * - monthly Bilan - surface melting
 * @param prev_snow snow in previous time step
 */
void bilan::melt_monthly(double prev_snow)
{
  double pom_pot, pom_akt;

  var[ts][DR] = 0;
  var[ts][ET] = var[ts][PET];

  pom_pot = var[ts][T] * param[Dgm].value + var[ts][P];
  pom_akt = prev_snow + var[ts][P] - var[ts][PET];
  if (pom_akt >= pom_pot) {
    var[ts][INF] = pom_pot;
    var[ts][SS] = pom_akt - var[ts][INF];
  }
  else {
    var[ts][SS] = 0;
    if (pom_akt > 0) {
      var[ts][INF] = pom_akt;
    }
    else {
      var[ts][INF] = 0;
      var[ts][ET] = var[ts][P] + prev_snow;
    }
  }
}

/**
 * - Bilan - soil water balance in winter
 * @param prev_W soil storage in previous time step
 */
void bilan::winter_balance(double prev_W)
{
  var[ts][SW] = prev_W + var[ts][INF];
  if (var[ts][SW] >= param[Spa].value) { /*je plno, co je navic, odtece*/
    var[ts][PERC] = var[ts][SW] - param[Spa].value;
    var[ts][SW] = param[Spa].value;
  }
  else /*neni plno a neodtejka*/
    var[ts][PERC] = 0;
}

/**
 * - Bilan - surface and soil water balance in summer
 * @param prev_W soil storage in previous time step
 */
void bilan::summer_balance(double prev_W)
{
  var[ts][SS] = 0;

  switch (type) {
    case DAILY:
      var[ts][DR] = 0.0;
      break;
    case MONTHLY:
      var[ts][DR] = param[Alf].value * pow(var[ts][P], 2) * prev_W / param[Spa].value;
      if (var[ts][DR] > var[ts][P])
        var[ts][DR] = var[ts][P];
      break;
    default:
      break;
  }
  var[ts][INF] = var[ts][P] - var[ts][DR]; /*vsechna srazka infiltruje*/
  if (var[ts][INF] < var[ts][PET]) { /*velka EP, vypari se vsechno, co naprsi, a jeste navic z pudy*/
    var[ts][SW] = prev_W * pow((double)M_E, (double)((var[ts][INF] - var[ts][PET]) / param[Spa].value));
    var[ts][ET] = var[ts][INF] + prev_W - var[ts][SW];
    var[ts][PERC] = 0;
  }
  else {
    var[ts][ET] = var[ts][PET];
    var[ts][SW] = prev_W + var[ts][INF] - var[ts][ET]; /*pudni zasoba se zvetsi o to, co se nevypari*/
    if (var[ts][SW] > param[Spa].value) { /*kdyz je pudni nadrz plna, pretece*/
      var[ts][PERC] = var[ts][SW] - param[Spa].value;
      var[ts][SW] = param[Spa].value;
    }
    else
      var[ts][PERC] = 0;
  }
}

/**
 * - includes withdrawals and release to groundwater storage and total runoff
 */
void bilan::include_water_use()
{
  if (water_use) {
    var[ts][GS] -= var[ts][POD];
    var[ts][RM] -= var[ts][POV] - var[ts][PVN] + var[ts][VYP];
    if (var[ts][GS] < 0)
      var[ts][GS] = 0;
    if (var[ts][RM] < 0)
      var[ts][RM] = 0;
  }
}

/**
 * - daily Bilan - runoff divider for all modes
 * @param mode model mode based on season
 * @param prev_DS direct runoff storage in previous time step
 * @param prev_RB groundwater storage in previous time step
 */
void bilan::divide_daily(int mode, double prev_DS, double prev_RB)
{
  switch (mode) {
    case TANI:
      var[ts][DR] = param[Mecd].value * pow(var[ts][PERC], 2); //část na přímý odtok
      if (var[ts][DR] > var[ts][PERC])
        var[ts][DR] = var[ts][PERC];
      var[ts][RC] = var[ts][PERC] - var[ts][DR]; //část do podzemní vody
      break;
    case LETNI:
      var[ts][DR] = param[Socd].value * pow(var[ts][PERC], 2);
      if (var[ts][DR] > var[ts][PERC])
        var[ts][DR] = var[ts][PERC];
      var[ts][RC] = var[ts][PERC] - var[ts][DR];
      break;
    case ZIMNI:
      var[ts][DR] = 0;  //pro zimní tam nic neteče
      var[ts][RC] = 0;
      break;
    default:
      break;
  }
  if (var[ts][RC] < 0) //aby nebyl záporný přítok a nebralo se z nádrže
    var[ts][RC] = 0;

  var[ts][BF] = param[Grdd].value * prev_RB; //baseflow
  var[ts][GS] = var[ts][RC] + prev_RB - var[ts][BF]; //change in groundwater storage
  var[ts][DS] = var[ts][DR] + (1 - param[Alfd].value) * prev_DS; //change in direct runoff storage
  var[ts][DR] = param[Alfd].value * var[ts][DS];
  var[ts][RM] = var[ts][BF] + var[ts][DR]; //total runoff consisting of baseflow and direct runoff

  include_water_use();
}

/**
 * - monthly Bilan - runoff divider for all modes
 * @param mode model mode based on season
 * @param prev_RB groundwater storage in previous time step
 */
void bilan::divide_monthly(int mode, double prev_RB)
{
  double pom_koef;
  switch (mode) {
    case TANI:
      pom_koef = param[Mec].value;
      break;
    case ZIMNI:
      pom_koef = param[Wic].value;
      break;
    case LETNI:
      pom_koef = param[Soc].value;
      break;
    default:
      throw bil_err("Unknown seasonal mode.");
      break;
  }
  var[ts][RC] = var[ts][PERC] * (1 - pom_koef);
  var[ts][BF] = param[Grd].value * prev_RB;
  var[ts][GS] = var[ts][RC] + prev_RB - var[ts][BF];
  var[ts][I] = pom_koef * var[ts][PERC];
  var[ts][RM] = var[ts][BF] + var[ts][I] + var[ts][DR];

  include_water_use();
}

/**
 * - calculates optimization criterion for observed and modelled runoff and baseflow
 * - NS and LNNS are residuals to 1 (to be minimized)
 * @param crit_type type of optimization criterion
 * @param weight_BF weight for baseflow
 * @param use_weights whether to use weights for time steps of runoff
 * @return value of the criterion
 */
long double bilan::calc_crit_RM_BF(unsigned crit_type, double weight_BF, bool use_weights)
{
  long double ok = calc_crit(crit_type, R, RM, use_weights);

  if (weight_BF > NUMERIC_EPS) {
    ok = (1 - weight_BF) * ok + weight_BF * calc_crit(crit_type, B, BF, use_weights);
  }
  return ok;
}

/**
 * - calculates optimization criterion for observed and modelled runoff, optionally using weights
 * - NS and LNNS are residuals to 1 (to be minimized)
 * @param crit_type type of optimization criterion
 * @param var_obs observed variable
 * @param var_mod modelled variable
 * @param use_weights whether to use weights for time steps of runoff
 * @return value of the criterion
 */
long double bilan::calc_crit(unsigned crit_type, unsigned var_obs, unsigned var_mod, bool use_weights)
{
  long double cit, jmen;
  long double ok = 0, mean = 0; //TDD otestovat NS

  if ((crit_type == optimizer<bilan_fcd*>::NS) || (crit_type == optimizer<bilan_fcd*>::LNNS)) {
    cit = 0;
    jmen = 0;
    mean = 0;

    for (ts = 0; ts < time_steps; ts++) {
      if (crit_type == optimizer<bilan_fcd*>::NS)
        mean = mean + var[ts][var_obs];
      else
        mean = mean + log(var[ts][var_obs]); //natural logarithm
    }
    mean = mean / time_steps;
  }

  long double tmp_weight;
  for (ts = 0; ts < time_steps; ts++) {
    if (use_weights) {
      if (var[ts][WEI] < NUMERIC_EPS && var[ts][WEI] > -NUMERIC_EPS)
        continue;
      tmp_weight = var[ts][WEI] / (sum_weights / time_steps);
    }
    else
      tmp_weight = 1;

    switch (crit_type) {
      case optimizer<bilan_fcd*>::MSE:
        ok = ok + tmp_weight * pow((var[ts][var_obs] - var[ts][var_mod]), 2); //standard error
        break;
      case optimizer<bilan_fcd*>::MAE:
        ok = ok + tmp_weight * abs(var[ts][var_obs] - var[ts][var_mod]); //mean absolute error
        break;
      case optimizer<bilan_fcd*>::MAPE:
        ok = ok + tmp_weight * abs(var[ts][var_obs] - var[ts][var_mod]) / var[ts][var_obs]; //mean absolute percentage error
        break;
      case optimizer<bilan_fcd*>::NS: //Nash-Sutcliffe efficiency
        cit = cit + tmp_weight * pow(var[ts][var_obs] - var[ts][var_mod], 2);
        jmen = jmen + pow(var[ts][var_obs] - mean, 2);
        break;
      case optimizer<bilan_fcd*>::LNNS: //logarithmic Nash-Sutcliffe efficiency
        cit = cit + tmp_weight * pow(log(var[ts][var_obs]) - log(var[ts][var_mod]), 2);
        jmen = jmen + pow(log(var[ts][var_obs]) - mean, 2);
        break;
      default:
        break;
    }
  }

  if ((crit_type == optimizer<bilan_fcd*>::MSE) || (crit_type == optimizer<bilan_fcd*>::MAE) || (crit_type == optimizer<bilan_fcd*>::MAPE))
    ok = ok / time_steps;
  else if ((crit_type == optimizer<bilan_fcd*>::NS) || (crit_type == optimizer<bilan_fcd*>::LNNS))
    ok = cit / jmen;

  if (ok == numeric_limits<long double>::infinity()) //zero modelled value matters for LNNS
    throw bil_err("Optimization criterion value is infinity (probably due to zero observed or modelled value).");

  return ok;
}

/**
 * - calculates sum of weights for optimization
 */
void bilan_fcd::calc_sum_weights()
{
  pbil->sum_weights = pbil->get_var_sum(bilan::WEI);
}

/**
 * - checks if input variables needed for optimization are loaded
 * @param is_weight_BF whether baseflow will be used for optimization
 */
void bilan_fcd::check_vars_for_optim(bool is_weight_BF)
{
  pbil->check_vars_for_optim(is_weight_BF);
}

/**
 * - gets value of parameter
 * @param par_n parameter serial number
 * @param par_type type of parameter
 * @return value of parameter
 */
double bilan_fcd::get_param(unsigned par_n, optimizer_gen<bilan_fcd*>::param_type par_type)
{
  switch (par_type) {
  case parameter::INIT:
      return pbil->param[par_n].initial;
    case parameter::CURR:
      return pbil->param[par_n].value;
    case parameter::LOWER:
      return pbil->param[par_n].lower;
    case parameter::UPPER:
      return pbil->param[par_n].upper;
    default:
      throw bil_err("Undefined parameter type.");
      break;
  }
}

/**
 * - sets value of parameter
 * @param par_n parameter serial number
 * @param par_type type of parameter
 * @param value value to be set
 */
void bilan_fcd::set_param(unsigned par_n, optimizer_gen<bilan_fcd*>::param_type par_type, double value)
{
  switch (par_type) {
    case parameter::INIT:
      pbil->param[par_n].initial = value;
      break;
    case parameter::CURR:
      pbil->param[par_n].value = value;
      break;
    case parameter::LOWER:
      pbil->param[par_n].lower = value;
      break;
    case parameter::UPPER:
      pbil->param[par_n].upper = value;
      break;
    default:
      break;
  }
}

/**
 * - gets number of model parameters
 * @return number of parameters
 */
unsigned bilan_fcd::get_param_count()
{
  return pbil->par_count;
}

/**
 * - gets number of fixed parameters in model
 * @return number of fixed parameters
 */
unsigned bilan_fcd::get_param_fix_count()
{
  return pbil->par_fix_count;
}

/**
 * - gets parameter name
 * @param par_n parameter number
 */
std::string bilan_fcd::get_param_name(unsigned par_n)
{
  return pbil->get_param_name(par_n);
}

/**
 * - runs the model
 * @param init_GS initial groundwater storage
 */
void bilan_fcd::run(long double init_GS)
{
  pbil->run(init_GS);
}

/**
 * - calculates optimization criterion value
 * @param crit criterion type
 * @param weight_BF weight for baseflow
 * @param use_weights whether to use weights
 * @return criterion value
 */
long double bilan_fcd::calc_crit(unsigned crit, double weight_BF, bool use_weights)
{
  return pbil->calc_crit_RM_BF(crit, weight_BF, use_weights);
}
