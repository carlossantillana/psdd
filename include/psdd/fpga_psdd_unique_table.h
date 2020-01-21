//
// Created by Yujia Shen on 10/20/17.
//

#ifndef STRUCTURED_BAYESIAN_NETWORK_FPGA_PSDD_UNIQUE_TABLE_H
#define STRUCTURED_BAYESIAN_NETWORK_FPGA_PSDD_UNIQUE_TABLE_H
#include <vector>
#include <unordered_set>
extern "C" {
#include <sdd/sddapi.h>
};
class FPGAPsddNode;

class FPGAPsddUniqueTable {
 public:
  ~FPGAPsddUniqueTable() = default;
  FPGAPsddNode *GetUniqueNode(FPGAPsddNode *node, uintmax_t *node_index);
  void DeleteFPGAPsddNodesWithoutFlagIndexes(const std::unordered_set<uintmax_t> &flag_index);
  void DeleteUnusedFPGAPsddNodes(const std::vector<FPGAPsddNode *> &used_psdd_nodes);
  static FPGAPsddUniqueTable *GetFPGAPsddUniqueTable();
};

#endif //STRUCTURED_BAYESIAN_NETWORK_FPGA_PSDD_UNIQUE_TABLE_H
