#define BUFFER_SIZE 1024

extern "C" {
void fpga_evaluate(
        const unsigned int *in1, // Read-Only Vector 1
        const unsigned int *in2, // Read-Only Vector 2
        unsigned int *result,       // Output Result
        int size                   // Size in integer
        )
{

#pragma HLS INTERFACE m_axi port=in1  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=in2  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=in1  bundle=control
#pragma HLS INTERFACE s_axilite port=in2  bundle=control
#pragma HLS INTERFACE s_axilite port=result bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    unsigned int v1_buffer[BUFFER_SIZE];    // Local memory to store vector1
    unsigned int v2_buffer[BUFFER_SIZE];    // Local memory to store vector2
    unsigned int vout_buffer[BUFFER_SIZE];  // Local Memory to store result

    for (int j = 0 ; j < size ; j++){
        result[j] = j+1;
    }
}
}
