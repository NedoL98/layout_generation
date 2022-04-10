#include "genetic.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <unordered_set>

void Chromosome::Init(const size_t induct_checkpoints_num, const double ratio_to_keep) {
  const auto iota_and_shuffle = [] (std::vector<size_t>& vec, const size_t size) {
    vec.resize(size);
    std::iota(vec.begin(), vec.end(), 0);
    std::random_shuffle(vec.begin(), vec.end());
  };

  iota_and_shuffle(induct_checkpoints_permutation, induct_checkpoints_num);
  induct_checkpoints_permutation.resize(induct_checkpoints_permutation.size() * ratio_to_keep);
  max_checkpoint_idx = induct_checkpoints_num;
}

void Chromosome::Crossover(const Chromosome& other) {
  std::unordered_set<size_t> other_induct_checkpoints(
      other.induct_checkpoints_permutation.begin(), other.induct_checkpoints_permutation.end());
  for (const auto& checkpoint_pos : induct_checkpoints_permutation) {
    other_induct_checkpoints.erase(checkpoint_pos);
  }
  if (other_induct_checkpoints.empty()) {
    return;
  }
  std::vector<size_t> induct_checkpoints_diff(
      other_induct_checkpoints.begin(), other_induct_checkpoints.end());
  for (auto& checkpoint_pos : induct_checkpoints_permutation) {
    if (induct_checkpoints_diff.empty()) {
      break;
    }
    if (rand() / static_cast<double>(RAND_MAX) < 0.05) {
      checkpoint_pos = induct_checkpoints_diff[rand() % induct_checkpoints_diff.size()];
      induct_checkpoints_diff.erase(std::find(
          induct_checkpoints_diff.begin(), induct_checkpoints_diff.end(), checkpoint_pos));
    }
  }

  assert(induct_checkpoints_permutation.size() == std::set<size_t>(
      induct_checkpoints_permutation.begin(), induct_checkpoints_permutation.end()).size()
      && "Element after crossover are not unique");
}

void Chromosome::Mutate() {
  std::unordered_set<size_t> unused_induct_checkpoints;
  for (size_t i = 0; i < max_checkpoint_idx; ++i) {
    unused_induct_checkpoints.insert(i);
  }
  for (auto checkpoint : induct_checkpoints_permutation) {
    unused_induct_checkpoints.erase(checkpoint);
  }

  for (auto& checkpoint_pos : induct_checkpoints_permutation) {
    if (unused_induct_checkpoints.empty()) {
      break;
    }
    if (rand() / static_cast<double>(RAND_MAX) < 0.05) {
      size_t rand_idx = rand() % unused_induct_checkpoints.size();
      auto rand_it = unused_induct_checkpoints.begin();
      while (rand_idx > 0) {
        ++rand_it;
        --rand_idx;
      }
      const size_t rand_checkpoint = *rand_it;
      unused_induct_checkpoints.erase(rand_it);
      unused_induct_checkpoints.insert(checkpoint_pos);
      checkpoint_pos = rand_checkpoint;
    }
  }

  assert(induct_checkpoints_permutation.size() == std::set<size_t>(
      induct_checkpoints_permutation.begin(), induct_checkpoints_permutation.end()).size()
      && "Element after mutate are not unique");
}

Generation::Generation(
      const size_t generation_size,
      const size_t induct_checkpoints,
      const double kept_checkpoint_ratio,
      const size_t seed) {
  srand(seed);
  chromosomes.resize(generation_size);
  for (Chromosome& chromosome : chromosomes) {
    chromosome.Init(induct_checkpoints, kept_checkpoint_ratio);
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

  const auto best_chromosome_it = std::max_element(
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
  for (auto& chromosome : new_generation.chromosomes) {
    for (const auto& other_chromosome : new_generation.chromosomes) {
        chromosome.Crossover(other_chromosome);
    }
  }

  *this = std::move(new_generation);
}
