#include <psdd/fpga_kernel_psdd_node.h>

#define BUFFER_SIZE 1024

extern "C" {
void fpga_evaluate(
        const ap_uint<32> *fpga_serialized_psdd_, // Read-Only Vector 1
        const PsddNodeStruct *fpga_node_vector, // Read-Only Vector 2
        unsigned int *result,       // Output Result
        int num_queries                   // Size in integer
        )
{

#pragma HLS INTERFACE m_axi port=fpga_serialized_psdd_  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=fpga_node_vector  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=fpga_serialized_psdd_  bundle=control
#pragma HLS INTERFACE s_axilite port=fpga_node_vector  bundle=control
#pragma HLS INTERFACE s_axilite port=out bundle=control
#pragma HLS INTERFACE s_axilite port=num_queries bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    unsigned int v1_buffer[BUFFER_SIZE];    // Local memory to store vector1
    unsigned int v2_buffer[BUFFER_SIZE];    // Local memory to store vector2
    unsigned int vout_buffer[BUFFER_SIZE];  // Local Memory to store result

    for (int j = 0 ; j < num_queries ; j++){
        result[j] = j+1;
    }
}
}
