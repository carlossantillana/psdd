#include <psdd/fpga_kernel_psdd_node.h>

#define BUFFER_SIZE 1024

extern "C" {
void fpga_evaluate(
        const ap_uint<32>* serialized_nodes, // Read-Only Vector 1
        const unsigned int *in2, // Read-Only Vector 2
        unsigned int *result,       // Output Result
        int size                   // Size in integer
        )
{

#pragma HLS INTERFACE m_axi port=serialized_nodes  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=in2  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=serialized_nodes  bundle=control
#pragma HLS INTERFACE s_axilite port=in2  bundle=control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (int j = 0 ; j < size ; j++){
        result[j] = in2[j];
    }
}
}
