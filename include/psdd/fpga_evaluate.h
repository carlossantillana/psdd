#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_psdd_node.h>

double EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> &instantiation,
                      std::array<uint32_t, PSDD_SIZE>  fpga_serialized_psdd_evaluate,
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      uint32_t children_vector[TOTAL_CHILDREN],
                      double parameter_vector [TOTAL_PARAM]);
#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
