#include <psdd/fpga_kernel_psdd_node.h>

extern "C" {
  //This works
  // result[j] =fpga_node_vector[j](32, 33);
void fpga_evaluate(
        const ap_uint<32>* serialized_nodes, // Read-Only Vector 1
        const ap_uint<256> *fpga_node_vector, // Read-Only Vector 2
        const ap_uint<32> *children_vector,
        const ap_fixed<32,10,AP_RND> *parameter_vector,
        const ap_fixed<32,4,AP_RND> *bool_param_vector,
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
#pragma HLS interface m_axi port = results offset = slave bundle = gmem
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

    for (int j = 0 ; j < 32 ; j++){
      if (j %2 == 0)
        result[j] =bool_param_vector[j];
      else
      result[j] =parameter_vector[j];

    }
}
}
