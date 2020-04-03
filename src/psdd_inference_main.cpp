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

bool verifyResults(std::vector<float, aligned_allocator<float>> &result , const char *psdd_filename, PsddManager *reference_psdd_manager,
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
  std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>>  fpga_node_vector (PSDD_SIZE);
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> children_vector (TOTAL_CHILDREN);
  std::vector<ap_fixed<32,10,AP_RND>, aligned_allocator<ap_fixed<32,10,AP_RND>>> parameter_vector (TOTAL_PARAM);
  std::vector<ap_fixed<32,4,AP_RND>, aligned_allocator<ap_fixed<32,4,AP_RND>>> bool_param_vector (TOTAL_BOOL_PARAM);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> flippers (55);

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

  //Allocate Memory in Host Memory
  size_t fpga_serialized_psdd_size_bytes = sizeof(fpga_serialized_psdd_[0]) * PSDD_SIZE;
  size_t fpga_node_vector_size_bytes = sizeof(fpga_node_vector[0]) * PSDD_SIZE;
  size_t children_vector_size_bytes = sizeof(children_vector[0]) * TOTAL_CHILDREN;
  size_t parameter_vector_size_bytes = sizeof(parameter_vector[0]) * TOTAL_PARAM;
  size_t bool_param_vector_size_bytes = sizeof(bool_param_vector[0]) * TOTAL_BOOL_PARAM;
  size_t flippers_size_bytes = sizeof(flippers[0]) * 55;
  size_t result_size_bytes = sizeof(int) * 3;
  std::vector<int, aligned_allocator<int>> result (3);

  #define DATA_SIZE 4096
  size_t vector_size_bytes = sizeof(int) * DATA_SIZE;
  std::vector<int,aligned_allocator<int>> source_hw_results(DATA_SIZE);

  std::vector<int,aligned_allocator<int>> source_in1(DATA_SIZE);
  std::vector<int,aligned_allocator<int>> source_in2(DATA_SIZE);
  cl_int err;

  // OPENCL HOST CODE AREA START
      // get_xil_devices() is a utility API which will find the xilinx
      // platforms and will return list of devices connected to Xilinx platform
      std::vector<cl::Device> devices = xcl::get_xil_devices();
      cl::Device device = devices[0];

      OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
      OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
      OCL_CHECK(err, std::string device_name = device.getInfo<CL_DEVICE_NAME>(&err));

      // find_binary_file() is a utility API which will search the xclbin file for
      // targeted mode (sw_emu/hw_emu/hw) and for targeted platforms.
      std::string binaryFile = xcl::find_binary_file(device_name,"fpga_evaluate");

      // import_binary_file() ia a utility API which will load the binaryFile
      // and will return Binaries.
      cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
      devices.resize(1);
      OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));
      OCL_CHECK(err, cl::Kernel krnl_vector_add(program,"fpga_evaluate", &err));

      // Allocate Buffer in Global Memory
      // Buffers are allocated using CL_MEM_USE_HOST_PTR for efficient memory and
      // Device-to-host communication
      OCL_CHECK(err, cl::Buffer buffer_in1   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
              fpga_serialized_psdd_size_bytes, fpga_serialized_psdd_.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in2   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
              fpga_node_vector_size_bytes, fpga_node_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
              vector_size_bytes, source_hw_results.data(), &err));

      // Copy input data to device global memory
      OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1, buffer_in2},0/* 0 means from host*/));

      int size = DATA_SIZE;
      OCL_CHECK(err, err = krnl_vector_add.setArg(0, buffer_in1));
      OCL_CHECK(err, err = krnl_vector_add.setArg(1, buffer_in2));
      OCL_CHECK(err, err = krnl_vector_add.setArg(2, buffer_output));
      OCL_CHECK(err, err = krnl_vector_add.setArg(3, size));

      // Launch the Kernel
      // For HLS kernels global and local size is always (1,1,1). So, it is recommended
      // to always use enqueueTask() for invoking HLS kernel
      OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));

      // Copy Result from Device Global Memory to Host Local Memory
      OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},CL_MIGRATE_MEM_OBJECT_HOST));
      q.finish();
  // OPENCL HOST CODE AREA END
std::cout << "results\n";
for (int i =0; i < 3; i++){
  std::cout << source_hw_results[i] << ", ";
}
std::cout << "\n ground truth\n";

for (int i =0; i < 3; i++){
  std::cout << fpga_node_vector[i].node_type_ << ", ";
}
std::cout << std::endl;
 // Compare the results of the Device to the simulation
  // return  verifyResults(result, psdd_filename, reference_psdd_manager, var_mask, instantiation, flippers);
  return 0;

}

bool verifyResults(std::vector<float, aligned_allocator<float>> &result , const char *psdd_filename, PsddManager *reference_psdd_manager,
    std::bitset<MAX_VAR> var_mask, std::bitset<MAX_VAR> instantiation, std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &flippers ){
   PsddNode *reference_result_node = reference_psdd_manager->ReadPsddFile(psdd_filename, 0);
   auto reference_serialized_psdd = psdd_node_util::SerializePsddNodes(reference_result_node);
   double reference_results [NUM_QUERIES] = {0};
   psdd_node_util::EvaluateToCompareFPGA(var_mask, instantiation, reference_serialized_psdd, reference_results, flippers);
   float difference = 0;
   int num_queries_clean = NUM_QUERIES;
   for (int i =0; i < NUM_QUERIES; i++){
     float tmpDiff = 0;
     std::cout << "i: " << i << " reference : " << reference_results[i] << " results: "  << result[i] << std::endl;
     if (reference_results[i] != -std::numeric_limits<float>::infinity()){
     tmpDiff = std::pow((reference_results[i] - result[i]),2);
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
