\name{bil.get.optim}
\alias{bil.get.optim}
\alias{sbil.get.optim}
\title{Getting the optimization settings}
\usage{
bil.get.optim(object)

sbil.get.optim(object)
}
\arguments{
  \item{object}{pointer to model instance or to system of
  catchments instance}
}
\value{
List of criterion value and optimization settings, for
two-step gradient algorithm: \item{crit_part1}{criterion
for the first part} \item{crit_part2}{criterion for the
second part} \item{crit_value}{resulting criterion value
(after the second part)} \item{init_GS}{initial groundwater
storage} \item{max_iter}{maximum number of iterations}
\item{weight_BF}{weight for baseflow} For shuffled complex
evolution combined with differential evolution:
\item{DE_type}{differential evolution version}
\item{comp_size}{complex size} \item{crit}{optimization
criterion} \item{crit_value}{resulting criterion value
(after the second part)} \item{cross}{crossover parameter}
\item{ens_count}{number of runs in ensemble}
\item{init_GS}{initial groundwater storage}
\item{maxn_shuffles}{number of shuffling}
\item{mutat_f}{mutation parameter} \item{mutat_k}{mutation
parameter} \item{n_comp}{number of complexes}
\item{n_gen_comp}{number of generations in one complex}
\item{seed}{seed used for random number generator}
\item{weight_BF}{weight for baseflow}
}
\description{
For the model or for the system of catchments, the function
gets criterion value and optimization settings: type of
criteria, weight for baseflow, initial groundwater storage
and specific settings for applied optimization type.
}
\examples{
b = bil.new("m")
s = sbil.new(b)
bil.set.optim(b, "DE")
bil.get.optim(b)
sbil.get.optim(s)
}
\seealso{
\code{\link{bil.set.optim}}
}

