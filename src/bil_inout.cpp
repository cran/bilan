#include "bil_model.h"

using namespace std;

/**
 * - creates instance of file info with zeros
 */
input_file_info::input_file_info()
{
  nrow = nrow_blank = ncol = 0;
  old_style = false;
}

/**
 * - reads header from input file
 * @param file_name name of file
 */
void input_file_info::read_header(string file_name)
{
  ifstream in_stream(file_name.c_str());
  if (!in_stream) {
    throw bil_err("The input file '" + file_name + "' does not exist.");
  }

  string tmp;
  for (unsigned r = 0; r < 4; r++) {
    getline(in_stream, rows[r]);
  }
  nrow = 4; //to get number of rows
  nrow_blank = 0; //blank lines to be skipped
  while(getline(in_stream, tmp)) {
    if (tmp.find_first_not_of(" \t\r\n") == string::npos)
      nrow_blank++;
    else
      nrow++;
  }

  //check if file is old-style formatted (3 rows of header) or not (1 row - date header)
  old_style = false;
  stringstream st_stream;
  list<string> tmp_row;
  st_stream << rows[1];
  while (st_stream >> tmp)
    tmp_row.push_back(tmp);
  if (tmp_row.size() == 1) {
    old_style = true;
    BIL_OSTREAM << "The input file '" << file_name << "' is old-style formatted.\n";
  }
  //count number of columns
  st_stream.clear();
  st_stream << rows[3];
  tmp_row.clear();
  ncol = 0;
  while (st_stream >> tmp)
    ncol++;

  in_stream.close();
}

/**
 * - reads observed data from text file
 * - the first row is initial date, optionally followed by catchemnt area, old-style format also allowed
 * - standalone year means begin of hydrological year
 * @param file_name name of file
 * @param input_var array of input variables
 * @param input_var_count effective length of input_var (cannot be greater than number of columns in file)
 */
void bilan::read_file(string file_name, unsigned input_var[], unsigned input_var_count)
{
  input_file_info info;
  info.read_header(file_name);

  //check if input_var count correct
  if (input_var_count > info.ncol)
    throw bil_err("Number of columns in file '" + file_name + "' is less than number of input variables.");
  else if (input_var_count < info.ncol)
    BIL_OSTREAM << "The input file '" << file_name << "' contains more columns than input variables, some columns will be omitted.\n";

  //check if there are duplicates in the list of variables
  vector<unsigned> tmp_input_var(input_var, input_var + input_var_count);
  sort(tmp_input_var.begin(), tmp_input_var.end());
  for (unsigned v = 0; v < input_var_count - 1; v++) {
    if (tmp_input_var[v] == tmp_input_var[v + 1])
      BIL_OSTREAM << "File '" << file_name << "': Variable " << get_var_name(tmp_input_var[v]) << " is set for more columns, only the last one will be used.\n";
  }

  //check if matches real counts and counts in header
  stringstream st_stream;
  if (info.old_style) {
    unsigned header_nrow, header_ncol;
    st_stream << info.rows[0];
    st_stream >> header_nrow;
    st_stream.clear();
    st_stream << info.rows[1];
    st_stream >> header_ncol;
    if (info.nrow - 3 != header_nrow)
      BIL_OSTREAM << "File '" << file_name << "': Number of rows (" << info.nrow - 3 << ") does not equal to number in header (" << header_nrow << ").\n";
    if (info.ncol != header_ncol)
      BIL_OSTREAM << "File '" << file_name << "': Number of columns (" << info.ncol << ") does not equal to number in header (" << header_ncol << ").\n";
  }
  //date handling
  string tmp_str_date;
  if (info.old_style)
    tmp_str_date = info.rows[2];
  else
    tmp_str_date = info.rows[0];
  st_stream.clear();
  st_stream << tmp_str_date;
  list<string> first_line;
  string tmp_string;
  unsigned num_read_count = 0;
  while (num_read_count < 4 && st_stream >> tmp_string) { //max. 4 numbers are read
    first_line.push_back(tmp_string);
    num_read_count++;
  }
  //last number catchment area if the fourth or decimal at any last place
  string last_number = first_line.back();
  if (first_line.size() == 4 || last_number.find_first_of('.') != string::npos) {
    st_stream.clear();
    st_stream.str(last_number);
    st_stream >> area;
    first_line.pop_back();
  }
  //string list to unsigned for dates
  list<unsigned> tmp_date;
  unsigned tmp_number;
  for (list<string>::iterator fli = first_line.begin(); fli != first_line.end(); ++fli) {
    st_stream.clear();
    st_stream.str(*fli);
    st_stream >> tmp_number;
    tmp_date.push_back(tmp_number);
  }

  date init_date;
  try {
    switch (tmp_date.size()) {
      case 1:
        init_date = date(tmp_date.front() - 1, 11, 1); //standalone year means begin of hydrological year
        break;
      case 2:
        init_date = date(tmp_date.front(), tmp_date.back(), 1);
        break;
      case 3: {
          list<unsigned>::iterator ui = tmp_date.begin();
          ++ui;
          init_date = date(tmp_date.front(), *ui, tmp_date.back());
        }
        break;
      default:
        throw bil_err("Invalid date format.");
        break;
    }
  }
  catch (bil_err &error) {
    throw bil_err("File '" + file_name + "': " + error.descr);
  }
  //read data
  ifstream in_stream(file_name.c_str());
  if (!in_stream) {
    throw bil_err("The input file '" + file_name + "' does not exist.");
  }
  unsigned header = 1;
  if (info.old_style)
    header = 3;
  for (unsigned r = 0; r < header; r++) {
    getline(in_stream, info.rows[r]);
  }
  info.nrow = info.nrow - header;

  for (unsigned v = 0; v < input_var_count; v++) {
    if (input_var[v] == POD || input_var[v] == POV || input_var[v] == PVN || input_var[v] == VYP) {
      set_water_use(true);
      break;
    }
  }
  init_var(info.nrow);
  set_calendar(init_date.year, init_date.month, init_date.day);

  long double tmp_double;
  string curr_row;
  unsigned col, r_eff = 0;
  for (unsigned r = 0; r < info.nrow + info.nrow_blank; r++) {
    getline(in_stream, curr_row);
    st_stream.clear(); //no error flag
    st_stream.str(""); //to be empty
    st_stream << curr_row;

    if (curr_row.find_first_not_of(" \t\r\n") != string::npos) {
      for (col = 0; col < input_var_count; col++) {
        if (!(st_stream >> tmp_double))
          throw bil_err("File '" + file_name + "': Incomplete line found:\n" + curr_row);
        var[r_eff][input_var[col]] = tmp_double;
      }
      r_eff++;
    }
  }
  in_stream.close();

  input_file = file_name;
  are_chars = false;
  for (col = 0; col < input_var_count; col++)
    var_is_input[input_var[col]] = true;

  if (info.nrow_blank > 0)
    BIL_OSTREAM << "File '" << file_name << "': " << info.nrow_blank << " blank lines skipped.\n";
}

/**
 * - reads parameters from output file
 * @param file_name name of file
 */
void bilan::read_params_file(string file_name)
{
  ifstream in_stream(file_name.c_str());
  if (!in_stream) {
    throw bil_err("The input file '" + file_name + "' does not exist.");
  }

  unsigned r, p;
  string tmp, tmp_line;
  for (r = 0; r < 15; r++) {
    getline(in_stream, tmp);
  }
  //remove end of line and quotes
  tmp = tmp.substr(1, 33);

  map<string, double> par_nam_val;
  if (tmp == "Resulting parameters of the model") { //old style
    const unsigned ALL_PARAMS = 8;
    string *tmp_names = new string[ALL_PARAMS];
    stringstream st_stream, st_stream_val;

    getline(in_stream, tmp_line);
    replace(tmp_line.begin(), tmp_line.end(), ',', ' ');
    st_stream.str(tmp_line);
    for (p = 0; p < ALL_PARAMS; p++) {
      st_stream >> tmp;
      tmp_names[p] = tmp.substr(1, 3); //without quotes
    }
    getline(in_stream, tmp_line);
    replace(tmp_line.begin(), tmp_line.end(), ',', ' ');
    st_stream_val.str(tmp_line);
    double *tmp_values = new double[ALL_PARAMS];
    for (p = 0; p < ALL_PARAMS; p++) {
      st_stream_val >> tmp_values[p];
      //for daily type, all parameters were outputed, but Dgw and Wic have no meaning
      if (!(type == DAILY && (tmp_names[p] == "Dgw" || tmp_names[p] == "Wic"))) {
        par_nam_val.insert(make_pair(tmp_names[p], tmp_values[p]));
      }
    }
    delete[] tmp_values;
    delete[] tmp_names;
  }
  else {
    in_stream.clear();
    in_stream.seekg(0, ios::beg);

    //check if correct model type
    for (r = 0; r < par_count + 5; r++) {
      getline(in_stream, tmp);
    }
    if (tmp.substr(0, 3) != "OK\t")
      throw bil_err("Parameters loaded from file '" + file_name + "' do not match model type.");

    in_stream.clear();
    in_stream.seekg(0, ios::beg);

    //skip initial date
    for (r = 0; r < 3; r++) {
      getline(in_stream, tmp);
    }
    double tmp_par;
    for (p = 0; p < par_count; p++) {
      in_stream >> tmp >> tmp_par;
      par_nam_val.insert(make_pair(tmp, tmp_par));
    }
  }
  in_stream.close();

  init_par(par_nam_val, INIT);
  init_par(par_nam_val, CURR);
}

/**
 * - calculates monthly variables for daily type
 * - number of months found in calendar, only complete months used
 */
void bilan::calc_var_mon()
{
  unsigned m;
  if (var_mon) {
    for (m = 0; m < months; m++) {
      delete[] var_mon[m];
    }
    delete[] var_mon;
  }
  delete[] calen_mon;
  calen_mon = 0;

  //number of months from calendar
  date init_date, last_date, eff_init_date, eff_last_date;
  unsigned ts_init, ts_last;
  init_date = eff_init_date = calen[0];
  last_date = eff_last_date = calen[time_steps - 1];

  months = (last_date.year - init_date.year - 1) * 12 + last_date.month + (13 - init_date.month);
  if (init_date.day != 1) {
    months--;
    eff_init_date.increase(date::MONTH);
    eff_init_date.day = 1;
  }
  if (last_date < eff_init_date)
    throw bil_err("Too short time-series to calculate monthly values of variables.");

  ts_init = 0;
  while (calen[ts_init] != eff_init_date)
    ts_init++;

  date tmp_date = last_date;
  tmp_date.increase(date::DAY);
  if (tmp_date.day != 1) {
    months--;
    eff_last_date.day = 1;
    eff_last_date.decrease(date::DAY);
  }
  if (eff_last_date < eff_init_date)
    throw bil_err("Too short time-series to calculate monthly values of variables.");

  ts_last = time_steps - 1;
  while (calen[ts_last] != eff_last_date)
    ts_last--;

  var_mon = new long double*[months];
  calen_mon = new date[months];
  for (m = 0; m < months; m++) {
    var_mon[m] = new long double[var_count];
    for (unsigned v = 0; v < var_count; v++)
      var_mon[m][v] = 999;
  }

  long double sum;
  unsigned tmp_m;
  for (unsigned v = 0; v < var_count; v++) {
    ts = ts_init;
    for (m = 0; m < months; m++) {
      tmp_m = calen[ts].month;
      sum = 0;
      while (ts < ts_last + 1 && calen[ts].month == tmp_m) {
        sum += var[ts][v];
        ts++;
      }
      calen_mon[m] = calen[ts - 1];
      calen_mon[m].day = 1;

      if (v == T || v == H || v == SW || v == SS || v == GS || v == DS)
        var_mon[m][v] = sum / calen[ts - 1].day;
      else
        var_mon[m][v] = sum;
    }
  }
}

/**
 * - calculates number of complete hydrological years and initial time step init_m
 * - for daily model var_mon series must be calculated before
 * @param calen calendar with monthly dates (calen for monthly and calen_mon for daily type)
 * @param months total number of months (time_steps or months)
 */
void bilan::calc_years_count(date *calen, unsigned months)
{
  const unsigned begin_hydrol_year = 11;
  bool no_year = false;

  //to exclude incomplete hydrological years
  init_m = 0;
  while (calen[init_m].month != begin_hydrol_year) {
    if (init_m == months - 1) { //beginning is not reached till end of time-serie
      no_year = true;
      break;
    }
    init_m++;
  }

  unsigned last_m = months - 1;
  if (!no_year) {
    const unsigned end_hydrol_year = 10;
    while (calen[last_m].month != end_hydrol_year) {
      if (last_m == 0) { //end is not reached till beginning of time-serie
        no_year = true;
        break;
      }
      last_m--;
    }
  }
  if (no_year)
    years = 0;
  else
    years = (last_m - init_m + 1) / months_in_year;
}

/**
 * - calculates monthly characteristics for all variables
 * - number of years must be calculated (calc_years_count)
 * - because characteristics calculated from data of complete hydrological years only
 * - for daily model var_mon series must be calculated before
 * - attributes years and init_m used
 * - resulting chars stored in char_mon
 */
void bilan::calc_char_mon()
{
  unsigned m;
  if (char_mon) {
    for (m = 0; m < months_in_year; m++) {
      delete[] char_mon[m];
    }
    delete[] char_mon;
  }
  char_mon = 0;

  const unsigned months_in_year = 12;
  long double tmp_value;
  long double **var_ser = 0;

  switch (type) {
    case DAILY:
      var_ser = var_mon;
      break;
    case MONTHLY:
      var_ser = var;
      break;
    default:
      break;
  }

  unsigned v, y;

  char_mon = new long double*[months_in_year];
  for (m = 0; m < months_in_year; m++) {
    char_mon[m] = new long double[var_count * 3]; //min, mean, max
    for (v = 0; v < var_count; v++) {
      char_mon[m][v * 3] = 999999; //min
      char_mon[m][v * 3 + 1] = 0; //mean
      char_mon[m][v * 3 + 2] = -999999; //max
    }
  }

  for (v = 0; v < var_count; v++) {
    for (y = 0; y < years; y++) {
      for (m = 0; m < months_in_year; m++) {
        tmp_value = var_ser[init_m + y * 12 + m][v];
        if (tmp_value < char_mon[m][v * 3])
          char_mon[m][v * 3] = tmp_value;
        if (tmp_value > char_mon[m][v * 3 + 2])
          char_mon[m][v * 3 + 2] = tmp_value;
        char_mon[m][v * 3 + 1] += tmp_value;
      }
    }
    for (m = 0; m < months_in_year; m++) {
      if (years == 0) {
        char_mon[m][v * 3] = 0;
        char_mon[m][v * 3 + 1] = 0;
        char_mon[m][v * 3 + 2] = 0;
      }
      else {
        char_mon[m][v * 3 + 1] /= years;
      }
    }
  }
  if (years == 0)
    BIL_OSTREAM << "Too short time-series to calculate monthly chars (set to 0)." << endl;
}

/**
 * - calculates monthly characteristics completely
 * - nothing needs to be calculated before
 * - no calculation in case there are up-to-date chars
 * - wrapper for calc_var_mon, calc_years_count and calc_char_mon
 */
void bilan::calc_chars()
{
  if (!are_chars) {
    if (type == DAILY) {
      calc_var_mon(); //recalculation
      calc_years_count(calen_mon, months);
    }
    else {
      calc_years_count(calen, time_steps);
    }
    calc_char_mon();
    are_chars = true;
  }
}

/**
 * - writes resulting time-series of variables into specified stream
 * @param out_stream output stream
 * @param var time-series of variables (daily or monhtly)
 * @param time_steps number of days or months
 */
void bilan::write_series_file(ofstream& out_stream, long double **var, unsigned time_steps)
{
  unsigned v;
  out_stream << "\n\n";
  for (v = 0; v < var_count; v++) {
    if (v != 0)
      out_stream << "\t";
    out_stream << get_var_name(v);
  }

  for (ts = 0; ts < time_steps; ts++) {
    out_stream << "\n";
    for (v = 0; v < var_count; v++) {
      if (v != 0)
        out_stream << "\t";
      if (is_value_na(ts, v))
        out_stream << "NA";
      else
        out_stream << static_cast<double>(var[ts][v]);
    }
  }
}

/**
 * - writes resulting monthly characteristics into text file
 * - calc_char_mon() must be called before
 * @param out_stream output stream
 */
void bilan::write_chars_file(ofstream& out_stream)
{
  unsigned m, v;
  out_stream << "\n\n";

  for (v = 0; v < var_count; v++) {
    out_stream << get_var_name(v) << "\n";
    for (m = 0; m < months_in_year; m++) {
      if (m < 2)
        out_stream << m + 11;
      else
        out_stream << m - 1;

      if (is_var_na(v))
        out_stream << "\tNA\tNA\tNA\n";
      else
        out_stream << "\t" << static_cast<double>(char_mon[m][v * 3]) << "\t"  << static_cast<double>(char_mon[m][v * 3 + 1]) << "\t" << static_cast<double>(char_mon[m][v * 3 + 2]) << "\n";
    }
    out_stream << "\n";
  }
}

/**
 * - writes results with monthly time-series of variables into text file
 * - for daily calculates monthly series from daily
 * @param file_name name of output file
 * @param out_type type of output file - daily, monthly or default series or characteristics
 */
void bilan::write_file(string file_name, output_type out_type)
{
  if (!var)
    throw bil_err("No variables to output.");
  if (!param)
    throw bil_err("No parameters to output.");

  ofstream out_stream(file_name.c_str());
  if (!out_stream) {
    throw bil_err("The output file '" + file_name + "' cannot be used.");
  }
  out_stream << "Initial\n" << calen[0] << "\n";
  for (unsigned p = 0; p < par_count; p++)
    out_stream << "\n" << get_param_name(p) << "\t" << param[p].value;

  out_stream << "\n\nOK\t" << static_cast<double>(get_optim_ok());

  switch (out_type) {
    case SERIES:
      write_series_file(out_stream, var, time_steps);
      break;
    case SERIES_DAILY:
      if (type == DAILY)
        write_series_file(out_stream, var, time_steps);
      else
        throw bil_err("Daily series cannot be written for monthly type of model.");
      break;
    case SERIES_MONTHLY:
      if (type == DAILY) {
        calc_var_mon(); //recalculation - results may be changed in a run before
        write_series_file(out_stream, var_mon, months);
      }
      else
        write_series_file(out_stream, var, time_steps);
      break;
    case CHARS:
      calc_chars();
      write_chars_file(out_stream);
      break;
    default:
      break;
  }
  out_stream.close();
}
