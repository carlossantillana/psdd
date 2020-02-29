#include <psdd/fpga_psdd_node.h>
#include <array>
#include <iostream>

double EvaluateWithoutPointer(const std::bitset<MAX_VAR> &variables,
                     const std::bitset<MAX_VAR> &instantiation,
                     uint32_t  serialized_nodes [PSDD_SIZE],
                     FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                     uint32_t children_vector[TOTAL_CHILDREN],
                   double parameter_vector[TOTAL_PARAM]) {
  double evaluation_cache [PSDD_SIZE];
  EvaluateWithoutPointer_pipeline:for(int j = PSDD_SIZE -1; j >= 0; j--){
    uintmax_t cur_node_idx = serialized_nodes[j];

    if (fpga_node_vector[cur_node_idx].node_type_ == LITERAL_NODE_TYPE) {
      if (variables[fpga_node_vector[cur_node_idx].variable_index_]) {
        if ( instantiation[fpga_node_vector[cur_node_idx].variable_index_] == (fpga_node_vector[cur_node_idx].literal_ > 0) ) {
          evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
              1;
        } else {
          evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
              0;
        }
      } else {
        evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
            1;
      }
    } else if (fpga_node_vector[cur_node_idx].node_type_ == TOP_NODE_TYPE) {
      if (variables[fpga_node_vector[cur_node_idx].variable_index_]) {
        if (instantiation[fpga_node_vector[cur_node_idx].variable_index_]) {
          evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = fpga_node_vector[cur_node_idx].true_parameter_;
        } else {
          evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = fpga_node_vector[cur_node_idx].false_parameter_;
        }
      } else {
        evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] =
            1;
      }
    } else {
      uint32_t element_size = fpga_node_vector[cur_node_idx].children_size;
      double cur_prob = 0;

      EvaluateWithoutPointer_unroll:for (size_t i = 0; i < element_size; ++i) {
        uint32_t cur_prime_idx = fpga_node_vector[children_vector[fpga_node_vector[cur_node_idx].children_offset + i]].node_index_;
        uint32_t cur_sub_idx = fpga_node_vector[children_vector[fpga_node_vector[cur_node_idx].children_offset + fpga_node_vector[cur_node_idx].children_size + i]].node_index_;
        double tmp = evaluation_cache[fpga_node_vector[cur_prime_idx].node_index_] * evaluation_cache[fpga_node_vector[cur_sub_idx].node_index_];
        tmp *= parameter_vector[fpga_node_vector[cur_node_idx].parameter_offset + i];

        if (cur_prob == 0) {
          // if this is zero
          cur_prob =  tmp;
          continue;
        } else if (tmp == 0) {
//          cur_prob = cur_prob;    j = j-1;
          continue;
        } else {
          if (cur_prob > tmp) {
            cur_prob =  cur_prob * (tmp / cur_prob);
          } else {
            cur_prob = tmp * (cur_prob / tmp);
          }
        }
      }
      evaluation_cache[fpga_node_vector[cur_node_idx].node_index_] = cur_prob;
    }
  }
  return evaluation_cache[fpga_node_vector[serialized_nodes[0]].node_index_];
 }
