#include <assert.h>
#include <stdio.h>
#include "ap_int.h"
#include <ap_fixed.h>
#include <psdd/fpga_kernel_psdd_node.h>
#include <iostream>

extern "C" {
void fpga_evaluate(const ap_uint<21>* fpga_serialized_psdd_, const FPGAPsddNodeStruct* fpga_node_vector,
  const ap_uint<22>* children_vector, const ap_fixed<21,8,AP_RND>* parameter_vector, const ap_fixed<14,2,AP_RND>* bool_param_vector, const ap_uint<12>* flippers, float* result, const int num_elems) {
#pragma HLS interface m_axi port = fpga_serialized_psdd_ offset = slave bundle = gmem
#pragma HLS interface m_axi port = fpga_node_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = children_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = parameter_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS interface m_axi port = flippers offset = slave bundle = gmem
#pragma HLS interface m_axi port = result offset = slave bundle = gmem
#pragma HLS interface s_axilite port = fpga_serialized_psdd_ bundle = control
#pragma HLS interface s_axilite port = fpga_node_vector bundle = control
#pragma HLS interface s_axilite port = children_vector bundle = control
#pragma HLS interface s_axilite port = parameter_vector bundle = control
#pragma HLS interface s_axilite port = bool_param_vector bundle = control
#pragma HLS interface s_axilite port = flippers bundle = control
#pragma HLS interface s_axilite port = result bundle = control
#pragma HLS interface s_axilite port = num_elems bundle = control
#pragma HLS interface s_axilite port = return bundle = control
  assert(num_elems <= 4096);  // this helps HLS estimate the loop trip count
  /***************************
   * your code goes here ... *
   ***************************/
   printf("hello world\n\n");
   std::cout << "fpga serialized psdd[0]: " << fpga_serialized_psdd_[0] << std::endl;
   std::cout << "FPGAPsddNodeStruct.node_index_[0]: " << fpga_node_vector[0].node_index_  << " node_type: " << fpga_node_vector[0].node_type_ << std::endl;
   std::cout << "children_vector[0]: " << children_vector[0] << std::endl;
   std::cout << "parameter_vector[0]: " << float(parameter_vector[0]) << std::endl;
   std::cout << "bool_param_vector[0]: " << float(bool_param_vector[0])  << std::endl;
   std::cout << "flippers[0]: " << flippers[0] << std::endl;
   printf("end world\n\n");

}

}  // extern "C"
