#include <assert.h>
#include <stdio.h>
#include "ap_int.h"
#include <ap_fixed.h>
#include <psdd/fpga_kernel_psdd_node.h>
#include <iostream>
#include <bitset>


void load12Bit(const ap_uint<12>* data_dram, ap_uint<12>* data_local, int burstLength){
  #pragma HLS inline off
  loadInts: for (int i = 0; i < burstLength; i++){
  #pragma HLS pipeline
    data_local[i] = data_dram[i];
  }
}

 void load20Bit(const ap_uint<21>* data_dram, ap_uint<21>* data_local, int burstLength){
   #pragma HLS inline off
   loadInts: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
 void load21Bit(const ap_uint<22>* data_dram, ap_uint<22>* data_local, int burstLength){
   #pragma HLS inline off
   loadInts: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }

 void loadStructs(const FPGAPsddNodeStruct* data_dram, FPGAPsddNodeStruct* data_local, int burstLength){
   #pragma HLS inline off
   loadStruct: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }

 void loadFloats(const ap_fixed<21,8,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int burstLength){
   #pragma HLS inline off
   loadFloat: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
 void loadFloatsSmall(const ap_fixed<14,2,AP_RND >* data_dram, ap_fixed<14,2,AP_RND >* data_local, int burstLength){
   #pragma HLS inline off
   loadFloatSmall: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }

 void load(ap_uint<21>  local_serialized_nodes [PSDD_SIZE], const ap_uint<21>  serialized_nodes [PSDD_SIZE], FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE],
   const FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE], ap_uint<22> local_children_vector[TOTAL_CHILDREN], const ap_uint<22> children_vector[TOTAL_CHILDREN],
    ap_fixed<21,8,AP_RND > local_parameter_vector[TOTAL_PARAM], const ap_fixed<21,8,AP_RND > parameter_vector[TOTAL_PARAM],
    ap_fixed<14,2,AP_RND > local_bool_param_vector [TOTAL_BOOL_PARAM], const ap_fixed<14,2,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM],
     ap_uint<12> local_flippers [55], const ap_uint<12> flippers [55]){
   load12Bit(flippers, local_flippers, 55);
   load20Bit(serialized_nodes, local_serialized_nodes, PSDD_SIZE);
   loadStructs(fpga_node_vector, local_fpga_node_vector, PSDD_SIZE);
   load21Bit(children_vector, local_children_vector, TOTAL_CHILDREN);
   loadFloats(parameter_vector, local_parameter_vector, TOTAL_PARAM);
   loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
   return;
 }

extern "C" {
void fpga_evaluate(const ap_uint<21>* serialized_nodes, const FPGAPsddNodeStruct* fpga_node_vector,
  const ap_uint<22>* children_vector, const ap_fixed<21,8,AP_RND>* parameter_vector, const ap_fixed<14,2,AP_RND>* bool_param_vector,
   const ap_uint<12>* flippers, float* results, const int num_queries) {
#pragma HLS interface m_axi port = serialized_nodes offset = slave bundle = gmem
#pragma HLS interface m_axi port = fpga_node_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = children_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = flippers offset = slave bundle = gmem
#pragma HLS interface m_axi port = results offset = slave bundle = gmem
#pragma HLS interface s_axilite port = serialized_nodes bundle = control
#pragma HLS interface s_axilite port = fpga_node_vector bundle = control
#pragma HLS interface s_axilite port = children_vector bundle = control
#pragma HLS interface s_axilite port = parameter_vector bundle = control
#pragma HLS interface s_axilite port = bool_param_vector bundle = control
#pragma HLS interface s_axilite port = flippers bundle = control
#pragma HLS interface s_axilite port = results bundle = control
#pragma HLS interface s_axilite port = num_queries bundle = control
#pragma HLS interface s_axilite port = return bundle = control
  assert(num_queries <= 4096);  // this helps HLS estimate the loop trip count
 std::bitset<MAX_VAR> local_variables;
 local_variables.set();

static ap_uint<12> local_flippers [55];
static ap_uint<21> local_serialized_nodes [PSDD_SIZE];
static FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE];
static ap_uint<22> local_children_vector[TOTAL_CHILDREN];
static ap_fixed<21,8,AP_RND > local_parameter_vector[TOTAL_PARAM];
static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
load(local_serialized_nodes, serialized_nodes, local_fpga_node_vector,
  fpga_node_vector, local_children_vector, children_vector, local_parameter_vector,
  parameter_vector, local_bool_param_vector, bool_param_vector, local_flippers, flippers);

#pragma HLS RESOURCE variable=local_serialized_nodes core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_children_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_bool_param_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram

for (int m = 0; m < num_queries; m++){
  static float evaluation_cache [PSDD_SIZE];
  std::bitset<MAX_VAR> local_instantiation;
  local_instantiation[local_flippers[m%55]] = !local_instantiation[local_flippers[m%55]];

#pragma HLS RESOURCE variable=local_evaluation_cache core=XPM_MEMORY uram
  for(int j = PSDD_SIZE -1; j >= 0; j--){
#pragma HLS pipeline
    uint cur_node_idx = local_serialized_nodes[j];
    if (local_fpga_node_vector[cur_node_idx].node_type_ == LITERAL_NODE_TYPE) {
     if (local_variables[local_fpga_node_vector[cur_node_idx].variable_index_]) {
       if (local_instantiation[local_fpga_node_vector[cur_node_idx].variable_index_] == (local_fpga_node_vector[cur_node_idx].literal_ > 0) ) {
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = 0;
       } else {
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] =
             -std::numeric_limits<float>::infinity();
       }
     } else {
       evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = 0;
     }
   } else if (local_fpga_node_vector[cur_node_idx].node_type_ == TOP_NODE_TYPE) {
     if (local_variables[local_fpga_node_vector[cur_node_idx].variable_index_]) {
       if (local_instantiation[local_fpga_node_vector[cur_node_idx].variable_index_]) {
         evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = local_bool_param_vector[fpga_node_vector[cur_node_idx].bool_param_offset];
       } else {
         evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = local_bool_param_vector[fpga_node_vector[cur_node_idx].bool_param_offset +1];
       }
     } else {
       evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = 0;
     }
   }
 }

  for(int j = PSDD_SIZE -1; j >= 0; j--){
//  #pragma HLS pipeline
    uint cur_node_idx = local_serialized_nodes[j];
    if (local_fpga_node_vector[cur_node_idx].node_type_ == DECISION_NODE_TYPE){
    uint element_size = local_fpga_node_vector[cur_node_idx].children_size;
    float max_prob = -std::numeric_limits<float>::infinity();

    assert(element_size <= MAX_CHILDREN);
      for (size_t i = 0; i < element_size; ++i) {
#pragma HLS pipeline
        uint cur_prime_idx = local_fpga_node_vector[local_children_vector[local_fpga_node_vector[cur_node_idx].children_offset + i]].node_index_;
        uint cur_sub_idx = local_fpga_node_vector[children_vector[local_fpga_node_vector[cur_node_idx].children_offset + local_fpga_node_vector[cur_node_idx].children_size + i]].node_index_;
        float tmp = evaluation_cache[local_fpga_node_vector[cur_prime_idx].node_index_] + evaluation_cache[local_fpga_node_vector[cur_sub_idx].node_index_] +  float (local_parameter_vector[local_fpga_node_vector[cur_node_idx].parameter_offset + i]);
        max_prob = (max_prob == -std::numeric_limits<float>::infinity() || max_prob < tmp) ? tmp : max_prob;
      }
       evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = max_prob;
    }
  }
  results[m] = evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_];

}
}

}  // extern "C"
