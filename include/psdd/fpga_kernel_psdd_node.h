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
const uint PSDD_SIZE = 580817;
const uint MAX_CHILDREN = 57;
const uint TOTAL_CHILDREN = 770511;
const short TOTAL_BOOL_PARAM = 792;
const short TOTAL_LITERALS = 6714;
const short TOTAL_VARIABLES = 7110;
const short TOTAL_VARIABLE_INDEXES = 396;
const uint TOTAL_CHILDREN_SIZE = 580817;
const uint MERGED_LOOP_LEN = 777724;
const uint NUM_DISTICT_QUERIES = 50;
// #define MAX_VAR 65536
// try this soon
#define MAX_VAR 4096

//For grids network
// const uint PSDD_SIZE = 51;
// const uint MAX_CHILDREN = 2;
// const uint TOTAL_CHILDREN = 156;
// const uint TOTAL_PARAM = 78;
// const short TOTAL_BOOL_PARAM = 48;
// #define MAX_VAR 2048

//For Mastermind Network
// const uint PSDD_SIZE = 42558;
// const uint MAX_CHILDREN = 2;
// const uint TOTAL_CHILDREN = 45272;
// const short TOTAL_BOOL_PARAM = 1;
// const short TOTAL_LITERALS = 2328;
// const short TOTAL_VARIABLES = 1;
// const short TOTAL_VARIABLE_INDEXES = 1;
// const uint TOTAL_CHILDREN_SIZE = 42558;
// const uint MERGED_LOOP_LEN = 47688;
// const uint NUM_DISTICT_QUERIES = 1;
// #define MAX_VAR 2048

struct PsddNodeStruct {
  ap_uint<32> node_type_;
  ap_uint<32> children_size;
  ap_uint<32> children_offset;
  ap_uint<32> packed;

};

#endif // STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
