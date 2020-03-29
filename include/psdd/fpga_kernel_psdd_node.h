//
// Created by Yujia Shen on 10/19/17.
//

#ifndef FPGA_KERNEL_STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
#define FPGA_KERNEL_STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
#include <cstddef>
#include <utility>
#include <gmpxx.h>
#include "ap_int.h"
#include <ap_fixed.h>

#define LITERAL_NODE_TYPE 1
#define DECISION_NODE_TYPE 2
#define TOP_NODE_TYPE 3

//For map_network network


const int PsddBurstLength = 580817;
const int ChildrenBurstLength = 1541021;
const int ParamBurstLength = 770511;
#define  MAX_VAR 65536


struct FPGAPsddNodeStruct {
  ap_uint<21> node_index_;
  ap_uint<2> node_type_;
  char children_size;
  ap_uint<21> children_offset;
  ap_uint<20> parameter_offset;
  short variable_index_;
  ap_uint<10> bool_param_offset;
  int32_t literal_;
};

#endif // STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
