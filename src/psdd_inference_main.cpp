#include <iostream>
#include <psdd/fpgacnf.h>
#include <psdd/cnf.h>
#include <psdd/optionparser.h>
#include <psdd/fpga_psdd_node.h>
#include <string>
#include <fstream>
#include <xcl2/xcl2.hpp>
#include <cmath>
#include <time.h>
#include <vector>
#include <string>
extern "C" {
#include <sdd/sddapi.h>
}

using std::cout;
using std::endl;
using std::string;
using std::vector;

bool verifyResultsMPE(std::vector<float, aligned_allocator<float>> &result , const char *psdd_filename, PsddManager *reference_psdd_manager,
   std::bitset<MAX_VAR> var_mask, vector<std::bitset<MAX_VAR>, aligned_allocator<std::bitset<MAX_VAR>>> instantiations);
bool verifyResultsMAR(std::vector<float, aligned_allocator<float>> &resultTrue , std::vector<float, aligned_allocator<float>> &resultFalse,
  const char *psdd_filename, PsddManager *reference_psdd_manager);
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
enum optionIndex { UNKNOWN, HELP, CNF_EVID };

const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", option::Arg::None,
     "USAGE: example [options]\n\n \tOptions:"},
    {HELP, 0, "h", "help", option::Arg::None,
     "--help  \tPrint usage and exit."},
    {CNF_EVID, 0, "", "cnf_evid", Arg::Required,
     "--cnf_evid  evid file, represented using CNF."},
    {UNKNOWN, 0, "", "", option::Arg::None,
     "\nExamples:\n./psdd_inference  psdd_filename vtree_filename \n"},
    {0, 0, 0, 0, 0, 0}};


int main(int argc, char** argv)
{
  //My Code
  std::cout << "starting main\n";
  std::vector<ap_uint<64>,aligned_allocator<ap_uint<64>>> dram_data (MERGED_LOOP_LEN);
  std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>>  fpga_node_vector (PSDD_SIZE);
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> prime_vector (TOTAL_CHILDREN);
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> sub_vector (TOTAL_CHILDREN);
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>> bool_param_vector (TOTAL_BOOL_PARAM);
  std::vector<std::bitset<MAX_VAR>, aligned_allocator<std::bitset<MAX_VAR>>> instantiations (NUM_DISTICT_QUERIES);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> literal_vector (TOTAL_LITERALS);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> literal_index_vector (TOTAL_LITERALS);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> literal_variable_vector (TOTAL_LITERALS);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> top_variable_vector (TOTAL_VARIABLE_INDEXES);
  std::vector<ap_int<32>, aligned_allocator<ap_int<32>>> variable_index_vector (TOTAL_VARIABLE_INDEXES);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> children_size_vector (TOTAL_CHILDREN_SIZE);
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> children_offset_vector (TOTAL_CHILDREN_SIZE);

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
 const char *query = parse.nonOption(3);
 Vtree *psdd_vtree = sdd_vtree_read(vtree_filename);
 FPGAPsddManager *psdd_manager = FPGAPsddManager::GetFPGAPsddManagerFromVtree(psdd_vtree);
 PsddManager *reference_psdd_manager = PsddManager::GetPsddManagerFromVtree(psdd_vtree);
 sdd_vtree_free(psdd_vtree);

 if (strcmp(query, "mpe_query") == 0) {
   FPGAPsddNode *result_node = psdd_manager->ReadFPGAPsddFile(
   psdd_filename, 0,
   dram_data,
   fpga_node_vector,
   prime_vector,
   sub_vector,
   bool_param_vector, literal_vector, literal_index_vector,literal_variable_vector, top_variable_vector, variable_index_vector,
   children_size_vector, children_offset_vector
    );
   uint32_t root_node_idx = result_node->node_index_;

   std::vector<SddLiteral> variables = vtree_util::VariablesUnderVtree(psdd_manager->vtree());
   auto fpga_serialized_psdd = fpga_psdd_node_util::SerializePsddNodes(result_node);
   auto fpga_serialized_psdd_evaluate = fpga_psdd_node_util::SerializePsddNodesEvaluate(root_node_idx, fpga_node_vector, prime_vector, sub_vector);
   std::cout << "inside MPE\n";

   std::bitset<MAX_VAR> var_mask;
   var_mask.set();

   //Loads queries for different networks
   std::ifstream File;
   if (strcmp(psdd_filename, "../networks/weighted_map_network.psdd") == 0){
     File.open("mpeMapNetwork.txt");
     for(int a = 0; a < NUM_DISTICT_QUERIES; a++){
       int current;
       File >> current;
       std::bitset<MAX_VAR> tmp;
       tmp[current] = 1;
       instantiations.at(a) = tmp;
     }
   }

   if (strcmp(psdd_filename, "../networks/mastermind.psdd") == 0){
     File.open("mpeMasterMind.txt");
      for (int a = 0; a < NUM_DISTICT_QUERIES; a++){
        string current = "";
        File >> current;
        instantiations.at(a) = std::bitset<MAX_VAR>(current);
      }
  }
   File.close();

    //Allocate Memory in Host Memory
    size_t dram_data_size_bytes = sizeof(dram_data[0]) * MERGED_LOOP_LEN;
    size_t bool_param_vector_size_bytes = sizeof(bool_param_vector[0]) * TOTAL_BOOL_PARAM;
    size_t instantiation_vector_size_bytes = sizeof(instantiations[0]) * NUM_DISTICT_QUERIES;
    size_t literal_vector_size_bytes = sizeof(literal_vector[0]) * TOTAL_LITERALS;
    size_t literal_vector_index_size_bytes = sizeof(literal_index_vector[0]) * TOTAL_LITERALS;
    size_t literal_variable_vector_size_bytes = sizeof(literal_variable_vector[0]) * TOTAL_LITERALS;
    size_t top_variable_vector_size_bytes = sizeof(top_variable_vector[0]) * TOTAL_VARIABLE_INDEXES;
    size_t variable_index_vector_size_bytes = sizeof(variable_index_vector[0]) * TOTAL_VARIABLE_INDEXES;
    size_t children_size_vector_size_bytes = sizeof(children_size_vector[0]) * TOTAL_CHILDREN_SIZE;
    size_t children_offset_vector_size_bytes = sizeof(children_offset_vector[0]) * TOTAL_CHILDREN_SIZE;
    size_t fpga_serialized_psdd_evaluate_size_bytes = sizeof(fpga_serialized_psdd[0]) * TOTAL_CHILDREN_SIZE;

    int num_queries = NUM_QUERIES;

    size_t result_size_bytes = sizeof(float) * NUM_QUERIES;
    std::vector<float, aligned_allocator<float>> result (NUM_QUERIES);
    cl_int err;

    printf("NUM_QUERIES:%d\n", (int)NUM_QUERIES);

    clock_t time_req  = clock();

    // OPENCL HOST CODE AREA START
        std::vector<cl::Device> devices = xcl::get_xil_devices();
        cl::Device device = devices[0];

        OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
        OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
        OCL_CHECK(err, std::string device_name = device.getInfo<CL_DEVICE_NAME>(&err));

        std::string binary = argv[2];
        auto fileBuf = xcl::read_binary_file(binary);
        cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};

        devices.resize(1);
        OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));
        OCL_CHECK(err, cl::Kernel krnl_evaluate(program,"fpga_evaluate", &err));
        OCL_CHECK(err, cl::Buffer buffer_in0   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                dram_data_size_bytes, dram_data.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in1   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                instantiation_vector_size_bytes, instantiations.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in5  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    bool_param_vector_size_bytes, bool_param_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in7  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    literal_vector_size_bytes, literal_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in8  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    literal_variable_vector_size_bytes, literal_variable_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in9  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    top_variable_vector_size_bytes, top_variable_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in10  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    children_size_vector_size_bytes, children_size_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in11  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    children_offset_vector_size_bytes, children_offset_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in12  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    literal_vector_index_size_bytes, literal_index_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in13  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    variable_index_vector_size_bytes, variable_index_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                result_size_bytes, result.data(), &err));

        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in0, buffer_in1,// buffer_in2, buffer_in3, buffer_in4,
          buffer_in5, buffer_in7, buffer_in8, buffer_in9, buffer_in10, buffer_in11, buffer_in12, buffer_in13},0/* 0 means from host*/));

        OCL_CHECK(err, err = krnl_evaluate.setArg(0, buffer_in0));
        OCL_CHECK(err, err = krnl_evaluate.setArg(1, buffer_in1));
        OCL_CHECK(err, err = krnl_evaluate.setArg(2, buffer_in5));
        OCL_CHECK(err, err = krnl_evaluate.setArg(3, buffer_in7));
        OCL_CHECK(err, err = krnl_evaluate.setArg(4, buffer_in8));
        OCL_CHECK(err, err = krnl_evaluate.setArg(5, buffer_in9));
        OCL_CHECK(err, err = krnl_evaluate.setArg(6, buffer_in10));
        OCL_CHECK(err, err = krnl_evaluate.setArg(7, buffer_in11));
        OCL_CHECK(err, err = krnl_evaluate.setArg(8, buffer_in12));
        OCL_CHECK(err, err = krnl_evaluate.setArg(9, buffer_in13));
        OCL_CHECK(err, err = krnl_evaluate.setArg(10, buffer_output));
        OCL_CHECK(err, err = krnl_evaluate.setArg(11, num_queries));

        time_t start_time = time(NULL);
  	auto start = std::chrono::steady_clock::now();

        OCL_CHECK(err, err = q.enqueueTask(krnl_evaluate));

        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},CL_MIGRATE_MEM_OBJECT_HOST));
        q.finish();
  	auto end = std::chrono::steady_clock::now();
  	double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  	time_taken *= 1e-9;

        time_t end_time = time(NULL);
        double diff_t;
        diff_t = difftime(end_time, start_time);
        printf("Execution time: %d\n", diff_t);
  	 printf("FPGA Kernel time is %f s\n", time_taken);

    // OPENCL HOST CODE AREA END
    verifyResultsMPE(result, psdd_filename, reference_psdd_manager, var_mask, instantiations);
  }
  if (strcmp(query, "mar_query") == 0) {
      //Allocate Memory in Host Memory
    std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> node_type_vector (PSDD_SIZE);
    std::vector<ap_fixed<32,8,AP_RND>, aligned_allocator<ap_fixed<32,8,AP_RND>>> parameter_vector (TOTAL_CHILDREN);
    std::vector<float, aligned_allocator<float>> resultTrue (NUM_QUERIES);
    std::vector<float, aligned_allocator<float>> resultFalse (NUM_QUERIES);

    FPGAPsddNode *result_node = psdd_manager->ReadFPGAPsddFileOld(psdd_filename, 0, fpga_node_vector, prime_vector, sub_vector, parameter_vector, bool_param_vector,
       literal_vector, literal_index_vector,literal_variable_vector, top_variable_vector, variable_index_vector, children_size_vector, children_offset_vector, node_type_vector);
   std::vector<SddLiteral> variables = vtree_util::VariablesUnderVtree(psdd_manager->vtree());
   auto fpga_serialized_psdd = fpga_psdd_node_util::SerializePsddNodes(result_node);
   uint32_t root_node_idx = result_node->node_index_;

   auto fpga_serialized_psdd_evaluate = fpga_psdd_node_util::SerializePsddNodesEvaluate(root_node_idx, fpga_node_vector, prime_vector, sub_vector);
   std::cout << "inside MAR\n";
    std::bitset<MAX_VAR> var_mask;
    var_mask.set();
    std::bitset<MAX_VAR> instantiation;
    //Allocate Memory in Host Memory
    size_t node_type_vector_size_bytes = sizeof(node_type_vector[0]) * PSDD_SIZE;
    size_t children_vector_size_bytes = sizeof(prime_vector[0]) * TOTAL_CHILDREN;
    size_t parameter_vector_size_bytes = sizeof(parameter_vector[0]) * TOTAL_CHILDREN;
    size_t bool_param_vector_size_bytes = sizeof(bool_param_vector[0]) * TOTAL_BOOL_PARAM;
    size_t literal_vector_size_bytes = sizeof(literal_vector[0]) * TOTAL_LITERALS;
    size_t literal_vector_index_size_bytes = sizeof(literal_index_vector[0]) * TOTAL_LITERALS;
    size_t literal_variable_vector_size_bytes = sizeof(literal_variable_vector[0]) * TOTAL_LITERALS;
    size_t top_variable_vector_size_bytes = sizeof(top_variable_vector[0]) * TOTAL_VARIABLE_INDEXES;
    size_t variable_index_vector_size_bytes = sizeof(variable_index_vector[0]) * TOTAL_VARIABLE_INDEXES;
    size_t children_size_vector_size_bytes = sizeof(children_size_vector[0]) * TOTAL_CHILDREN_SIZE;
    size_t children_offset_vector_size_bytes = sizeof(children_offset_vector[0]) * TOTAL_CHILDREN_SIZE;
    size_t fpga_serialized_psdd_evaluate_size_bytes = sizeof(fpga_serialized_psdd[0]) * TOTAL_CHILDREN_SIZE;
    size_t result_size_bytes = sizeof(float) * NUM_QUERIES;
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
        OCL_CHECK(err, cl::Kernel krnl_mar(program,"fpga_mar", &err));

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
        OCL_CHECK(err, cl::Buffer buffer_in7  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    literal_vector_size_bytes, literal_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in8  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    literal_variable_vector_size_bytes, literal_variable_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in9  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    top_variable_vector_size_bytes, top_variable_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in10  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    children_size_vector_size_bytes, children_size_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in11  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    children_offset_vector_size_bytes, children_offset_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in12  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    literal_vector_index_size_bytes, literal_index_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_in13  (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                    variable_index_vector_size_bytes, variable_index_vector.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_output1(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                result_size_bytes, resultTrue.data(), &err));
        OCL_CHECK(err, cl::Buffer buffer_output2(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                result_size_bytes, resultFalse.data(), &err));

        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1, buffer_in2, buffer_in3, buffer_in4, buffer_in5,
          buffer_in7, buffer_in8, buffer_in9, buffer_in10, buffer_in11, buffer_in12, buffer_in13},0/* 0 means from host*/));

        OCL_CHECK(err, err = krnl_mar.setArg(0, buffer_in1));
        OCL_CHECK(err, err = krnl_mar.setArg(1, buffer_in2));
        OCL_CHECK(err, err = krnl_mar.setArg(2, buffer_in3));
        OCL_CHECK(err, err = krnl_mar.setArg(3, buffer_in4));
        OCL_CHECK(err, err = krnl_mar.setArg(4, buffer_in5));
        OCL_CHECK(err, err = krnl_mar.setArg(5, buffer_in7));
        OCL_CHECK(err, err = krnl_mar.setArg(6, buffer_in8));
        OCL_CHECK(err, err = krnl_mar.setArg(7, buffer_in9));
        OCL_CHECK(err, err = krnl_mar.setArg(8, buffer_in10));
        OCL_CHECK(err, err = krnl_mar.setArg(9, buffer_in11));
        OCL_CHECK(err, err = krnl_mar.setArg(10, buffer_in12));
        OCL_CHECK(err, err = krnl_mar.setArg(11, buffer_in13));
        OCL_CHECK(err, err = krnl_mar.setArg(12, buffer_output1));
        OCL_CHECK(err, err = krnl_mar.setArg(13, buffer_output2));
        OCL_CHECK(err, err = krnl_mar.setArg(14, NUM_QUERIES));

        time_t start_time = time(NULL);
        verifyResultsMAR(resultTrue, resultFalse, psdd_filename, reference_psdd_manager);

        OCL_CHECK(err, err = q.enqueueTask(krnl_mar));

        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output1, buffer_output2},CL_MIGRATE_MEM_OBJECT_HOST));
        q.finish();
        time_t end_time = time(NULL);
        printf("Kernel start time: %s\n", ctime(&start_time));
        printf("Kernel end time: %s\n", ctime(&end_time));
        double diff_t;
        diff_t = difftime(end_time, start_time);
        printf("Execution time: %d\n", diff_t);
   }
  return 0;
}

bool verifyResultsMPE(std::vector<float, aligned_allocator<float>> &result , const char *psdd_filename, PsddManager *reference_psdd_manager,
    std::bitset<MAX_VAR> var_mask, vector<std::bitset<MAX_VAR>, aligned_allocator<std::bitset<MAX_VAR>>> instantiations){
   PsddNode *reference_result_node = reference_psdd_manager->ReadPsddFile(psdd_filename, 0);
   auto reference_serialized_psdd = psdd_node_util::SerializePsddNodes(reference_result_node);
   //NUM_QUERIES   vvv
   double reference_results [NUM_QUERIES] = {0};
   time_t start_time = time(NULL);
	auto start = std::chrono::steady_clock::now();
   psdd_node_util::EvaluateToCompareFPGA(var_mask, instantiations, reference_serialized_psdd, reference_results);

	auto end = std::chrono::steady_clock::now();
   time_t end_time = time(NULL);
   //printf("CPU start time: %s\n", ctime(&start_time));
   //printf("CPU end time: %s\n", ctime(&end_time));
   double diff_t;
   diff_t = difftime(end_time, start_time);
	double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	time_taken *= 1e-9;
	printf("CPU Kernel time is %f s\n", time_taken);

   //printf("Execution time: %d\n", diff_t);
   float difference = 0;
   // Change back to num _queries
   for (uint i =0; i < NUM_QUERIES; i++){
     float tmpDiff = 0;
     // if (reference_results[i] != -std::numeric_limits<float>::infinity())
     //    std::cout << "i: " << i << " reference : " << reference_results[i] << " results: "  << result[i] << std::endl;
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
bool verifyResultsMAR(std::vector<float, aligned_allocator<float>> &resultTrue , std::vector<float, aligned_allocator<float>> &resultFalse,
  const char *psdd_filename, PsddManager *reference_psdd_manager){
    bool valid = true;
    PsddNode *reference_result_node = reference_psdd_manager->ReadPsddFile(psdd_filename, 0);
    auto reference_serialized_psdd = psdd_node_util::SerializePsddNodes(reference_result_node);
    auto mar_result = psdd_node_util::GetMarginals(reference_serialized_psdd);
    std::cout << "verify MAR\n";
    int index = 0;
    for (auto result_pair : mar_result) {
      std::cout << result_pair.first << ":"
                << result_pair.second.first.parameter() << "|"
                << result_pair.second.second.parameter() << ",";
    index++;
    }
    std::cout << "\n index: " << index << endl;
    return valid;
}
