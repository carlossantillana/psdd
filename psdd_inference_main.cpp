//
// Created by Jason Shen on 4/22/18.
//

#include <iostream>
#include <psdd/fpgacnf.h>
#include <psdd/cnf.h>
#include <psdd/optionparser.h>
#include <psdd/fpga_psdd_node.h>
extern "C" {
#include <sdd/sddapi.h>
}
FPGAPsddNodeStruct fpga_node_vector [PSDD_SIZE];

struct Arg : public option::Arg {
  static void printError(const char *msg1, const option::Option &opt,
                         const char *msg2) {
    fprintf(stderr, "%s", msg1);
    fwrite(opt.name, (size_t)opt.namelen, 1, stderr);
    fprintf(stderr, "%s", msg2);
  }

  static option::ArgStatus Required(const option::Option &option, bool msg) {
    if (option.arg != 0)
      return option::ARG_OK;

    if (msg)
      printError("Option '", option, "' requires an argument\n");
    return option::ARG_ILLEGAL;
  }

  static option::ArgStatus Numeric(const option::Option &option, bool msg) {
    char *endptr = 0;
    if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {
    };
    if (endptr != option.arg && *endptr == 0)
      return option::ARG_OK;

    if (msg)
      printError("Option '", option, "' requires a numeric argument\n");
    return option::ARG_ILLEGAL;
  }
};
enum optionIndex { UNKNOWN, HELP, MPE_QUERY, MAR_QUERY, CNF_EVID };

const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", option::Arg::None,
     "USAGE: example [options]\n\n \tOptions:"},
    {HELP, 0, "h", "help", option::Arg::None,
     "--help  \tPrint usage and exit."},
    {MPE_QUERY, 0, "", "mpe_query", option::Arg::None, ""},
    {MAR_QUERY, 0, "", "mar_query", option::Arg::None, ""},
    {CNF_EVID, 0, "", "cnf_evid", Arg::Required,
     "--cnf_evid  evid file, represented using CNF."},
    {UNKNOWN, 0, "", "", option::Arg::None,
     "\nExamples:\n./psdd_inference  psdd_filename vtree_filename \n"},
    {0, 0, 0, 0, 0, 0}};

int main(int argc, const char *argv[]) {
  argc -= (argc > 0);
  argv += (argc > 0); // skip program name argv[0] if present
  option::Stats stats(usage, argc, argv);
  std::vector<option::Option> options(stats.options_max);
  std::vector<option::Option> buffer(stats.buffer_max);
  option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
  if (parse.error())
    return 1;
  if (options[HELP] || argc == 0) {
    option::printUsage(std::cout, usage);
    return 0;
  }
  const char *psdd_filename = parse.nonOption(0);
  const char *vtree_filename = parse.nonOption(1);
  std::cout << "starting read vtree\n";
  Vtree *psdd_vtree = sdd_vtree_read(vtree_filename);
  FPGAPsddManager *psdd_manager = FPGAPsddManager::GetFPGAPsddManagerFromVtree(psdd_vtree);
  PsddManager *reference_psdd_manager = PsddManager::GetPsddManagerFromVtree(psdd_vtree);
  sdd_vtree_free(psdd_vtree);
  std::cout << "starting read psdd file\n";
  FPGAPsddNode *result_node = psdd_manager->ReadFPGAPsddFile(psdd_filename, 0, fpga_node_vector);
  PsddNode *reference_result_node = reference_psdd_manager->ReadPsddFile(psdd_filename, 0);
  uint32_t root_node_idx = result_node->node_index_;

  std::cout << "finished read psdd file\n";
    std::cout << "starting serialize psdd\n";
  std::vector<SddLiteral> variables =
      vtree_util::VariablesUnderVtree(psdd_manager->vtree());
  auto fpga_serialized_psdd = fpga_psdd_node_util::SerializePsddNodes(result_node);
  auto reference_serialized_psdd = psdd_node_util::SerializePsddNodes(reference_result_node);
  auto fpga_serialized_psdd_evaluate = fpga_psdd_node_util::SerializePsddNodesEvaluate(root_node_idx, fpga_node_vector);

  std::cout << "starting fpga getMPE\n";
  auto fpga_mpe_result = fpga_psdd_node_util::GetMPESolution(fpga_serialized_psdd);
  auto reference_mpe_result = psdd_node_util::GetMPESolution(reference_serialized_psdd);
  std::cout << "finished fpga getMPE\n";
  std::cout << "fpga_mpe.first == reference_mpe.first: " << ((fpga_mpe_result.first  == reference_mpe_result.first)  ? "true" : "false" ) << std::endl;
  std::cout << "fpga_mpe.second == reference_mpe.second: " << ((fpga_mpe_result.second.parameter()  == reference_mpe_result.second.parameter())  ? "true" : "false" ) << std::endl;

  std::bitset<MAX_VAR> var_mask;
  var_mask.set();
  auto reference_marginals = psdd_node_util::Evaluate(var_mask, reference_mpe_result.first, reference_serialized_psdd);
  std::cout << "starting evaluate\n";
  std::array<uint32_t, PSDD_SIZE> fpga_serialized_psdd_;
  std::copy(fpga_serialized_psdd_evaluate.begin(), fpga_serialized_psdd_evaluate.begin() + z, fpga_serialized_psdd_.begin());
  auto fpga_marginals = fpga_psdd_node_util::EvaluateWithoutPointer(var_mask, fpga_mpe_result.first, fpga_serialized_psdd_, fpga_node_vector);
  std::cout << "finished evaluate\n";

  //
  std::cout << "fpga marginal: " << fpga_marginals << std::endl; //Carlos, please switch to this one after modification
  std::cout << "reference marginal: " << reference_marginals.parameter() << std::endl;
  // for (auto i = 0; i < 100; ++i){
  //     std::cout << "starting fpga evaluate\n";
  //     fpga_psdd_node_util::Evaluate(var_mask, fpga_mpe_result.first, fpga_serialized_psdd);
  //       std::cout << "finished fpga getMPE\n";
  //
  // }
  delete (psdd_manager);
  delete (reference_psdd_manager);
}
