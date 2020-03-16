#include <psdd/fpga_psdd_node.h>
#include <psdd/fpga_evaluate.h>
#include <iostream>
#include <assert.h>
#include "ap_int.h"


 void load20Bit(const ap_uint<20>* data_dram, ap_uint<20>* data_local, int burstLength){
   #pragma HLS inline off
   loadInts: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
 void load21Bit(const ap_uint<21>* data_dram, ap_uint<21>* data_local, int burstLength){
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

 void loadFloats(const ap_fixed<18,7,AP_RND >* data_dram, ap_fixed<18,7,AP_RND >* data_local, int burstLength){
   #pragma HLS inline off
   loadFloat: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
 void loadFloatsSmall(const ap_fixed<12,1,AP_RND >* data_dram, ap_fixed<12,1,AP_RND >* data_local, int burstLength){
   #pragma HLS inline off
   loadFloatSmall: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
 void loadBool(const bool* data_dram, bool* data_local, int burstLength){
   #pragma HLS inline off
   loadBool: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
 void load(ap_uint<20>  local_serialized_nodes [PSDD_SIZE], ap_uint<20>  serialized_nodes [PSDD_SIZE], FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE],
   FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE], ap_uint<21> local_children_vector[TOTAL_CHILDREN], ap_uint<21> children_vector[TOTAL_CHILDREN], ap_fixed<18,7,AP_RND > local_parameter_vector[TOTAL_PARAM],
   ap_fixed<18,7,AP_RND > parameter_vector[TOTAL_PARAM], ap_fixed<12,1,AP_RND > local_bool_param_vector [TOTAL_BOOL_PARAM], ap_fixed<12,1,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM], bool local_instantiation[MAX_VAR], bool instantiation[MAX_VAR]){
   load20Bit(serialized_nodes, local_serialized_nodes, PsddBurstLength);
   loadStructs(fpga_node_vector, local_fpga_node_vector, PsddBurstLength);
   load21Bit(children_vector, local_children_vector, ChildrenBurstLength);
   loadFloats(parameter_vector, local_parameter_vector, ParamBurstLength);
   loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
   loadBool(instantiation, local_instantiation, MAX_VAR);
   return;
 }

//FPGA
 void EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      bool instantiation[MAX_VAR],
                      ap_uint<20>  serialized_nodes [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      ap_uint<21> children_vector[TOTAL_CHILDREN],
                      ap_fixed<18,7,AP_RND > parameter_vector[TOTAL_PARAM],
                      ap_fixed<12,1,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM],
					            float results[NUM_QUERIES]) {
  const std::bitset<MAX_VAR> local_variables = variables;
  bool local_instantiation[MAX_VAR];
  ap_uint<20> local_serialized_nodes [PSDD_SIZE];
  FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE];
  ap_uint<21> local_children_vector[TOTAL_CHILDREN];
  ap_fixed<18,7,AP_RND > local_parameter_vector[TOTAL_PARAM];
  ap_fixed<12,1,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
  load(local_serialized_nodes, serialized_nodes, local_fpga_node_vector,
    fpga_node_vector, local_children_vector, children_vector, local_parameter_vector,
    parameter_vector, local_bool_param_vector, bool_param_vector, local_instantiation, instantiation);

std::cout << std::endl;
#pragma HLS RESOURCE variable=local_serialized_nodes core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_children_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_bool_param_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram

  for (int m = 0; m < NUM_QUERIES; m++){
    float evaluation_cache [PSDD_SIZE];
#pragma HLS RESOURCE variable=local_evaluation_cache core=XPM_MEMORY uram
    for(int j = PSDD_SIZE -1; j >= 0; j--){
#pragma HLS pipeline
      uintmax_t cur_node_idx = local_serialized_nodes[j];
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
      uintmax_t cur_node_idx = local_serialized_nodes[j];
      if (local_fpga_node_vector[cur_node_idx].node_type_ == DECISION_NODE_TYPE){
      uint32_t element_size = local_fpga_node_vector[cur_node_idx].children_size;
      float max_prob = -std::numeric_limits<float>::infinity();

      assert(element_size <= MAX_CHILDREN);
        for (size_t i = 0; i < element_size; ++i) {
#pragma HLS pipeline
          uint32_t cur_prime_idx = local_fpga_node_vector[local_children_vector[local_fpga_node_vector[cur_node_idx].children_offset + i]].node_index_;
          uint32_t cur_sub_idx = local_fpga_node_vector[children_vector[local_fpga_node_vector[cur_node_idx].children_offset + local_fpga_node_vector[cur_node_idx].children_size + i]].node_index_;
          float tmp = evaluation_cache[local_fpga_node_vector[cur_prime_idx].node_index_] + evaluation_cache[local_fpga_node_vector[cur_sub_idx].node_index_] +  float (local_parameter_vector[local_fpga_node_vector[cur_node_idx].parameter_offset + i]);
          max_prob = (max_prob == -std::numeric_limits<float>::infinity() || max_prob < tmp) ? tmp : max_prob;
        }
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = max_prob;
      }
    }
    std::cout << "evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_]: " << evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_] << std::endl;
    results[m] = evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_];
  }
}

//CSIM
