#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_psdd_node.h>

double EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> &instantiation,
                      ap_uint<20> fpga_serialized_psdd_evaluate [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      ap_uint<21> children_vector[TOTAL_CHILDREN],
                      ap_fixed<23,7,AP_RND > parameter_vector [TOTAL_PARAM]);
#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
