#pragma once

#include <vector>

struct Chromosome {
  void Init(
      const size_t eject_checkpoints_num,
      const size_t induct_checkpoints_num,
      const size_t seed = 42);

  std::vector<size_t> eject_checkpoints_permutation;
  std::vector<size_t> induct_checkpoints_permutation;
};

class Generation {
public:
  Generation(
      const size_t generation_size,
      const size_t eject_checkpoints_num,
      const size_t induct_checkpoints_num);

  const std::vector<Chromosome>& GetChromosomes() const {
    return chromosomes;
  }

private:
  std::vector<Chromosome> chromosomes;
};
