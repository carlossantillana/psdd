#include <iostream>
#include <psdd/fpgacnf.h>
#include <psdd/cnf.h>
#include <psdd/optionparser.h>
#include <psdd/fpga_psdd_node.h>
#include <string>
#include <fstream>
#include <xcl2/xcl2.hpp>
#include <cmath>
#include <ctime>
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
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> prime_vector (TOTAL_CHILDREN);
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> sub_vector (TOTAL_CHILDREN);
  std::vector<ap_fixed<32,8,AP_RND>, aligned_allocator<ap_fixed<32,8,AP_RND>>> parameter_vector (TOTAL_CHILDREN);
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>> bool_param_vector (TOTAL_BOOL_PARAM);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> flippers (50);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> literal_vector (TOTAL_LITERALS);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> variable_vector (TOTAL_VARIABLES);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> children_size_vector (TOTAL_CHILDREN_SIZE);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> children_offset_vector (TOTAL_CHILDREN_SIZE);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> node_type_vector (PSDD_SIZE);



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
    prime_vector, sub_vector, parameter_vector, bool_param_vector, literal_vector, variable_vector,
     children_size_vector, children_offset_vector, node_type_vector);
 uint32_t root_node_idx = result_node->node_index_;

 std::vector<SddLiteral> variables = vtree_util::VariablesUnderVtree(psdd_manager->vtree());
 auto fpga_serialized_psdd = fpga_psdd_node_util::SerializePsddNodes(result_node);
 auto fpga_serialized_psdd_evaluate = fpga_psdd_node_util::SerializePsddNodesEvaluate(root_node_idx, fpga_node_vector, prime_vector, sub_vector);

 std::bitset<MAX_VAR> var_mask;
 var_mask.set();
 //Read mpe_query
  std::bitset<MAX_VAR> instantiation;
 std::ifstream File;
 File.open("allPossibleSolutions.txt");
 for(int a = 0; a < 50; a++){
   int tmp;
   File >> tmp;
   flippers[a] = tmp;
 }

 File.close();

  //Allocate Memory in Host Memory
  size_t node_type_vector_size_bytes = sizeof(node_type_vector[0]) * PSDD_SIZE;
  size_t children_vector_size_bytes = sizeof(prime_vector[0]) * TOTAL_CHILDREN;
  size_t parameter_vector_size_bytes = sizeof(parameter_vector[0]) * TOTAL_CHILDREN;
  size_t bool_param_vector_size_bytes = sizeof(bool_param_vector[0]) * TOTAL_BOOL_PARAM;
  size_t flippers_size_bytes = sizeof(flippers[0]) * 50;
  size_t literal_vector_size_bytes = sizeof(literal_vector[0]) * TOTAL_LITERALS;
  size_t variable_vector_size_bytes = sizeof(variable_vector[0]) * TOTAL_VARIABLES;
  size_t children_size_vector_size_bytes = sizeof(children_size_vector[0]) * TOTAL_CHILDREN_SIZE;
  size_t children_offset_vector_size_bytes = sizeof(children_offset_vector[0]) * TOTAL_CHILDREN_SIZE;
  size_t fpga_serialized_psdd_evaluate_size_bytes = sizeof(fpga_serialized_psdd[0]) * TOTAL_CHILDREN_SIZE;


  size_t result_size_bytes = sizeof(float) * NUM_QUERIES;
  std::vector<float, aligned_allocator<float>> result (NUM_QUERIES);
  cl_int err;
  clock_t time_req  = clock();

  // OPENCL HOST CODE AREA START
      std::vector<cl::Device> devices = xcl::get_xil_devices();
      cl::Device device = devices[0];

      OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
      OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
      OCL_CHECK(err, std::string device_name = device.getInfo<CL_DEVICE_NAME>(&err));

      std::string binary = argv[2];
      std::cout <<"binary: " << binary << std::endl;
      auto fileBuf = xcl::read_binary_file(binary);
      cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};

      devices.resize(1);
      OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));
      OCL_CHECK(err, cl::Kernel krnl_vector_add(program,"fpga_evaluate", &err));

      OCL_CHECK(err, cl::Buffer buffer_in1   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
              node_type_vector_size_bytes, node_type_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in2  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                children_vector_size_bytes, prime_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in3  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                children_vector_size_bytes, sub_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in4  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  parameter_vector_size_bytes, parameter_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in5  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  bool_param_vector_size_bytes, bool_param_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in6  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  flippers_size_bytes, flippers.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in7  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  literal_vector_size_bytes, literal_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in8  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  variable_vector_size_bytes, variable_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in9  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  children_size_vector_size_bytes, children_size_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_in10  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                  children_offset_vector_size_bytes, children_offset_vector.data(), &err));
      OCL_CHECK(err, cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
              result_size_bytes, result.data(), &err));

      OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1, buffer_in2, buffer_in3, buffer_in4, buffer_in5,buffer_in6, buffer_in7, buffer_in8, buffer_in9, buffer_in10},0/* 0 means from host*/));

      OCL_CHECK(err, err = krnl_vector_add.setArg(0, buffer_in1));
      OCL_CHECK(err, err = krnl_vector_add.setArg(1, buffer_in2));
      OCL_CHECK(err, err = krnl_vector_add.setArg(2, buffer_in3));
      OCL_CHECK(err, err = krnl_vector_add.setArg(3, buffer_in4));
      OCL_CHECK(err, err = krnl_vector_add.setArg(4, buffer_in5));
      OCL_CHECK(err, err = krnl_vector_add.setArg(5, buffer_in6));
      OCL_CHECK(err, err = krnl_vector_add.setArg(6, buffer_in7));
      OCL_CHECK(err, err = krnl_vector_add.setArg(7, buffer_in8));
      OCL_CHECK(err, err = krnl_vector_add.setArg(8, buffer_in9));
      OCL_CHECK(err, err = krnl_vector_add.setArg(9, buffer_in10));
      OCL_CHECK(err, err = krnl_vector_add.setArg(10, buffer_output));
      OCL_CHECK(err, err = krnl_vector_add.setArg(11, NUM_QUERIES));


      OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));

      OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},CL_MIGRATE_MEM_OBJECT_HOST));
      q.finish();
      time_req = clock() - time_req;
      cout << "Kernel took " << (float)time_req/CLOCKS_PER_SEC << " seconds to do " << NUM_QUERIES << " queries" << endl;

  // OPENCL HOST CODE AREA END
  return  verifyResults(result, psdd_filename, reference_psdd_manager, var_mask, instantiation, flippers);
}

bool verifyResults(std::vector<float, aligned_allocator<float>> &result , const char *psdd_filename, PsddManager *reference_psdd_manager,
    std::bitset<MAX_VAR> var_mask, std::bitset<MAX_VAR> instantiation, std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &flippers ){
   PsddNode *reference_result_node = reference_psdd_manager->ReadPsddFile(psdd_filename, 0);
   auto reference_serialized_psdd = psdd_node_util::SerializePsddNodes(reference_result_node);
   //NUM_QUERIES   vvv
   double reference_results [NUM_QUERIES] = {0};
   clock_t time_req = clock();
   psdd_node_util::EvaluateToCompareFPGA(var_mask, instantiation, reference_serialized_psdd, reference_results, flippers);
   time_req = clock()- time_req;
cout << "CPU took " << (float)time_req/CLOCKS_PER_SEC << " seconds to do " << NUM_QUERIES << " queries" << endl;
   float difference = 0;
   // Change back to num _queries
   for (uint i =0; i < NUM_QUERIES; i++){
     float tmpDiff = 0;
     // std::cout << "i: " << i << " reference : " << reference_results[i] << " results: "  << result[i] << std::endl;
     if (reference_results[i] != -std::numeric_limits<float>::infinity()){
     tmpDiff = std::pow((reference_results[i] - result[i]),2);
   }
     if (tmpDiff > .1){
       std::cout << "ERROR ERROR DIFFERENCE  (" << tmpDiff << ") larger than .1 SOMETHING BAD HAPPENED (result: " << result[i] << ")\n";
     }
     difference += tmpDiff;
   }
   double RMSE = sqrt(difference/NUM_QUERIES);
   std::cout << "RMSE: " << RMSE << std::endl;
   if (RMSE > .1){
     std::cout << "TEST FAILED\n";
     return false;
   }
   std::cout << "TEST PASSED\n";
     return true;
 }
