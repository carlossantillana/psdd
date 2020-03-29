#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_kernel_psdd_node.h>
#include <bitset>

extern "C" {
void fpga_evaluate(const std::bitset<MAX_VAR> &variables,
                      std::bitset<MAX_VAR> & instantiation,
                      ap_uint<21> fpga_serialized_psdd_evaluate [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      ap_uint<22> children_vector[TOTAL_CHILDREN],
                      ap_fixed<21,8,AP_RND > parameter_vector [TOTAL_PARAM],
                      ap_fixed<14,2,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM],
  					          ap_uint<12> flippers [55], float results[3]);
}
#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
