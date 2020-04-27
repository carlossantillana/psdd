#include <psdd/fpga_kernel_psdd_node.h>
#include <assert.h>

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

   void load20Bit_staggered(const ap_uint<32>* data_dram, ap_uint<20>* data_local, int start,  int burstLength){
     #pragma HLS inline off
     int j = 0;
     load20BitStaggered: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i](19,0);
       j++;
     }
   }

   void loadFloats_staggered(const ap_fixed<32,8,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int start, int burstLength){
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
      ap_uint<12> local_flippers [50], const ap_uint<32> flippers [50], ap_int<13> local_literal_vector [TOTAL_LITERALS], const ap_int<32> literal_vector [TOTAL_LITERALS],
    ap_int<14> local_literal_variable_vector [TOTAL_LITERALS], const ap_int<32> literal_variable_vector [TOTAL_LITERALS], ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES], const ap_int<32> top_variable_vector [TOTAL_VARIABLE_INDEXES],
    ap_uint<6> local_children_size_vector [PSDD_SIZE], const ap_uint<32> children_size_vector [PSDD_SIZE],
   ap_uint<20>* local_literal_index_vector, const ap_uint<32>* literal_index_vector, ap_uint<20>* local_variable_index_vector, const ap_uint<32>* variable_index_vector) {
     loadBool(local_variables, MAX_VAR, 1);
     loadBool2(is_decision_vector, local_is_decision_vector, PSDD_SIZE);
     load6Bit(children_size_vector, local_children_size_vector, PSDD_SIZE);
     load12Bit(flippers, local_flippers, 50);
     load13Bit(literal_vector, local_literal_vector, TOTAL_LITERALS);
     load14Bit(literal_variable_vector, local_literal_variable_vector, TOTAL_LITERALS);
     load14Bit(top_variable_vector, local_top_variable_vector, TOTAL_VARIABLE_INDEXES);
     load20Bit(literal_index_vector, local_literal_index_vector, TOTAL_LITERALS);
     load20Bit(variable_index_vector, local_variable_index_vector, TOTAL_VARIABLE_INDEXES);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
     return;
   }

void fpga_evaluate(
        const ap_int<32> *is_decision_vector, // Read-Only Vector 2
        const ap_uint<32> *prime_vector,
        const ap_uint<32> *sub_vector,
        const ap_fixed<32,8,AP_RND> *parameter_vector,
        const ap_fixed<32,2,AP_RND> *bool_param_vector,
        const ap_uint<32> *flippers,
        const ap_int<32> *literal_vector,
        const ap_int<32> *literal_variable_vector,
        const ap_int<32> *top_variable_vector,
        const ap_uint<32> *children_size_vector,
        const ap_uint<32> *literal_index_vector,
        const ap_uint<32> *variable_index_vector,
        float *result,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port=is_decision_vector  offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=prime_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=sub_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = flippers offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = top_variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_size_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_index_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = variable_index_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=is_decision_vector  bundle=control
#pragma HLS INTERFACE s_axilite port=prime_vector bundle=control
#pragma HLS INTERFACE s_axilite port=sub_vector bundle=control
#pragma HLS INTERFACE s_axilite port = parameter_vector bundle = control
#pragma HLS INTERFACE s_axilite port = bool_param_vector bundle = control
#pragma HLS INTERFACE s_axilite port = flippers bundle = control
#pragma HLS INTERFACE s_axilite port = literal_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = top_variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_size_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_index_vector bundle = control
#pragma HLS INTERFACE s_axilite port = variable_index_vector bundle = control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=num_queries bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

  assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
  static bool local_variables [MAX_VAR];
  static bool local_instantiation [MAX_VAR];
  static ap_uint<12> local_flippers [50];
  static ap_uint<20> local_prime_vector[TOTAL_CHILDREN];
  static ap_uint<20> local_sub_vector[TOTAL_CHILDREN];
  static ap_fixed<21,8,AP_RND > local_parameter_vector[MAX_CHILDREN];
  static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
  static bool local_is_decision_vector[PSDD_SIZE];
  static ap_int<13> local_literal_vector [TOTAL_LITERALS];
  static ap_int<14> local_literal_variable_vector [TOTAL_LITERALS];
  static ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES];
  static ap_uint<6> local_children_size_vector [PSDD_SIZE];
  static ap_uint<20> local_literal_index_vector[TOTAL_LITERALS];
  static ap_uint<20> local_variable_index_vector[TOTAL_VARIABLE_INDEXES];

  #pragma HLS RESOURCE variable=local_instantiation core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_flippers core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_bool_param_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_is_decision_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_variable_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_top_variable_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_literal_index_vector core=XPM_MEMORY uram
  #pragma HLS RESOURCE variable=local_variable_index_vector core=XPM_MEMORY uram

  load(local_variables, local_is_decision_vector,
  is_decision_vector, local_bool_param_vector, bool_param_vector,local_flippers, flippers,
   local_literal_vector, literal_vector, local_literal_variable_vector, literal_variable_vector,  local_top_variable_vector, top_variable_vector,
  local_children_size_vector, children_size_vector, local_literal_index_vector,
  literal_index_vector, local_variable_index_vector, variable_index_vector);
  static float evaluation_cache [PSDD_SIZE];

  load20Bit_staggered(sub_vector, local_sub_vector, 0, TOTAL_CHILDREN);
  load20Bit_staggered(prime_vector, local_prime_vector, 0, TOTAL_CHILDREN);

  for (uint m = 0; m < num_queries; m++){
    local_instantiation[local_flippers[m%50]] = !local_instantiation[local_flippers[m%50]];

    LoopLiteral:for(uint cur_node_idx = 0; cur_node_idx < TOTAL_LITERALS; cur_node_idx++){
  #pragma HLS pipeline
       if (local_variables[local_literal_variable_vector[cur_node_idx]]) {
         if (local_instantiation[local_literal_variable_vector[cur_node_idx]] == (local_literal_vector[cur_node_idx] > 0) ) {
           evaluation_cache[local_literal_index_vector[cur_node_idx]] = 0;
         } else {
           evaluation_cache[local_literal_index_vector[cur_node_idx]] =
               -std::numeric_limits<float>::infinity();
         }
       } else {
         evaluation_cache[local_literal_index_vector[cur_node_idx]] = 0;
       }
   }

   LoopTop:for(uint cur_node_idx = 0; cur_node_idx < TOTAL_VARIABLE_INDEXES; cur_node_idx++){
  #pragma HLS pipeline
      if (local_variables[local_top_variable_vector[cur_node_idx]]) {
        if (local_instantiation[local_top_variable_vector[cur_node_idx]]) {
          evaluation_cache[local_variable_index_vector[cur_node_idx]] = local_bool_param_vector[cur_node_idx];
        } else {
          evaluation_cache[local_variable_index_vector[cur_node_idx]] = local_bool_param_vector[cur_node_idx +1];
        }
      } else {
        evaluation_cache[local_variable_index_vector[cur_node_idx]] = 0;
      }
    }
     local_instantiation[local_flippers[m%50]] = !local_instantiation[local_flippers[m%50]];
     int currentChild = 0;
    LoopDecision:for(uint cur_node_idx = 0; cur_node_idx < PSDD_SIZE; cur_node_idx++){
      if (local_is_decision_vector[cur_node_idx]){
      short element_size = local_children_size_vector[cur_node_idx];
      float max_prob = -std::numeric_limits<float>::infinity();
      //try getting the next 200 sets of children  at a time  from local_children children_size_vector
      loadFloats_staggered(parameter_vector, local_parameter_vector, currentChild, MAX_CHILDREN);
      assert(element_size <= MAX_CHILDREN);
        InnerLoop:for (uint i = 0; i < element_size; ++i) {
  #pragma HLS pipeline
          float tmp = evaluation_cache[local_prime_vector[currentChild]] + evaluation_cache[local_sub_vector[currentChild]] +  float (local_parameter_vector[i]);
          if ( max_prob < tmp) {
            max_prob = tmp;
          }
          currentChild++;
        }
         evaluation_cache[cur_node_idx] = max_prob;
      }
    }
    result[m] = evaluation_cache[PSDD_SIZE -1];
    }
  }
}
