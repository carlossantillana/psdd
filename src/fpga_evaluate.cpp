#include <psdd/fpga_kernel_psdd_node.h>
#include <bitset>
#include <assert.h>

extern "C" {
  void load12Bit(const ap_uint<32>* data_dram, ap_uint<12>* data_local, int burstLength){
    #pragma HLS inline off
    loadInts: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i];
    }
  }

   void load20Bit(const ap_uint<32>* data_dram, ap_uint<21>* data_local, int burstLength){
     #pragma HLS inline off
     loadInts: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }
   void load21Bit(const ap_uint<32>* data_dram, ap_uint<22>* data_local, int burstLength){
     #pragma HLS inline off
     loadInts: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void loadStructs(const ap_uint<256>* data_dram, FPGAPsddNodeStruct* data_local, int burstLength){
     #pragma HLS inline off
     loadStruct: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i].node_index_ = data_dram[i](31, 0);
       data_local[i].node_type_ = data_dram[i](33,32);
       data_local[i].children_size = data_dram[i](95, 64);
       data_local[i].children_offset = data_dram[i](127, 96);
       data_local[i].parameter_offset = data_dram[i](159, 128);
       data_local[i].variable_index_ = data_dram[i](191, 160);
       data_local[i].bool_param_offset = data_dram[i](223, 192);
       data_local[i].literal_ = data_dram[i](255, 224);
     }
   }

   void loadFloats(const ap_fixed<32,10,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int burstLength){
     #pragma HLS inline off
     loadFloat: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }
   void loadFloatsSmall(const ap_fixed<32,4,AP_RND >* data_dram, ap_fixed<14,2,AP_RND >* data_local, int burstLength){
     #pragma HLS inline off
     loadFloatSmall: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void load(ap_uint<21>  local_serialized_nodes [PSDD_SIZE], const ap_uint<32>  serialized_nodes [PSDD_SIZE], FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE],
     const ap_uint<256> fpga_node_vector[PSDD_SIZE], ap_uint<22> local_children_vector[TOTAL_CHILDREN], const ap_uint<32> children_vector[TOTAL_CHILDREN],
      ap_fixed<21,8,AP_RND > local_parameter_vector[TOTAL_PARAM], const ap_fixed<32,10,AP_RND > parameter_vector[TOTAL_PARAM],
      ap_fixed<14,2,AP_RND > local_bool_param_vector [TOTAL_BOOL_PARAM], const ap_fixed<32,4,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM],
       ap_uint<12> local_flippers [55], const ap_uint<32> flippers [55]){
     load12Bit(flippers, local_flippers, 55);
     load20Bit(serialized_nodes, local_serialized_nodes, PSDD_SIZE);
     loadStructs(fpga_node_vector, local_fpga_node_vector, PSDD_SIZE);
     load21Bit(children_vector, local_children_vector, TOTAL_CHILDREN);
     loadFloats(parameter_vector, local_parameter_vector, TOTAL_PARAM);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
     return;
   }

void fpga_evaluate(
        const ap_uint<32>* serialized_nodes, // Read-Only Vector 1
        const ap_uint<256> *fpga_node_vector, // Read-Only Vector 2
        const ap_uint<32> *children_vector,
        const ap_fixed<32,10,AP_RND> *parameter_vector,
        const ap_fixed<32,4,AP_RND> *bool_param_vector,
        const ap_uint<32> *flippers,
        float *result,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port=serialized_nodes  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=fpga_node_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=children_vector  offset=slave bundle=gmem
#pragma HLS interface m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = flippers offset = slave bundle = gmem
#pragma HLS interface m_axi port = results offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=serialized_nodes  bundle=control
#pragma HLS INTERFACE s_axilite port=fpga_node_vector  bundle=control
#pragma HLS INTERFACE s_axilite port=children_vector  bundle=control
#pragma HLS interface s_axilite port = parameter_vector bundle = control
#pragma HLS interface s_axilite port = bool_param_vector bundle = control
#pragma HLS interface s_axilite port = flippers bundle = control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=num_queries bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

assert(num_queries <= 4096);  // this helps HLS estimate the loop trip count
// std::bitset<MAX_VAR> local_variables;
// local_variables.set();

ap_uint<12> local_flippers [55];
ap_uint<21> local_serialized_nodes [PSDD_SIZE];
FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE];
ap_uint<22> local_children_vector[TOTAL_CHILDREN];
ap_fixed<21,8,AP_RND > local_parameter_vector[TOTAL_PARAM];
ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
load(local_serialized_nodes, serialized_nodes, local_fpga_node_vector,
fpga_node_vector, local_children_vector, children_vector, local_parameter_vector,
parameter_vector, local_bool_param_vector, bool_param_vector, local_flippers, flippers);

#pragma HLS RESOURCE variable=local_serialized_nodes core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_children_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_bool_param_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram
// #pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram

    for (int j = 0 ; j < num_queries ; j++){
      #pragma HLS pipeline
        result[j] = local_fpga_node_vector[j].node_index_;
    }
}
}
