//
// Created by Jason Shen on 4/22/18.
//

#include <iostream>
#include <psdd/fpgacnf.h>
#include <psdd/cnf.h>
#include <psdd/optionparser.h>
#include <psdd/fpga_psdd_node.h>
#include <psdd/fpga_evaluate.h>
#include<fstream>

extern "C" {
#include <sdd/sddapi.h>
}
FPGAPsddNodeStruct fpga_node_vector [PSDD_SIZE];
ap_uint<21> children_vector [TOTAL_CHILDREN];
ap_fixed<19,7,AP_RND > parameter_vector [TOTAL_PARAM];
ap_fixed<12,1,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM];
bool verifyResults(float * results, const char *psdd_filename, PsddManager *reference_psdd_manager,
  std::bitset<MAX_VAR> var_mask, std::bitset<MAX_VAR> instantiation, int flippers [242]);
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
  Vtree *psdd_vtree = sdd_vtree_read(vtree_filename);
  FPGAPsddManager *psdd_manager = FPGAPsddManager::GetFPGAPsddManagerFromVtree(psdd_vtree);
  PsddManager *reference_psdd_manager = PsddManager::GetPsddManagerFromVtree(psdd_vtree);
  sdd_vtree_free(psdd_vtree);
  FPGAPsddNode *result_node = psdd_manager->ReadFPGAPsddFile(psdd_filename, 0, fpga_node_vector,
     children_vector, parameter_vector, bool_param_vector);
  ap_uint<20> correctPsddSize = 0;

  uint32_t root_node_idx = result_node->node_index_;

  std::vector<SddLiteral> variables = vtree_util::VariablesUnderVtree(psdd_manager->vtree());
  auto fpga_serialized_psdd = fpga_psdd_node_util::SerializePsddNodes(result_node);
  auto fpga_mpe_result = fpga_psdd_node_util::GetMPESolution(fpga_serialized_psdd);
  auto fpga_serialized_psdd_evaluate = fpga_psdd_node_util::SerializePsddNodesEvaluate(root_node_idx, fpga_node_vector, children_vector);

  std::bitset<MAX_VAR> var_mask;
  var_mask.set();
  //Read mpe_query
  std::bitset<MAX_VAR> instantiation;
  std::ifstream File;
  int flippers [242];
  int total=242;
  // File.open("mpe.txt");
  // // int REVERT_LATER = 0;
  // for(int a = 1; a <= MAX_VAR; a++){
  //   bool tmp;
  //   File >> tmp;
  //
  //   if (tmp == 1 ){
  //     flippers[total--] = MAX_VAR - a;
  //
  //   }
  // }
  File.close();
  ap_uint<20> fpga_serialized_psdd_ [PSDD_SIZE];
  for (int i = 0; i < PSDD_SIZE; i++){
    fpga_serialized_psdd_[i] = fpga_serialized_psdd_evaluate[i];
  }

  //FPGA
  float result [NUM_QUERIES] = {0};
  EvaluateWithoutPointer(var_mask, instantiation, fpga_serialized_psdd_,
    fpga_node_vector, children_vector, parameter_vector, bool_param_vector, result, flippers);
    // bool validResults = verifyResults(result, psdd_filename, reference_psdd_manager, var_mask, instantiation, flippers);
    delete (psdd_manager);
    delete (reference_psdd_manager);

  }

  //CSIM


  //FPGA
  bool verifyResults(float * results, const char *psdd_filename, PsddManager *reference_psdd_manager,
     std::bitset<MAX_VAR> var_mask, std::bitset<MAX_VAR> instantiation, int flippers [242]){
    PsddNode *reference_result_node = reference_psdd_manager->ReadPsddFile(psdd_filename, 0);
    auto reference_serialized_psdd = psdd_node_util::SerializePsddNodes(reference_result_node);
    double reference_results [NUM_QUERIES] = {0};
    psdd_node_util::EvaluateToCompareFPGA(var_mask, instantiation, reference_serialized_psdd, reference_results, flippers);
    float difference = 0;
    int num_queries_clean = NUM_QUERIES;
    for (int i =0; i < NUM_QUERIES; i++){
      float tmpDiff = 0;
      std::cout << "i: " << i << " reference : " << reference_results[i] << " results: "  << results[i] << std::endl;
      if (reference_results[i] != -std::numeric_limits<float>::infinity()){
      tmpDiff = std::pow((reference_results[i] - results[i]),2);
    } else{
      num_queries_clean--;
    }
       // std::cout << "node index: " << i << " reference prob: " << evaluation_cache.at(i).parameter_ << " fpga prob " << log(evaluation_cache_fpga[i]) << " difference: " << tmpDiff << std::endl;
      if (tmpDiff > .1){
        std::cout << "ERROR ERROR DIFFERENCE  (" << tmpDiff << ") larger than .1 SOMETHING BAD HAPPENED \n";
      }
      difference += tmpDiff;
    }
    std::cout << "RMSE: " << sqrt(difference/NUM_QUERIES) << std::endl;
      return true;
  }
