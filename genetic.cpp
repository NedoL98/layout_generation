#include "genetic.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

void Chromosome::Init(const size_t eject_checkpoints_num, const double ratio_to_keep) {
  const auto iota_and_shuffle = [] (std::vector<size_t>& vec, const size_t size) {
    vec.resize(size);
    std::iota(vec.begin(), vec.end(), 0);
    std::random_shuffle(vec.begin(), vec.end());
  };

  iota_and_shuffle(eject_checkpoints_permutation, eject_checkpoints_num);
  eject_checkpoints_permutation.resize(eject_checkpoints_permutation.size() * ratio_to_keep);
}

void Chromosome::Crossover(const Chromosome& /* other */) {
}

void Chromosome::Mutate() {
  for (auto& checkpoint_pos : eject_checkpoints_permutation) {
    if (rand() / static_cast<double>(RAND_MAX) < 0.05) {
      std::swap(
          checkpoint_pos,
          eject_checkpoints_permutation[rand() % eject_checkpoints_permutation.size()]);
    }
  }
}

Generation::Generation(
      const size_t generation_size,
      const size_t eject_checkpoints,
      const double kept_checkpoint_ratio,
      const size_t seed) {
  srand(seed);
  chromosomes.resize(generation_size);
  for (Chromosome& chromosome : chromosomes) {
    chromosome.Init(eject_checkpoints, kept_checkpoint_ratio);
  }
}

void Generation::Evolve() {
  Generation new_generation;
  double total_score = std::accumulate(
      chromosomes.begin(),
      chromosomes.end(),
      0.0,
      [](const double accumulated, const Chromosome chromosome) {
    assert(chromosome.score_opt);
    return accumulated + chromosome.score_opt.value();
  });

  const auto best_chromosome_it = std::min_element(
      chromosomes.begin(),
      chromosomes.end(),
      [](const Chromosome& lhs, const Chromosome& rhs) {
    return lhs.score_opt.value() < rhs.score_opt.value();
  });

  std::vector<double> scores;
  scores.reserve(chromosomes.size());
  for (const auto& chromosome : chromosomes) {
    scores.push_back(chromosome.score_opt.value());
  }

  std::default_random_engine generator;
  std::discrete_distribution<int> distribution(scores.begin(), scores.end());
  new_generation.chromosomes.push_back(*best_chromosome_it);
  while (new_generation.chromosomes.size() < chromosomes.size()) {
    new_generation.chromosomes.push_back(chromosomes[distribution(generator)]);
  }

  for (auto& chromosome : new_generation.chromosomes) {
    chromosome.Mutate();
  }

  *this = std::move(new_generation);
}
