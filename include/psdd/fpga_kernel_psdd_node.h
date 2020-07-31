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
// const uint PSDD_SIZE = 580817;
// const uint MAX_CHILDREN = 57;
// const uint TOTAL_CHILDREN = 770511;
// const short TOTAL_BOOL_PARAM = 792;
// const short TOTAL_LITERALS = 6714;
// const short TOTAL_VARIABLES = 7110;
// const short TOTAL_VARIABLE_INDEXES = 396;
// const uint TOTAL_CHILDREN_SIZE = 580817;
// const uint MERGED_LOOP_LEN = 777724;
// const uint NUM_DISTICT_QUERIES = 50;
// const uint NUM_VAR = 3357;
// #define MAX_VAR 4096
// const uint PRIME_BIT_WIDTH = 24;
// const uint SUB_BIT_WIDTH = 24;
// const uint PARAM_BIT_WIDTH = 16;
// const uint PARAM_DEC_WIDTH = 8;
// const uint BOOL_BIT_WIDTH = 14;
// const uint BOOL_DEC_WIDTH = 2;
// const uint NODE_TYPE_BIT_WIDTH = 3;
// const uint LITERAL_BIT_WIDTH = 13;
// const uint LITERAL_VAR_BIT_WIDTH = 14;
// const uint TOP_VAR_BIT_WIDTH = 14;
// const uint CHILD_SIZE_BIT_WIDTH = 6;
// const uint CHILD_OFFSET_BIT_WIDTH = 20;
// const uint LITERAL_IDX_BIT_WIDTH = 20;
// const uint VAR_IDX_BIT_WIDTH = 20;

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
// const uint TOTAL_DECISION = 40230;
// const uint NUM_VAR = 1220;
// #define MAX_VAR 2048
// const uint PRIME_BIT_WIDTH = 16;
// const uint SUB_BIT_WIDTH = 16;
// const uint PARAM_BIT_WIDTH = 18;
// const uint PARAM_DEC_WIDTH = 6;
// const uint BOOL_BIT_WIDTH = 14;
// const uint BOOL_DEC_WIDTH = 2;
// const uint NODE_TYPE_BIT_WIDTH = 3;
// const uint LITERAL_BIT_WIDTH = 13;
// const uint LITERAL_VAR_BIT_WIDTH = 14;
// const uint TOP_VAR_BIT_WIDTH = 14;
// const uint CHILD_SIZE_BIT_WIDTH = 6;
// const uint CHILD_OFFSET_BIT_WIDTH = 20;
// const uint LITERAL_IDX_BIT_WIDTH = 20;
// const uint VAR_IDX_BIT_WIDTH = 20;


//For Blockmap Network
// const uint PSDD_SIZE = 3548;
// const uint MAX_CHILDREN = 2;
// const uint TOTAL_CHILDREN = 2334;
// const short TOTAL_BOOL_PARAM = 1;
// const short TOTAL_LITERALS = 1218;
// const short TOTAL_VARIABLES = 1;
// const short TOTAL_VARIABLE_INDEXES = 1;
// const uint TOTAL_CHILDREN_SIZE = 3548;
// const uint MERGED_LOOP_LEN = 47688;
// const uint NUM_DISTICT_QUERIES = 1;
// const uint TOTAL_DECISION = 2330;
// const uint NUM_VAR = 700;
// #define MAX_VAR 1024
// const uint PRIME_BIT_WIDTH = 16;
// const uint SUB_BIT_WIDTH = 16;
// const uint PARAM_BIT_WIDTH = 18;
// const uint PARAM_DEC_WIDTH = 6;
// const uint BOOL_BIT_WIDTH = 14;
// const uint BOOL_DEC_WIDTH = 2;
// const uint NODE_TYPE_BIT_WIDTH = 3;
// const uint LITERAL_BIT_WIDTH = 13;
// const uint LITERAL_VAR_BIT_WIDTH = 14;
// const uint TOP_VAR_BIT_WIDTH = 14;
// const uint CHILD_SIZE_BIT_WIDTH = 6;
// const uint CHILD_OFFSET_BIT_WIDTH = 20;
// const uint LITERAL_IDX_BIT_WIDTH = 20;
// const uint VAR_IDX_BIT_WIDTH = 20;

//For BN_78 Network
const uint PSDD_SIZE = 42558;
const uint MAX_CHILDREN = 2;
const uint TOTAL_CHILDREN = 45272;
const short TOTAL_BOOL_PARAM = 1;
const short TOTAL_LITERALS = 2328;
const short TOTAL_VARIABLES = 1;
const short TOTAL_VARIABLE_INDEXES = 1;
const uint TOTAL_CHILDREN_SIZE = 42558;
const uint MERGED_LOOP_LEN = 47688;
const uint NUM_DISTICT_QUERIES = 1;
const uint TOTAL_DECISION = 40230;
const uint NUM_VAR = 1220;
#define MAX_VAR 2048
const uint PRIME_BIT_WIDTH = 16;
const uint SUB_BIT_WIDTH = 16;
const uint PARAM_BIT_WIDTH = 18;
const uint PARAM_DEC_WIDTH = 6;
const uint BOOL_BIT_WIDTH = 14;
const uint BOOL_DEC_WIDTH = 2;
const uint NODE_TYPE_BIT_WIDTH = 3;
const uint LITERAL_BIT_WIDTH = 13;
const uint LITERAL_VAR_BIT_WIDTH = 14;
const uint TOP_VAR_BIT_WIDTH = 14;
const uint CHILD_SIZE_BIT_WIDTH = 6;
const uint CHILD_OFFSET_BIT_WIDTH = 20;
const uint LITERAL_IDX_BIT_WIDTH = 20;
const uint VAR_IDX_BIT_WIDTH = 20;

//For fs-04 Network
// const uint PSDD_SIZE = 52789;
// const uint MAX_CHILDREN = 2;
// const uint TOTAL_CHILDREN = 45272;
// const short TOTAL_BOOL_PARAM = 1;
// const short TOTAL_LITERALS = 2328;
// const short TOTAL_VARIABLES = 1;
// const short TOTAL_VARIABLE_INDEXES = 1;
// const uint TOTAL_CHILDREN_SIZE = 52789;
// const uint MERGED_LOOP_LEN = 47688;
// const uint NUM_DISTICT_QUERIES = 1;
// const uint TOTAL_DECISION = 40230;
// const uint NUM_VAR = 262;
// #define MAX_VAR 512
// const uint PRIME_BIT_WIDTH = 16;
// const uint SUB_BIT_WIDTH = 16;
// const uint PARAM_BIT_WIDTH = 18;
// const uint PARAM_DEC_WIDTH = 6;
// const uint BOOL_BIT_WIDTH = 14;
// const uint BOOL_DEC_WIDTH = 2;
// const uint NODE_TYPE_BIT_WIDTH = 3;
// const uint LITERAL_BIT_WIDTH = 13;
// const uint LITERAL_VAR_BIT_WIDTH = 14;
// const uint TOP_VAR_BIT_WIDTH = 14;
// const uint CHILD_SIZE_BIT_WIDTH = 6;
// const uint CHILD_OFFSET_BIT_WIDTH = 20;
// const uint LITERAL_IDX_BIT_WIDTH = 20;
// const uint VAR_IDX_BIT_WIDTH = 20;

//For students Network
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
// const uint TOTAL_DECISION = 40230;
// const uint NUM_VAR = 1220;
// #define MAX_VAR 2048
// const uint PRIME_BIT_WIDTH = 16;
// const uint SUB_BIT_WIDTH = 16;
// const uint PARAM_BIT_WIDTH = 18;
// const uint PARAM_DEC_WIDTH = 6;
// const uint BOOL_BIT_WIDTH = 14;
// const uint BOOL_DEC_WIDTH = 2;
// const uint NODE_TYPE_BIT_WIDTH = 3;
// const uint LITERAL_BIT_WIDTH = 13;
// const uint LITERAL_VAR_BIT_WIDTH = 14;
// const uint TOP_VAR_BIT_WIDTH = 14;
// const uint CHILD_SIZE_BIT_WIDTH = 6;
// const uint CHILD_OFFSET_BIT_WIDTH = 20;
// const uint LITERAL_IDX_BIT_WIDTH = 20;
// const uint VAR_IDX_BIT_WIDTH = 20;

//For grids network
// const uint PSDD_SIZE = 51;
// const uint MAX_CHILDREN = 2;
// const uint TOTAL_CHILDREN = 156;
// const uint TOTAL_PARAM = 78;
// const short TOTAL_BOOL_PARAM = 48;
// #define MAX_VAR 2048
// const uint PRIME_BIT_WIDTH = 16;
// const uint SUB_BIT_WIDTH = 16;
// const uint PARAM_BIT_WIDTH = 18;
// const uint PARAM_DEC_WIDTH = 6;
// const uint BOOL_BIT_WIDTH = 14;
// const uint BOOL_DEC_WIDTH = 2;
// const uint NODE_TYPE_BIT_WIDTH = 3;
// const uint LITERAL_BIT_WIDTH = 13;
// const uint LITERAL_VAR_BIT_WIDTH = 14;
// const uint TOP_VAR_BIT_WIDTH = 14;
// const uint CHILD_SIZE_BIT_WIDTH = 6;
// const uint CHILD_OFFSET_BIT_WIDTH = 20;
// const uint LITERAL_IDX_BIT_WIDTH = 20;
// const uint VAR_IDX_BIT_WIDTH = 20;

struct PsddNodeStruct {
  ap_uint<32> node_type_;
  ap_uint<32> children_size;
  ap_uint<32> children_offset;
  ap_uint<32> packed;
};

#endif // STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
