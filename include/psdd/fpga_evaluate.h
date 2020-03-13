#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_psdd_node.h>

void EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      ap_uint<20> fpga_serialized_psdd_evaluate [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      ap_uint<21> children_vector[TOTAL_CHILDREN],
                      ap_fixed<18,7,AP_RND > parameter_vector [TOTAL_PARAM],
                      ap_fixed<12,1,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM]);
#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
