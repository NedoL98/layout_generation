#pragma once

#include <optional>
#include <vector>

class Chromosome {
  friend class Generation;

 public:
  void Init(const size_t induct_checkpoints_num, const double ratio_to_keep, const size_t idx);

  void Crossover(const Chromosome& other, const double enthropy);
  void Mutate(const double enthropy);
  void SetScore(const double score) {
    score_opt = score;
  }
  void Invalidate() {
    score_opt = std::nullopt;
  }
  bool IsInvalid() const {
    return !score_opt.has_value();
  }

  std::vector<size_t> GetCheckpointsPermutation() const {
    return induct_checkpoints_permutation;
  }

 private:
  void MutationSwap(const double enthropy = 0.3);
  void MutationShift(const double enthropy = 0.3);

  std::vector<size_t> induct_checkpoints_permutation;
  std::optional<double> score_opt;
  size_t max_checkpoint_idx;
  size_t idx;
};

class Generation {
public:
  Generation() = default;
  Generation(
      const size_t generation_size,
      const size_t induct_checkpoints_num,
      const double kept_checkpoint_ratio,
      const double entropy,
      const size_t seed = 42);

  const std::vector<Chromosome>& GetChromosomes() const {
    return chromosomes;
  }
  std::vector<Chromosome>& GetChromosomesMutable() {
    return chromosomes;
  }

  void Evolve();

private:
  std::vector<Chromosome> chromosomes;
  double entropy;
};
