#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_kernel_psdd_node.h>
#include <bitset>


void fpga_evaluate(bool variables[MAX_VAR],
                      std::bitset<MAX_VAR> & instantiation,
                      ap_uint<21> fpga_serialized_psdd_evaluate [580817],
                      FPGAPsddNodeStruct fpga_node_vector[580817],
                      ap_uint<22> children_vector[1541021],
                      ap_fixed<21,8,AP_RND > parameter_vector [770511],
                      ap_fixed<14,2,AP_RND > bool_param_vector [792],
  					          float results[3], ap_uint<12> flippers [55]);

#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
