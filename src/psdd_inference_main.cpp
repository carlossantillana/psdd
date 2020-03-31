#include <iostream>
#include <psdd/fpgacnf.h>
#include <psdd/cnf.h>
#include <psdd/optionparser.h>
#include <psdd/fpga_psdd_node.h>
#include <string>
#include <fstream>
#include <xcl2/xcl2.hpp>
#include <cmath>
#include <vector>
#include <string>
extern "C" {
#include <sdd/sddapi.h>
}

using std::cout;
using std::endl;
using std::string;
using std::vector;
std::vector<FPGAPsddNodeStruct,aligned_allocator<FPGAPsddNodeStruct>>  fpga_node_vector (PSDD_SIZE);
std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> children_vector (TOTAL_CHILDREN);
std::vector<ap_fixed<32,10,AP_RND>, aligned_allocator<ap_fixed<32,10,AP_RND>>> parameter_vector (TOTAL_PARAM);
std::vector<ap_fixed<32,4,AP_RND>, aligned_allocator<ap_fixed<32,4,AP_RND>>> bool_param_vector (TOTAL_BOOL_PARAM);
std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> flippers (55);

bool verifyResults(std::vector<float, aligned_allocator<float>> &results , const char *psdd_filename, PsddManager *reference_psdd_manager,
   std::bitset<MAX_VAR> var_mask, std::bitset<MAX_VAR> instantiation, std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &flippers );
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


int main(int argc, char** argv)
{
  //My Code
  std::cout << "starting main\n";
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
 ap_uint<32> correctPsddSize = 0;

 uint32_t root_node_idx = result_node->node_index_;

 std::vector<SddLiteral> variables = vtree_util::VariablesUnderVtree(psdd_manager->vtree());
 auto fpga_serialized_psdd = fpga_psdd_node_util::SerializePsddNodes(result_node);
 // auto fpga_mpe_result = fpga_psdd_node_util::GetMPESolution(fpga_serialized_psdd);
 auto fpga_serialized_psdd_evaluate = fpga_psdd_node_util::SerializePsddNodesEvaluate(root_node_idx, fpga_node_vector, children_vector);

 std::bitset<MAX_VAR> var_mask;
 var_mask.set();
 //Read mpe_query
  std::bitset<MAX_VAR> instantiation;
 std::ifstream File;
 File.open("allPossibleSolutions.txt");
 for(int a = 0; a < 55; a++){
   int tmp;
   File >> tmp;
   flippers[a] = tmp;
 }

 File.close();
 std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> fpga_serialized_psdd_ (PSDD_SIZE);   //Input Matrix 1

 for (uint i = 0; i < PSDD_SIZE; i++){
   fpga_serialized_psdd_[i] = fpga_serialized_psdd_evaluate[i];
 }
std::cout << "right before fpga\n";
std::vector<float, aligned_allocator<float>> result (NUM_QUERIES);

  //Allocate Memory in Host Memory
  size_t fpga_serialized_psdd_size_bytes = sizeof(fpga_serialized_psdd_[0]) * PSDD_SIZE;
  size_t fpga_node_vector_size_bytes = sizeof(fpga_node_vector[0]) * PSDD_SIZE;
  size_t children_vector_size_bytes = sizeof(children_vector[0]) * TOTAL_CHILDREN;
  size_t parameter_vector_size_bytes = sizeof(parameter_vector[0]) * 770511;
  size_t bool_param_vector_size_bytes = sizeof(bool_param_vector[0]) * 792;
  size_t flippers_size_bytes = sizeof(flippers[0]) * 55;
  size_t result_size_bytes = sizeof(result[0]) * NUM_QUERIES;

//OPENCL HOST CODE AREA START
  vector<cl::Device> devices = xcl::get_xil_devices();
  cl::Device device = devices[0];

  cl::Context context(device);
  cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE);
  string device_name = device.getInfo<CL_DEVICE_NAME>();

  string binaryFile = xcl::find_binary_file(device_name, "fpga_evaluate");
  cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
  devices.resize(1);
  cl::Program program(context, devices, bins);
  cl::Kernel kernel(program, "fpga_evaluate");

  vector<cl::Memory> inBufVec, outBufVec;
  cl::Buffer buffer_in1(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                        fpga_serialized_psdd_size_bytes, fpga_serialized_psdd_.data());
  cl::Buffer buffer_in2(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                        fpga_node_vector_size_bytes, fpga_node_vector.data());
  cl::Buffer buffer_in3(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                        children_vector_size_bytes, children_vector.data());
  cl::Buffer buffer_in4(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                        parameter_vector_size_bytes, parameter_vector.data());
  cl::Buffer buffer_in5(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                        bool_param_vector_size_bytes, bool_param_vector.data());
  cl::Buffer buffer_in6(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                        flippers_size_bytes, flippers.data());
  cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                           result_size_bytes, result.data());
  inBufVec.push_back(buffer_in1);
  inBufVec.push_back(buffer_in2);
  inBufVec.push_back(buffer_in3);
  inBufVec.push_back(buffer_in4);
  inBufVec.push_back(buffer_in5);
  inBufVec.push_back(buffer_in6);
  outBufVec.push_back(buffer_output);
  std::cout << "parameter_vector[0]: " << parameter_vector[0] << std::endl;
  std::cout << "bool_param_vector[0]: " << bool_param_vector[0] << std::endl;

  //Copy input data to device global memory
  q.enqueueMigrateMemObjects(inBufVec, 0/* 0 means from host*/);

  auto krnl_vector_add = cl::KernelFunctor<cl::Buffer&, cl::Buffer&, cl::Buffer&,
                                            cl::Buffer&, cl::Buffer&, cl::Buffer&,
                                           cl::Buffer&, int>(kernel);

  //Launch the Kernel
  krnl_vector_add(cl::EnqueueArgs(q, cl::NDRange(1, 1, 1), cl::NDRange(1, 1, 1)),
                  buffer_in1, buffer_in2, buffer_in3, buffer_in4, buffer_in5,
                  buffer_in6, buffer_output, NUM_QUERIES);

  //Copy Result from Device Global Memory to Host Local Memory
  q.enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
  q.finish();
//OPENCL HOST CODE AREA END

  // Compare the results of the Device to the simulation
  return  verifyResults(result, psdd_filename, reference_psdd_manager, var_mask, instantiation, flippers);
}

bool verifyResults(std::vector<float, aligned_allocator<float>> &results , const char *psdd_filename, PsddManager *reference_psdd_manager,
    std::bitset<MAX_VAR> var_mask, std::bitset<MAX_VAR> instantiation, std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &flippers ){
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
   double RMSE = sqrt(difference/NUM_QUERIES);
   std::cout << "RMSE: " << RMSE << std::endl;
   if (RMSE > .1){
     return false;
   }
     return true;
 }
