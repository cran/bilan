/**
 * @file
 * - system class
 */

#ifndef BIL_SYSTEM_H_INCLUDED
#define BIL_SYSTEM_H_INCLUDED

#include "bil_model.h"

class bil_system;

/**
 * - functoid class for access to methods when optimizing
 */
class bilsys_fcd
{
  public:
    bilsys_fcd() { psbil = 0; };
    //! creation of functoid for a system
    bilsys_fcd(bil_system *sbil) { psbil = sbil; };
    //! sets pointer to system
    void set(bil_system *sbil) { psbil = sbil; };
    void calc_sum_weights(); //!< calculates sum of weights for optimization
    void check_vars_for_optim(bool is_weight_BF); //!< checks availability of variables for optimization
    double get_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type); //!< gets value of parameter
    void set_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type, double value); //!< sets value of parameter
    unsigned get_param_count(); //!< gets number of parameters
    unsigned get_param_fix_count(); //!< gets number of fixed parameters
    std::string get_param_name(unsigned par_n); //!< gets name of parameter
    void run(long double init_GS); //!< runs the model
    long double calc_crit(unsigned crit, double weight_BF, bool use_weights); //!< calculates optimization criterion value

  private:
    bil_system* psbil; //!< pointer to a system
};

/**
 * - system of catchments
 */
class bil_system
{
  public:
    bil_system();
    ~bil_system();
    void add_catchment(bilan *bil); //!< adds a Bilan instance to the system
    void remove_catchment(unsigned cat_n); //!< removes a model from the system
    unsigned get_catch_count(bool only_opt); //!< gets number of catchments
    bilan get_catch(unsigned cat_n); //!< gets catchment
    bilan get_catch_opt(unsigned cat_n); //!< gets optimized catchment
    void calc_pet(); //!< calculates PET estimation for all catchments (now only by tabular method)
    void prepare_opt(); //!< prepares optimization
    void optimize(); //!< runs optimization for the system
    void calc_sum_weights(); //!< calculates sum of weights for catchments to be optimized
    void check_vars_for_optim(bool is_weight_BF); //!< checks availability of variables for optimization
    double get_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type); //!< gets parameter value
    void set_param(unsigned par_n, optimizer_gen<bilsys_fcd*>::param_type par_type, double value); //!< sets parameter value
    //! gets optimization criterion value for the system
    long double get_optim_ok() { return optim->get_ok(); };
    //! gets type of optimization criterion for the system
    unsigned get_optim_crit_type() { return optim->crit_type; };
    unsigned get_param_count(); //!< gets number of all parameters
    unsigned get_param_fix_count(); //!< gets number of fixed parameters
    std::string get_param_name(unsigned par_n); //!< gets parameter name
    void run(long double init_GS); //!< runs model for all catchments
    long double calc_crit(unsigned crit, double weight_BF, bool use_weights); //!< mean criterion for catchments

    bilsys_fcd fcd; //!< functoid to be used in optimization
    optimizer_gen<bilsys_fcd*> *optim; //!< optimization settings and variables (gradient or DE method)

  private:
    unsigned catch_count; //!< number of catchments
    unsigned catch_opt_count; //!< number of catchments used for optimization
    std::list<bilan> catchs; //!< catchments as Bilan instances
    std::vector<bilan*> catchs_opt; //!< catchments used for optimization
    unsigned par_count_catch; //!< number of parameters for one catchment
    unsigned par_fix_count_catch; //!< number of fixed parameters for one catchment

};

#endif // BIL_SYSTEM_H_INCLUDED
