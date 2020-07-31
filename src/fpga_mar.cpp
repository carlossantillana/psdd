#include "../include/psdd/fpga_kernel_psdd_node.h"
#include <assert.h>
#include <iostream>

extern "C" {
  void loadBool(bool* data_local, int burstLength, char value){
    #pragma HLS inline off
    loadBool: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = value;
    }
  }
  void loadBool2(const ap_int<32> * data_dram, bool* data_local, int burstLength){
    #pragma HLS inline off
    load2Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](0,0);
    }
  }

  void load3Bit(const ap_int<32> * data_dram, ap_int<3>* data_local, int burstLength){
    #pragma HLS inline off
    load2Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](2,0);
    }
  }
  void load6Bit(const ap_uint<32>* data_dram, ap_uint<6>* data_local, int burstLength){
    #pragma HLS inline off
    load6Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](5,0);
    }
  }
  void load12Bit(const ap_uint<32>* data_dram, ap_uint<12>* data_local, int burstLength){
    #pragma HLS inline off
    load12Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](11,0);
    }
  }
  void load13Bit(const ap_int<32>* data_dram, ap_int<13>* data_local, int burstLength){
    #pragma HLS inline off
    load13Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](12,0);
    }
  }

  void load14Bit(const ap_int<32>* data_dram, ap_int<14>* data_local, int burstLength){
    #pragma HLS inline off
    load14Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](13,0);
    }
  }

  void load20Bit(const ap_uint<32>* data_dram, ap_uint<20>* data_local, int burstLength){
    #pragma HLS inline off
    load20Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](19,0);
    }
  }

   void load21Bit(const ap_uint<32>* data_dram, ap_uint<21>* data_local, int burstLength){
     #pragma HLS inline off
     load21Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](20,0);
     }
   }

   void load15Bit_staggered(const ap_uint<32>* data_dram, ap_uint<15>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load15BitStaggered: for (int i = start; i < start + burstLength; i++){
     #pragma HLS pipeline
       data_local[j++] = data_dram[i](14,0);
     }
   }

   void load_prime_staggered(const ap_uint<32>* data_dram, ap_uint<PRIME_BIT_WIDTH>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load16BitStaggered: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j++] = data_dram[i](PRIME_BIT_WIDTH-1,0);
     }
   }
   void load_sub_staggered(const ap_uint<32>* data_dram, ap_uint<SUB_BIT_WIDTH>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load16BitStaggered: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j++] = data_dram[i](SUB_BIT_WIDTH-1,0);
     }
   }

   void loadFloats_staggered(const float* data_dram, float* data_local, int start, int burstLength){
     #pragma HLS inline off
     int j = 0;
     loadFloatStaggered:  for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] =  data_dram[i];
       j++;
     }
   }

   void loadFloatsSmall(const ap_fixed<32,BOOL_DEC_WIDTH,AP_RND >* data_dram, ap_fixed<14,BOOL_DEC_WIDTH,AP_RND >* data_local, int burstLength){
     #pragma HLS inline off
     loadFloatSmall: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }
   void loadFloat(float* data_local, int burstLength, double value){
     #pragma HLS inline off
     loadFloat: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = value;
     }
   }

   void load(bool local_variables[MAX_VAR],  ap_int<NODE_TYPE_BIT_WIDTH> local_node_type_vector[PSDD_SIZE],
    const ap_int<32>  node_type_vector[PSDD_SIZE], ap_fixed<BOOL_BIT_WIDTH,BOOL_DEC_WIDTH,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM], const ap_fixed<32,2,AP_RND > bool_param_vector[TOTAL_BOOL_PARAM],
    ap_int<LITERAL_BIT_WIDTH> local_literal_vector [TOTAL_LITERALS], const ap_int<32> literal_vector [TOTAL_LITERALS],
    ap_int<LITERAL_VAR_BIT_WIDTH> local_literal_variable_vector [TOTAL_LITERALS], const ap_int<32> literal_variable_vector [TOTAL_LITERALS],
    ap_int<TOP_VAR_BIT_WIDTH> local_top_variable_vector [TOTAL_VARIABLE_INDEXES], const ap_int<32> top_variable_vector [TOTAL_VARIABLE_INDEXES],
    ap_uint<CHILD_SIZE_BIT_WIDTH> local_children_size_vector [PSDD_SIZE], const ap_uint<32> children_size_vector [PSDD_SIZE],
    ap_uint<CHILD_OFFSET_BIT_WIDTH>* local_children_offset_vector, const ap_uint<32>* children_offset_vector,
    ap_uint<LITERAL_IDX_BIT_WIDTH>* local_literal_index_vector, const ap_uint<32>* literal_index_vector,
    ap_uint<VAR_IDX_BIT_WIDTH>* local_variable_index_vector, const ap_uint<32>* variable_index_vector,
    ap_uint<SUB_BIT_WIDTH> local_sub_vector[TOTAL_CHILDREN], const ap_uint<32> sub_vector[TOTAL_CHILDREN],
    ap_uint<PRIME_BIT_WIDTH> local_prime_vector[TOTAL_CHILDREN], const ap_uint<32> prime_vector[TOTAL_CHILDREN],
    float derivatives[PSDD_SIZE], float marginalsTrue [NUM_VAR], float marginalsFalse [NUM_VAR]) {
     loadBool(local_variables, MAX_VAR, 1);
     load3Bit(node_type_vector, local_node_type_vector, PSDD_SIZE);
     load6Bit(children_size_vector, local_children_size_vector, PSDD_SIZE);
     load13Bit(literal_vector, local_literal_vector, TOTAL_LITERALS);
     load14Bit(literal_variable_vector, local_literal_variable_vector, TOTAL_LITERALS);
     load14Bit(top_variable_vector, local_top_variable_vector, TOTAL_VARIABLE_INDEXES);
     load20Bit(literal_index_vector, local_literal_index_vector, TOTAL_LITERALS);
     load20Bit(children_offset_vector, local_children_offset_vector, TOTAL_CHILDREN_SIZE);
     load20Bit(variable_index_vector, local_variable_index_vector, TOTAL_VARIABLE_INDEXES);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
     loadFloat(derivatives, PSDD_SIZE, 0);
     loadFloat(marginalsTrue, NUM_VAR, 0);
     loadFloat(marginalsFalse, NUM_VAR, 0);
     return;
   }

//Base 10
void fpga_mar(
        const ap_int<32> *node_type_vector, // Read-Only Vector 2
        const ap_uint<32> *prime_vector,
        const ap_uint<32> *sub_vector,
        const float *parameter_vector,
        const ap_fixed<32,BOOL_DEC_WIDTH,AP_RND> *bool_param_vector,
        const ap_int<32> *literal_vector,
        const ap_int<32> *literal_variable_vector,
        const ap_int<32> *top_variable_vector,
        const ap_uint<32> *children_size_vector,
        const ap_uint<32> *children_offset_vector,
        const ap_uint<32> *literal_index_vector,
        const ap_uint<32> *variable_index_vector,
        const ap_uint<32> *order,
        float *resultTrue,       // Output Result
        float *resultFalse,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port=node_type_vector  offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=prime_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=sub_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = top_variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_size_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_offset_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_index_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = variable_index_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = order offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = resultTrue offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port = resultFalse offset=slave bundle=gmem2
#pragma HLS INTERFACE s_axilite port = node_type_vector  bundle=control
#pragma HLS INTERFACE s_axilite port = prime_vector bundle=control
#pragma HLS INTERFACE s_axilite port = sub_vector bundle=control
#pragma HLS INTERFACE s_axilite port = parameter_vector bundle = control
#pragma HLS INTERFACE s_axilite port = bool_param_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = top_variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_size_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_offset_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_index_vector bundle = control
#pragma HLS INTERFACE s_axilite port = variable_index_vector bundle = control
#pragma HLS INTERFACE s_axilite port = order bundle = control
#pragma HLS INTERFACE s_axilite port = resultTrue bundle=control
#pragma HLS INTERFACE s_axilite port = resultFalse bundle=control
#pragma HLS INTERFACE s_axilite port = num_queries bundle=control
#pragma HLS INTERFACE s_axilite port = return bundle=control

  assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
  static bool local_variables [MAX_VAR];
  static bool local_instantiation [MAX_VAR];
  static ap_uint<PRIME_BIT_WIDTH> local_prime_vector[MAX_CHILDREN];
  static ap_uint<SUB_BIT_WIDTH> local_sub_vector[MAX_CHILDREN];
  static float local_parameter_vector[MAX_CHILDREN];
  static ap_fixed<BOOL_BIT_WIDTH,BOOL_DEC_WIDTH,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
  static ap_int<NODE_TYPE_BIT_WIDTH> local_node_type_vector[PSDD_SIZE];
  static ap_int<LITERAL_BIT_WIDTH> local_literal_vector [TOTAL_LITERALS];
  static ap_int<LITERAL_VAR_BIT_WIDTH> local_literal_variable_vector [TOTAL_LITERALS];
  static ap_int<TOP_VAR_BIT_WIDTH> local_top_variable_vector [TOTAL_VARIABLE_INDEXES];
  static ap_uint<CHILD_SIZE_BIT_WIDTH> local_children_size_vector [PSDD_SIZE];
  static ap_uint<CHILD_OFFSET_BIT_WIDTH> local_children_offset_vector [TOTAL_CHILDREN_SIZE];
  static ap_uint<LITERAL_IDX_BIT_WIDTH> local_literal_index_vector[TOTAL_LITERALS];
  static ap_uint<VAR_IDX_BIT_WIDTH> local_variable_index_vector[TOTAL_VARIABLE_INDEXES];
  float marginalsTrue [NUM_VAR];
  float marginalsFalse [NUM_VAR];
  float derivatives [PSDD_SIZE];

  #pragma HLS RESOURCE variable=local_instantiation core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=node_type_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_variable_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_top_variable_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_index_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_variable_index_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram

  load(local_variables, local_node_type_vector,
  node_type_vector, local_bool_param_vector, bool_param_vector,
   local_literal_vector, literal_vector, local_literal_variable_vector, literal_variable_vector,  local_top_variable_vector, top_variable_vector,
  local_children_size_vector, children_size_vector, local_children_offset_vector, children_offset_vector, local_literal_index_vector,
  literal_index_vector, local_variable_index_vector, variable_index_vector, local_sub_vector, sub_vector, local_prime_vector, prime_vector, derivatives,
  marginalsTrue, marginalsFalse);

  for (uint m = 0; m < num_queries; m++){
    derivatives[0] = 1;
    uint cur_decn_node = 0;
    for (int i = 0; i < PSDD_SIZE; i++){
      if (local_node_type_vector[order[i]] == DECISION_NODE_TYPE) {
        short element_size = local_children_size_vector[order[i]];
        load_sub_staggered(sub_vector, local_sub_vector, local_children_offset_vector[order[i]], element_size);
        load_prime_staggered(prime_vector, local_prime_vector, local_children_offset_vector[order[i]], element_size);
        loadFloats_staggered(parameter_vector, local_parameter_vector, local_children_offset_vector[order[i]], element_size);
        float cur_derivative = derivatives[PSDD_SIZE - 1 - order[i]];
        assert(element_size <= MAX_CHILDREN);
        InnerLoopPrime:for (uint j = 0; j < element_size; j++) {
          // #pragma HLS UNROLL <- would this be faster??
          #pragma HLS pipeline
          #pragma HLS dependence variable=derivatives inter false
            derivatives[PSDD_SIZE - 1 - local_prime_vector[j]] += cur_derivative * (local_parameter_vector[j]);
        }
          InnerLoopSub:for (uint j = 0; j < element_size; j++) {
            // #pragma HLS UNROLL
            #pragma HLS pipeline
            #pragma HLS dependence variable=derivatives inter false
              derivatives[PSDD_SIZE - 1 - local_sub_vector[j]] += cur_derivative * (local_parameter_vector[j]);
            }
        cur_decn_node++;
      }
    }

  LoopLiteral:for(int cur_node_idx = TOTAL_LITERALS-1; cur_node_idx >= 0; cur_node_idx--) {
    #pragma HLS dependence variable=marginalsTrue inter false
    #pragma HLS dependence variable=marginalsFalse inter false
    #pragma HLS pipeline

    if (local_literal_vector[cur_node_idx] > 0) {
      marginalsTrue[local_literal_variable_vector[cur_node_idx]] += derivatives[PSDD_SIZE - 1 - local_literal_index_vector[cur_node_idx]];
    } else {
      marginalsFalse[local_literal_variable_vector[cur_node_idx]] += derivatives[PSDD_SIZE - 1 - local_literal_index_vector[cur_node_idx]];
    }
 }

 LoopTop:for(int cur_node_idx = TOTAL_VARIABLE_INDEXES-1; cur_node_idx >= 0 && TOTAL_VARIABLE_INDEXES > 1; cur_node_idx--) {
  #pragma HLS pipeline
  #pragma HLS dependence variable=marginalsTrue inter false
  #pragma HLS dependence variable=marginalsFalse inter false

   marginalsFalse[local_top_variable_vector[cur_node_idx]] += derivatives[PSDD_SIZE - 1 - local_variable_index_vector[cur_node_idx]] * float (local_bool_param_vector[cur_node_idx +1]);
   marginalsTrue[local_top_variable_vector[cur_node_idx]] += derivatives[PSDD_SIZE - 1 - local_variable_index_vector[cur_node_idx]] * float (local_bool_param_vector[cur_node_idx]);
  }

    FinalLoop:for (int i = 0; i < NUM_VAR; i++){
      #pragma HLS pipeline
      float partition = marginalsFalse[i] + marginalsTrue[i];
      resultTrue[i] = marginalsTrue[i] / partition;
      resultFalse[i] =  marginalsFalse[i] / partition;
    }
    loadFloat(derivatives, PSDD_SIZE, 0);
    loadFloat(marginalsTrue, NUM_VAR, 0);
    loadFloat(marginalsFalse, NUM_VAR, 0);
  }
}
}
