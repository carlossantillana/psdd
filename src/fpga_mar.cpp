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

   void load16Bit_staggered(const ap_uint<32>* data_dram, ap_uint<16>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load16BitStaggered: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j++] = data_dram[i](15,0);
     }
   }

   void loadFloats_staggered(const ap_fixed<32,8,AP_RND >* data_dram, ap_fixed<16,8,AP_RND >* data_local, int start, int burstLength){
     #pragma HLS inline off
     int j = 0;
     loadFloatStaggered:  for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i];
       j++;
     }
   }

   void loadFloatsSmall(const ap_fixed<32,2,AP_RND >* data_dram, ap_fixed<14,2,AP_RND >* data_local, int burstLength){
     #pragma HLS inline off
     loadFloatSmall: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void load(bool local_variables[MAX_VAR],  bool local_is_decision_vector[PSDD_SIZE],
      const ap_int<32>  is_decision_vector[PSDD_SIZE], ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM], const ap_fixed<32,2,AP_RND > bool_param_vector[TOTAL_BOOL_PARAM],
       ap_int<13> local_literal_vector [TOTAL_LITERALS], const ap_int<32> literal_vector [TOTAL_LITERALS],
    ap_int<14> local_literal_variable_vector [TOTAL_LITERALS], const ap_int<32> literal_variable_vector [TOTAL_LITERALS], ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES], const ap_int<32> top_variable_vector [TOTAL_VARIABLE_INDEXES],
    ap_uint<6> local_children_size_vector [PSDD_SIZE], const ap_uint<32> children_size_vector [PSDD_SIZE], ap_uint<20>* local_children_offset_vector, const ap_uint<32>* children_offset_vector,
    ap_uint<20>* local_literal_index_vector, const ap_uint<32>* literal_index_vector, ap_uint<20>* local_variable_index_vector, const ap_uint<32>* variable_index_vector, ap_uint<15> local_sub_vector[TOTAL_CHILDREN],
    const ap_uint<32> sub_vector[TOTAL_CHILDREN], ap_uint<16> local_prime_vector[TOTAL_CHILDREN], const ap_uint<32> prime_vector[TOTAL_CHILDREN], ap_fixed<16,8,AP_RND >local_parameter_vector[TOTAL_CHILDREN], const ap_fixed<32,8,AP_RND>parameter_vector[TOTAL_CHILDREN]) {
     loadBool(local_variables, MAX_VAR, 1);
     loadBool2(is_decision_vector, local_is_decision_vector, PSDD_SIZE);
     load6Bit(children_size_vector, local_children_size_vector, PSDD_SIZE);
     load13Bit(literal_vector, local_literal_vector, TOTAL_LITERALS);
     load14Bit(literal_variable_vector, local_literal_variable_vector, TOTAL_LITERALS);
     load14Bit(top_variable_vector, local_top_variable_vector, TOTAL_VARIABLE_INDEXES);
     load20Bit(literal_index_vector, local_literal_index_vector, TOTAL_LITERALS);
     load20Bit(children_offset_vector, local_children_offset_vector, TOTAL_CHILDREN_SIZE);
     load20Bit(variable_index_vector, local_variable_index_vector, TOTAL_VARIABLE_INDEXES);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
     return;
   }

void fpga_mar(
        const ap_int<32> *is_decision_vector, // Read-Only Vector 2
        const ap_uint<32> *prime_vector,
        const ap_uint<32> *sub_vector,
        const ap_fixed<32,8,AP_RND> *parameter_vector,
        const ap_fixed<32,2,AP_RND> *bool_param_vector,
        const ap_int<32> *literal_vector,
        const ap_int<32> *literal_variable_vector,
        const ap_int<32> *top_variable_vector,
        const ap_uint<32> *children_size_vector,
        const ap_uint<32> *children_offset_vector,
        const ap_uint<32> *literal_index_vector,
        const ap_uint<32> *variable_index_vector,
        float *resultTrue,       // Output Result
        float *resultFalse,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port=is_decision_vector  offset=slave bundle=gmem0
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
#pragma HLS INTERFACE m_axi port = resultTrue offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port = resultFalse offset=slave bundle=gmem2
#pragma HLS INTERFACE s_axilite port = is_decision_vector  bundle=control
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
#pragma HLS INTERFACE s_axilite port = resultTrue bundle=control
#pragma HLS INTERFACE s_axilite port = resultFalse bundle=control
#pragma HLS INTERFACE s_axilite port = num_queries bundle=control
#pragma HLS INTERFACE s_axilite port = return bundle=control

  assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
  static bool local_variables [MAX_VAR];
  static bool local_instantiation [MAX_VAR];
  static ap_uint<16> local_prime_vector[MAX_CHILDREN];
  static ap_uint<15> local_sub_vector[MAX_CHILDREN];
  static ap_fixed<16,8,AP_RND > local_parameter_vector[MAX_CHILDREN];
  static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
  static bool local_is_decision_vector[PSDD_SIZE];
  static ap_int<13> local_literal_vector [TOTAL_LITERALS];
  static ap_int<14> local_literal_variable_vector [TOTAL_LITERALS];
  static ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES];
  static ap_uint<6> local_children_size_vector [PSDD_SIZE];
  static ap_uint<20> local_children_offset_vector [TOTAL_CHILDREN_SIZE];
  static ap_uint<20> local_literal_index_vector[TOTAL_LITERALS];
  static ap_uint<20> local_variable_index_vector[TOTAL_VARIABLE_INDEXES];
  //Might not be needed?? User data can be replaced with local_literal_vector etc...
  static int user_data [PSDD_SIZE];
  float marginalsTrue [1220];
  float marginalsFalse [1220];
  float derivatives [PSDD_SIZE];
  int index = 0;
  InitLoop:for (int i = 0; i < PSDD_SIZE; i++){
    #pragma HLS pipeline
    user_data[i] = index++;
    derivatives[i] = -std::numeric_limits<float>::infinity();
  }
  for (int i = 0; i < 1220; i++){
    marginalsTrue[i] = -std::numeric_limits<float>::infinity();
    marginalsFalse[i] = -std::numeric_limits<float>::infinity();
  }

  derivatives[0] = 0;
  #pragma HLS RESOURCE variable=local_instantiation core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_is_decision_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_variable_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_top_variable_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_index_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_variable_index_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram


  load(local_variables, local_is_decision_vector,
  is_decision_vector, local_bool_param_vector, bool_param_vector,
   local_literal_vector, literal_vector, local_literal_variable_vector, literal_variable_vector,  local_top_variable_vector, top_variable_vector,
  local_children_size_vector, children_size_vector, local_children_offset_vector, children_offset_vector, local_literal_index_vector,
  literal_index_vector, local_variable_index_vector, variable_index_vector, local_sub_vector, sub_vector, local_prime_vector, prime_vector, local_parameter_vector, parameter_vector);

  for (uint m = 0; m < num_queries; m++){

    uint cur_decn_node = 0;
    LoopDecision:for(int cur_node_idx = PSDD_SIZE-1; cur_node_idx >= 0; cur_node_idx--) {
      // std::cout << cur_node_idx << ", ";
      if (local_is_decision_vector[cur_node_idx]) {
        // std::cout << "at decision node: " << cur_node_idx << std::endl;
      short element_size = local_children_size_vector[cur_node_idx];
       load15Bit_staggered(sub_vector, local_sub_vector, local_children_offset_vector[cur_node_idx], element_size);
       load16Bit_staggered(prime_vector, local_prime_vector, local_children_offset_vector[cur_node_idx], element_size);
       loadFloats_staggered(parameter_vector, local_parameter_vector, local_children_offset_vector[cur_node_idx], element_size);
        float cur_derivative = derivatives[user_data[cur_node_idx]];
        assert(element_size <= MAX_CHILDREN);
        // std::cout << " c: {" ;
          InnerLoop:for (uint i = 0; i < element_size; ++i) {
    // #pragma HLS pipeline
            // std::cout << local_prime_vector[i] << ", ";
            float tmp = float (local_parameter_vector[i]);
            if (cur_derivative == -std::numeric_limits<double>::infinity()){
              tmp = float (local_parameter_vector[i]);
            } else if (float (local_parameter_vector[i]) == -std::numeric_limits<double>::infinity()){
              continue;
            } else {
              if (cur_derivative > float (local_parameter_vector[i])) {
                tmp = cur_derivative + std::log1p(std::exp(float (local_parameter_vector[i]) - cur_derivative));
              } else {
                tmp = float (local_parameter_vector[i]) + std::log1p(std::exp(cur_derivative - float (local_parameter_vector[i])));
              }
            }
            if (derivatives[user_data[local_prime_vector[i]]] == -std::numeric_limits<double>::infinity()){
              derivatives[user_data[local_prime_vector[i]]] = tmp;
            } else if (tmp == -std::numeric_limits<double>::infinity()){
              continue;
            } else {
              if (derivatives[user_data[local_prime_vector[i]]] > tmp) {
                derivatives[user_data[local_prime_vector[i]]] = derivatives[user_data[local_prime_vector[i]]] + std::log1p(std::exp(tmp - derivatives[user_data[local_prime_vector[i]]]));
              } else {
                derivatives[user_data[local_prime_vector[i]]] = tmp + std::log1p(std::exp(derivatives[user_data[local_prime_vector[i]]] - tmp));
              }
            }

            if (derivatives[user_data[local_sub_vector[i]]] == -std::numeric_limits<double>::infinity()){
              derivatives[user_data[local_sub_vector[i]]] = tmp;
            } else if (tmp == -std::numeric_limits<double>::infinity()){
              continue;
            } else {
              if (derivatives[user_data[local_sub_vector[i]]] > tmp) {
                derivatives[user_data[local_sub_vector[i]]] = derivatives[user_data[local_sub_vector[i]]] + std::log1p(std::exp(tmp - derivatives[user_data[local_sub_vector[i]]]));
              } else {
                derivatives[user_data[local_sub_vector[i]]] = tmp + std::log1p(std::exp(derivatives[user_data[local_sub_vector[i]]] - tmp));
              }
            }
          }
          // std::cout << "} ";
          cur_decn_node++;
      }
    }
    // std::cout << "\nmax decision node: " << cur_decn_node << std::endl;
   //  LoopTop:for(uint cur_node_idx = 0; cur_node_idx < TOTAL_VARIABLE_INDEXES; cur_node_idx++) {
   // // #pragma HLS pipeline
   //     marginalsFalse[local_top_variable_vector[cur_node_idx]] = marginalsFalse[local_top_variable_vector[cur_node_idx]] + derivatives[user_data[local_variable_index_vector[cur_node_idx]]]
   //       * float (local_bool_param_vector[cur_node_idx +1]);
   //     marginalsTrue[local_top_variable_vector[cur_node_idx]] = marginalsTrue[local_top_variable_vector[cur_node_idx]] + derivatives[user_data[local_variable_index_vector[cur_node_idx]]]
   //       * float(local_bool_param_vector[cur_node_idx]);
   //   }

  LoopLiteral:for(int cur_node_idx = TOTAL_LITERALS-1; cur_node_idx >= 0; cur_node_idx--) {
  // #pragma HLS pipeline
    if (local_literal_vector[cur_node_idx] > 0) {
      if (marginalsTrue[local_literal_variable_vector[cur_node_idx]] == -std::numeric_limits<double>::infinity()){
        marginalsTrue[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]];
      } else if (derivatives[user_data[local_literal_index_vector[cur_node_idx]]] == -std::numeric_limits<double>::infinity()){
        continue;
      } else {
        if (marginalsTrue[local_literal_variable_vector[cur_node_idx]] > derivatives[user_data[local_literal_index_vector[cur_node_idx]]]) {
          marginalsTrue[local_literal_variable_vector[cur_node_idx]] = marginalsTrue[local_literal_variable_vector[cur_node_idx]] + std::log1p(std::exp(derivatives[user_data[local_literal_index_vector[cur_node_idx]]] - marginalsTrue[local_literal_variable_vector[cur_node_idx]]));
        } else {
          marginalsTrue[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]] + std::log1p(std::exp(marginalsTrue[local_literal_variable_vector[cur_node_idx]] - derivatives[user_data[local_literal_index_vector[cur_node_idx]]]));
        }
      }
    } else {
      if (marginalsFalse[local_literal_variable_vector[cur_node_idx]] == -std::numeric_limits<double>::infinity()){
        marginalsFalse[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]];
      } else if (derivatives[user_data[local_literal_index_vector[cur_node_idx]]] == -std::numeric_limits<double>::infinity()){
        continue;
      } else {
        if (marginalsFalse[local_literal_variable_vector[cur_node_idx]] > derivatives[user_data[local_literal_index_vector[cur_node_idx]]]) {
          marginalsFalse[local_literal_variable_vector[cur_node_idx]] = marginalsFalse[local_literal_variable_vector[cur_node_idx]] + std::log1p(std::exp(derivatives[user_data[local_literal_index_vector[cur_node_idx]]] - marginalsFalse[local_literal_variable_vector[cur_node_idx]]));
        } else {
          marginalsFalse[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]] + std::log1p(std::exp(marginalsFalse[local_literal_variable_vector[cur_node_idx]] - derivatives[user_data[local_literal_index_vector[cur_node_idx]]]));
        }
      }
    }
 }
  // std::cout << "fpga\n";
    FinalLoop:for (int i = 0; i < 1220; i++){
      // user_data[i] = 0;
      float partition = marginalsFalse[i];
      if (marginalsFalse[i] == -std::numeric_limits<double>::infinity()){
        partition = marginalsTrue[i];
      } else if (marginalsTrue[i] == -std::numeric_limits<double>::infinity()){
        continue;
      } else {
        if (marginalsFalse[i] > marginalsTrue[i]) {
          partition = marginalsFalse[i] + std::log1p(std::exp(marginalsTrue[i] - marginalsFalse[i]));
        } else {
          partition = marginalsTrue[i] + std::log1p(std::exp(marginalsFalse[i] - marginalsTrue[i]));
        }
      }
      resultTrue[i] = marginalsTrue[i] - partition;
      resultFalse[i] =  marginalsFalse[i] - partition;
      // std::cout << "i: " << i << " true: " << marginalsTrue[i] << " false: " << marginalsFalse[i] << " partition[i] " << partition <<  std::endl;
    }
  }
}
}
