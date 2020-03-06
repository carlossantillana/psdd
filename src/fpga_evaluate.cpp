#include <psdd/fpga_psdd_node.h>
#include <iostream>
#include <assert.h>

//For Small
//const int PsddBurstLength = 51;
//const int ChildrenBurstLength = 156;
//const int ParamBurstLength = 78;
//For Large
 const int PsddBurstLength = 580817;
 const int ChildrenBurstLength = 1541021;
 const int ParamBurstLength = 770511;

 void loadInts(const uint32_t* data_dram, uint32_t* data_local, int burstLength){
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

 void loadFloats(const float* data_dram, float* data_local, int burstLength){
   #pragma HLS inline off
   loadFloat: for (int i = 0; i < burstLength; i++){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }
//
// void Compute(const std::bitset<MAX_VAR> &variables,
//                      const std::bitset<MAX_VAR> &instantiation,
//                      uint32_t  serialized_nodes [PsddBurstLength],
//                      FPGAPsddNodeStruct fpga_node_vector[PsddBurstLength],
//                      uint32_t children_vector[ChildrenBurstLength],
//                    double parameter_vector[ParamBurstLength]){
// #pragma HLS inline off
//   compute: for (int i = PsddBurstLength-1; i >= 0; i--){
// #pragma HLS pipeline
//     if(i >= 0){
//
//     }
//   }
// }

 double EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> &instantiation,
                      uint32_t  serialized_nodes [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      uint32_t children_vector[TOTAL_CHILDREN],
                      float parameter_vector[TOTAL_PARAM]) {
  float evaluation_cache [PSDD_SIZE];
  const std::bitset<MAX_VAR> local_variables = variables;
  const std::bitset<MAX_VAR> local_instantiation = instantiation;
  uint32_t  local_serialized_nodes [PSDD_SIZE];
  loadInts(serialized_nodes, local_serialized_nodes, PsddBurstLength);
  FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE];
  loadStructs(fpga_node_vector, local_fpga_node_vector, PsddBurstLength);
  uint32_t local_children_vector[TOTAL_CHILDREN];
  loadInts(children_vector, local_children_vector, ChildrenBurstLength);
  float local_parameter_vector[TOTAL_PARAM];
  loadFloats(parameter_vector, local_parameter_vector, ParamBurstLength);
#pragma HLS RESOURCE variable=evaluation_cache core=XPM_MEMORY uram
  for(int j = PSDD_SIZE -1; j >= 0; j--){
 #pragma HLS pipeline
    uintmax_t cur_node_idx = local_serialized_nodes[j];
    if (local_fpga_node_vector[cur_node_idx].node_type_ == LITERAL_NODE_TYPE) {
     if (local_variables[local_fpga_node_vector[cur_node_idx].variable_index_]) {
       if ( instantiation[local_fpga_node_vector[cur_node_idx].variable_index_] == (local_fpga_node_vector[cur_node_idx].literal_ > 0) ) {
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
       if (instantiation[local_fpga_node_vector[cur_node_idx].variable_index_]) {
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = local_fpga_node_vector[cur_node_idx].true_parameter_;
       } else {
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = local_fpga_node_vector[cur_node_idx].false_parameter_;
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
        float tmp = evaluation_cache[local_fpga_node_vector[cur_prime_idx].node_index_] + evaluation_cache[local_fpga_node_vector[cur_sub_idx].node_index_] +  (local_parameter_vector[local_fpga_node_vector[cur_node_idx].parameter_offset + i]);
        max_prob = (max_prob == -std::numeric_limits<float>::infinity() || max_prob < tmp) ? tmp : max_prob;
      }
       evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = max_prob;
    }
  }
  return evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_];
}
