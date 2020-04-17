#include <psdd/fpga_kernel_psdd_node.h>
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
  void load2Bit(const ap_uint<32>* data_dram, ap_uint<2>* data_local, int burstLength){
    #pragma HLS inline off
    load12Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](1,0);
    }
  }
  void load6Bit(const ap_uint<32>* data_dram, ap_uint<6>* data_local, int burstLength){
    #pragma HLS inline off
    load12Bit: for (int i = 0; i < burstLength; i++){
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

   void load21Bit(const ap_uint<32>* data_dram, ap_uint<21>* data_local, int burstLength){
     #pragma HLS inline off
     load20Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](20,0);
     }
   }
   void load20Bit(const ap_uint<32>* data_dram, ap_uint<20>* data_local, int burstLength){
     #pragma HLS inline off
     load21Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](19,0);
     }
   }

   void load20Bit_staggered(const ap_uint<32>* data_dram, ap_uint<20>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load21Bit: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i](19,0);
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

   void loadFloats_staggered(const ap_fixed<32,8,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int start, int burstLength){
     #pragma HLS inline off
     int j = 0;
     loadFloat:  for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i];
       j++;
     }
   }


   void load(bool local_variables[MAX_VAR],  ap_uint<2> local_node_type_vector[PSDD_SIZE],
      const ap_uint<32> node_type_vector[PSDD_SIZE], ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM], const ap_fixed<32,2,AP_RND > bool_param_vector[TOTAL_BOOL_PARAM],
      ap_uint<12> local_flippers [50], const ap_uint<32> flippers [50], ap_int<13> local_literal_vector [TOTAL_LITERALS], const ap_int<32> literal_vector [TOTAL_LITERALS],
    ap_int<14> local_variable_vector [TOTAL_VARIABLES], const ap_int<32> variable_vector [TOTAL_VARIABLES],  ap_uint<20> local_children_offset_vector [TOTAL_CHILDREN_SIZE],
    const ap_uint<32> children_offset_vector [TOTAL_CHILDREN_SIZE], ap_uint<6> local_children_size_vector [PSDD_SIZE], const ap_uint<32> children_size_vector [PSDD_SIZE]){
     loadBool(local_variables, MAX_VAR, 1);
     load2Bit(node_type_vector, local_node_type_vector, PSDD_SIZE);
     load6Bit(children_size_vector, local_children_size_vector, PSDD_SIZE);
     load12Bit(flippers, local_flippers, 50);
     load13Bit(literal_vector, local_literal_vector, TOTAL_LITERALS);
     load14Bit(variable_vector, local_variable_vector, TOTAL_VARIABLES);
     load20Bit(children_offset_vector, local_children_offset_vector, TOTAL_CHILDREN_SIZE);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
     return;
   }

void fpga_evaluate(
        const ap_uint<32> *node_type_vector, // Read-Only Vector 2
        const ap_uint<32> *prime_vector,
        const ap_uint<32> *sub_vector,
        const ap_fixed<32,8,AP_RND> *parameter_vector,
        const ap_fixed<32,2,AP_RND> *bool_param_vector,
        const ap_uint<32> *flippers,
        const ap_int<32> *literal_vector,
        const ap_int<32> *variable_vector,
        const ap_uint<32> *children_size_vector,
        const ap_uint<32> *children_offset_vector,
        float *result,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port=node_type_vector  offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=prime_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=sub_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = flippers offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_size_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_offset_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=node_type_vector  bundle=control
#pragma HLS INTERFACE s_axilite port=prime_vector bundle=control
#pragma HLS INTERFACE s_axilite port=sub_vector bundle=control
#pragma HLS INTERFACE s_axilite port = parameter_vector bundle = control
#pragma HLS INTERFACE s_axilite port = bool_param_vector bundle = control
#pragma HLS INTERFACE s_axilite port = flippers bundle = control
#pragma HLS INTERFACE s_axilite port = literal_vector bundle = control
#pragma HLS INTERFACE s_axilite port = variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_size_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_offset_vector bundle = control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=num_queries bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
static bool local_variables [MAX_VAR];
static bool local_instantiation [MAX_VAR];
static ap_uint<12> local_flippers [50];
static ap_uint<20> local_prime_vector[MAX_CHILDREN];
static ap_uint<20> local_sub_vector[MAX_CHILDREN];
static ap_fixed<21,8,AP_RND > local_parameter_vector[MAX_CHILDREN];
static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
static ap_uint<2> local_node_type_vector[PSDD_SIZE];
static ap_int<13> local_literal_vector [TOTAL_LITERALS];
static ap_int<14> local_variable_vector [TOTAL_VARIABLES];
static ap_uint<20> local_children_offset_vector [TOTAL_CHILDREN_SIZE];
static ap_uint<6> local_children_size_vector [PSDD_SIZE];

load(local_variables, local_node_type_vector,
node_type_vector, local_bool_param_vector, bool_param_vector,local_flippers, flippers,
 local_literal_vector, literal_vector, local_variable_vector, variable_vector,
local_children_offset_vector, children_offset_vector, local_children_size_vector, children_size_vector);
static float evaluation_cache [PSDD_SIZE];

#pragma HLS RESOURCE variable=local_bool_param_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_literal_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_variable_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_flippers core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_instantiation core=XPM_MEMORY uram

for (uint m = 0; m < num_queries; m++){
  short currentLiteral = 0;
  short current_bool_param = 0;
  short currentVariable = 0;
  loadBool(local_instantiation, MAX_VAR, 0);
  local_instantiation[local_flippers[m%50]] = !local_instantiation[local_flippers[m%50]];
#pragma HLS RESOURCE variable=local_evaluation_cache core=XPM_MEMORY uram
  Loop1:for(uint cur_node_idx = 0; cur_node_idx < PSDD_SIZE; cur_node_idx++){
#pragma HLS pipeline
    if (local_node_type_vector[cur_node_idx] == LITERAL_NODE_TYPE) {
     if (local_variables[local_variable_vector[currentVariable]]) {
       if (local_instantiation[local_variable_vector[currentVariable]] == (local_literal_vector[currentLiteral++] > 0) ) {
         evaluation_cache[cur_node_idx] = 0;
       } else {
         evaluation_cache[cur_node_idx] =
             -std::numeric_limits<float>::infinity();
       }
     } else {
       evaluation_cache[cur_node_idx] = 0;
     }
     currentVariable++;
   } else if (local_node_type_vector[cur_node_idx] == TOP_NODE_TYPE) {
     if (local_variables[local_variable_vector[currentVariable]]) {
       if (local_instantiation[local_variable_vector[currentVariable]]) {
         evaluation_cache[cur_node_idx] = local_bool_param_vector[current_bool_param];
         current_bool_param+= 2;
       } else {
         evaluation_cache[cur_node_idx] = local_bool_param_vector[current_bool_param +1];
         current_bool_param+= 2;
       }
     } else {
       evaluation_cache[cur_node_idx] = 0;
     }
     currentVariable++;
   }
 }

  Loop2:for(uint cur_node_idx = 0; cur_node_idx < PSDD_SIZE; cur_node_idx++){
    if (local_node_type_vector[cur_node_idx] == DECISION_NODE_TYPE){
    short element_size = local_children_size_vector[cur_node_idx];
    float max_prob = -std::numeric_limits<float>::infinity();
    load20Bit_staggered(sub_vector, local_sub_vector, local_children_offset_vector[cur_node_idx], element_size);
    load20Bit_staggered(prime_vector, local_prime_vector, local_children_offset_vector[cur_node_idx], element_size);
    loadFloats_staggered(parameter_vector, local_parameter_vector, local_children_offset_vector[cur_node_idx], element_size);

    assert(element_size <= MAX_CHILDREN);
      InnerLoop:for (uint i = 0; i < element_size; ++i) {
#pragma HLS pipeline
        float tmp = evaluation_cache[local_prime_vector[i]] + evaluation_cache[local_sub_vector[i]] +  float (local_parameter_vector[i]);
        if ( max_prob < tmp) {
          max_prob = tmp;
        }
      }
       evaluation_cache[cur_node_idx] = max_prob;
    }
  }
  //For more than one query, less accurate for debugging
  result[m] = evaluation_cache[580816];
}
//Loads every intermediate value for a single query.
// LoadResult:for(uint i = 0; i < PSDD_SIZE; i++){
//   #pragma HLS pipeline
//     result[i] = evaluation_cache[i];
//   }
}
}
