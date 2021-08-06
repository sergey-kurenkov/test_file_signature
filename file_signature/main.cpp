#include <file_signature/file_signature.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iostream>
#include <string>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "input-file", po::value<std::string>(), "input file")(
      "signature-file", po::value<std::string>(), "signature file")(
      "block-size", po::value<int>()->default_value(1 << 20), "block size");

  po::variables_map opts;

  try {
    po::store(po::parse_command_line(argc, argv, desc), opts);
    po::notify(opts);
  } catch (std::exception& exc) {
    std::cerr << exc.what() << "\n";
    return 1;
  }

  if (opts.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (opts.count("input-file") == 0 || opts.count("signature-file") == 0) {
    std::cout << desc << "\n";
    return 1;
  }

  try {
    file_signature::generate(opts["input-file"].as<std::string>(),
                             opts["signature-file"].as<std::string>(),
                             opts["block-size"].as<int>());
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
