#include "bilan-r.h"

using namespace std;
using namespace Rcpp;

//all functions must return SEXP
RcppExport SEXP new_bil(SEXP bil_type)
{
  string mod_type = as<string>(bil_type);
  bilan::bilan_type type;

  if (mod_type == "D" || mod_type == "d")
    type = bilan::DAILY;
  else if (mod_type == "M" || mod_type == "m")
    type = bilan::MONTHLY;
  else
    return wrap("Unknown model type.");

  bilan *bil = new bilan(type);
  bil->optim = new optimizer<bilan_fcd*>();
  bil->optim->set_functoid(&bil->fcd);

  //second argument must be FALSE
  XPtr<bilan> model_ptr(bil, false);

  return model_ptr;
}

RcppExport SEXP clone_model(SEXP orig_model_ptr)
{
  XPtr<bilan> bil_orig(orig_model_ptr);
  bilan *bil = new bilan(bil_orig->get_type());
  *bil = *bil_orig;
  XPtr<bilan> model_ptr(bil, false);

  return model_ptr;
}

void check_var_water_use(bilan *bil, StringVector var_names)
{
  string var_name;
  unsigned var_count = var_names.size();
  for (unsigned v = 0; v < var_count; v++) {
    var_name = var_names[v];
    if (var_name == "POD" || var_name == "POV" || var_name == "PVN" || var_name == "VYP") {
      bil->set_water_use(true);
      break;
    }
  }
}

RcppExport SEXP read_file(SEXP model_ptr, SEXP file_name, SEXP Rinput_vars)
{
  XPtr<bilan> bil(model_ptr);
  string file = as<string>(file_name);
  string err = "";
  StringVector input_vars = as<StringVector>(Rinput_vars);
  unsigned input_var_count = input_vars.size();
  unsigned *input_var_pos = new unsigned[input_var_count];

  try {
    check_var_water_use(bil, input_vars);
    string var_name;
    for (unsigned v = 0; v < input_var_count; v++) {
      var_name = input_vars[v];
      input_var_pos[v] = bil->get_var_pos(var_name);
    }
    std::streambuf* Rcout_buf = BIL_OSTREAM.rdbuf(); //save buffer
    std::ostringstream oss;
    BIL_OSTREAM.rdbuf(oss.rdbuf()); //redirect Rcout to string

    bil->read_file(file, input_var_pos, input_var_count);

    BIL_OSTREAM.rdbuf(Rcout_buf); //restore original buffer
    if (oss.str() != "") {
      Environment base("package:base");
      Function warning = base["warning"];
      warning(oss.str());
    }
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  delete[] input_var_pos;
  return wrap(err);
}

RcppExport SEXP write_file(SEXP model_ptr, SEXP file_name, SEXP Rtype)
{
  XPtr<bilan> bil(model_ptr);
  string file = as<string>(file_name);
  string err = "";
  bilan::output_type type = static_cast<bilan::output_type>(as<unsigned>(Rtype));
  try {
    std::streambuf* Rcout_buf = BIL_OSTREAM.rdbuf();
    std::ostringstream oss;
    BIL_OSTREAM.rdbuf(oss.rdbuf());

    bil->write_file(file, type);

    BIL_OSTREAM.rdbuf(Rcout_buf);
    if (oss.str() != "") {
      Environment base("package:base");
      Function warning = base["warning"];
      warning(oss.str());
    }
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP set_input_vars(SEXP model_ptr, SEXP Rinput_vars, SEXP Rinit_date, SEXP Rappend)
{
  XPtr<bilan> bil(model_ptr);

  DataFrame input_vars = as<DataFrame>(Rinput_vars);
  Date init_date(Rinit_date);
  bool append = as<bool>(Rappend);

  unsigned var_count = input_vars.size();
  StringVector var_names = input_vars.names();
  StringVector tmp_first_col = input_vars[0];
  unsigned nrow = tmp_first_col.size();

  string err = "";
  try {
    check_var_water_use(bil, var_names);
    if (append) {
      if (nrow != bil->time_steps)
        throw bil_err("Different number of time steps in original data and data to be appended.");
    }
    else {
      bil->init_var(nrow);
      bil->set_calendar(init_date.getYear(), init_date.getMonth(), init_date.getDay());
    }

    string var_name;
    unsigned var_pos;
    for (unsigned c = 0; c < var_count; c++) {
      var_name = var_names[c];
      try {
        var_pos = bil->get_var_pos(var_name);

        NumericVector tmp_col = input_vars[c];
        for (unsigned r = 0; r < nrow; r++) {
          bil->var[r][var_pos] = tmp_col[r];
        }
        bil->var_is_input[var_pos] = true;
      }
      catch (bil_err &error) {
        Environment base("package:base");
        Function warning = base["warning"];
        warning(error.descr + " Omitted.\n");
      }
    }
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP set_area(SEXP model_ptr, SEXP Rarea)
{
  XPtr<bilan> bil(model_ptr);
  double area = as<double>(Rarea);
  bil->set_area(area);
  return wrap(0);
}

RcppExport SEXP copy_params(SEXP model_ptr, SEXP orig_model_ptr)
{
  XPtr<bilan> bil(model_ptr);
  XPtr<bilan> bil_orig(orig_model_ptr);

  string err = "";
  try {
    if (bil->par_count != bil_orig->par_count)
      throw bil_err("Number of parameters in the model to be set is different.");

    for (unsigned p = 0; p < bil_orig->par_count; p++)
      bil->param[p] = bil_orig->param[p];
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP set_params(SEXP model_ptr, SEXP Rpar_names, SEXP Rpar_values, SEXP Rtype)
{
  XPtr<bilan> bil(model_ptr);

  vector<string> par_names = as<vector<string > >(Rpar_names);
  vector<double> par_values = as<vector<double > >(Rpar_values);
  map<string, double> par_nam_val;

  unsigned p, par_count = par_names.size();
  for (p = 0; p < par_count; p++) {
    par_nam_val.insert(make_pair(par_names[p], par_values[p]));
  }

  bilan::param_type type = static_cast<bilan::param_type>(as<unsigned>(Rtype));

  string err = "";
  try {
    std::streambuf* Rcout_buf = BIL_OSTREAM.rdbuf();
    std::ostringstream oss;
    BIL_OSTREAM.rdbuf(oss.rdbuf());

    bil->init_par(par_nam_val, type);

    BIL_OSTREAM.rdbuf(Rcout_buf);
    if (oss.str() != "") {
      Environment base("package:base");
      Function warning = base["warning"];
      warning(oss.str());
    }
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP read_params_file(SEXP model_ptr, SEXP file_name)
{
  XPtr<bilan> bil(model_ptr);
  string file = as<string>(file_name);

  string err = "";
  try {
    std::streambuf* Rcout_buf = BIL_OSTREAM.rdbuf();
    std::ostringstream oss;
    BIL_OSTREAM.rdbuf(oss.rdbuf());

    bil->read_params_file(file);

    BIL_OSTREAM.rdbuf(Rcout_buf);
    if (oss.str() != "") {
      Environment base("package:base");
      Function warning = base["warning"];
      warning(oss.str());
    }
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP copy_optim(SEXP model_ptr, SEXP orig_model_ptr)
{
  XPtr<bilan> bil(model_ptr);
  XPtr<bilan> bil_orig(orig_model_ptr);

  string err = "";
  try {
    if (bil->optim->get_type() != bil_orig->optim->get_type()) {
      delete bil->optim;
      if (bil_orig->optim->get_type() == optimizer_gen<bilan_fcd*>::BS) {
        bil->optim = new optimizer<bilan_fcd*>();
        bil->optim->set_functoid(&bil->fcd);
      }
      else {
        bil->optim = new DE_optim<bilan_fcd*>();
        bil->optim->set_functoid(&bil->fcd);
      }
    }
    *(bil->optim) = *(bil_orig->optim);
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP set_optim(SEXP model_ptr, SEXP Rcrit_part1, SEXP Rcrit_part2, SEXP Rweight_BF, SEXP Rmax_iter, SEXP Rinit_GS, SEXP Ruse_weights)
{
  XPtr<bilan> bil(model_ptr);

  unsigned crit_part1 = as<unsigned>(Rcrit_part1);
  unsigned crit_part2 = as<unsigned>(Rcrit_part2);
  double weight_BF = as<double>(Rweight_BF);
  unsigned max_iter = as<unsigned>(Rmax_iter);
  long double init_GS = as<long double>(Rinit_GS);
  bool use_weights = as<bool>(Ruse_weights);

  string err = "";
  try {
    delete bil->optim;
    optimizer<bilan_fcd*> tmp_optim;
    tmp_optim.set_functoid(&bil->fcd);
    tmp_optim.set(crit_part1, crit_part2, weight_BF, use_weights, max_iter, init_GS);
    bil->optim = new optimizer<bilan_fcd*>();
    *(bil->optim) = tmp_optim;
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: Optimization was not set - " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP set_DE_optim(SEXP model_ptr, SEXP Rcrit, SEXP RDE_type, SEXP Rn_comp, SEXP Rcomp_size, SEXP Rcross, SEXP Rmutat_f, SEXP Rmutat_k,SEXP Rmaxn_shuffles, SEXP Rn_gen_comp, SEXP Rens_count, SEXP Rseed, SEXP Rweight_BF, SEXP Rinit_GS, SEXP Ruse_weights)
{
  XPtr<bilan> bil(model_ptr);

  unsigned crit = as<unsigned>(Rcrit);
  unsigned DE_type = as<unsigned>(RDE_type);
  unsigned n_comp = as<unsigned>(Rn_comp);
  unsigned comp_size = as<unsigned>(Rcomp_size);
  double cross = as<double>(Rcross);
  double mutat_f = as<double>(Rmutat_f);
  double mutat_k = as<double>(Rmutat_k);
  unsigned maxn_shuffles = as<unsigned>(Rmaxn_shuffles);
  unsigned n_gen_comp = as<unsigned>(Rn_gen_comp);
  unsigned ens_count = as<unsigned>(Rens_count);
  int seed = as<int>(Rseed);
  double weight_BF = as<double>(Rweight_BF);
  long double init_GS = as<long double>(Rinit_GS);
  bool use_weights = as<bool>(Ruse_weights);

  string err = "";
  try {
    delete bil->optim;
    DE_optim<bilan_fcd*> tmp_optim;
    tmp_optim.set_functoid(&bil->fcd);
    tmp_optim.set(crit, DE_type, n_comp, comp_size, cross, mutat_f, mutat_k, maxn_shuffles, n_gen_comp, ens_count, seed, weight_BF, use_weights, init_GS);
    bil->optim = new DE_optim<bilan_fcd*>();
    *(bil->optim) = tmp_optim;
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: Optimization was not set - " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP pet(SEXP model_ptr, SEXP type_pet, SEXP latit)
{
  XPtr<bilan> bil(model_ptr);
  string type = as<string>(type_pet);
  long double latitude = as<long double>(latit);
  string err;

  try {
    if (type == "tab")
      bil->pet_estim_tab();
    else if (type == "latit")
      bil->pet_estim_latit(latitude);
    else
      throw bil_err("Unknown type of PET estimation.");
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP run(SEXP model_ptr, SEXP Rinit_GS)
{
  XPtr<bilan> bil(model_ptr);
  long double init_GS = as<long double>(Rinit_GS);
  string err;

  try {
    bil->run(init_GS);
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP optimize(SEXP model_ptr)
{
  XPtr<bilan> bil(model_ptr);
  string err;

  try {
    bil->optim->optimize();
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP get_params(SEXP model_ptr)
{
  XPtr<bilan> bil(model_ptr);

  DataFrame params;

  vector<double> tmp_par_curr(bil->par_count), tmp_par_low(bil->par_count);
  vector<double> tmp_par_upp(bil->par_count), tmp_par_init(bil->par_count);
  vector<string> tmp_par_name(bil->par_count);
  for (unsigned p = 0; p < bil->par_count; p++) {
    tmp_par_curr[p] = bil->param[p].value;
    tmp_par_low[p] = bil->param[p].lower;
    tmp_par_upp[p] = bil->param[p].upper;
    tmp_par_init[p] = bil->param[p].initial;
    tmp_par_name[p] = bil->get_param_name(p);
  }
  return DataFrame::create(Named("name") = tmp_par_name, Named("current") = tmp_par_curr, Named("lower") = tmp_par_low, Named("upper") = tmp_par_upp, Named("initial") = tmp_par_init);
}

RcppExport SEXP get_vars(SEXP model_ptr, SEXP Rget_chars)
{
  XPtr<bilan> bil(model_ptr);
  bool get_chars = as<bool>(Rget_chars);

  DataFrame vars;
  if (!get_chars) {
    vector<long double> tmp_var(bil->time_steps);
    for (unsigned v = 0; v < bil->var_count; v++) {
      for (unsigned ts = 0; ts < bil->time_steps; ts++) {
        if (bil->is_value_na(ts, v))
          tmp_var[ts] = NA_REAL;
        else
          tmp_var[ts] = bil->var[ts][v];
      }
      vars.push_back(tmp_var, bil->get_var_name(v));
    }
  }
  else {
    string err;
    try {
      //recalculation of chars
      if (bil->get_type() == bilan::DAILY) {
        bil->calc_var_mon();
        bil->calc_years_count(bil->calen_mon, bil->months);
      }
      else {
        bil->calc_years_count(bil->calen, bil->time_steps);
      }
      std::streambuf* Rcout_buf = BIL_OSTREAM.rdbuf();
      std::ostringstream oss;
      BIL_OSTREAM.rdbuf(oss.rdbuf());

      bil->calc_char_mon();

      BIL_OSTREAM.rdbuf(Rcout_buf);
      if (oss.str() != "") {
        Environment base("package:base");
        Function warning = base["warning"];
        warning(oss.str());
      }

      const unsigned months_in_year = 12;
      vector<long double> tmp_min(months_in_year), tmp_mean(months_in_year), tmp_max(months_in_year);
      for (unsigned v = 0; v < bil->var_count; v++) {
        if (bil->is_var_na(v)) {
          fill(tmp_min.begin(), tmp_min.end(), NA_REAL);
          fill(tmp_mean.begin(), tmp_mean.end(), NA_REAL);
          fill(tmp_max.begin(), tmp_max.end(), NA_REAL);
        }
        else {
          for (unsigned m = 0; m < months_in_year; m++) {
            tmp_min[m] = bil->char_mon[m][v * 3];
            tmp_mean[m] = bil->char_mon[m][v * 3 + 1];
            tmp_max[m] = bil->char_mon[m][v * 3 + 2];
          }
        }
        vars.push_back(tmp_min, bil->get_var_name(v) + ".min");
        vars.push_back(tmp_mean, bil->get_var_name(v) + ".mean");
        vars.push_back(tmp_max, bil->get_var_name(v) + ".max");
      }
    }
    catch (std::exception &exc) {
      err = exc.what();
    }
    catch (bil_err &error) {
      err = "\n*** Bilan error: " + error.descr;
    }
    if (!err.empty())
      return wrap(err);
  }
  return vars;
}

RcppExport SEXP get_dates(SEXP model_ptr)
{
  XPtr<bilan> bil(model_ptr);
  vector<string> dates_vec(bil->time_steps);
  for (unsigned ts = 0; ts < bil->time_steps; ts++) {
    ostringstream os;
    os << bil->calen[ts];
    dates_vec[ts] = os.str();
  }
  return wrap(dates_vec);
}

RcppExport SEXP get_optim(SEXP model_ptr)
{
  XPtr<bilan> bil(model_ptr);
  map<string, string> sett;
  sett = bil->optim->get_settings();

  return wrap(sett);
}

RcppExport SEXP get_info(SEXP model_ptr)
{
  XPtr<bilan> bil(model_ptr);

  List info;

  bilan::bilan_type type = bil->get_type();
  switch (type) {
    case bilan::DAILY:
      info["time_step"] = "day";
      break;
    case bilan::MONTHLY:
      info["time_step"] = "month";
      break;
    default:
      break;
  }
  unsigned time_steps = bil->time_steps;
  info["time_steps"] = time_steps;
  info["period_begin"] = bil->get_init_date();
  info["period_end"] = bil->calen[time_steps - 1].to_string();

  vector<string> var_names(bil->var_count);
  for (unsigned v = 0; v < bil->var_count; v++) {
    var_names[v] = bil->get_var_name(v);
  }
  info["var_names"] = var_names;

  return info;
}

RcppExport SEXP get_ens_resul(SEXP model_ptr)
{
  XPtr<bilan> bil(model_ptr);

  double **ensem_resul = bil->optim->get_ens_resul();

  if (ensem_resul == 0) { //gradient
    return wrap(0);
  }
  else { //differential evolution
    unsigned dim = bil->optim->par_count+ 2;
    unsigned ens_count = bil->optim->get_ens_count();

    List ens_resul;
    vector<double> tmp_ens_resul(ens_count);
    string tmp_name;
    for (unsigned d = 0; d < dim; d++) {
      for (unsigned ens = 0; ens < ens_count; ens++) {
        tmp_ens_resul[ens] = ensem_resul[ens][d];
      }
      if (d == dim - 2) {
        ostringstream os;
        os << "OF" << bil->optim->crit_type;
        tmp_name = os.str();
      }
      else if (d == dim - 1)
        tmp_name = "evals";
      else
        tmp_name = bil->get_param_name(d);

      ens_resul.push_back(tmp_ens_resul, tmp_name);
    }
    return as<DataFrame>(ens_resul);
  }
}

RcppExport SEXP get_state(SEXP model_ptr, SEXP Rst_date, SEXP Rinit_GS)
{
  XPtr<bilan> bil(model_ptr);
  Date st_date = as<Date>(Rst_date);
  long double init_GS = as<long double>(Rinit_GS);
  string err;
  bil_state state;
  try {
    date state_date(st_date.getYear(), st_date.getMonth(), st_date.getDay());
    state = bil->get_state(init_GS, state_date);
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  if (!err.empty())
    return wrap(err);

  List output;
  output["date"] = state.calen.to_string();
  output["season"] = state.season;
  output["SS"] = state.st_var[bil_state::stSS];
  output["SW"] = state.st_var[bil_state::stSW];
  output["GS"] = state.st_var[bil_state::stGS];
  output["DS"] = state.st_var[bil_state::stDS];

  return output;
}

RcppExport SEXP run_from_state(SEXP model_ptr, SEXP Rorig_state)
{
  XPtr<bilan> bil(model_ptr);
  List Rstate(Rorig_state);
  bil_state state;
  Date st_date = as<Date>(Rstate["date"]);
  date tmp_date(st_date.getYear(), st_date.getMonth(), st_date.getDay());
  state.calen = tmp_date;
  state.season = as<unsigned>(Rstate["season"]);
  state.st_var[bil_state::stSS] = as<long double>(Rstate["SS"]);
  state.st_var[bil_state::stSW] = as<long double>(Rstate["SW"]);
  state.st_var[bil_state::stGS] = as<long double>(Rstate["GS"]);
  state.st_var[bil_state::stDS] = as<long double>(Rstate["DS"]);

  string err;
  try {
    bil->run_from_state(state);
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}
