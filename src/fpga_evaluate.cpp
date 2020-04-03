#include <psdd/fpga_kernel_psdd_node.h>

#define BUFFER_SIZE 1024

extern "C" {
void fpga_evaluate(
        const ap_uint<32>* serialized_nodes, // Read-Only Vector 1
        const ap_uint<256> *fpga_node_vector, // Read-Only Vector 2
        float *result,       // Output Result
        int size                   // Size in integer
        )
{

#pragma HLS INTERFACE m_axi port=serialized_nodes  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=fpga_node_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=serialized_nodes  bundle=control
#pragma HLS INTERFACE s_axilite port=fpga_node_vector  bundle=control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (int j = 0 ; j < size ; j++){
        result[j] =fpga_node_vector[j](32, 33);
    }
}
}
