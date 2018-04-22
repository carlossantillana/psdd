//
// Created by Yujia Shen on 3/20/18.
//

#ifndef PSDD_PSDD_MANAGER_H
#define PSDD_PSDD_MANAGER_H
#include "psdd_node.h"
#include "psdd_unique_table.h"
extern "C" {
#include <sddapi.h>
};

class PsddManager {
 public:
  static PsddManager *GetPsddManagerFromSddVtree(Vtree *sdd_vtree,
                                                 const std::unordered_map<uint32_t, uint32_t> &variable_mapping);
  static PsddManager *GetPsddManagerFromVtree(Vtree *psdd_vtree);
  ~PsddManager();
  void DeleteUnusedPsddNodes(const std::vector<PsddNode *> &used_nodes);
  PsddNode *ConvertSddToPsdd(SddNode *root_node,
                             Vtree *sdd_vtree,
                             uintmax_t flag_index,
                             const std::unordered_map<uint32_t, uint32_t> &variable_mapping);
  PsddNode *GetTrueNode(Vtree *target_vtree_node, uintmax_t flag_index);
  PsddTopNode *GetPsddTopNode(uint32_t variable_index,
                              uintmax_t flag_index,
                              const PsddParameter &positive_parameter,
                              const PsddParameter &negative_parameter);
  PsddDecisionNode *GetConformedPsddDecisionNode(const std::vector<PsddNode *> &primes,
                                                 const std::vector<PsddNode *> &subs,
                                                 const std::vector<PsddParameter> &params,
                                                 uintmax_t flag_index);
  PsddLiteralNode *GetPsddLiteralNode(int32_t literal, uintmax_t flag_index);
  PsddNode *NormalizePsddNode(Vtree *target_vtree_node, PsddNode *target_psdd_node, uintmax_t flag_index);
  // input psdd may or may not from the same manager, but it has to conform a consistent vtree.
  PsddNode *LoadPsddNode(Vtree *target_vtree, PsddNode *root_psdd_node, uintmax_t flag_index);
  // arguments assumed to conformed to the same vtree as the one used by this manager.
  std::pair<PsddNode *, PsddParameter> Multiply(PsddNode *arg1, PsddNode *arg2, uintmax_t flag_index);
  Vtree *vtree() const;
  PsddNode* ReadPsddFile(const char* psdd_filename, uintmax_t flag_index);
 private:
  PsddManager(Vtree *vtree, PsddUniqueTable *unique_table);
  PsddNode *GetTrueNode(Vtree *target_vtree_node,
                        uintmax_t flag_index,
                        std::unordered_map<SddLiteral, PsddNode *> *true_node_map);
  PsddNode *NormalizePsddNode(Vtree *target_vtree_node,
                              PsddNode *target_psdd_node,
                              uintmax_t flag_index,
                              std::unordered_map<SddLiteral, PsddNode *> *true_node_map);
  Vtree *vtree_;
  PsddUniqueTable *unique_table_;
  uintmax_t node_index_;
  std::unordered_map<uint32_t, Vtree *> leaf_vtree_map_; // keys are variable index
};

#endif //PSDD_PSDD_MANAGER_H
