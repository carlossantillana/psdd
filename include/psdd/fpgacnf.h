//
// Created by jason on 2/28/18.
//

#ifndef STRUCTURED_BAYESIAN_NETWORK_FPGA_CNF_H
#define STRUCTURED_BAYESIAN_NETWORK_FPGA_CNF_H

#include <vector>
#include <cstddef>

#include <psdd/fpga_psdd_node.h>
#include <psdd/fpga_psdd_manager.h>

class fpga_CNF {
 public:
  fpga_CNF() = default;
  explicit fpga_CNF(const char *filename);
  explicit fpga_CNF(const std::vector<std::vector<SddLiteral>> &clauses);
  const std::vector<std::vector<SddLiteral>> &clauses() const;
  /*
  PsddNode *CompileToSddWithEvidence(const std::unordered_map<uint32_t, bool> &evid, Vtree *vtree) const;
  bool CheckConstraintWithPartialInstantiation(const std::bitset<MAX_VAR> &variable_mask,
                                               const std::bitset<MAX_VAR> &variable_instantiation) const;
  */
  FPGAPsddNode* Compile(FPGAPsddManager* psdd_manager, uintmax_t flag_index) const;
 private:
  std::vector<std::vector<SddLiteral>> clauses_;
};

#endif //STRUCTURED_BAYESIAN_NETWORK_CNF_H
