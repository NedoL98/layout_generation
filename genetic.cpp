#include "genetic.h"

#include <algorithm>
#include <random>

void Chromosome::Init(
    const size_t eject_checkpoints_num,
    const size_t induct_checkpoints_num,
    const size_t seed) {
  srand(seed);

  const auto iota_and_shuffle = [] (std::vector<size_t>& vec, const size_t size) {
    vec.resize(size);
    std::iota(vec.begin(), vec.end(), 0);
    std::random_shuffle(vec.begin(), vec.end());
  };

  iota_and_shuffle(eject_checkpoints_permutation, eject_checkpoints_num);
  iota_and_shuffle(induct_checkpoints_permutation, induct_checkpoints_num);
}

Generation::Generation(
      const size_t generation_size,
      const size_t eject_checkpoints,
      const size_t induct_checkpoints) {
  chromosomes.resize(generation_size);
  for (Chromosome& chromosome : chromosomes) {
    chromosome.Init(eject_checkpoints, induct_checkpoints);
  }
}
