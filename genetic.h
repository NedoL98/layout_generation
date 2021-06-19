#pragma once

#include <optional>
#include <vector>

struct Chromosome {
  void Init(
      const size_t eject_checkpoints_num,
      const size_t induct_checkpoints_num,
      const size_t seed = 42);

  void Crossover(const Chromosome& other);
  void Mutate();
  void SetScore(const double score) {
    score_opt = score;
  }

  std::vector<size_t> eject_checkpoints_permutation;
  std::vector<size_t> induct_checkpoints_permutation;
  std::optional<double> score_opt;
};

class Generation {
public:
  Generation() = default;
  Generation(
      const size_t generation_size,
      const size_t eject_checkpoints_num,
      const size_t induct_checkpoints_num);

  const std::vector<Chromosome>& GetChromosomes() const {
    return chromosomes;
  }
  std::vector<Chromosome>& GetChromosomesMutable() {
    return chromosomes;
  }

  Chromosome GetBestChromosome() const {
    // by construction
    return chromosomes.front();
  }

  void Evolve();

private:
  std::vector<Chromosome> chromosomes;
};
