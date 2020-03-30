#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_kernel_psdd_node.h>
#include <bitset>

extern "C" {
void fpga_evaluate(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> & instantiation,
                      const ap_uint<21> *fpga_serialized_psdd_evaluate,
                      const FPGAPsddNodeStruct *fpga_node_vector,
                      const ap_uint<22> *children_vector,
                      const ap_fixed<21,8,AP_RND > *parameter_vector,
                      const ap_fixed<14,2,AP_RND > *bool_param_vector,
  					          const ap_uint<12> *flippers, float *results);
}
#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
