#include "bil_system.h"

using namespace std;

/**
 * - creates catchment system instance
 */
bil_system::bil_system()
{
  catch_count = 0;
  catch_opt_count = 0;
  par_count_catch = 0;
  par_fix_count_catch = 0;
  fcd.set(this);
  optim = new optimizer<bilsys_fcd*>(); //default optimization
  optim->set_functoid(&fcd);
}

bil_system::~bil_system()
{
  delete optim;
}

/**
 * - adds a catchment to the system
 * @param bil catchment as a Bilan instance
 */
void bil_system::add_catchment(bilan *bil)
{
  catchs.push_back(*bil);
  catch_count++;
}

/**
 * - removes a catchment from the system
 * @param cat_n serial number of required catchment
 */
void bil_system::remove_catchment(unsigned cat_n)
{
  if (cat_n >= get_catch_count(false))
    throw bil_err("Required catchment does not exist in the system.");

  list<bilan>::iterator ci = catchs.begin();
  advance(ci, cat_n);
  catchs.erase(ci);
  catch_count--;
}

/**
 * - gets number of catchment in the system
 * @param only_opt whether to get number of optimized catchments only
 * @return number of catchments
 */
unsigned bil_system::get_catch_count(bool only_opt)
{
  if (only_opt)
    return catch_opt_count;
  else
    return catch_count;
}

/**
 * - gets catchment from list of all catchments by index - inefficient
 * @param cat_n serial number of required catchment
 * @return model for the required catchment
 */
bilan bil_system::get_catch(unsigned cat_n)
{
  if (cat_n >= get_catch_count(false))
    throw bil_err("Required catchment does not exist in the system.");

  list<bilan>::iterator ci = catchs.begin();
  advance(ci, cat_n);
  return *ci;
}

/**
 * - gets optimized catchment
 * @param cat_n serial number of required catchment
 * @return model for the required catchment
 */
bilan bil_system::get_catch_opt(unsigned cat_n)
{
  if (cat_n >= get_catch_count(true))
    throw bil_err("Required catchment does not exist in the system.");

  return *catchs_opt[cat_n];
}

/**
 * - calculates PET estimation for all catchments (now only by tabular method)
 */
void bil_system::calc_pet()
{
  for (list<bilan>::iterator ci = catchs.begin(); ci != catchs.end(); ++ci)
    ci->pet_estim_tab();
}

/**
 * - prepares one set of parameters from all catchments
 * - checks if catchments have the same type and length of data as the first one has
 * - checks if areas of catchments are set, otherwise catchment is rejected
 * - intended to be called immediately before optimization (only its setting can follow)
 */
void bil_system::prepare_opt()
{
  list<bilan>::iterator ci;
  bilan::bilan_type tmp_type = bilan::MONTHLY;
  unsigned tmp_time_steps = 0, tmp_c = 1, tmp_catch_opt_count = 0;
  date tmp_first_date;
  bool first_area_found = false; //first catchment with area is used as reference
  for (ci = catchs.begin(); ci != catchs.end(); ++ci) {
    if (ci->get_area() > NUMERIC_EPS) {
      if (!first_area_found) {
          tmp_type = ci->get_type();
          tmp_time_steps = ci->time_steps;
          tmp_first_date = ci->calen[0];
          ci->set_system_optim(true);
          tmp_catch_opt_count++;
          //number of parameters from the first catchment (must be same for all catchments)
          par_count_catch = ci->par_count;
          par_fix_count_catch = ci->par_fix_count;
          first_area_found = true;
      }
      else if (ci->get_type() != tmp_type || ci->time_steps != tmp_time_steps || ci->calen[0] != tmp_first_date)
        BIL_OSTREAM << "Catchment " << tmp_c << " has different model type or data period and will not be used for optimization.\n";
      else {
        ci->set_system_optim(true);
        tmp_catch_opt_count++;
      }
    }
    else
      BIL_OSTREAM << "Catchment " << tmp_c << " will not be used for optimization because its area has not been set.\n";
    tmp_c++;
  }
  catch_opt_count = tmp_catch_opt_count;

  //catchments for optimization to temporary vector (due to access to parameters)
  catchs_opt.resize(catch_opt_count);
  tmp_c = 0;
  for (ci = catchs.begin(); ci != catchs.end(); ++ci) {
    if (ci->get_system_optim()) {
      catchs_opt[tmp_c] = &(*ci);
      tmp_c++;
    }
  }
}

/**
 * - runs the previously set optimization algorithm for the system
 */
void bil_system::optimize()
{
  if (catch_opt_count == 0)
    throw bil_err("System contains no catchment.");
  optim->optimize();
}

/**
 * - calculates sum of weights for catchments to be optimized
 */
void bil_system::calc_sum_weights()
{
  for (unsigned cat = 0; cat < catch_opt_count; cat++) {
    catchs_opt[cat]->sum_weights = catchs_opt[cat]->get_var_sum(bilan::WEI);
  }
}

/**
 * - checks if input variables needed for optimization are loaded for all catchments
 * @param is_weight_BF whether baseflow will be used for optimization
 */
void bil_system::check_vars_for_optim(bool is_weight_BF)
{
  for (unsigned cat = 0; cat < catch_opt_count; cat++) {
    catchs_opt[cat]->check_vars_for_optim(is_weight_BF);
  }
}

/**
 * - gets value of parameter
 * @param par_n parameter serial number within all catchments
 * @param par_type type of parameter
 * @return value of parameter
 */
double bil_system::get_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type)
{
  unsigned catch_n = par_n / par_count_catch; //division of integer to get integer
  unsigned par_catch_n = par_n % par_count_catch;
  switch (par_type) {
    case parameter::INIT:
      return catchs_opt[catch_n]->param[par_catch_n].initial;
    case parameter::CURR:
      return catchs_opt[catch_n]->param[par_catch_n].value;
    case parameter::LOWER:
      return catchs_opt[catch_n]->param[par_catch_n].lower;
    case parameter::UPPER:
      return catchs_opt[catch_n]->param[par_catch_n].upper;
    default:
      throw bil_err("Undefined parameter type.");
      break;
  }
}

/**
 * - sets value of parameter
 * @param par_n parameter serial number within all catchments
 * @param par_type type of parameter
 * @param value value to be set
 */
void bil_system::set_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type, double value)
{
  unsigned catch_n = par_n / par_count_catch;
  unsigned par_catch_n = par_n % par_count_catch;
  switch (par_type) {
    case parameter::INIT:
      catchs_opt[catch_n]->param[par_catch_n].initial = value;
      break;
    case parameter::CURR:
      catchs_opt[catch_n]->param[par_catch_n].value = value;
      break;
    case parameter::LOWER:
      catchs_opt[catch_n]->param[par_catch_n].lower = value;
      break;
    case parameter::UPPER:
      catchs_opt[catch_n]->param[par_catch_n].upper = value;
      break;
    default:
      break;
  }
}

/**
 * - gets number of parameters from all catchments
 * @return number of parameters
 */
unsigned bil_system::get_param_count()
{
  return par_count_catch * catch_opt_count;
}

/**
 * - gets number of fixed parameters from all catchments
 * @return number of fixed parameters
 */
unsigned bil_system::get_param_fix_count()
{
  return par_fix_count_catch * catch_opt_count;
}

/**
 * - gets parameter name
 * @param par_n parameter number
 */
std::string bil_system::get_param_name(unsigned par_n)
{
  return catchs_opt[0]->get_param_name(par_n % par_count_catch);
}

/**
 * - runs the model for all catchments
 * @param init_GS initial groundwater storage, same for each catchment
 */
void bil_system::run(long double init_GS)
{
  for (unsigned cat = 0; cat < catch_opt_count; cat++) {
    catchs_opt[cat]->run(init_GS);
  }
}

/**
 * - calculates optimization criterion value for a system
 * - resulting criterion as simple mean of criteria for each catchments
 * - in case of two catchments adds penalty for negative difference in flows between the second and first catchment
 * @param crit criterion type
 * @param weight_BF weight for baseflow
 * @param use_weights whether to use weights
 * @return criterion value
 */
long double bil_system::calc_crit(unsigned crit, double weight_BF, bool use_weights)
{
  long double tmp_crit = 0;
  for (unsigned cat = 0; cat < catch_opt_count; cat++) {
    tmp_crit += catchs_opt[cat]->calc_crit_RM_BF(crit, weight_BF, use_weights);
  }
  unsigned neg_flows = 0;
  if (catch_opt_count == 2) {
    for (unsigned ts = 0; ts < catchs_opt[1]->time_steps; ts++) {
      if (catchs_opt[1]->get_flow_m3s1(ts, bilan::RM) - catchs_opt[0]->get_flow_m3s1(ts, bilan::RM) < 0) {
        neg_flows++;
      }
    }
  }
  return (tmp_crit + 0.1 * neg_flows) / static_cast<long double>(catch_opt_count);
}

/**
 * - calculates sum of weights for optimization
 */
void bilsys_fcd::calc_sum_weights()
{
  psbil->calc_sum_weights();
}

/**
 * - checks if input variables needed for optimization are loaded
 * @param is_weight_BF whether baseflow will be used for optimization
 */
void bilsys_fcd::check_vars_for_optim(bool is_weight_BF)
{
  psbil->check_vars_for_optim(is_weight_BF);
}

/**
 * - gets value of parameter
 * @param par_n parameter serial number
 * @param par_type type of parameter
 * @return value of parameter
 */
double bilsys_fcd::get_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type)
{
  return psbil->get_param(par_n, par_type);
}

/**
 * - sets value of parameter
 * @param par_n parameter serial number
 * @param par_type type of parameter
 * @param value value to be set
 */
void bilsys_fcd::set_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type, double value)
{
  psbil->set_param(par_n, par_type, value);
}

/**
 * - gets number of parameters from all catchments
 * @return number of parameters
 */
unsigned bilsys_fcd::get_param_count()
{
  return psbil->get_param_count();
}

/**
 * - gets number of fixed parameters from all catchments
 * @return number of fixed parameters
 */
unsigned bilsys_fcd::get_param_fix_count()
{
  return psbil->get_param_fix_count();
}

/**
 * - gets parameter name
 * @param par_n parameter number
 */
std::string bilsys_fcd::get_param_name(unsigned par_n)
{
  return psbil->get_param_name(par_n);
}

/**
 * - runs the model for all catchments
 * @param init_GS initial groundwater storage, same for each catchment
 */
void bilsys_fcd::run(long double init_GS)
{
  psbil->run(init_GS);
}

/**
 * - calculates optimization criterion value for a system of catchments
 * @param crit criterion type
 * @param weight_BF weight for baseflow
 * @param use_weights whether to use weights
 * @return criterion value
 */
long double bilsys_fcd::calc_crit(unsigned crit, double weight_BF, bool use_weights)
{
  return psbil->calc_crit(crit, weight_BF, use_weights);
}
