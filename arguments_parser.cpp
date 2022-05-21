#include "arguments_parser.h"

cxxopts::ParseResult ParseArguments(int argc, char* argv[]) {
  cxxopts::Options options(argv[0], "Example command line options");
  options.positional_help("[file] [optional_args]");

  options
      .add_options()
      ("f, file", "Path to graph file", cxxopts::value<std::string>())
      ("a, agents", "Number of agents", cxxopts::value<size_t>()->default_value("10"))
      ("s, assignments", "Number of assignments in one chain", cxxopts::value<size_t>()->default_value("100"))
      ("c, chains", "Number of assignment chains", cxxopts::value<size_t>()->default_value("3"))
      ("r, checkpoints_ratio", "Eject checkpoints ratio", cxxopts::value<double>()->default_value("0.2"))
      ("e, epochs", "Number of epochs", cxxopts::value<size_t>()->default_value("50"))
      ("p, entropy", "Entropy of the genetic algorithm", cxxopts::value<double>()->default_value("0.3"))
      ("h, chromosomes", "Number of chromosomes", cxxopts::value<size_t>()->default_value("3"));

  std::vector<std::string> positional_args = {"file"};
  options.parse_positional(positional_args.begin(), positional_args.end());

  cxxopts::ParseResult result = options.parse(argc, argv);

  if (result.count("help") || result.arguments().size() < positional_args.size()) {
      std::cout << options.help() << std::endl;
      exit(0);
  }

  return result;
}