//
// Created by Yujia Shen on 3/20/18.
//

#ifndef PSDD_FPGA_PSDD_MANAGER_H
#define PSDD_FPGA_PSDD_MANAGER_H
#include <psdd/fpga_psdd_node.h>
#include <psdd/fpga_psdd_unique_table.h>
#include <xcl2/xcl2.hpp>

extern "C" {
#include <sdd/sddapi.h>
};

class FPGAPsddManager {
public:
  static FPGAPsddManager *GetFPGAPsddManagerFromSddVtree(
      Vtree *sdd_vtree,
      const std::unordered_map<uint32_t, uint32_t> &variable_mapping);
static FPGAPsddManager *GetFPGAPsddManagerFromVtree(Vtree *psdd_vtree);

  ~FPGAPsddManager();
void DeleteUnusedFPGAPsddNodes(const std::vector<FPGAPsddNode *> &used_nodes);
FPGAPsddNode *GetUniqueNode(FPGAPsddNode *node, uintmax_t *node_index);
  FPGAPsddNode *ConvertSddToPsdd(SddNode *root_node, Vtree *sdd_vtree,
                             uintmax_t flag_index);
  // variable_mapping : key is the sdd literal in the root_node, and the
  // corresponding value is the PSDD literals
  FPGAPsddNode *FromSdd(SddNode *root_node, Vtree *sdd_vtree, uintmax_t flag_index,
                    const std::unordered_set<SddLiteral> &used_variables);
  FPGAPsddNode *FromSdd(SddNode *root_node, Vtree *sdd_vtree, uintmax_t flag_index,
                    Vtree *sub_psdd_vtree);

  FPGAPsddNode *GetTrueFPGANode(Vtree *target_vtree_node, uintmax_t flag_index);

  FPGAPsddNode *GetFPGAPsddTopNode(uint32_t variable_index, uintmax_t flag_index,
                              const PsddParameter &positive_parameter,
                              const PsddParameter &negative_parameter);

FPGAPsddNode *
  GetConformedFPGAPsddDecisionNode(const std::vector<FPGAPsddNode *> &primes,
                               const std::vector<FPGAPsddNode *> &subs,
                               const std::vector<PsddParameter> &params,
                               uintmax_t flag_index);

FPGAPsddNode *GetFPGAPsddLiteralNode(int32_t literal, uintmax_t flag_index);

FPGAPsddNode *NormalizeFPGAPsddNode(Vtree *target_vtree_node,
                              FPGAPsddNode *target_psdd_node, uintmax_t flag_index);
  FPGAPsddNode *LoadFPGAPsddNode(Vtree *target_vtree, FPGAPsddNode *root_psdd_node,
                         uintmax_t flag_index);
  std::pair<FPGAPsddNode *, PsddParameter> Multiply(FPGAPsddNode *arg1, FPGAPsddNode *arg2,
                                                uintmax_t flag_index);
  Vtree *vtree() const;
  FPGAPsddNode *ReadFPGAPsddFile(const char *psdd_filename, uintmax_t flag_index, std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>> &fpga_node_vector,
    std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector, std::vector<ap_fixed<32,8,AP_RND>, aligned_allocator<ap_fixed<32,8,AP_RND>>> & parameter_vector ,
    std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>> &bool_param_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_vector,
  std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &variable_vector );
  std::vector<FPGAPsddNode *> SampleParametersForMultipleFPGAPsdds(
      RandomDoubleGenerator *generator,
      const std::vector<FPGAPsddNode *> &root_psdd_nodes, uintmax_t flag_index);
  FPGAPsddNode *SampleParameters(RandomDoubleGenerator *generator,
                             FPGAPsddNode *target_root_node, uintmax_t flag_index);
  FPGAPsddNode *LearnPsddParameters(
      FPGAPsddNode *target_structure,
      const std::unordered_map<int32_t, BatchedPsddValue> &examples,
      size_t data_size, PsddParameter alpha, uintmax_t flag_index);

private:
  FPGAPsddManager(Vtree *vtree, FPGAPsddUniqueTable *unique_table);

FPGAPsddNode *
  GetTrueFPGANode(Vtree *target_vtree_node, uintmax_t flag_index,
              std::unordered_map<SddLiteral, FPGAPsddNode *> *true_node_map);
  FPGAPsddNode *
  NormalizeFPGAPsddNode(Vtree *target_vtree_node, FPGAPsddNode *target_psdd_node,
                    uintmax_t flag_index,
                    std::unordered_map<SddLiteral, FPGAPsddNode *> *true_node_map);
  Vtree *vtree_;
  FPGAPsddUniqueTable *fpga_unique_table_;
  uintmax_t node_index_;
  std::unordered_map<uint32_t, Vtree *>
      leaf_vtree_map_;
};

#endif // PSDD_PSDD_MANAGER_H
