//
// Created by Yujia Shen on 10/19/17.
//

#ifndef FPGA_STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
#define FPGA_STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
#include <cstddef>

#include <cstdint>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>
extern "C" {
#include <sdd/sddapi.h>
};
#include <gmpxx.h>
#include <psdd/binary_data.h>
#include <psdd/psdd_parameter.h>
#include <psdd/psdd_node.h>
#include <psdd/random_double_generator.h>
#include <unordered_set>
#include "ap_int.h"
#include <ap_fixed.h>

#define LITERAL_NODE_TYPE 1
#define DECISION_NODE_TYPE 2
#define TOP_NODE_TYPE 3

//For map_network network
const uint32_t PSDD_SIZE = 580817;
const uint32_t MAX_CHILDREN = 57;
const uint32_t TOTAL_CHILDREN = 1541021;
const uint32_t TOTAL_PARAM = 770511;
const short TOTAL_BOOL_PARAM = 792;

//For grids network
// const uint32_t PSDD_SIZE = 51;
// const uint32_t MAX_CHILDREN = 2;
// const uint32_t TOTAL_CHILDREN = 156;
// const uint32_t TOTAL_PARAM = 78;
// const short TOTAL_BOOL_PARAM = 48;


using BatchedPsddValue = std::vector<bool>;
class PsddTopNode;
class PsddLiteralNode;
class PsddDecisionNode;


class FPGAPsddNode {
public:
  FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node, uintmax_t flag_index);
  FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node);
  //literal
  FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node, uintmax_t flag_index,
                  int32_t literal);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                  uintmax_t flag_index, int32_t literal);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node, int32_t literal);
  FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node,
                   uintmax_t flag_index, const std::vector<FPGAPsddNode *> &primes,
                   const std::vector<FPGAPsddNode *> &subs,
                   const std::vector<PsddParameter> &parameters);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                   uintmax_t flag_index, const std::vector<FPGAPsddNode *> &primes,
                   const std::vector<FPGAPsddNode *> &subs,
                   const std::vector<PsddParameter> &parameters);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                   uintmax_t flag_index, const std::vector<FPGAPsddNode *> &primes,
                   const std::vector<FPGAPsddNode *> &subs);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                   const std::vector<FPGAPsddNode *> &primes,
                   const std::vector<FPGAPsddNode *> &subs);
  FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node, uintmax_t flag_index,
              uint32_t variable_index, PsddParameter true_parameter,
              PsddParameter false_parameter);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node, uintmax_t flag_index,
              uint32_t variable_index);
  FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
              uint32_t variable_index);
  FPGAPsddNode(PsddLiteralNode * literal);
  FPGAPsddNode(PsddDecisionNode * decision);
  FPGAPsddNode(PsddTopNode * top);
  ~FPGAPsddNode();
  int node_type() const; // 1 is literal node, 2 is decision node, 3
                                     // is top node with variable index
  uintmax_t node_index() const;
  uintmax_t flag_index() const;
  bool operator==(const FPGAPsddNode &other) const;
  PsddParameter true_parameter() const;
  PsddParameter false_parameter() const;
  FPGAPsddNode *psdd_top_node() { return this; }
  FPGAPsddNode *psdd_decision_node() { return this; }
  FPGAPsddNode *psdd_literal_node() { return this; }
  bool IsConsistent(const std::unordered_map<uint32_t, bool>
                                &partial_instantiation);
  bool IsConsistent(const std::bitset<MAX_VAR> &instantiation,
                    uint32_t variable_size);
  bool sign() const;
  std::size_t hash_value() const;
  Vtree *vtree_node() const;
  uintmax_t user_data() const;
  uint32_t variable_index() const;
  int32_t literal() const;

  void SetUserData(uintmax_t user_data);
  bool activation_flag() const;
  void SetActivationFlag();
  void SetNodeType(int newNodeType);
  void ResetActivationFlag();
  void ResetDataCount();
  void DirectSample(std::bitset<MAX_VAR> *instantiation,
                            RandomDoubleFromUniformGenerator *generator);
  const std::vector<FPGAPsddNode *> &primes() const;
  const std::vector<FPGAPsddNode *> &subs() const;
  const std::vector<uintmax_t> &data_counts() const;
  const std::vector<PsddParameter> &parameters() const;
  void IncrementTrueDataCount(uintmax_t increment_size);
  void IncrementFalseDataCount(uintmax_t increment_size);
  void IncrementDataCount(uintmax_t index, uintmax_t increment_size);

  uintmax_t getNodeIndex();
  Vtree* getVtreeNode();
  uintmax_t getFlagIndex();
  bool getSign();
  int32_t getLiteral();
  std::vector<FPGAPsddNode *> getPrimes();
  std::vector<FPGAPsddNode *> getSubs();
  std::vector<PsddParameter> getParams();
  std::vector<uintmax_t> getDataCount();
  uint32_t getVariableIndex();
  PsddParameter getTrueParam();
  PsddParameter getFalseParam();
  uintmax_t getFalseDataCount();
  uintmax_t getTrueDataCount();

  uintmax_t true_data_count() const;
  uintmax_t false_data_count() const;
  void SetBatchedPsddValue(BatchedPsddValue value) {
    batched_psdd_value_ = std::move(value);
  }
  const BatchedPsddValue &batched_psdd_value() const {
    return batched_psdd_value_;
  }
  BatchedPsddValue *MutableBatchedPsddValue() { return &batched_psdd_value_; }
  void SetBatchedPsddContextValue(BatchedPsddValue context_value) {
    batched_psdd_context_value_ = std::move(context_value);
  }
  const BatchedPsddValue &batched_psdd_context_value() const {
    return batched_psdd_context_value_;
  }
  BatchedPsddValue *MutableBatchedPsddContextValue() {
    return &batched_psdd_context_value_;
  }

public:
  void set_hash_value(std::size_t hash_value);
  void CalculateHashValue();
  int32_t literal_;

public:
  uintmax_t node_index_;
  Vtree *vtree_node_;
  uintmax_t user_data_;
  uintmax_t flag_index_;
  int node_type_;
  std::size_t hash_value_;
  bool activation_flag_;
  BatchedPsddValue batched_psdd_value_;
  BatchedPsddValue batched_psdd_context_value_;
  std::vector<FPGAPsddNode *> primes_;
  std::vector<FPGAPsddNode *> subs_;
  std::vector<PsddParameter> parameters_;
  std::vector<uintmax_t> data_counts_;
  uint32_t variable_index_;
  PsddParameter true_parameter_;
  PsddParameter false_parameter_;
  uintmax_t true_data_count_;
  uintmax_t false_data_count_;
};
struct FPGAPsddNodeStruct {
  ap_uint<21> node_index_;
  ap_uint<2> node_type_;
  char children_size;
  ap_uint<21> children_offset;
  ap_uint<20> parameter_offset;
  short variable_index_;
  ap_uint<10> bool_param_offset;
  int32_t literal_;
} ;
namespace fpga_vtree_util {
std::vector<Vtree *> SerializeVtree(Vtree *root);
Vtree *CopyVtree(Vtree *root);
Vtree *
CopyVtree(Vtree *root,
          const std::unordered_map<SddLiteral, SddLiteral> &variable_map);
std::vector<SddLiteral> VariablesUnderVtree(Vtree *root);
Vtree *ProjectVtree(Vtree *orig_vtree,
                    const std::vector<SddLiteral> &variables);
Vtree *SubVtreeByVariables(Vtree *root,
                           const std::unordered_set<SddLiteral> &variables);
std::vector<SddLiteral> LeftToRightLeafTraverse(Vtree *root);
} // namespace vtree_util

namespace fpga_psdd_node_util {
std::vector<FPGAPsddNode *> SerializePsddNodes(FPGAPsddNode *root);
std::vector<FPGAPsddNode *>
SerializePsddNodes(const std::vector<FPGAPsddNode *> &root_nodes);
std::vector<uint32_t> SerializePsddNodesEvaluate(uint32_t root, FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE]
                              ,ap_uint<22> children_vector[TOTAL_CHILDREN]);
std::vector<uint32_t> SerializePsddNodesEvaluate(const std::vector<uint32_t> &root_nodes,
          FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE], ap_uint<22> children_vector[TOTAL_CHILDREN]);
std::unordered_map<uintmax_t, FPGAPsddNode *>
GetCoveredPsddNodes(const std::vector<FPGAPsddNode *> &root_nodes);
void SetActivationFlag(const std::bitset<MAX_VAR> &evidence,
                       const std::vector<FPGAPsddNode *> &serialized_psdd_nodes);
std::pair<std::bitset<MAX_VAR>, Probability>
GetMPESolution(const std::vector<FPGAPsddNode *> &serialized_psdd_nodes);
std::pair<std::bitset<MAX_VAR>, Probability>
GetMPESolution(FPGAPsddNode *psdd_node);
mpz_class ModelCount(const std::vector<FPGAPsddNode *> &serialized_nodes);
Probability Evaluate(const std::bitset<MAX_VAR> &variables,
                     const std::bitset<MAX_VAR> &instantiation,
                     const std::vector<FPGAPsddNode *> &serialized_nodes);
Probability Evaluate(const std::bitset<MAX_VAR> &variables,
                     const std::bitset<MAX_VAR> &instantiation,
                     FPGAPsddNode *root_node);
uint32_t get_variable_index(FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE]);

float * EvaluateToCompare(const std::bitset<MAX_VAR> &variables,
                      const std::bitset<MAX_VAR> &instantiation,
                      ap_uint<21>  serialized_nodes [PSDD_SIZE],
                      FPGAPsddNodeStruct fpga_node_vector[PSDD_SIZE],
                      ap_uint<22> children_vector[TOTAL_CHILDREN],
                      ap_fixed<21,8,AP_RND > parameter_vector[TOTAL_PARAM],
                      ap_fixed<14,2,AP_RND > bool_param_vector [TOTAL_BOOL_PARAM]);

bool IsConsistent(FPGAPsddNode *node, const std::bitset<MAX_VAR> &variable_mask,
                  const std::bitset<MAX_VAR> &partial_instantiation);
bool IsConsistent(const std::vector<FPGAPsddNode *> &nodes,
                  const std::bitset<MAX_VAR> &variable_mask,
                  const std::bitset<MAX_VAR> &partial_instantiation);

FPGAPsddNode* ConvertToStaticNode(PsddNode* cur_root_node);

SddNode *ConvertPsddNodeToSddNode(
    const std::vector<FPGAPsddNode *> &serialized_psdd_nodes,
    const std::unordered_map<SddLiteral, SddLiteral> &variable_map,
    SddManager *sdd_manager);

void WritePsddToFile(FPGAPsddNode *root_node, const char *output_filename);

std::unordered_map<uint32_t, std::pair<Probability, Probability>>
GetMarginals(const std::vector<FPGAPsddNode *> &serialized_nodes);

uintmax_t GetPsddSize(FPGAPsddNode *root_node);

} // namespace psdd_node_util
#endif // STRUCTURED_BAYESIAN_NETWORK_PSDD_NODE_H
