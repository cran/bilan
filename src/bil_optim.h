/**
 * @file
 * - optimization classes
 */

#ifndef BIL_OPTIM_H_INCLUDED
#define BIL_OPTIM_H_INCLUDED

#include <string>
#include <cmath>
#include <map>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>

//! epsilon related to machine precision
#define NUMERIC_EPS numeric_limits<long double>::epsilon()

class bil_err;

/**
 * - general optimization settings
 */
template <class FCD>
class optimizer_gen
{
  public:
    optimizer_gen();
    optimizer_gen(const optimizer_gen& orig);
    virtual optimizer_gen* clone() const = 0; //!< workaround representing virtual copy constructor
    virtual optimizer_gen& operator=(const optimizer_gen& orig);
    void set_functoid(FCD functoid); //!< functoid setter
    //! optimization type
    enum optim_type {BS, DE};
    void init(); //!< arrays allocation
    void set(unsigned crit_type, double weight_BF, bool use_weights, long double init_GS); //!< general settings
    //! performs optimization
    virtual void optimize() = 0;
    virtual std::map<std::string, std::string> get_settings(); //!< gets general settings
    //! gets type of optimization
    virtual optim_type get_type() = 0;
    //! gets number of ensembles
    virtual unsigned get_ens_count() = 0;
    //! gets ensemble results
    virtual double** get_ens_resul() = 0;
    //! writes optimization results to a file
    virtual void write(std::string file_name) = 0;
    //! gets criterion value
    long double get_ok() { return ok; };

    virtual ~optimizer_gen();
    enum {MSE, MAE, NS, LNNS, MAPE}; //!< optimization criterion types
    //! the same parameter types as in bilan class
    enum param_type {INIT, CURR, LOWER, UPPER};

    //!@{
    /**
     * @name
     * lower and upper limit of parameters
     */
    double *dm, *hm;
    //!@}
    unsigned par_count; //!< number of optimized parameters; if equal to zero, optimization not initialized by init()
    unsigned crit_type; //!< optimization criterion type
    double weight_BF; //!< weight for criterion of baseflow modelled and observed (between 0 and 1, complement to 1 is weight for runoff)
    bool use_weights; //!< whether to use weights for time steps of runoff

  protected:
    long double ok; //!< optimization criterion value
    long double init_GS; //!< initial groundwater storage
    static const unsigned crit_count = 5; //!< number of optimization criteria
    static const std::string crit_names[]; //!< names of optimization criteria
    FCD fcd; //!< functoid to get access to bilan or bil_system member functions
};

/**
 * - optimization settings and variables
 */
template <class FCD>
class optimizer : public optimizer_gen<FCD>
{
  public:
    optimizer();
    optimizer(const optimizer& orig);
    optimizer* clone() const; //!< "virtual copy constructor"
    virtual optimizer_gen<FCD>& operator=(const optimizer_gen<FCD>& orig);
    void init(); //!< array initialization
    virtual ~optimizer();
    void set(unsigned crit_part1, unsigned crit_part2, double weight_BF, bool use_weights, unsigned max_iter, long double init_GS); //!< optimization settings
    bool opti(); //!< main optimization function
    virtual void optimize(); //!< calibrates model parameters
    virtual std::map<std::string, std::string> get_settings(); //!< gets optimization settings
    //! gets BS type
    virtual typename optimizer_gen<FCD>::optim_type get_type() { return optimizer_gen<FCD>::BS; };
    virtual unsigned get_ens_count(); //!< gets ensemble size - 0
    virtual double** get_ens_resul(); //!< gets ensemble results - null
    virtual void write(std::string file_name); //!< writes parameters and criterion

    unsigned crit[2]; //!< optimization criterion for two parts of algorithm
    unsigned max_iter; //!< maximum number of iterations

  private:
    bool sub_opti();
    void n210();

    unsigned is_fix; //!< identifier of the second optimization part with some parameters fixed
    double *ap; //!< current values of parameters
    double *fixp; //!< values of parameters fixed after first part of optimization
    //!@{
    /** @name
     *  absolute and relative parameter change, previous and temporal values of parameters
     */
    double *ddelta, *delta, *prevp, *tmpp;
    //!@}
    //!@{
    /** @name
     *  auxiliary for gradient optimization
     */
    bool *is_close_low, *is_close_upp, *nsign, *les;
    //!@}
    int lc; //!< auxiliary for gradient optimization
    unsigned prev_step; //!< type of previous parameter change in sub_opti
    unsigned par; //!< number of parameter being changed
    bool nsave; //!< auxiliary for gradient optimization
    //!@{
    /** @name
     *  criterion value: current, the best after each parameter change, the best after change of all parameters
     */
    long double ys, yx, yy;
    //!@}
    //!@{
    /** @name
     *  for checking if parameter is close to limit
     */
    double check_low, check_upp;
    //!@}
    unsigned par_fix_count; //!< number of fixed parameters after the first part
    unsigned iter; //!< number of current iteration
    unsigned bisec; //!< number of current bisection

    //!@{
    /** @name
     *  first iteration and the end
     */
    bool start, end;
    //!@}
    unsigned bisec_count; //!< number of bisections
};

//! auxiliary for array sorting according to criterion value
typedef struct _model_index
{
  double model_fitness; //!< criterion value
  unsigned model_index; //!< array index
} t_model_index;

/**
 * - optimization settings and variables
 */
template <class FCD>
class DE_optim : public optimizer_gen<FCD>
{
  public:
    DE_optim();
    DE_optim(const DE_optim& orig);
    DE_optim* clone() const; //!< "virtual copy constructor"
    virtual optimizer_gen<FCD>& operator=(const optimizer_gen<FCD>& orig);
    virtual ~DE_optim();

    void init(); //!< allocates arrays
    void set(unsigned crit_type, unsigned DE_type, unsigned n_comp, unsigned comp_size, double cross, double mutat_f, double mutat_k, unsigned maxn_shuffles, unsigned n_gen_comp, unsigned ens_count, int seed, double weight_BF, bool use_weights, long double init_GS); //!< change DE settings
    virtual void optimize(); //!< ensemble run
    virtual std::map<std::string, std::string> get_settings(); //!< gets optimization settings
    //! gets DE type
    virtual typename optimizer_gen<FCD>::optim_type get_type() { return optimizer_gen<FCD>::DE; };
    virtual unsigned get_ens_count(); //!< gets ensemble size
    virtual double** get_ens_resul(); //!< gets ensemble results
    virtual void write(std::string file_name); //!< writes ensemble_resul to a file
    enum {BEST_ONE_BIN, BEST_TWO_BIN, RAND_TWO_BIN};
    unsigned DE_type; //!< type of differential evolution algorithm
    double **ensemble_resul; //!< best models parameters, criterion and number of model evaluations for each ensemble (ensemble, n_parof+1)

  private:
    unsigned n_parof; //!< size of parameters + 1 (objective function) - initialized even if par_count is 0, used for allocation

    bool reject_outside; //!< whether to reject the outside bounds individuals
    double cross; //!< crossover param
    double mutat_f; //!< mutation param
    double mutat_k; //!< mutation param

    unsigned popul_size; //!< total population (sets of parameters) number
    double **popul_parent; //!< population parameters and fitness - matrix(par_count+1, popul_size)
    double **popul_parent_tmp; //!< dtto temporal
    double *best_model; //!< best model param and fitness (par_count+1)

    unsigned n_comp; //!< the number of complexes to be shuffled
    unsigned comp_size; //!< number of population (sets of parameters) in one complex
    double ***comp; //!< data in complexes 3d (par_count+1, comp_size, n_comp), last rows fitness
    double **offsprings_tmp; //!< models of one complex - help variable

    unsigned model_eval; //!< number of model evaluations
    unsigned n_gen_comp; //!< maximum number of allowed function evaluations in each partial complex population
    unsigned maxn_shuffles; //!< the maximum number of complex shuffling

    unsigned ens_count; //!< number of optimization runs (optimization ensemble)
    unsigned seed; //!< seed for initialization of random number generator

    //!@{
    /** @name
     *  values of settings - for array allocation used in init, does not mean array size before
     */
    unsigned sett_comp_size, sett_n_comp, sett_ens_count;
    //!@}

    t_model_index *models_for_comp;//!< the structure for model comparison
    unsigned *rand_indexes; //!< the shuffled indexes for Latin hypercube

    void initialize_population();//!< initialization of all members of population
    void random_perm();//!< random permutation

    void make_comp_from_parent();//!< creates the complexes from parent matrix
    void make_parent_from_comp();//!< combines the complexes to the parents matrix
    void sort_param_parent();//!< sorts the parameters in parent matrix according to fitness
    void get_randoms_without_rep(unsigned *randoms, unsigned size, unsigned upper_limit, unsigned forbidden); //!< generates random unsigned integers without repetition

    unsigned rand_unsint(unsigned n);//!< random unsigned integer number generator
    double randu_gen();//!< random uniform double generator

    void DE(unsigned com); //!< differential evolution algorithm
    void SCE_DE(); //!< SCE-DE algorithm
};

#include "bil_optim_de.h"

using namespace std;
template<class FCD>
const string optimizer_gen<FCD>::crit_names[optimizer_gen<FCD>::crit_count] = {"MSE", "MAE", "NS", "LNNS", "MAPE"};

/**
 * - default optimization settings
 */
template<class FCD>
optimizer_gen<FCD>::optimizer_gen() : ok(0)
{
  par_count = 0;
  dm = hm = 0;
  crit_type = MSE;
  weight_BF = 0;
  use_weights = false;
  init_GS = 50;
}

/**
 * - functoid setter
 * @param functoid functoid to be set
 */
template <class FCD>
void optimizer_gen<FCD>::set_functoid(FCD functoid)
{
  fcd = functoid;
}

/**
 * - copy constructor
 */
template<class FCD>
optimizer_gen<FCD>::optimizer_gen(const optimizer_gen& orig)
{
  par_count = orig.par_count;
  crit_type = orig.crit_type;
  ok = orig.ok;
  weight_BF = orig.weight_BF;
  use_weights = orig.use_weights;
  init_GS = orig.init_GS;
  if (par_count != 0) {
    dm = new double[par_count];
    hm = new double[par_count];
    for (unsigned p = 0; p < par_count; p++) {
      dm[p] = orig.dm[p];
      hm[p] = orig.hm[p];
    }
  }
  else {
    dm = 0;
    hm = 0;
  }
}

/**
 * - initializes arrays of limits according to number of parameters
 * - checks if variables in model available
 * - designed to be called immediately before optimization
 */
template<class FCD>
void optimizer_gen<FCD>::init()
{
  unsigned tmp_par_count = fcd->get_param_count();
  if (tmp_par_count == 0)
    throw bil_err("Number of parameters cannot be zero.");

  par_count = tmp_par_count;
  delete[] dm;
  delete[] hm;
  dm = new double[par_count];
  hm = new double[par_count];

  if (use_weights)
    fcd->calc_sum_weights();

  fcd->check_vars_for_optim(weight_BF > NUMERIC_EPS);

  for (unsigned p = 0; p < par_count; p++) {
    dm[p] = fcd->get_param(p, this->LOWER);
    hm[p] = fcd->get_param(p, this->UPPER);
  }
}

/**
 * - sets general optimization options
 * @param crit_type current (for the first part for gradient) optimization criterion type
 * @param weight_BF criterion weight for baseflow
 * @param use_weights whether to use weights for time steps of runoff
 * @param init_GS initial groundwater storage
 */
template<class FCD>
void optimizer_gen<FCD>::set(unsigned crit_type, double weight_BF, bool use_weights, long double init_GS)
{
  if (weight_BF < 0 || weight_BF > 1)
    throw bil_err("Weight for baseflow should be between 0 and 1.");
  if (init_GS < 0)
    throw bil_err("Initial groundwater storage must be positive.");

  this->weight_BF = weight_BF;
  this->use_weights = use_weights;
  this->init_GS = init_GS;
  this->crit_type = crit_type;
}

/**
 * - gets general optimization settings
 * @return settings as map with name of settings and value as a string
 */
template<class FCD>
map<string, string> optimizer_gen<FCD>::get_settings()
{
  ostringstream os;

  map<string, string> sett;

  os << setprecision(15) << static_cast<double>(ok);
  sett.insert(pair<string, string>("crit_value", os.str()));
  os.str("");
  os << weight_BF;
  sett.insert(pair<string, string>("weight_BF", os.str()));
  os.str("");
  os << use_weights;
  sett.insert(pair<string, string>("use_weights", os.str()));
  os.str("");
  os << static_cast<double>(init_GS);
  sett.insert(pair<string, string>("init_GS", os.str()));

  return sett;
}

/**
 * - assignment operator
 */
template<class FCD>
optimizer_gen<FCD>& optimizer_gen<FCD>::operator=(const optimizer_gen<FCD>& orig)
{
  if (this != &orig) {
    //general part
    fcd = orig.fcd;
    par_count = orig.par_count;
    crit_type = orig.crit_type;
    ok = orig.ok;
    weight_BF = orig.weight_BF;
    use_weights = orig.use_weights;
    init_GS = orig.init_GS;
    delete[] dm;
    delete[] hm;
    if (par_count != 0) {
      dm = new double[par_count];
      hm = new double[par_count];
      for (unsigned p = 0; p < par_count; p++) {
        dm[p] = orig.dm[p];
        hm[p] = orig.hm[p];
      }
    }
    else {
      dm = 0;
      hm = 0;
    }
  }
  return *this;
}

/**
 * - deletes allocated arrays
 */
template<class FCD>
optimizer_gen<FCD>::~optimizer_gen()
{
  delete[] dm;
  delete[] hm;
}

/**
 * - default optimization settings
 */
template<class FCD>
optimizer<FCD>::optimizer() : is_fix(0), lc(0), prev_step(0), par(0), nsave(false), ys(0), yx(0), yy(0), check_low(0), check_upp(0), iter(0), bisec(0)
{
  crit[0] = this->MSE;
  crit[1] = this->MAPE;
  max_iter = 500;
  bisec_count = 30;
  par_fix_count = 0;
  ap = fixp = ddelta = delta = prevp = tmpp = 0;
  is_close_low = is_close_upp = 0;
  nsign = les = 0;
  start = true;
  end = false;
}

/**
 * - copy constructor
 */
template<class FCD>
optimizer<FCD>::optimizer(const optimizer<FCD>& orig) : optimizer_gen<FCD>(orig)
{
  crit[0] = orig.crit[0];
  crit[1] = orig.crit[1];
  max_iter = orig.max_iter;
  is_fix = orig.is_fix;
  par_fix_count = orig.par_fix_count;
  unsigned p;
  if (par_fix_count != 0) {
    fixp = new double[par_fix_count];
    for (p = 0; p < par_fix_count; p++) {
      fixp[p] = orig.fixp[p];
    }
  }
  else {
    fixp = 0;
  }
  if (this->par_count != 0) {
    ap = new double[this->par_count];
    ddelta = new double[this->par_count];
    delta = new double[this->par_count];
    prevp = new double[this->par_count];
    tmpp = new double[this->par_count];
    is_close_low = new bool[this->par_count];
    is_close_upp = new bool[this->par_count];
    nsign = new bool[this->par_count];
    les = new bool[this->par_count];
    for (p = 0; p < this->par_count; p++) {
      ap[p] = orig.ap[p];
      ddelta[p] = orig.ddelta[p];
      delta[p] = orig.delta[p];
      prevp[p] = orig.prevp[p];
      tmpp[p] = orig.tmpp[p];
      is_close_low[p] = orig.is_close_low[p];
      is_close_upp[p] = orig.is_close_upp[p];
      nsign[p] = orig.nsign[p];
      les[p] = orig.les[p];
    }
  }
  else {
    ap = ddelta = delta = prevp = tmpp = 0;
    is_close_low = is_close_upp = nsign = les = 0;
  }
  lc = orig.lc;
  prev_step = orig.prev_step;
  par = orig.par;
  nsave = orig.nsave;
  ys = orig.ys;
  yx = orig.yx;
  yy = orig.yy;
  check_low = orig.check_low;
  check_upp = orig.check_upp;
  iter = orig.iter;
  bisec = orig.bisec;
  start = orig.start;
  end = orig.end;
  bisec_count = orig.bisec_count;
}

/**
 * - "virtual copy constructor"
 */
template<class FCD>
optimizer<FCD>* optimizer<FCD>::clone() const
{
  return new optimizer<FCD>(*this);
}

/**
 * - assignment operator
 */
template<class FCD>
optimizer_gen<FCD>& optimizer<FCD>::operator=(const optimizer_gen<FCD>& orig)
{
  if (this != &orig) {
    optimizer_gen<FCD>::operator=(orig); //general part

    //specific part, virtual assignment operator needs this cast
    //supposed that only same types will be assigned
    const optimizer& tmp_orig = dynamic_cast<const optimizer&>(orig);

    crit[0] = tmp_orig.crit[0];
    crit[1] = tmp_orig.crit[1];
    max_iter = tmp_orig.max_iter;
    is_fix = tmp_orig.is_fix;
    par_fix_count = tmp_orig.par_fix_count;
    delete[] ap;
    delete[] fixp;
    delete[] ddelta;
    delete[] delta;
    delete[] prevp;
    delete[] tmpp;
    delete[] is_close_low;
    delete[] is_close_upp;
    delete[] nsign;
    delete[] les;
    unsigned p;
    if (par_fix_count != 0) {
      fixp = new double[par_fix_count];
      for (p = 0; p < par_fix_count; p++) {
        fixp[p] = tmp_orig.fixp[p];
      }
    }
    else {
      fixp = 0;
    }
    if (this->par_count != 0) {
      ap = new double[this->par_count];
      ddelta = new double[this->par_count];
      delta = new double[this->par_count];
      prevp = new double[this->par_count];
      tmpp = new double[this->par_count];
      is_close_low = new bool[this->par_count];
      is_close_upp = new bool[this->par_count];
      nsign = new bool[this->par_count];
      les = new bool[this->par_count];
      for (p = 0; p < this->par_count; p++) {
        ap[p] = tmp_orig.ap[p];
        ddelta[p] = tmp_orig.ddelta[p];
        delta[p] = tmp_orig.delta[p];
        prevp[p] = tmp_orig.prevp[p];
        tmpp[p] = tmp_orig.tmpp[p];
        is_close_low[p] = tmp_orig.is_close_low[p];
        is_close_upp[p] = tmp_orig.is_close_upp[p];
        nsign[p] = tmp_orig.nsign[p];
        les[p] = tmp_orig.les[p];
      }
    }
    else {
      ap = ddelta = delta = prevp = tmpp = 0;
      is_close_low = is_close_upp = nsign = les = 0;
    }
    lc = tmp_orig.lc;
    prev_step = tmp_orig.prev_step;
    par = tmp_orig.par;
    nsave = tmp_orig.nsave;
    ys = tmp_orig.ys;
    yx = tmp_orig.yx;
    yy = tmp_orig.yy;
    check_low = tmp_orig.check_low;
    check_upp = tmp_orig.check_upp;
    iter = tmp_orig.iter;
    bisec = tmp_orig.bisec;
    start = tmp_orig.start;
    end = tmp_orig.end;
    bisec_count = tmp_orig.bisec_count;
  }
  return *this;
}

/**
 * - change optimization settings
 * @param crit_part1 optimization criterion of the first part (MSE, MAE, NS, LNNS)
 * @param crit_part2 optimization criterion of the second part with some parameters fixed
 * @param weight_BF criterion weight for baseflow
 * @param max_iter maximum number of iterations
 * @param use_weights whether to use weights for time steps of runoff
 * @param init_GS initial groundwater storage
 */
template<class FCD>
void optimizer<FCD>::set(unsigned crit_part1, unsigned crit_part2, double weight_BF, bool use_weights, unsigned max_iter, long double init_GS)
{
  this->optimizer_gen<FCD>::set(crit[0], weight_BF, use_weights, init_GS);
  this->crit[0] = crit_part1;
  this->crit[1] = crit_part2;
  this->max_iter = max_iter;
}

/**
 * - optimization subfunction changing parameters
 * @return whether to terminate optimization and go to model run
 */
template<class FCD>
bool optimizer<FCD>::sub_opti()
{
  bool *iclose1, *iclose2;

  if (prev_step > 0) { //i.e. par != 0, parameters not from the first one
    if (ys < yx - yx * NUMERIC_EPS) { //changed parameter performed better, go to next one
      yx = ys;
      prev_step = 0;
      par++;
    }
  }
  else { //parameters taken from the first one
    prev_step = 0;
    if (ys < yy - yy * NUMERIC_EPS) {
      nsave = true;
      yx = ys;
      yy = ys;
    }
  }

  while (par < this->par_count) {
    //cout << iter << "\t" << lc << "\t" << par << "\t" << ap[par] << "\t" << ys << "\t" << yx << "\t"<< yy << endl; //TDD vypis
    if (les[par]) {
      iclose1 = is_close_low;
      iclose2 = is_close_upp;
    }
    else {
      iclose1 = is_close_upp;
      iclose2 = is_close_low;
    }

    if (prev_step == 0) {
      if (les[par]) {
        ap[par] = ap[par] - delta[par];
        nsign[par] = true;
      }
      else {
        ap[par] = ap[par] + delta[par];
        nsign[par] = false;
      }
      if (!iclose1[par]) {
        prev_step = 1;
        return true;
      }
    }
    if (prev_step != 2) {
      if (les[par]) {
        ap[par] = ap[par] + 2 * delta[par];
        nsign[par] = false;
      }
      else {
        ap[par] = ap[par] - 2 * delta[par];
        nsign[par] = true;
      }
      if (!iclose2[par]) {
        prev_step = 2;
        return true;
      }
    }

    if (les[par]) {
      ap[par] = ap[par] - delta[par];
      nsign[par] = true;
    }
    else {
      ap[par] = ap[par] + delta[par];
      nsign[par] = false;
    }
    prev_step = 0;
    par = par + 1;
  }
  //iterations for skipped fixed parameters count - to be consistent with unfixed part
  if (is_fix) {
    par = par_fix_count;
    iter = iter + par_fix_count;
  }
  else
    par = 0;

  if (yy > yx - yx * NUMERIC_EPS && yy < yx + yx * NUMERIC_EPS) { //no improvement after change of all parameters
    return false;
  }
  else {
    yy = yx;
    n210();
    return true;
  }
}

/**
 * - change of parameters when yx was better than yy
 */
template<class FCD>
void optimizer<FCD>::n210()
{
  unsigned p;
  for (p = 0; p < this->par_count; p++)
    delta[p] = abs(ddelta[p] * ap[p]);
  lc = 0;
  nsave = false;
  for (p = 0; p < this->par_count; p++) {
    les[p] = nsign[p];
    tmpp[p] = ap[p];
    ap[p] = 2 * ap[p] - prevp[p];
    prevp[p] = tmpp[p];
    check_low = ap[p] - 1.01 * delta[p];
    check_upp = ap[p] + 1.01 * delta[p];
    if (check_low > this->dm[p] + this->dm[p] * NUMERIC_EPS) {
      is_close_low[p] = false;
    }
    else {
      is_close_low[p] = true;
      ap[p] = prevp[p];
    }
    if (check_upp < this->hm[p] - this->hm[p] * NUMERIC_EPS) {
      is_close_upp[p] = false;
    }
    else {
      is_close_upp[p] = true;
      ap[p] = prevp[p];
    }
  }
}

/**
 * - the main optimization function
 */
template<class FCD>
bool optimizer<FCD>::opti()
{
  unsigned p;

  if (start) {
    bisec = 0;
    for (p = 0; p < this->par_count; p++) {
      les[p] = false;
      prevp[p] = tmpp[p] = ap[p];
      is_close_low[p] = false;
      is_close_upp[p] = false;

      delta[p] = abs(ddelta[p] * ap[p]);
      check_low = ap[p] - 1.01 * delta[p];
      if (check_low < this->dm[p] + this->dm[p] * NUMERIC_EPS) {
        ostringstream os;
        os << "Initial value of parameter '" << this->fcd->get_param_name(p) << "' is too close to its lower limit.";
        throw bil_err(os.str());
      }
      check_upp = ap[p] + 1.01 * delta[p];
      if (check_upp > this->hm[p] - this->hm[p] * NUMERIC_EPS) {
        ostringstream os;
        os << "Initial value of parameter '" << this->fcd->get_param_name(p) << "' is too close to its upper limit.";
        throw bil_err(os.str());
      }
    }
    lc = 0;
    if (is_fix) {
      par = par_fix_count;
      iter = par_fix_count;
    }
    else {
      par = 0;
      iter = 0;
    }

    yx = this->ok;
    yy = yx;
    prev_step = 0;
    start = false;
    nsave = false;
  }
  ys = this->ok;
  iter = iter + 1;

  if (iter > max_iter) {
    end = true;
    return true;
  }
  while (true) {
    if (sub_opti()) {
      return true;
    }
    lc = lc + 1;
    if (lc > 1) {
      if (bisec >= bisec_count) {
        end = true;
        return true;
      }
    }
    if (lc > 1 || nsave) {
      nsave = false;
      for (p = 0; p < this->par_count; p++) {
        ddelta[p] = ddelta[p] * 0.8;
        delta[p] = delta[p] * 0.8;
      }
      bisec++;
    }
    else {
      for (p = 0; p < this->par_count; p++) {
        ap[p] = prevp[p];
      }
    }
  }
}

/**
 * - initializes arrays according to total number of parameters
 * - and number of parameters that are fixed after first part of optimization
 */
template<class FCD>
void optimizer<FCD>::init()
{
  this->optimizer_gen<FCD>::init();
  par_fix_count = this->fcd->get_param_fix_count();
  delete[] ap;
  delete[] fixp;
  delete[] ddelta;
  delete[] delta;
  delete[] prevp;
  delete[] tmpp;
  delete[] is_close_low;
  delete[] is_close_upp;
  delete[] nsign;
  delete[] les;
  ap = new double[this->par_count];
  fixp = new double[par_fix_count];
  ddelta = new double[this->par_count];
  delta = new double[this->par_count];
  prevp = new double[this->par_count];
  tmpp = new double[this->par_count];
  is_close_low = new bool[this->par_count];
  is_close_upp = new bool[this->par_count];
  nsign = new bool[this->par_count];
  les = new bool[this->par_count];
}

/**
 * - deletes allocated arrays
 */
template<class FCD>
optimizer<FCD>::~optimizer()
{
  delete[] ap;
  delete[] fixp;
  delete[] ddelta;
  delete[] delta;
  delete[] prevp;
  delete[] tmpp;
  delete[] is_close_low;
  delete[] is_close_upp;
  delete[] nsign;
  delete[] les;
}

/**
 * - calibrates model parameters
 */
template<class FCD>
void optimizer<FCD>::optimize()
{
  unsigned p;

  init();

  for (is_fix = 0; is_fix <= 1; is_fix++) { //the second part with some fixed parameters
    //initialization of parameters
    for (p = 0; p < this->par_count; p++) {
      this->fcd->set_param(p, this->CURR, this->fcd->get_param(p, this->INIT));
      ap[p] = this->fcd->get_param(p, this->CURR);
      ddelta[p] = 0.1;
    }
    start = true;
    end = false;

    while (true) {
      if (is_fix) {
        for (p = 0; p < par_fix_count; p++)
          this->fcd->set_param(p, this->CURR, fixp[p]);
      }
      this->fcd->run(this->init_GS);
      this->ok = this->fcd->calc_crit(crit[is_fix], this->weight_BF, this->use_weights);

      if (!start && end) {
        if (!is_fix) {
          for (p = 0; p < par_fix_count; p++)
            fixp[p] = this->fcd->get_param(p, this->CURR);
        }
        break;
      }
      opti();

      // zpetne prirazeni parametru
      for (p = 0; p < this->par_count; p++)
        this->fcd->set_param(p, this->CURR, ap[p]);
    }
    //u Nashe-Sutcliffa odecteni od 1 (kalibruje se na minimum, nejlepsi jsou maxima)
    if (crit[is_fix] == this->NS || crit[is_fix] == this->LNNS)
      this->ok = 1 - this->ok;
  } //is_fix
}

/**
 * - gets optimization settings
 * @return settings as map with name of settings and value as a string
 */
template<class FCD>
map<string, string> optimizer<FCD>::get_settings()
{
  ostringstream os;

  map<string, string> sett;
  sett = this->optimizer_gen<FCD>::get_settings();
  sett.insert(pair<string, string>("crit_part1", this->crit_names[crit[0]]));
  sett.insert(pair<string, string>("crit_part2", this->crit_names[crit[1]]));
  os << setprecision(15) << max_iter;
  sett.insert(pair<string, string>("max_iter", os.str()));

  return sett;
}

/**
 * - gets ensemble size - ensemble not supported for this optimization type
 * @return zero
 */
template<class FCD>
unsigned optimizer<FCD>::get_ens_count()
{
  return 0;
}

/**
 * - gets ensemble results - ensemble not supported for this optimization type
 * @return null pointer
 */
template<class FCD>
double** optimizer<FCD>::get_ens_resul()
{
  return 0;
}

/**
 * - writes model parameters and criterion
 * - currently has not too much sense, because the same in output files
 * @param file_name name of output file
 */
template<class FCD>
void optimizer<FCD>::write(string file_name)
{
  ofstream out_stream(file_name.c_str());
  if (!out_stream) {
    throw bil_err("The output file '" + file_name + "' cannot be used.");
  }
  unsigned p;
  for (p = 0; p < this->par_count; p++)
    out_stream << this->fcd->get_param_name(p) << "\t";
  out_stream << "OK\n";

  for (p = 0; p < this->par_count; p++) {
    out_stream << this->fcd->get_param(p, this->CURR) << "\t";
  }
  out_stream << static_cast<double>(this->ok) << "\n";
  out_stream.close();
}

#endif // BIL_OPTIM_H_INCLUDED
