#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
#include <psdd/fpga_psdd_node.h>
//For Small
//const int PsddBurstLength = 51;
//const int ChildrenBurstLength = 156;
//const int ParamBurstLength = 78;
//For Large
 const int PsddBurstLength = 580817;
 const int ChildrenBurstLength = 1541021;
 const int ParamBurstLength = 770511;
//FPGA
void EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                      std::bitset<MAX_VAR> & instantiation,
                      ap_uint<20> fpga_serialized_psdd_evaluate [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      ap_uint<21> children_vector[TOTAL_CHILDREN],
                      ap_fixed<19,7,AP_RND > parameter_vector [TOTAL_PARAM],
                      ap_fixed<12,1,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM],
  					          float results[NUM_QUERIES], int flippers [242]);
//CSIM

#endif // FPGA_STRUCTURED_BAYESIAN_NETWORK_EVALUATE_H
