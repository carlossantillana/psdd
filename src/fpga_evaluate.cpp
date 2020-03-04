#include <psdd/fpga_psdd_node.h>
#include <iostream>
#include <assert.h>

//For Small
const int PsddBurstLength = 51;
const int ChildrenBurstLength = 156;
const int ParamBurstLength = 78;
//For Large
// const int PsddBurstLength = 1000;
// const int ChildrenBurstLength = 1541021;
// const int ParamBurstLength = 770511;

 void LoadInts(const uint32_t* data_dram, uint32_t* data_local, int burstLength){
   #pragma HLS inline off
   loadInts: for (int i = burstLength-1; i >= 0; i--){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }

 void LoadStructs(const FPGAPsddNodeStruct* data_dram, FPGAPsddNodeStruct* data_local, int burstLength){
   #pragma HLS inline off
   loadStruct: for (int i = burstLength-1; i >= 0; i--){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }

 void LoadFloats(const float* data_dram, float* data_local, int burstLength){
   #pragma HLS inline off
   loadFloat: for (int i = burstLength-1; i >= 0; i--){
   #pragma HLS pipeline
     data_local[i] = data_dram[i];
   }
 }

 void Compute(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> &instantiation,
                      uint32_t  serialized_nodes [PsddBurstLength],
                      FPGAPsddNodeStruct fpga_node_vector[PsddBurstLength],
                      uint32_t children_vector[ChildrenBurstLength],
                    double parameter_vector[ParamBurstLength]){
 #pragma HLS inline off
   compute: for (int i = PsddBurstLength-1; i >= 0; i--){
 #pragma HLS pipeline
     if(i >= 0){

     }
   }
 }

 double EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> &instantiation,
                      uint32_t  serialized_nodes [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      uint32_t children_vector[TOTAL_CHILDREN],
                      float parameter_vector[TOTAL_PARAM]) {
  float evaluation_cache [PSDD_SIZE];
  for(int j = PSDD_SIZE -1; j >= 0; j--){
 #pragma HLS pipeline
    uintmax_t cur_node_idx = serialized_nodes[j];
    if (fpga_node_vector[cur_node_idx].node_type_ == LITERAL_NODE_TYPE) {
     if (variables[fpga_node_vector[cur_node_idx].variable_index_]) {
       if ( instantiation[fpga_node_vector[cur_node_idx].variable_index_] == (fpga_node_vector[cur_node_idx].literal_ > 0) ) {
         evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
             0;
       } else {
         evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
             -std::numeric_limits<float>::infinity();
       }
     } else {
       evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
           0;
     }
   } else if (fpga_node_vector[cur_node_idx].node_type_ == TOP_NODE_TYPE) {
     if (variables[fpga_node_vector[cur_node_idx].variable_index_]) {
       if (instantiation[fpga_node_vector[cur_node_idx].variable_index_]) {
         evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = fpga_node_vector[cur_node_idx].true_parameter_;
       } else {
         evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = fpga_node_vector[cur_node_idx].false_parameter_;
       }
     } else {
       evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
           0;
     }
   }
 }

  for(int j = PSDD_SIZE -1; j >= 0; j--){
  //#pragma HLS pipeline
    uintmax_t cur_node_idx = serialized_nodes[j];
    if (fpga_node_vector[cur_node_idx].node_type_ == DECISION_NODE_TYPE){
    uint32_t element_size = fpga_node_vector[cur_node_idx].children_size;
    float max_prob = -std::numeric_limits<float>::infinity();

    assert(element_size <= MAX_CHILDREN);
      for (size_t i = 0; i < element_size; ++i) {
        #pragma HLS pipeline
        uint32_t cur_prime_idx = fpga_node_vector[children_vector[fpga_node_vector[cur_node_idx].children_offset + i]].node_index_;
        uint32_t cur_sub_idx = fpga_node_vector[children_vector[fpga_node_vector[cur_node_idx].children_offset + fpga_node_vector[cur_node_idx].children_size + i]].node_index_;
        float tmp = evaluation_cache[fpga_node_vector[cur_prime_idx].node_index_] + evaluation_cache[fpga_node_vector[cur_sub_idx].node_index_] +  (parameter_vector[fpga_node_vector[cur_node_idx].parameter_offset + i]);
        max_prob = (max_prob == -std::numeric_limits<float>::infinity() || max_prob < tmp) ? tmp : max_prob;
      }
       evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = max_prob;
    }
  }
  return evaluation_cache[fpga_node_vector[serialized_nodes[0]].node_index_];
}
