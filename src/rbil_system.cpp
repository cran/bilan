#include "bilan-r.h"
#include "bil_system.h"

using namespace std;
using namespace Rcpp;

RcppExport SEXP new_sbil()
{
  bil_system *sbil = new bil_system();
  sbil->optim = new optimizer<bilsys_fcd*>();
  sbil->optim->set_functoid(&sbil->fcd);

  XPtr<bil_system> bilsys_ptr(sbil, false);

  return bilsys_ptr;
}

RcppExport SEXP add_catchs(SEXP system_ptr, SEXP Rcatchs)
{
  XPtr<bil_system> sbil(system_ptr);
  List catchs = as<List>(Rcatchs);
  unsigned catchs_count = catchs.size();

  string err;
  try {
    for (unsigned c = 0; c < catchs_count; c++) {
      XPtr<bilan> bil = catchs[c];
      sbil->add_catchment(bil);
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

RcppExport SEXP remove_catchs(SEXP system_ptr, SEXP Rcat_ns)
{
  XPtr<bil_system> sbil(system_ptr);
  List cat_ns = as<List>(Rcat_ns);
  unsigned remove_count = cat_ns.size();

  string err;
  try {
    unsigned done_count = 0;
    for (unsigned cr = 0; cr < remove_count; cr++) {
      unsigned cat_n = as<unsigned>(cat_ns[cr]);
      cat_n -= done_count; //because list got shorter
      sbil->remove_catchment(cat_n);
      done_count++;
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

RcppExport SEXP get_catch(SEXP system_ptr, SEXP Rcat_n)
{
  XPtr<bil_system> sbil(system_ptr);
  unsigned cat_n = as<unsigned>(Rcat_n);

  string err;
  try {
    bilan cat = sbil->get_catch(cat_n);
    bilan *bil = new bilan(cat);
    XPtr<bilan> bilan_ptr(bil, false);
    return bilan_ptr;
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP sbil_get_optim(SEXP system_ptr)
{
  XPtr<bil_system> sbil(system_ptr);
  map<string, string> sett;
  sett = sbil->optim->get_settings();

  return wrap(sett);
}

RcppExport SEXP sbil_set_optim(SEXP system_ptr, SEXP Rcrit_part1, SEXP Rcrit_part2, SEXP Rweight_BF, SEXP Rmax_iter, SEXP Rinit_GS, SEXP Ruse_weights)
{
  XPtr<bil_system> sbil(system_ptr);

  unsigned crit_part1 = as<unsigned>(Rcrit_part1);
  unsigned crit_part2 = as<unsigned>(Rcrit_part2);
  double weight_BF = as<double>(Rweight_BF);
  unsigned max_iter = as<unsigned>(Rmax_iter);
  long double init_GS = as<long double>(Rinit_GS);
  bool use_weights = as<bool>(Ruse_weights);

  string err = "";
  try {
    delete sbil->optim;
    optimizer<bilsys_fcd*> tmp_optim;
    tmp_optim.set_functoid(&sbil->fcd);
    tmp_optim.set(crit_part1, crit_part2, weight_BF, use_weights, max_iter, init_GS);
    sbil->optim = new optimizer<bilsys_fcd*>();
    *(sbil->optim) = tmp_optim;
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: Optimization was not set - " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP sbil_set_DE_optim(SEXP system_ptr, SEXP Rcrit, SEXP RDE_type, SEXP Rn_comp, SEXP Rcomp_size, SEXP Rcross, SEXP Rmutat_f, SEXP Rmutat_k,SEXP Rmaxn_shuffles, SEXP Rn_gen_comp, SEXP Rens_count, SEXP Rseed, SEXP Rweight_BF, SEXP Rinit_GS, SEXP Ruse_weights)
{
  XPtr<bil_system> sbil(system_ptr);

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
    delete sbil->optim;
    DE_optim<bilsys_fcd*> tmp_optim;
    tmp_optim.set_functoid(&sbil->fcd);
    tmp_optim.set(crit, DE_type, n_comp, comp_size, cross, mutat_f, mutat_k, maxn_shuffles, n_gen_comp, ens_count, seed, weight_BF, use_weights, init_GS);
    sbil->optim = new DE_optim<bilsys_fcd*>();
    *(sbil->optim) = tmp_optim;
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: Optimization was not set - " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP sbil_run(SEXP system_ptr, SEXP Rinit_GS)
{
  XPtr<bil_system> sbil(system_ptr);
  long double init_GS = as<long double>(Rinit_GS);
  string err;

  try {
    sbil->run(init_GS);
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}

RcppExport SEXP sbil_optimize(SEXP system_ptr)
{
  XPtr<bil_system> sbil(system_ptr);
  string err;

  try {
    sbil->prepare_opt();
    sbil->optimize();
  }
  catch (std::exception &exc) {
    err = exc.what();
  }
  catch (bil_err &error) {
    err = "\n*** Bilan error: " + error.descr;
  }
  return wrap(err);
}
