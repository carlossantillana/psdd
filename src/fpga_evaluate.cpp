#include <psdd/fpga_kernel_psdd_node.h>
#include <assert.h>
#include <iostream>

extern "C" {

  void loadChar(char* data_local, int burstLength, char value){
    #pragma HLS inline off
    loadChar: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = value;
    }
  }
  void load12Bit(const ap_uint<32>* data_dram, ap_uint<12>* data_local, int burstLength){
    #pragma HLS inline off
    load12Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](11,0);
    }
  }

   void load21Bit(const ap_uint<32>* data_dram, ap_uint<21>* data_local, int burstLength){
     #pragma HLS inline off
     load20Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](20,0);
       // std::cout << "(local, origin): " << data_local[i].to_string() << ", " << data_dram[i] << std::endl;
     }
   }
   void load22Bit(const ap_uint<32>* data_dram, ap_uint<22>* data_local, int burstLength){
     #pragma HLS inline off
     load21Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](21,0);
     }
   }

   void loadFloatsSmall(const ap_fixed<32,2,AP_RND >* data_dram, ap_fixed<14,2,AP_RND >* data_local, int burstLength){
     #pragma HLS inline off
     loadFloatSmall: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void loadFloats(const ap_fixed<32,8,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int burstLength){
     #pragma HLS inline off
     loadFloat: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void loadStructs(const ap_uint<256>* data_dram, FPGAPsddNodeStruct* data_local, int burstLength){
     #pragma HLS inline off
     loadStruct: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i].node_index_ = data_dram[i](31, 0);
       data_local[i].node_type_ = data_dram[i](33,32);
       data_local[i].children_size = data_dram[i](95, 64);
       data_local[i].children_offset = data_dram[i](127, 96);
       data_local[i].parameter_offset = data_dram[i](159, 128);
       data_local[i].variable_index_ = data_dram[i](191, 160);
       data_local[i].bool_param_offset = data_dram[i](223, 192);
       data_local[i].literal_ = data_dram[i](255, 224);
     }
   }

   void load(char local_variables[MAX_VAR], char local_instantiation[MAX_VAR], ap_uint<21>  local_serialized_nodes [PSDD_SIZE], const ap_uint<32>  serialized_nodes [PSDD_SIZE], FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE],
     const ap_uint<256> fpga_node_vector[PSDD_SIZE], ap_uint<22> local_children_vector[TOTAL_CHILDREN], const ap_uint<32> children_vector[TOTAL_CHILDREN],
      ap_fixed<21,8,AP_RND > local_parameter_vector[TOTAL_PARAM], const ap_fixed<32,8,AP_RND > parameter_vector[TOTAL_PARAM],
      ap_fixed<14,2,AP_RND > local_bool_param_vector [TOTAL_BOOL_PARAM], const ap_fixed<32,2,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM],
       ap_uint<12> local_flippers [55], const ap_uint<32> flippers [55]){
     loadChar(local_variables, MAX_VAR, 1);
     loadChar(local_instantiation, MAX_VAR, 0);
     load12Bit(flippers, local_flippers, 55);
     load21Bit(serialized_nodes, local_serialized_nodes, PSDD_SIZE);
     load22Bit(children_vector, local_children_vector, TOTAL_CHILDREN);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
     loadFloats(parameter_vector, local_parameter_vector, TOTAL_PARAM);
     loadStructs(fpga_node_vector, local_fpga_node_vector, PSDD_SIZE);
     return;
   }

void fpga_evaluate(
        const ap_uint<32>* serialized_nodes, // Read-Only Vector 1
        const ap_uint<256> *fpga_node_vector, // Read-Only Vector 2
        const ap_uint<32> *children_vector,
        const ap_fixed<32,8,AP_RND> *parameter_vector,
        const ap_fixed<32,2,AP_RND> *bool_param_vector,
        const ap_uint<32> *flippers,
        float *result,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port=serialized_nodes  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=fpga_node_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=children_vector  offset=slave bundle=gmem
#pragma HLS interface m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = flippers offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=serialized_nodes  bundle=control
#pragma HLS INTERFACE s_axilite port=fpga_node_vector  bundle=control
#pragma HLS INTERFACE s_axilite port=children_vector  bundle=control
#pragma HLS interface s_axilite port = parameter_vector bundle = control
#pragma HLS interface s_axilite port = bool_param_vector bundle = control
#pragma HLS interface s_axilite port = flippers bundle = control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=num_queries bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

assert(num_queries <= 4096);  // this helps HLS estimate the loop trip count
static char local_variables [MAX_VAR];
static char local_instantiation [MAX_VAR];
static ap_uint<12> local_flippers [55];
static ap_uint<21> local_serialized_nodes [PSDD_SIZE];
static FPGAPsddNodeStruct local_fpga_node_vector[PSDD_SIZE];
static ap_uint<22> local_children_vector[TOTAL_CHILDREN];
static ap_fixed<21,8,AP_RND > local_parameter_vector[TOTAL_PARAM];
static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
load(local_variables, local_instantiation, local_serialized_nodes, serialized_nodes, local_fpga_node_vector,
fpga_node_vector, local_children_vector, children_vector, local_parameter_vector,
parameter_vector, local_bool_param_vector, bool_param_vector, local_flippers, flippers);

#pragma HLS RESOURCE variable=local_serialized_nodes core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_children_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_bool_param_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_parameter_vector core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=local_variables core=XPM_MEMORY uram

for (int m = 0; m < num_queries; m++){
  static float evaluation_cache [PSDD_SIZE];
  // std::bitset<MAX_VAR> local_instantiation;
  if (m >0)
  local_instantiation[local_flippers[m-1%55]] = !local_instantiation[local_flippers[m-1%55]];

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
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = local_bool_param_vector[local_fpga_node_vector[cur_node_idx].bool_param_offset];
       } else {
         evaluation_cache[local_fpga_node_vector[cur_node_idx].node_index_] = local_bool_param_vector[local_fpga_node_vector[cur_node_idx].bool_param_offset +1];
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
  result[m] = evaluation_cache[local_fpga_node_vector[serialized_nodes[0]].node_index_];

}

}
}
