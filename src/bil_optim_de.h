#ifndef BIL_OPTIM_DE_H_INCLUDED
#define BIL_OPTIM_DE_H_INCLUDED

using namespace std;

/**
 * - default empty DE optim settings
 */
template<class FCD>
DE_optim<FCD>::DE_optim()
{
  srand((unsigned) time(0));
  n_comp = n_parof = 0;
  cross = mutat_f = mutat_k = 0.0;
  model_eval = 0;
  DE_type = 0;
  seed = 0;
  sett_comp_size = sett_n_comp = sett_ens_count = 0;

  popul_size = comp_size = 0;
  n_gen_comp = maxn_shuffles = ens_count = 0;
  reject_outside = true;

  best_model = 0;
  popul_parent = popul_parent_tmp = 0;
  rand_indexes = 0;
  comp = 0;
  models_for_comp = 0;
  offsprings_tmp = 0;
  ensemble_resul = 0;
}

/**
 * - copy constructor
 */
template<class FCD>
DE_optim<FCD>::DE_optim(const DE_optim<FCD>& orig) : optimizer_gen<FCD>(orig)
{
  DE_type = orig.DE_type;
  ens_count = orig.ens_count;
  seed = orig.seed;
  sett_comp_size = orig.sett_comp_size;
  sett_n_comp = orig.sett_n_comp;
  sett_ens_count = orig.sett_ens_count;
  n_parof = orig.n_parof;
  reject_outside = orig.reject_outside;
  cross = orig.cross;
  mutat_f = orig.mutat_f;
  mutat_k = orig.mutat_k;
  popul_size = orig.popul_size;
  n_comp = orig.n_comp;
  comp_size = orig.comp_size;
  model_eval = orig.model_eval;
  n_gen_comp = orig.n_gen_comp;
  maxn_shuffles = orig.maxn_shuffles;

  unsigned par, sp;
  if (ens_count != 0) {
    ensemble_resul = new double*[ens_count];
    for (unsigned ens = 0; ens < ens_count; ens++) {
      ensemble_resul[ens] = new double[n_parof + 1];
      for (par = 0; par < n_parof + 1; par++)
        ensemble_resul[ens][par] = orig.ensemble_resul[ens][par];
    }
  }
  else
    ensemble_resul = 0;

  if (n_parof != 0) {
    popul_parent = new double*[n_parof];
    popul_parent_tmp = new double*[n_parof];
    offsprings_tmp = new double*[n_parof];
    best_model = new double[n_parof];
    for (par = 0; par < n_parof; par++) {
      best_model[par] = orig.best_model[par];
      if (popul_size != 0) {
        popul_parent[par] = new double[popul_size];
        popul_parent_tmp[par] = new double[popul_size];
        for (sp = 0; sp < popul_size; sp++) {
          popul_parent[par][sp] = orig.popul_parent[par][sp];
          popul_parent_tmp[par][sp] = orig.popul_parent_tmp[par][sp];
        }
      }
      else {
        popul_parent[par] = 0;
        popul_parent_tmp[par] = 0;
      }
      if (comp_size != 0) {
        offsprings_tmp[par] = new double[comp_size];
        for (sp = 0; sp < comp_size; sp++) {
          offsprings_tmp[par][sp] = orig.offsprings_tmp[par][sp];
        }
      }
      else {
        offsprings_tmp[par] = 0;
      }
    }
    comp = new double**[n_parof];
    for (par = 0; par < n_parof; par++) {
      if (comp_size != 0) {
        comp[par] = new double*[comp_size];
        for (sp = 0; sp < comp_size; sp++) {
          if (n_comp != 0) {
            comp[par][sp] = new double[n_comp];
            for (unsigned com = 0; com < n_comp; com++) {
              comp[par][sp][com] = orig.comp[par][sp][com];
            }
          }
          else
            comp[par][sp] = 0;
        }
      }
      else {
        comp[par] = 0;
      }
    }
  }
  else {
    popul_parent = popul_parent_tmp = 0;
    best_model = 0;
    comp = 0;
    offsprings_tmp = 0;
  }

  if (popul_size != 0) {
    rand_indexes = new unsigned[popul_size];
    models_for_comp = new t_model_index[popul_size];
    for (sp = 0; sp < popul_size; sp++) {
      rand_indexes[sp] = orig.rand_indexes[sp];
      models_for_comp[sp].model_fitness = orig.models_for_comp[sp].model_fitness;
      models_for_comp[sp].model_index = orig.models_for_comp[sp].model_index;
    }
  }
  else {
    rand_indexes = 0;
    models_for_comp = 0;
  }
}

/**
 * - "virtual copy constructor"
 */
template<class FCD>
DE_optim<FCD>* DE_optim<FCD>::clone() const
{
  return new DE_optim(*this);
}

/**
 * - assignment operator
 */
template<class FCD>
optimizer_gen<FCD>& DE_optim<FCD>::operator=(const optimizer_gen<FCD>& orig)
{
  if (this != &orig) {
    optimizer_gen<FCD>::operator=(orig); //general part

    //specific part
    const DE_optim& tmp_orig = dynamic_cast<const DE_optim&>(orig);

    DE_type = tmp_orig.DE_type;
    sett_comp_size = tmp_orig.sett_comp_size;
    sett_n_comp = tmp_orig.sett_n_comp;
    sett_ens_count = tmp_orig.sett_ens_count;

    unsigned par, sp, ens;
    for (ens = 0; ens < ens_count; ens++)
       delete[] ensemble_resul[ens];
    delete[] ensemble_resul;

    for (par = 0; par < n_parof; par++) {
      delete[] popul_parent[par];
      delete[] popul_parent_tmp[par];
      delete[] offsprings_tmp[par];
    }
    delete[] popul_parent;
    delete[] popul_parent_tmp;
    delete[] offsprings_tmp;

    for (par = 0; par < n_parof; par++) {
      for (sp = 0; sp < comp_size; sp++)
        delete[] comp[par][sp];
      delete[] comp[par];
    }
    delete[] comp;

    delete[] best_model;
    delete[] models_for_comp;
    delete[] rand_indexes;

    ens_count = tmp_orig.ens_count;
    seed = tmp_orig.seed;
    n_parof = tmp_orig.n_parof;
    reject_outside = tmp_orig.reject_outside;
    cross = tmp_orig.cross;
    mutat_f = tmp_orig.mutat_f;
    mutat_k = tmp_orig.mutat_k;
    popul_size = tmp_orig.popul_size;
    n_comp = tmp_orig.n_comp;
    comp_size = tmp_orig.comp_size;
    model_eval = tmp_orig.model_eval;
    n_gen_comp = tmp_orig.n_gen_comp;
    maxn_shuffles = tmp_orig.maxn_shuffles;

    if (ens_count != 0) {
      ensemble_resul = new double*[ens_count];
      for (ens = 0; ens < ens_count; ens++) {
        ensemble_resul[ens] = new double[n_parof + 1];
        for (par = 0; par < n_parof + 1; par++)
          ensemble_resul[ens][par] = tmp_orig.ensemble_resul[ens][par];
      }
    }
    else
      ensemble_resul = 0;

    if (n_parof != 0) {
      popul_parent = new double*[n_parof];
      popul_parent_tmp = new double*[n_parof];
      offsprings_tmp = new double*[n_parof];
      best_model = new double[n_parof];
      for (par = 0; par < n_parof; par++) {
        best_model[par] = tmp_orig.best_model[par];
        if (popul_size != 0) {
          popul_parent[par] = new double[popul_size];
          popul_parent_tmp[par] = new double[popul_size];
          for (sp = 0; sp < popul_size; sp++) {
            popul_parent[par][sp] = tmp_orig.popul_parent[par][sp];
            popul_parent_tmp[par][sp] = tmp_orig.popul_parent_tmp[par][sp];
          }
        }
        else {
          popul_parent[par] = 0;
          popul_parent_tmp[par] = 0;
        }
        if (comp_size != 0) {
          offsprings_tmp[par] = new double[comp_size];
          for (sp = 0; sp < comp_size; sp++) {
            offsprings_tmp[par][sp] = tmp_orig.offsprings_tmp[par][sp];
          }
        }
        else {
          offsprings_tmp[par] = 0;
        }
      }
      comp = new double**[n_parof];
      for (par = 0; par < n_parof; par++) {
        if (comp_size != 0) {
          comp[par] = new double*[comp_size];
          for (sp = 0; sp < comp_size; sp++) {
            if (n_comp != 0) {
              comp[par][sp] = new double[n_comp];
              for (unsigned com = 0; com < n_comp; com++) {
                comp[par][sp][com] = tmp_orig.comp[par][sp][com];
              }
            }
            else
              comp[par][sp] = 0;
          }
        }
        else {
          comp[par] = 0;
        }
      }
    }
    else {
      popul_parent = popul_parent_tmp = 0;
      best_model = 0;
      comp = 0;
      offsprings_tmp = 0;
    }

    if (popul_size != 0) {
      rand_indexes = new unsigned[popul_size];
      models_for_comp = new t_model_index[popul_size];
      for (sp = 0; sp < popul_size; sp++) {
        rand_indexes[sp] = tmp_orig.rand_indexes[sp];
        models_for_comp[sp].model_fitness = tmp_orig.models_for_comp[sp].model_fitness;
        models_for_comp[sp].model_index = tmp_orig.models_for_comp[sp].model_index;
      }
    }
    else {
      rand_indexes = 0;
      models_for_comp = 0;
    }
  }
  return *this;
}

/**
 * - deletes arrays of size given by alloc variables
 */
template<class FCD>
DE_optim<FCD>::~DE_optim()
{
  delete[] best_model;
  delete[] rand_indexes;

  unsigned par;
  for (par = 0; par < n_parof; par++)
    delete[] popul_parent[par];
  delete[] popul_parent;

  for (par = 0; par < n_parof; par++)
    delete[]  popul_parent_tmp[par];
  delete[] popul_parent_tmp;

  for (par = 0; par < n_parof; par++) {
    for (unsigned sp = 0; sp < comp_size; sp++)
      delete[] comp[par][sp];
    delete[] comp[par];
  }
  delete[] comp;

  delete[] models_for_comp;
  for (par = 0; par < n_parof; par++)
    delete[]  offsprings_tmp[par];
  delete[] offsprings_tmp;

  for (unsigned ens = 0; ens < ens_count; ens++)
     delete[] ensemble_resul[ens];
  delete[] ensemble_resul;
}

/**
 * - allocates the arrays according the DE settings and number of parameters
 */
template<class FCD>
void DE_optim<FCD>::init()
{
  this->optimizer_gen<FCD>::init();

  if (sett_n_comp == 0)
    throw bil_err("Number of complexes cannot be zero.");
  if (sett_comp_size == 0)
    throw bil_err("Number of populations in one complex cannot be zero.");

  delete[] best_model;

  unsigned par, sp, ens;
  for (par = 0; par < n_parof; par++)
    delete[]  popul_parent[par];
  delete[] popul_parent;

  for (par = 0; par < n_parof; par++)
    delete[]  popul_parent_tmp[par];
  delete[] popul_parent_tmp;

  for (par = 0; par < n_parof; par++) {
    for (sp = 0; sp < comp_size; sp++)
      delete[] comp[par][sp];
    delete[] comp[par];
  }
  delete[] comp;

  delete[] rand_indexes;
  delete[] models_for_comp;

  for (par = 0; par < n_parof; par++)
    delete[]  offsprings_tmp[par];
  delete[] offsprings_tmp;

  for (ens = 0; ens < ens_count; ens++)
     delete[] ensemble_resul[ens];
  delete[] ensemble_resul;

  n_parof = this->par_count + 1; //used for delete and now for allocate
  ens_count = sett_ens_count;
  comp_size = sett_comp_size;
  n_comp = sett_n_comp;
  popul_size = n_comp * comp_size;

  best_model = new double[n_parof];
  for (par = 0; par < n_parof; par++)
    best_model[par] = 999999.99;

  ensemble_resul = new double*[ens_count];
  for (ens = 0; ens < ens_count; ens++) {
  	ensemble_resul[ens] = new double[n_parof + 1];
  	for (par = 0; par < n_parof + 1; par++)
      ensemble_resul[ens][par] = 9999.99;
  }

  popul_parent = new double*[n_parof];
  for (par = 0; par < n_parof; par++){
  	popul_parent[par] = new double[popul_size];
  	for (sp = 0; sp < popul_size; sp++) {
  	    popul_parent[par][sp] = 99999.99;
  	}
  }
  popul_parent_tmp = new double*[n_parof];
  for (par = 0; par < n_parof; par++) {
  	popul_parent_tmp[par] = new double[popul_size];
  	for (sp = 0; sp < popul_size; sp++) {
      popul_parent_tmp[par][sp] = 9999.99;
    }
  }

  rand_indexes = new unsigned[popul_size];
  for (sp = 0; sp < popul_size; sp++)
    rand_indexes[sp] = 9999999;

  comp = new double**[n_parof];
  for (par = 0; par < n_parof; par++) {
    comp[par] = new double*[comp_size];
    for (sp = 0; sp < comp_size; sp++) {
      comp[par][sp] = new double[n_comp];
      for (unsigned com = 0; com < n_comp; com++) {
        comp[par][sp][com] = 9999.99;
      }
    }
  }
  models_for_comp = new t_model_index[popul_size];
  for (sp = 0; sp < popul_size; sp++) {
    models_for_comp[sp].model_fitness = popul_parent[n_parof - 1][sp];
    models_for_comp[sp].model_index = sp;
  }
  offsprings_tmp = new double*[n_parof];
  for (par = 0; par < n_parof; par++) {
  	offsprings_tmp[par] = new double[comp_size];
  	for (sp = 0; sp < comp_size; sp++) {
      offsprings_tmp[par][sp] = 9999.99;
  	}
  }
}

/**
 * - changes values of settings for DE optimization
 * @param crit_type optimization criterion
 * @param DE_type type of differential evolution
 * @param n_comp number of complexes
 * @param comp_size number of populations in one complex
 * @param cross crossover parameter
 * @param mutat_f mutation parameter
 * @param mutat_k mutation parameter
 * @param maxn_shuffles number of shufflings
 * @param n_gen_comp maximum number of generations within a complex
 * @param ens_count number of optimization runs (the size of ensemble)
 * @param seed seed to initialize random number generator, if seed <= 0, no initialization done
 * @param weight_BF criterion weight for baseflow
 * @param use_weights whether to use weights for time steps of runoff
 * @param init_GS initial groundwater storage
 */
template<class FCD>
void DE_optim<FCD>::set(unsigned crit_type, unsigned DE_type, unsigned n_comp, unsigned comp_size, double cross, double mutat_f, double mutat_k, unsigned maxn_shuffles, unsigned n_gen_comp, unsigned ens_count, int seed, double weight_BF, bool use_weights, long double init_GS)
{
  this->optimizer_gen<FCD>::set(crit_type, weight_BF, use_weights, init_GS);

  sett_n_comp = n_comp;
  this->DE_type = DE_type;
  this->seed = seed;
  this->cross = cross;
  this->mutat_f = mutat_f;
  this->mutat_k = mutat_k;
  this->maxn_shuffles = maxn_shuffles;
  this->n_gen_comp = n_gen_comp;
  sett_ens_count = ens_count;
  sett_comp_size = comp_size;
}

/**
 * - initializes all members of the population via Latin hypercube sampling
 * - runs the model first time
 */
template<class FCD>
void DE_optim<FCD>::initialize_population()
{
  unsigned par, sp;
  for (par = 0; par < this->par_count; par++) {
    random_perm();
  	for (sp = 0; sp < popul_size; sp++) {
      popul_parent[par][sp] = ((this->hm[par] - this->dm[par]) * (static_cast<double>(rand_indexes[sp]) - randu_gen()) / (static_cast<double>(popul_size))) + this->dm[par];
    }
  }
  model_eval = 0;
  for (sp = 0; sp < popul_size; sp++) {
    for (par = 0; par < this->par_count; par++)
      this->fcd->set_param(par, this->CURR, popul_parent[par][sp]);
    this->fcd->run(this->init_GS);
    model_eval++;
    popul_parent[this->par_count][sp] = this->fcd->calc_crit(this->crit_type, this->weight_BF, this->use_weights);
  }
  for (sp = 0; sp < popul_size; sp++) {
    models_for_comp[sp].model_fitness = popul_parent[this->par_count][sp];
    models_for_comp[sp].model_index = sp;
  }
}

/**
 * - random permutations of indexes after Richard Durstenfeld in 1964 in Communications of the ACM volume 7, issue 7, as "Algorithm 235: Random permutation"
 */
template<class FCD>
void DE_optim<FCD>::random_perm()
{
  unsigned sp, j = 0, tmp = 0;

  for (sp = 0; sp < popul_size; sp++) {
    rand_indexes[sp] = sp + 1;
  }
  for (sp = popul_size - 1; sp > 0; sp--) {
    j = rand_unsint(sp + 1);
    tmp = rand_indexes[j];
    rand_indexes[j] = rand_indexes[sp];
    rand_indexes[sp] = tmp;
  }
}

/**
 * - uniform random unsigned integer number generator
 * @param n upper limit for generated values (excluded)
 * @return random number
 */
template<class FCD>
unsigned DE_optim<FCD>::rand_unsint(unsigned n)
{
  unsigned limit = RAND_MAX - RAND_MAX % n;
  unsigned rnd;
  do {
    rnd = rand();
  } while (rnd >= limit);

  return rnd % n;
}

/**
 * - uniform random double number generator
 * @return random number between 0 and 1 (excluding 1)
 */
template<class FCD>
double DE_optim<FCD>::randu_gen()
{
  unsigned LIM_MAX;

  LIM_MAX = 1 + (static_cast<unsigned>(RAND_MAX)); //to exclude upper limit (number 1)
  return static_cast<double>(rand()) / static_cast<double>(LIM_MAX);
}

/**
 * - creates the complexes from parent population
 */
template<class FCD>
void DE_optim<FCD>::make_comp_from_parent()
{
  unsigned com, sp, par, tmp = 0;
  for (com = 0; com < n_comp; com++) {
    for (sp = 0; sp < comp_size; sp++) {
      for (par = 0; par < n_parof; par++) {
        comp[par][sp][com] = popul_parent[par][tmp];
      }
      tmp += n_comp;
    }
    tmp = com + 1;
  }
}

/**
 * - combining the complexes into the parent population and filling the array of models_for_comp with fitness values
 */
template<class FCD>
void DE_optim<FCD>::make_parent_from_comp()
{
  unsigned com, sp, par, tmp = 0;
  for (com = 0; com < n_comp; com++) {
    for (sp = 0; sp < comp_size; sp++) {
      for (par = 0; par < n_parof; par++) {
        popul_parent[par][tmp] = comp[par][sp][com];
      }
      tmp++;
    }
  }
  for (sp = 0; sp < popul_size; sp++) {
    models_for_comp[sp].model_fitness = popul_parent[this->par_count][sp];
    models_for_comp[sp].model_index = sp;
  }
}

/**
 * - the structure comparison for qsort with the preserving the model indexes
 */
inline int compare_structs(const void *a, const void *b)
{
  t_model_index *struct_a = (t_model_index *) a;
  t_model_index *struct_b = (t_model_index *) b;

  if (struct_a->model_fitness < struct_b->model_fitness) return 1;
  else if (struct_a->model_fitness == struct_b->model_fitness) return 0;
  else return -1;
}

/**
 * - sorting the population in parent according to the fitness
 */
template<class FCD>
void DE_optim<FCD>::sort_param_parent()
{
  qsort(models_for_comp, popul_size, sizeof(models_for_comp[0]), compare_structs);

  //swap parent model population
  unsigned sp, par;
  for (sp = 0; sp < popul_size; sp++) {
    for (par = 0; par < n_parof; par++) {
      popul_parent_tmp[par][sp] = popul_parent[par][models_for_comp[sp].model_index];
    }
  }
  for (sp = 0; sp < popul_size; sp++) {
    for (par = 0; par < n_parof; par++) {
      popul_parent[par][sp] = popul_parent_tmp[par][sp];
    }
  }
  //best model
  for (par = 0; par < n_parof; par++) {
    best_model[par] = popul_parent[par][popul_size - 1];
  }
}

/**
 * - fills given array by a sequence of random unsigned integers without repetitions
 * - checks if array has sufficient length for filling
 * @param randoms array to be filled
 * @param size array size
 * @param upper_limit limit for generated numbers (excluded)
 * @param forbidden one value forbidden to be generated
 */
template<class FCD>
void DE_optim<FCD>::get_randoms_without_rep(unsigned *randoms, unsigned size, unsigned upper_limit, unsigned forbidden)
{
  if (size >= upper_limit - 1) //-1 is due to one forbidden value
    throw bil_err("The limit does not allow to store unique randoms to given array.");
  unsigned tmp_rand, r, tmp_r;
  double tmp_drand;
  bool accept;
  for (r = 0; r < size; r++) {
    do {
      accept = true;
      tmp_drand = randu_gen();
      tmp_rand = static_cast<unsigned>(tmp_drand * upper_limit);
      if (tmp_rand == forbidden)
        accept = false;
      else {
        for (tmp_r = 0; tmp_r < r; tmp_r++) {
          if (tmp_rand == randoms[tmp_r]) {
            accept = false;
            break;
          }
        }
      }
    } while (accept == false);
    randoms[r] = tmp_rand;
  }
}

/**
 * - perfoms DE of type best/1/bin
 * @param com index of complex
 */
template<class FCD>
void DE_optim<FCD>::DE(unsigned com)
{
  unsigned sp_rand_size, *sp_rand, par_rand, sp, par, gen;
  switch (DE_type) {
    case BEST_ONE_BIN:
      sp_rand_size = 2;
      break;
    case BEST_TWO_BIN:
      sp_rand_size = 4;
      break;
    case RAND_TWO_BIN:
      sp_rand_size = 5;
      break;
    default:
      throw bil_err("Invalid DE type.");
      break;
  }
  sp_rand = new unsigned[sp_rand_size];
  for (gen = 0; gen < n_gen_comp; gen++) {
    for (sp = 0; sp < comp_size; sp++) {
      get_randoms_without_rep(sp_rand, sp_rand_size, comp_size, sp);
      par_rand = rand_unsint(this->par_count); //index of parameter to be mutated for any crossover probability
      for (par = 0; par < this->par_count; par++) {
        if (randu_gen() < cross || par == par_rand) {
          switch (DE_type) {
            case BEST_ONE_BIN:
              offsprings_tmp[par][sp] = best_model[par] + mutat_f * (comp[par][sp_rand[0]][com] - comp[par][sp_rand[1]][com]);
              break;
            case BEST_TWO_BIN:
              offsprings_tmp[par][sp] = best_model[par] + mutat_k * (comp[par][sp_rand[0]][com] - comp[par][sp_rand[3]][com]) + mutat_f * (comp[par][sp_rand[1]][com] - comp[par][sp_rand[2]][com]);
              break;
            case RAND_TWO_BIN:
              offsprings_tmp[par][sp] = comp[par][sp_rand[0]][com] + mutat_k * (comp[par][sp_rand[4]][com] - comp[par][sp_rand[3]][com]) + mutat_f * (comp[par][sp_rand[1]][com] - comp[par][sp_rand[2]][com]);
              break;
            default:
              break;
          }
          if (reject_outside) {
            if (offsprings_tmp[par][sp] < this->dm[par] || offsprings_tmp[par][sp] > this->hm[par])
              offsprings_tmp[par][sp] = comp[par][sp][com];
          }
        }
        else {
          offsprings_tmp[par][sp] = comp[par][sp][com];
        }

        this->fcd->set_param(par, this->CURR, offsprings_tmp[par][sp]);
      } //par
      this->fcd->run(this->init_GS);
      model_eval++;
      offsprings_tmp[this->par_count][sp] = this->fcd->calc_crit(this->crit_type, this->weight_BF, this->use_weights);
      if (offsprings_tmp[this->par_count][sp] < comp[this->par_count][sp][com]) {
        for (par = 0; par < n_parof; par++)
          comp[par][sp][com] = offsprings_tmp[par][sp];

        if (offsprings_tmp[this->par_count][sp] < best_model[this->par_count]) {
          for (par = 0; par < n_parof; par++) {
            best_model[par] = offsprings_tmp[par][sp];
          }
        }
      }
    }
  } //end of generation loop in one complex
  delete[] sp_rand;
}

/**
 * - shuffled complex evolution using differential evolution of given type
 */
template<class FCD>
void DE_optim<FCD>::SCE_DE()
{
  for (unsigned k = 0; k < maxn_shuffles; k++) {
    sort_param_parent();
    make_comp_from_parent();
    for (unsigned com = 0; com < n_comp; com++) {
      DE(com);
    }
    make_parent_from_comp();
  }
}

/**
 * - ensemble SCE-DE optimization run
 * - last ensemble results are assigned to current model (to allow write of results by write_file)
 */
template<class FCD>
void DE_optim<FCD>::optimize()
{
  init();
  if (n_comp == 0)
    throw bil_err("DE optimization is not set and cannot be used.");

  unsigned par;
  if (seed > 0)
    srand(seed);
  for (unsigned ens = 0; ens < ens_count; ens++) {
    initialize_population();
 	  SCE_DE();

    for (par = 0; par < this->par_count; par++)
      ensemble_resul[ens][par] = best_model[par];
    if (this->crit_type == this->NS || this->crit_type == this->LNNS)
      ensemble_resul[ens][this->par_count] = 1 - best_model[this->par_count];
    else
      ensemble_resul[ens][this->par_count] = best_model[this->par_count];
    ensemble_resul[ens][n_parof] = static_cast<double>(model_eval);
  }
  //last ensemble results to current model
  for (par = 0; par < this->par_count; par++) {
    this->fcd->set_param(par, this->CURR, ensemble_resul[ens_count - 1][par]);
  }
  this->fcd->run(this->init_GS);
  this->ok = this->fcd->calc_crit(this->crit_type, this->weight_BF, this->use_weights);
  if (this->crit_type == this->NS || this->crit_type == this->LNNS)
    this->ok = 1 - this->ok;
}

/**
 * - gets DE optimization settings
 * @return settings as map with name of settings and value as a string
 */
template<class FCD>
map<string, string> DE_optim<FCD>::get_settings()
{
  ostringstream os;

  map<string, string> sett;
  sett = this->optimizer_gen<FCD>::get_settings();

  sett.insert(pair<string, string>("crit", this->crit_names[this->crit_type]));
  os << setprecision(15) << DE_type;
  sett.insert(pair<string, string>("DE_type", os.str()));
  os.str("");
  os << sett_n_comp;
  sett.insert(pair<string, string>("n_comp", os.str()));
  os.str("");
  os << sett_comp_size;
  sett.insert(pair<string, string>("comp_size", os.str()));
  os.str("");
  os << cross;
  sett.insert(pair<string, string>("cross", os.str()));
  os.str("");
  os << mutat_f;
  sett.insert(pair<string, string>("mutat_f", os.str()));
  os.str("");
  os << mutat_k;
  sett.insert(pair<string, string>("mutat_k", os.str()));
  os.str("");
  os << maxn_shuffles;
  sett.insert(pair<string, string>("maxn_shuffles", os.str()));
  os.str("");
  os << n_gen_comp;
  sett.insert(pair<string, string>("n_gen_comp", os.str()));
  os.str("");
  os << sett_ens_count;
  sett.insert(pair<string, string>("ens_count", os.str()));
  os.str("");
  os << seed;
  sett.insert(pair<string, string>("seed", os.str()));

  return sett;
}

/**
 * - gets ensemble size
 * @return ensemble size
 */
template<class FCD>
unsigned DE_optim<FCD>::get_ens_count()
{
  return ens_count;
}

/**
 * - gets ensemble results
 * @return pointer to ensemble results array
 */
template<class FCD>
double** DE_optim<FCD>::get_ens_resul()
{
  return ensemble_resul;
}

/**
 * - writes best model parameters, criterion and number of iterations for each ensemble
 * @param file_name name of output file
 */
template<class FCD>
void DE_optim<FCD>::write(string file_name)
{
  ofstream out_stream(file_name.c_str());
  if (!out_stream) {
    throw bil_err("The output file '" + file_name + "' cannot be used.");
  }
  unsigned par, ct;
  out_stream << "ensemble\t";
  for (par = 0; par < this->par_count; par++)
    out_stream << this->fcd->get_param_name(par) << "\t";
  out_stream << "OK\t";
  for (ct = 0; ct < this->crit_count; ct++)
    out_stream << this->crit_names[ct] << "\t";
  out_stream << "iter\n";

  for (unsigned ens = 0; ens < ens_count; ens++) {
    out_stream << ens + 1 << "\t";
    for (par = 0; par < this->par_count; par++) {
      out_stream << ensemble_resul[ens][par] << "\t";
      this->fcd->set_param(par, this->CURR, ensemble_resul[ens][par]);
    }
    out_stream << ensemble_resul[ens][this->par_count] << "\t"; //calibration criterion value

    //calculate other criteria than used for calibration
    this->fcd->run(this->init_GS);
    this->ok = this->fcd->calc_crit(this->crit_type, this->weight_BF, this->use_weights);
    if (this->crit_type == this->NS || this->crit_type == this->LNNS)
      this->ok = 1 - this->ok;
    for (unsigned ct = this->MSE; ct <= this->MAPE; ct++) {
      if (ct == this->NS || ct == this->LNNS)
        out_stream << static_cast<double>(1 - this->fcd->calc_crit(ct, this->weight_BF, this->use_weights)) << "\t";
      else
        out_stream << static_cast<double>(this->fcd->calc_crit(ct, this->weight_BF, this->use_weights)) << "\t";
    }
    out_stream << ensemble_resul[ens][n_parof] << "\n"; //number of model_eval
  }
  out_stream.close();
}

#endif // BIL_OPTIM_DE_H_INCLUDED
