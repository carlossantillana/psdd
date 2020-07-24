#include "../include/psdd/fpga_kernel_psdd_node.h"
#include <assert.h>
#include <iostream>

#define FMUL_LAT 10


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

   void load16Bit_staggered(const ap_uint<32>* data_dram, ap_uint<16>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load16BitStaggered: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j++] = data_dram[i](15,0);
     }
   }

   void loadFloats_staggered(const ap_fixed<32,PARAM_DEC_WIDTH,AP_RND >* data_dram, ap_fixed<PARAM_BIT_WIDTH,PARAM_DEC_WIDTH,AP_RND >* data_local, int start, int burstLength){
     #pragma HLS inline off
     int j = 0;
     loadFloatStaggered:  for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i];
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
        const ap_fixed<32,PARAM_DEC_WIDTH,AP_RND> *parameter_vector,
        const ap_fixed<32,BOOL_DEC_WIDTH,AP_RND> *bool_param_vector,
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
#pragma HLS INTERFACE s_axilite port = resultTrue bundle=control
#pragma HLS INTERFACE s_axilite port = resultFalse bundle=control
#pragma HLS INTERFACE s_axilite port = num_queries bundle=control
#pragma HLS INTERFACE s_axilite port = return bundle=control

  assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
  static bool local_variables [MAX_VAR];
  static bool local_instantiation [MAX_VAR];
  static ap_uint<PRIME_BIT_WIDTH> local_prime_vector[MAX_CHILDREN];
  static ap_uint<SUB_BIT_WIDTH> local_sub_vector[MAX_CHILDREN];
  static ap_fixed<PARAM_BIT_WIDTH,PARAM_DEC_WIDTH,AP_RND > local_parameter_vector[MAX_CHILDREN];
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

  derivatives[0] = 1;

  for (uint m = 0; m < num_queries; m++){
    uint cur_decn_node = 0;
    LoopDecision:for(int cur_node_idx = PSDD_SIZE-1; cur_node_idx >= 0; cur_node_idx--) {
      if (local_node_type_vector[cur_node_idx] == DECISION_NODE_TYPE) {
        short element_size = local_children_size_vector[cur_node_idx];
        load16Bit_staggered(sub_vector, local_sub_vector, local_children_offset_vector[cur_node_idx], element_size);
        load16Bit_staggered(prime_vector, local_prime_vector, local_children_offset_vector[cur_node_idx], element_size);
        loadFloats_staggered(parameter_vector, local_parameter_vector, local_children_offset_vector[cur_node_idx], element_size);
        float cur_derivative = derivatives[PSDD_SIZE - 1 - cur_node_idx];
        assert(element_size <= MAX_CHILDREN);
        InnerLoopPrime:for (uint i = 0; i < element_size; i++) {
          // #pragma HLS UNROLL <- would this be faster??
          #pragma HLS pipeline
          #pragma HLS dependence variable=derivatives inter false
            derivatives[PSDD_SIZE - 1 - local_prime_vector[i]] += cur_derivative * float(local_parameter_vector[i]);
          }
          InnerLoopSub:for (uint i = 0; i < element_size; i++) {
            // #pragma HLS UNROLL
            #pragma HLS pipeline
            #pragma HLS dependence variable=derivatives inter false
              derivatives[PSDD_SIZE- 1 - local_sub_vector[i]] += cur_derivative * float(local_parameter_vector[i]);
            }
        cur_decn_node++;
      }
    }
    LoopTop:for(int cur_node_idx = TOTAL_VARIABLE_INDEXES-1; cur_node_idx >= 0 && TOTAL_VARIABLE_INDEXES > 1; cur_node_idx--) {
   #pragma HLS pipeline
   #pragma HLS dependence variable=marginalsTrue inter false
   #pragma HLS dependence variable=marginalsFalse inter false

    marginalsFalse[local_top_variable_vector[cur_node_idx]] += derivatives[PSDD_SIZE - 1 - local_variable_index_vector[cur_node_idx]] * float (local_bool_param_vector[cur_node_idx +1]);
    marginalsTrue[local_top_variable_vector[cur_node_idx]] += derivatives[PSDD_SIZE - 1 - local_variable_index_vector[cur_node_idx]] * float (local_bool_param_vector[cur_node_idx]);
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
    FinalLoop:for (int i = 0; i < NUM_VAR; i++){
      #pragma HLS pipeline
      float partition = marginalsFalse[i] + marginalsTrue[i];
      resultTrue[i] = marginalsTrue[i] / partition;
      resultFalse[i] =  marginalsFalse[i] / partition;
    }
  }
}
}


//Log Scale

// void fpga_mar(
//         const ap_int<32> *node_type_vector, // Read-Only Vector 2
//         const ap_uint<32> *prime_vector,
//         const ap_uint<32> *sub_vector,
//         const ap_fixed<32,8,AP_RND> *parameter_vector,
//         const ap_fixed<32,2,AP_RND> *bool_param_vector,
//         const ap_int<32> *literal_vector,
//         const ap_int<32> *literal_variable_vector,
//         const ap_int<32> *top_variable_vector,
//         const ap_uint<32> *children_size_vector,
//         const ap_uint<32> *children_offset_vector,
//         const ap_uint<32> *literal_index_vector,
//         const ap_uint<32> *variable_index_vector,
//         float *resultTrue,       // Output Result
//         float *resultFalse,       // Output Result
//         int num_queries)
// {
// #pragma HLS INTERFACE m_axi port=node_type_vector  offset=slave bundle=gmem0
// #pragma HLS INTERFACE m_axi port=prime_vector  offset=slave bundle=gmem
// #pragma HLS INTERFACE m_axi port=sub_vector  offset=slave bundle=gmem
// #pragma HLS INTERFACE m_axi port = parameter_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = bool_param_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = literal_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = literal_variable_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = top_variable_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = children_size_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = children_offset_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = literal_index_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = variable_index_vector offset = slave bundle = gmem
// #pragma HLS INTERFACE m_axi port = resultTrue offset=slave bundle=gmem1
// #pragma HLS INTERFACE m_axi port = resultFalse offset=slave bundle=gmem2
// #pragma HLS INTERFACE s_axilite port = node_type_vector  bundle=control
// #pragma HLS INTERFACE s_axilite port = prime_vector bundle=control
// #pragma HLS INTERFACE s_axilite port = sub_vector bundle=control
// #pragma HLS INTERFACE s_axilite port = parameter_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = bool_param_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = literal_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = literal_variable_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = top_variable_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = children_size_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = children_offset_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = literal_index_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = variable_index_vector bundle = control
// #pragma HLS INTERFACE s_axilite port = resultTrue bundle=control
// #pragma HLS INTERFACE s_axilite port = resultFalse bundle=control
// #pragma HLS INTERFACE s_axilite port = num_queries bundle=control
// #pragma HLS INTERFACE s_axilite port = return bundle=control
//
//   assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
//   static bool local_variables [MAX_VAR];
//   static bool local_instantiation [MAX_VAR];
//   static ap_uint<16> local_prime_vector[MAX_CHILDREN];
//   static ap_uint<16> local_sub_vector[MAX_CHILDREN];
//   static ap_fixed<16,8,AP_RND > local_parameter_vector[MAX_CHILDREN];
//   static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
//   static ap_int<3> local_node_type_vector[PSDD_SIZE];
//   static ap_int<13> local_literal_vector [TOTAL_LITERALS];
//   static ap_int<14> local_literal_variable_vector [TOTAL_LITERALS];
//   static ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES];
//   static ap_uint<6> local_children_size_vector [PSDD_SIZE];
//   static ap_uint<20> local_children_offset_vector [TOTAL_CHILDREN_SIZE];
//   static ap_uint<20> local_literal_index_vector[TOTAL_LITERALS];
//   static ap_uint<20> local_variable_index_vector[TOTAL_VARIABLE_INDEXES];
//   //Might not be needed?? User data can be replaced with local_literal_vector etc...
//   static int user_data [PSDD_SIZE];
//   float marginalsTrue [NUM_VAR];
//   float marginalsFalse [NUM_VAR];
//   float derivatives [PSDD_SIZE];
//   int index = 0;
//   InitLoop:for (int i = PSDD_SIZE-1; i >= 0 ; i--){
//     #pragma HLS pipeline
//     user_data[i] = index++;
//     derivatives[i] = -std::numeric_limits<float>::infinity();
//   }
//   for (int i = 0; i < NUM_VAR; i++){
//     marginalsTrue[i] = -std::numeric_limits<float>::infinity();
//     marginalsFalse[i] = -std::numeric_limits<float>::infinity();
//   }
//
//   derivatives[0] = 0;
//   #pragma HLS RESOURCE variable=local_instantiation core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=node_type_vector core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_literal_vector core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_literal_variable_vector core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_top_variable_vector core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_literal_index_vector core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_variable_index_vector core=XPM_MEMORY uram
//   #pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram
//
//
//   load(local_variables, local_node_type_vector,
//   node_type_vector, local_bool_param_vector, bool_param_vector,
//    local_literal_vector, literal_vector, local_literal_variable_vector, literal_variable_vector,  local_top_variable_vector, top_variable_vector,
//   local_children_size_vector, children_size_vector, local_children_offset_vector, children_offset_vector, local_literal_index_vector,
//   literal_index_vector, local_variable_index_vector, variable_index_vector, local_sub_vector, sub_vector, local_prime_vector, prime_vector, local_parameter_vector, parameter_vector);
//
//   for (uint m = 0; m < num_queries; m++){
//
//     uint cur_decn_node = 0;
//     LoopDecision:for(int cur_node_idx = PSDD_SIZE-1; cur_node_idx >= 0; cur_node_idx--) {
//       if (local_node_type_vector[cur_node_idx] == DECISION_NODE_TYPE) {
//       short element_size = local_children_size_vector[cur_node_idx];
//        load16Bit_staggered(sub_vector, local_sub_vector, local_children_offset_vector[cur_node_idx], element_size);
//        load16Bit_staggered(prime_vector, local_prime_vector, local_children_offset_vector[cur_node_idx], element_size);
//        loadFloats_staggered(parameter_vector, local_parameter_vector, local_children_offset_vector[cur_node_idx], element_size);
//         float cur_derivative = derivatives[user_data[cur_node_idx]];
//         assert(element_size <= MAX_CHILDREN);
//           InnerLoop:for (uint i = 0; i < element_size; i++) {
//     // #pragma HLS pipeline
//             float tmp = cur_derivative + float(local_parameter_vector[i]);
//             if (derivatives[user_data[local_prime_vector[i]]] == -std::numeric_limits<double>::infinity()){
//               derivatives[user_data[local_prime_vector[i]]] = tmp;
//             } else if (tmp == -std::numeric_limits<double>::infinity()){
//               continue;
//             }
//              else {
//               if (derivatives[user_data[local_prime_vector[i]]] > tmp) {
//                 derivatives[user_data[local_prime_vector[i]]] = derivatives[user_data[local_prime_vector[i]]] + std::log1p(std::exp(tmp - derivatives[user_data[local_prime_vector[i]]]));
//               } else {
//                 derivatives[user_data[local_prime_vector[i]]] = tmp + std::log1p(std::exp(derivatives[user_data[local_prime_vector[i]]] - tmp));
//               }
//             }
//             if (derivatives[user_data[local_sub_vector[i]]] == -std::numeric_limits<double>::infinity()){
//               derivatives[user_data[local_sub_vector[i]]] = tmp;
//             } else if (tmp == -std::numeric_limits<double>::infinity()){
//               continue;
//             } else {
//               if (derivatives[user_data[local_sub_vector[i]]] > tmp) {
//                 derivatives[user_data[local_sub_vector[i]]] = derivatives[user_data[local_sub_vector[i]]] + std::log1p(std::exp(tmp - derivatives[user_data[local_sub_vector[i]]]));
//               } else {
//                 derivatives[user_data[local_sub_vector[i]]] = tmp + std::log1p(std::exp(derivatives[user_data[local_sub_vector[i]]] - tmp));
//               }
//             }
//           }
//           cur_decn_node++;
//       }
//     }
//     LoopTop:for(int cur_node_idx = TOTAL_VARIABLE_INDEXES-1; cur_node_idx >= 0 && TOTAL_VARIABLE_INDEXES > 1; cur_node_idx--) {
//    // #pragma HLS pipeline
//          float tmpFalse = derivatives[user_data[local_variable_index_vector[cur_node_idx]]] + float (local_bool_param_vector[cur_node_idx +1]);
//          float tmpTrue = derivatives[user_data[local_variable_index_vector[cur_node_idx]]] + float(local_bool_param_vector[cur_node_idx]);
//
//          if (marginalsFalse[local_top_variable_vector[cur_node_idx]] == -std::numeric_limits<double>::infinity()){
//            marginalsFalse[local_top_variable_vector[cur_node_idx]] = tmpFalse;
//          } else if (tmpFalse == -std::numeric_limits<double>::infinity()){
//            continue;
//          } else {
//            if (marginalsFalse[local_top_variable_vector[cur_node_idx]] > tmpFalse) {
//              marginalsFalse[local_top_variable_vector[cur_node_idx]] = marginalsFalse[local_top_variable_vector[cur_node_idx]] + std::log1p(std::exp(tmpFalse - marginalsFalse[local_top_variable_vector[cur_node_idx]]));
//            } else {
//              marginalsFalse[local_top_variable_vector[cur_node_idx]] = tmpFalse + std::log1p(std::exp(marginalsFalse[local_top_variable_vector[cur_node_idx]] - tmpFalse));
//            }
//          }
//
//          if (marginalsTrue[local_top_variable_vector[cur_node_idx]] == -std::numeric_limits<double>::infinity()){
//            marginalsTrue[local_top_variable_vector[cur_node_idx]] = tmpTrue;
//          } else if (tmpTrue == -std::numeric_limits<double>::infinity()){
//            continue;
//          } else {
//            if (marginalsTrue[local_top_variable_vector[cur_node_idx]] > tmpTrue) {
//              marginalsTrue[local_top_variable_vector[cur_node_idx]] = marginalsTrue[local_top_variable_vector[cur_node_idx]] + std::log1p(std::exp(tmpTrue - marginalsTrue[local_top_variable_vector[cur_node_idx]]));
//            } else {
//              marginalsTrue[local_top_variable_vector[cur_node_idx]] = tmpTrue + std::log1p(std::exp(marginalsTrue[local_top_variable_vector[cur_node_idx]] - tmpTrue));
//            }
//          }
//      }
//
//
//   LoopLiteral:for(int cur_node_idx = TOTAL_LITERALS-1; cur_node_idx >= 0; cur_node_idx--) {
//   // #pragma HLS pipeline
//     if (local_literal_vector[cur_node_idx] > 0) {
//       if (marginalsTrue[local_literal_variable_vector[cur_node_idx]] == -std::numeric_limits<double>::infinity()){
//         marginalsTrue[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]];
//       } else if (derivatives[user_data[local_literal_index_vector[cur_node_idx]]] == -std::numeric_limits<double>::infinity()){
//         continue;
//       } else {
//         if (marginalsTrue[local_literal_variable_vector[cur_node_idx]] > derivatives[user_data[local_literal_index_vector[cur_node_idx]]]) {
//           marginalsTrue[local_literal_variable_vector[cur_node_idx]] = marginalsTrue[local_literal_variable_vector[cur_node_idx]] + std::log1p(std::exp(derivatives[user_data[local_literal_index_vector[cur_node_idx]]] - marginalsTrue[local_literal_variable_vector[cur_node_idx]]));
//         } else {
//           marginalsTrue[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]] + std::log1p(std::exp(marginalsTrue[local_literal_variable_vector[cur_node_idx]] - derivatives[user_data[local_literal_index_vector[cur_node_idx]]]));
//         }
//       }
//     } else {
//       if (marginalsFalse[local_literal_variable_vector[cur_node_idx]] == -std::numeric_limits<double>::infinity()){
//         marginalsFalse[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]];
//       } else if (derivatives[user_data[local_literal_index_vector[cur_node_idx]]] == -std::numeric_limits<double>::infinity()){
//         continue;
//       } else {
//         if (marginalsFalse[local_literal_variable_vector[cur_node_idx]] > derivatives[user_data[local_literal_index_vector[cur_node_idx]]]) {
//           marginalsFalse[local_literal_variable_vector[cur_node_idx]] = marginalsFalse[local_literal_variable_vector[cur_node_idx]] + std::log1p(std::exp(derivatives[user_data[local_literal_index_vector[cur_node_idx]]] - marginalsFalse[local_literal_variable_vector[cur_node_idx]]));
//         } else {
//           marginalsFalse[local_literal_variable_vector[cur_node_idx]] = derivatives[user_data[local_literal_index_vector[cur_node_idx]]] + std::log1p(std::exp(marginalsFalse[local_literal_variable_vector[cur_node_idx]] - derivatives[user_data[local_literal_index_vector[cur_node_idx]]]));
//         }
//       }
//     }
//  }
//     FinalLoop:for (int i = 0; i < NUM_VAR; i++){
//       float partition = marginalsFalse[i];
//       if (marginalsFalse[i] == -std::numeric_limits<double>::infinity()){
//         partition = marginalsTrue[i];
//       } else if (marginalsTrue[i] == -std::numeric_limits<double>::infinity()){
//         continue;
//       } else {
//         if (marginalsFalse[i] > marginalsTrue[i]) {
//           partition = marginalsFalse[i] + std::log1p(std::exp(marginalsTrue[i] - marginalsFalse[i]));
//         } else {
//           partition = marginalsTrue[i] + std::log1p(std::exp(marginalsFalse[i] - marginalsTrue[i]));
//         }
//       }
//       resultTrue[i] = marginalsTrue[i] - partition;
//       resultFalse[i] =  marginalsFalse[i] - partition;
//     }
//   }
// }
