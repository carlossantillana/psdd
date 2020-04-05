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
  virtual ~FPGAPsddUniqueTable() = default;
  virtual FPGAPsddNode *GetUniqueNode(FPGAPsddNode *node, uintmax_t *node_index) = 0;
  virtual void DeleteFPGAPsddNodesWithoutFlagIndexes(const std::unordered_set<uintmax_t> &flag_index) = 0;
  virtual void DeleteUnusedFPGAPsddNodes(const std::vector<FPGAPsddNode *> &used_psdd_nodes) = 0;
  static FPGAPsddUniqueTable *GetFPGAPsddUniqueTable();
};

#endif //STRUCTURED_BAYESIAN_NETWORK_FPGA_PSDD_UNIQUE_TABLE_H
