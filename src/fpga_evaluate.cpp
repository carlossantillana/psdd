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
  double evaluation_cache [PSDD_SIZE];
   uint32_t local_serialized_nodes [PsddBurstLength];
   LoadInts(serialized_nodes, local_serialized_nodes, PsddBurstLength);
   FPGAPsddNodeStruct local_fpga_node_vector[PsddBurstLength];
   LoadStructs(fpga_node_vector,local_fpga_node_vector, PsddBurstLength);
   uint32_t local_children_vector[ChildrenBurstLength];
   LoadInts(children_vector, local_children_vector, ChildrenBurstLength);
   float local_parameter_vector[ParamBurstLength];
   LoadFloats(parameter_vector, local_parameter_vector, ParamBurstLength);

   const int kMinTripCount = 0;
   const int kMaxTripCount = kMinTripCount + PSDD_SIZE /PsddBurstLength;

  for(int j = PSDD_SIZE -1; j >= 0; j--){
#pragma HLS pipeline
	  #pragma HLS LOOP_TRIPCOUNT min=PSDD_SIZE max=PSDD_SIZE

    uintmax_t cur_node_idx = local_serialized_nodes[j];
     if (local_fpga_node_vector[cur_node_idx].node_type_ == LITERAL_NODE_TYPE) {
      if (variables[local_fpga_node_vector[cur_node_idx].variable_index_]) {
        if ( instantiation[local_fpga_node_vector[cur_node_idx].variable_index_] == (local_fpga_node_vector[cur_node_idx].literal_ > 0) ) {
          evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] =
              1;
        } else {
          evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] =
              0;
        }
      } else {
        evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] =
            1;
      }
    } else if (local_fpga_node_vector[cur_node_idx].node_type_ == TOP_NODE_TYPE) {
      if (variables[local_fpga_node_vector[cur_node_idx].variable_index_]) {
        if (instantiation[local_fpga_node_vector[cur_node_idx].variable_index_]) {
          evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = local_fpga_node_vector[cur_node_idx].true_parameter_;
        } else {
          evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = local_fpga_node_vector[cur_node_idx].false_parameter_;
        }
      } else {
        evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] =
            1;
      }
    } else {
      uint32_t element_size = local_fpga_node_vector[cur_node_idx].children_size;
      double cur_prob = 0;

      assert(element_size <= MAX_CHILDREN);
      for (size_t i = 0; i < element_size; ++i) {
#pragma HLS dependence variable=cur_prob inter false
#pragma HLS LOOP_TRIPCOUNT min=0 max=MAX_CHILDREN
#pragma HLS unroll factor=8

        uint32_t cur_prime_idx = local_fpga_node_vector[local_children_vector[local_fpga_node_vector[cur_node_idx].children_offset + i]].node_index_;
        uint32_t cur_sub_idx = local_fpga_node_vector[local_children_vector[local_fpga_node_vector[cur_node_idx].children_offset + local_fpga_node_vector[cur_node_idx].children_size + i]].node_index_;
        double tmp = evaluation_cache[local_fpga_node_vector[cur_prime_idx].node_index_] * evaluation_cache[local_fpga_node_vector[cur_sub_idx].node_index_] *  local_parameter_vector[local_fpga_node_vector[cur_node_idx].parameter_offset + i];
#pragma HLS dependence variable=tmp inter false

//        tmp *= local_parameter_vector[local_fpga_node_vector[cur_node_idx].parameter_offset + i];

        if (cur_prob == 0) {
          // if this is zero
          cur_prob =  tmp;
          continue;
        } else if (tmp == 0) {
//          cur_prob = cur_prob;
          continue;
        } else {
          if (cur_prob > tmp) {
            cur_prob =  cur_prob * (tmp / cur_prob);
          } else {
            cur_prob = tmp * (cur_prob / tmp);
          }
        }
      }
      evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = cur_prob;
    }
  }
  return evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_];
 }
