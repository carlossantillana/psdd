#include "../include/psdd/fpga_kernel_psdd_node.h"
#include <assert.h>
#include <stdio.h>
#include <hls_stream.h>
#include <bitset>
#include <iostream>

  void loadBool(bool* data_local, int burstLength, char value){
    #pragma HLS inline
    loadBool: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = value;
    }
  }
  void loadBitset(const std::bitset<MAX_VAR>* data_dram, bool data_local [NUM_DISTICT_QUERIES][MAX_VAR]){
    #pragma HLS inline
    loadBitset: for (int i = 0; i < NUM_DISTICT_QUERIES; i++){
      for(int j = 0; j < MAX_VAR; ++j){
        #pragma HLS pipeline
        data_local[i][j] = data_dram[i][j];
      }
    }
  }

  void load2Bit(const ap_uint<32>* data_dram, ap_uint<2>* data_local, int burstLength){
    #pragma HLS inline
    load12Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](1,0);
    }
  }
  void load6Bit(const ap_uint<32>* data_dram, ap_uint<6>* data_local, int burstLength){
    #pragma HLS inline
    load12Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](5,0);
    }
  }
  void load13Bit(const ap_int<32>* data_dram, ap_int<13>* data_local, int burstLength){
    #pragma HLS inline
    load13Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](12,0);
    }
  }

  void load14Bit(const ap_int<32>* data_dram, ap_int<14>* data_local, int burstLength){
    #pragma HLS inline
    load14Bit: for (int i = 0; i < burstLength; i++){
    #pragma HLS pipeline
      data_local[i] = data_dram[i](13,0);
    }
  }

   void load21Bit(const ap_uint<32>* data_dram, ap_uint<21>* data_local, int burstLength){
     #pragma HLS inline
     load20Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](20,0);
     }
   }
   void load20Bit(const ap_uint<32>* data_dram, ap_uint<20>* data_local, int burstLength){
     #pragma HLS inline
     load21Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i](19,0);
     }
   }

   void load20Bit_staggered(const ap_uint<32>* data_dram, ap_uint<20>* data_local, int start,  int burstLength){
     #pragma HLS inline
     int j = 0;
     load21Bit: for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i](19,0);
       j++;
     }
   }

   void loadFloatsSmall(const ap_fixed<32,2,AP_RND >* data_dram, ap_fixed<14,2,AP_RND >* data_local, int burstLength){
     #pragma HLS inline
     loadFloatSmall: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void loadfixed21Bit(const ap_fixed<32,8,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int burstLength){
     #pragma HLS inline
     load21Bit: for (int i = 0; i < burstLength; i++){
     #pragma HLS pipeline
       data_local[i] = data_dram[i];
     }
   }

   void loadFloats_staggered(const ap_fixed<32,8,AP_RND >* data_dram, ap_fixed<21,8,AP_RND >* data_local, int start, int burstLength){
     #pragma HLS inline
     int j = 0;
     loadFloat:  for (int i = start; i < start +burstLength; i++){
     #pragma HLS pipeline
       data_local[j] = data_dram[i];
       j++;
     }
   }

   void load(bool local_variables[MAX_VAR],
  	ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM], const ap_fixed<32,2,AP_RND > bool_param_vector[TOTAL_BOOL_PARAM], bool local_instantiations [NUM_DISTICT_QUERIES][MAX_VAR], const std::bitset<MAX_VAR> instantiations [NUM_DISTICT_QUERIES],
    ap_int<13> local_literal_vector [TOTAL_LITERALS], const ap_int<32> literal_vector [TOTAL_LITERALS],
    ap_int<14> local_literal_variable_vector [TOTAL_LITERALS], const ap_int<32> literal_variable_vector [TOTAL_LITERALS], ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES], const ap_int<32> top_variable_vector [TOTAL_VARIABLE_INDEXES],
    ap_uint<20>* local_literal_index_vector, const ap_uint<32>* literal_index_vector, ap_uint<20>* local_variable_index_vector, const ap_uint<32>* variable_index_vector) {
#pragma HLS inline
     loadBool(local_variables, MAX_VAR, 1);
     loadBitset(instantiations, local_instantiations);
     load13Bit(literal_vector, local_literal_vector, TOTAL_LITERALS);
     load14Bit(literal_variable_vector, local_literal_variable_vector, TOTAL_LITERALS);
     load14Bit(top_variable_vector, local_top_variable_vector, TOTAL_VARIABLE_INDEXES);
     load20Bit(literal_index_vector, local_literal_index_vector, TOTAL_LITERALS);
     load20Bit(variable_index_vector, local_variable_index_vector, TOTAL_VARIABLE_INDEXES);
     loadFloatsSmall(bool_param_vector, local_bool_param_vector, TOTAL_BOOL_PARAM);
   }

void comp(
	hls::stream< ap_uint<64> > & dram_fifo,
        const ap_fixed<32,2,AP_RND> *bool_param_vector,
        const std::bitset<MAX_VAR> *instantiations,
        const ap_int<32> *literal_vector,
        const ap_int<32> *literal_variable_vector,
        const ap_int<32> *top_variable_vector,
        const ap_uint<32> *children_size_vector,
        const ap_uint<32> *children_offset_vector,
        const ap_uint<32> *literal_index_vector,
        const ap_uint<32> *variable_index_vector,
        float *result,       // Output Result
        int num_queries)
{

	assert(num_queries <= 2048);  // this helps HLS estimate the loop trip count
	static bool local_variables [MAX_VAR];
  static bool local_instantiations [NUM_DISTICT_QUERIES][MAX_VAR];
	static ap_fixed<14,2,AP_RND > local_bool_param_vector[TOTAL_BOOL_PARAM];
	static ap_int<13> local_literal_vector [TOTAL_LITERALS];
	static ap_int<14> local_literal_variable_vector [TOTAL_LITERALS];
	static ap_int<14> local_top_variable_vector [TOTAL_VARIABLE_INDEXES];
	static ap_uint<20> local_literal_index_vector[TOTAL_LITERALS];
	static ap_uint<20> local_variable_index_vector[TOTAL_VARIABLE_INDEXES];
	static float evaluation_cache [PSDD_SIZE];
	static float evaluation_cache2 [PSDD_SIZE];

	load(
			local_variables,
			local_bool_param_vector, bool_param_vector,
      local_instantiations, instantiations,
			local_literal_vector, literal_vector,
			local_literal_variable_vector, literal_variable_vector,  local_top_variable_vector, top_variable_vector,
			local_literal_index_vector, literal_index_vector,
			local_variable_index_vector, variable_index_vector
	    );

	numQueries:for (uint m = 0; m < num_queries; m++){
		literals:for(uint lit_idx = 0; lit_idx < TOTAL_LITERALS; lit_idx++){
#pragma HLS pipeline
			if (local_variables[local_literal_variable_vector[lit_idx]]) {
				if (local_instantiations[m%NUM_DISTICT_QUERIES][local_literal_variable_vector[lit_idx]] == (local_literal_vector[lit_idx] > 0) ) {
					evaluation_cache[local_literal_index_vector[lit_idx]] = 0;
					evaluation_cache2[local_literal_index_vector[lit_idx]] = 0;
				} else {
					evaluation_cache[local_literal_index_vector[lit_idx]] = -std::numeric_limits<float>::infinity();
					evaluation_cache2[local_literal_index_vector[lit_idx]] = -std::numeric_limits<float>::infinity();
				}
			} else {
				evaluation_cache[local_literal_index_vector[lit_idx]] = 0;
				evaluation_cache2[local_literal_index_vector[lit_idx]] = 0;
			}
		}

		top:for(uint var_idx = 0; var_idx < TOTAL_VARIABLE_INDEXES; var_idx++){
      //Not sure why this pragma breaks mastermind bench
// #pragma HLS pipeline
			if (local_variables[local_top_variable_vector[var_idx]]) {
				if (local_instantiations[m%NUM_DISTICT_QUERIES][local_top_variable_vector[var_idx]]) {
					evaluation_cache[local_variable_index_vector[var_idx]] = local_bool_param_vector[var_idx];
					evaluation_cache2[local_variable_index_vector[var_idx]] = local_bool_param_vector[var_idx];
				} else {
					evaluation_cache[local_variable_index_vector[var_idx]] = local_bool_param_vector[var_idx +1];
					evaluation_cache2[local_variable_index_vector[var_idx]] = local_bool_param_vector[var_idx +1];
				}
			} else {
				evaluation_cache[local_variable_index_vector[var_idx]] = 0;
				evaluation_cache2[local_variable_index_vector[var_idx]] = 0;
			}
		}
		uint cur_node_idx = 0;
		float max_prob = -std::numeric_limits<float>::infinity();
		decision:for (uint n = 0; n < MERGED_LOOP_LEN; n++) {
#pragma HLS pipeline
#pragma HLS dependence variable=evaluation_cache inter false
#pragma HLS dependence variable=evaluation_cache2 inter false
			ap_uint<64> dram_fifo_data = dram_fifo.read();
			bool do_comp = dram_fifo_data(62,62); //1, 3
			bool inc_idx = dram_fifo_data(61,61); //0, 2
			ap_uint<21> parameter_temp = dram_fifo_data(60,40);
			ap_fixed<21,8,AP_RND > parameter = *((ap_fixed<21,8,AP_RND >*)&parameter_temp);
			ap_uint<20> sub_idx = dram_fifo_data(39,20);
			ap_uint<20> prime_idx = dram_fifo_data(19,0);

			float tmp = evaluation_cache[prime_idx] + evaluation_cache2[sub_idx] +  float (parameter);

			if ( do_comp == true && max_prob < tmp) {
				max_prob = tmp;
			}
			if ( do_comp == true ) {
				evaluation_cache[cur_node_idx] = max_prob;
				evaluation_cache2[cur_node_idx] = max_prob;
			}

			//printf("node(%d) edge(%d) do_comp:%d inc_idx:%d prime id:%d sub id:%d prime prob:%f sub prob:%f local param:%f prob:%f max_prob:%f\n", cur_node_idx, n, int(do_comp), int(inc_idx), int(prime_idx), int(sub_idx), evaluation_cache[prime_idx],  evaluation_cache[sub_idx], float (parameter), tmp, max_prob );
			if( inc_idx == true ){
				//printf("%d:%f\n", cur_node_idx, max_prob);
				max_prob = -std::numeric_limits<float>::infinity();
				cur_node_idx++;
			}
		}
		result[m] = evaluation_cache[PSDD_SIZE-1];
	}

}

void dram_read(
	const ap_uint<64> *dram_port,
	hls::stream< ap_uint<64> > & dram_fifo,
        int num_queries
)
{
	for (uint m = 0; m < num_queries; m++){
		for (uint n = 0; n < MERGED_LOOP_LEN; n++) {
#pragma HLS pipeline
			ap_uint<64> dram_port_temp = dram_port[n];
			dram_fifo.write(dram_port_temp);
		}
	}
}

extern "C" {

void fpga_evaluate(
	const ap_uint<64> *dram_port,
        const std::bitset<MAX_VAR> *instantiations,
        const ap_fixed<32,2,AP_RND> *bool_param_vector,
        const ap_int<32> *literal_vector,
        const ap_int<32> *literal_variable_vector,
        const ap_int<32> *top_variable_vector,
        const ap_uint<32> *children_size_vector,
        const ap_uint<32> *children_offset_vector,
        const ap_uint<32> *literal_index_vector,
        const ap_uint<32> *variable_index_vector,
        float *result,       // Output Result
        int num_queries)
{
#pragma HLS INTERFACE m_axi port = dram_port  offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port = instantiations  offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port = bool_param_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = top_variable_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_size_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = children_offset_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = literal_index_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = variable_index_vector offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = result offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port= dram_port  bundle=control
#pragma HLS INTERFACE s_axilite port= instantiations  bundle=control
#pragma HLS INTERFACE s_axilite port = bool_param_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = top_variable_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_size_vector bundle = control
#pragma HLS INTERFACE s_axilite port = children_offset_vector bundle = control
#pragma HLS INTERFACE s_axilite port = literal_index_vector bundle = control
#pragma HLS INTERFACE s_axilite port = variable_index_vector bundle = control
#pragma HLS INTERFACE s_axilite port = result bundle=control
#pragma HLS INTERFACE s_axilite port = num_queries bundle=control
#pragma HLS INTERFACE s_axilite port = return bundle=control

#pragma HLS dataflow

	hls::stream< ap_uint<64> > dram_fifo;

	#pragma HLS STREAM variable=dram_fifo  depth=512 dim=1
	//#pragma HLS RESOURCE variable=dram_fifo core=FIFO_BRAM

	dram_read( dram_port, dram_fifo, num_queries );
	comp( dram_fifo, bool_param_vector, instantiations, literal_vector, literal_variable_vector, top_variable_vector, children_size_vector, children_offset_vector, literal_index_vector, variable_index_vector, result, num_queries);
}


}
