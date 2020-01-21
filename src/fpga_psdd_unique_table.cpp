//
// Created by Yujia Shen on 10/20/17.
//


#include <psdd/fpga_psdd_node.h>
#include <psdd/fpga_psdd_unique_table.h>

#include <unordered_set>

namespace {
struct FPGAUniqueTableFunctional {
  std::size_t operator()(const FPGAPsddNode *node) const {
    return node->hash_value();
  }
  bool operator()(const FPGAPsddNode *node_a, const FPGAPsddNode *node_b) const {
    if (node_a->node_type() != node_b->node_type()) {
      return false;
    }
    if (node_a->node_type() == node_b->node_type()) {
      return *(node_a) == *(node_b);
    }
  }
};
class FPGAPsddUniqueTableImp : public FPGAPsddUniqueTable {
public:
  FPGAPsddUniqueTableImp() : FPGAPsddUniqueTable() {}
  ~FPGAPsddUniqueTableImp();
  FPGAPsddNode *GetUniqueNode(FPGAPsddNode *node, uintmax_t *node_index) {
    if (node->node_type() == 1) {
      auto cur_literal_node = node;
      SddLiteral cur_node_vtree_position =
          sdd_vtree_position(cur_literal_node->vtree_node());
      auto literal_node_map_at_vtree =
          literal_node_table_.find(cur_node_vtree_position);
      if (literal_node_map_at_vtree != literal_node_table_.end()) {
        auto found_node =
            literal_node_map_at_vtree->second.find(cur_literal_node);
        if (found_node == literal_node_map_at_vtree->second.end()) {
          literal_node_map_at_vtree->second.insert(cur_literal_node);
          if (node_index != nullptr) {
            *node_index += 1;
          }
          return node;
        } else {
          FPGAPsddNode *found_node_ptr = *found_node;
          delete (node);
          return found_node_ptr;
        }
      } else {
        literal_node_table_[cur_node_vtree_position] =
            std::unordered_set<FPGAPsddNode *, FPGAUniqueTableFunctional,
                               FPGAUniqueTableFunctional>({cur_literal_node});
        if (node_index != nullptr) {
          *node_index += 1;
        }
        return node;
      }
    } else if (node->node_type() == 2) {
      auto cur_decision_node =  node;
      SddLiteral cur_node_vtree_position =
          sdd_vtree_position(cur_decision_node->vtree_node());
      auto decision_node_map_at_vtree =
          decision_node_table_.find(cur_node_vtree_position);
      if (decision_node_map_at_vtree != decision_node_table_.end()) {
        auto found_node =
            decision_node_map_at_vtree->second.find(cur_decision_node);
        if (found_node == decision_node_map_at_vtree->second.end()) {
          decision_node_map_at_vtree->second.insert(cur_decision_node);
          if (node_index != nullptr) {
            *node_index += 1;
          }
          return node;
        } else {
          FPGAPsddNode *found_node_ptr = *found_node;
          delete (node);
          return found_node_ptr;
        }
      } else {
        decision_node_table_[cur_node_vtree_position] =
            std::unordered_set<FPGAPsddNode *, FPGAUniqueTableFunctional,
                               FPGAUniqueTableFunctional>({cur_decision_node});
        if (node_index != nullptr) {
          *node_index += 1;
        }
        return node;
      }
    } else {
      // node_type == 3
      auto cur_top_node = node;
      SddLiteral cur_node_vtree_position =
          sdd_vtree_position(cur_top_node->vtree_node());
      auto top_node_map_at_vtree =
          top_node_table_.find(cur_node_vtree_position);
      if (top_node_map_at_vtree != top_node_table_.end()) {
        auto found_node = top_node_map_at_vtree->second.find(cur_top_node);
        if (found_node == top_node_map_at_vtree->second.end()) {
          top_node_map_at_vtree->second.insert(cur_top_node);
          if (node_index != nullptr) {
            *node_index += 1;
          }
          return node;
        } else {
          FPGAPsddNode *found_node_ptr = *found_node;
          delete (node);
          return found_node_ptr;
        }
      } else {
        top_node_table_[cur_node_vtree_position] =
            std::unordered_set<FPGAPsddNode *, FPGAUniqueTableFunctional,
                               FPGAUniqueTableFunctional>({cur_top_node});
        if (node_index != nullptr) {
          *node_index += 1;
        }
        return node;
      }
    }
  }
  void DeleteFPGAPsddNodesWithoutFlagIndexes(
      const std::unordered_set<uintmax_t> &flag_index) {
    // check decision map
    auto decision_table_it = decision_node_table_.begin();
    while (decision_table_it != decision_node_table_.end()) {
      auto node_it = decision_table_it->second.begin();
      while (node_it != decision_table_it->second.end()) {
        if (flag_index.find((*node_it)->flag_index()) == flag_index.end()) {
          node_it = decision_table_it->second.erase(node_it);
        } else {
          ++node_it;
        }
      }
      if (decision_table_it->second.empty()) {
        decision_table_it = decision_node_table_.erase(decision_table_it);
      } else {
        ++decision_table_it;
      }
    }
    // check literal map
    auto literal_table_it = literal_node_table_.begin();
    while (literal_table_it != literal_node_table_.end()) {
      auto node_it = literal_table_it->second.begin();
      while (node_it != literal_table_it->second.end()) {
        if (flag_index.find((*node_it)->flag_index()) == flag_index.end()) {
          node_it = literal_table_it->second.erase(node_it);
        } else {
          ++node_it;
        }
      }
      if (literal_table_it->second.empty()) {
        literal_table_it = literal_node_table_.erase(literal_table_it);
      } else {
        ++literal_table_it;
      }
    }
    // check top map
    auto top_table_it = top_node_table_.begin();
    while (top_table_it != top_node_table_.end()) {
      auto node_it = top_table_it->second.begin();
      while (node_it != top_table_it->second.end()) {
        if (flag_index.find((*node_it)->flag_index()) == flag_index.end()) {
          node_it = top_table_it->second.erase(node_it);
        } else {
          ++node_it;
        }
      }
      if (top_table_it->second.empty()) {
        top_table_it = top_node_table_.erase(top_table_it);
      } else {
        ++top_table_it;
      }
    }
  }

  // TODO:testing this function
  void DeleteUnusedFPGAPsddNodes(
      const std::vector<FPGAPsddNode *> &used_psdd_nodes) {
    auto covered_nodes = fpga_psdd_node_util::GetCoveredPsddNodes(used_psdd_nodes);
    // check decision map
    auto decision_table_it = decision_node_table_.begin();
    while (decision_table_it != decision_node_table_.end()) {
      auto node_it = decision_table_it->second.begin();
      while (node_it != decision_table_it->second.end()) {
        if (covered_nodes.find((*node_it)->node_index()) ==
            covered_nodes.end()) {
          node_it = decision_table_it->second.erase(node_it);
        } else {
          ++node_it;
        }
      }
      if (decision_table_it->second.empty()) {
        decision_table_it = decision_node_table_.erase(decision_table_it);
      } else {
        ++decision_table_it;
      }
    }
    // check literal map
    auto literal_table_it = literal_node_table_.begin();
    while (literal_table_it != literal_node_table_.end()) {
      auto node_it = literal_table_it->second.begin();
      while (node_it != literal_table_it->second.end()) {
        if (covered_nodes.find((*node_it)->node_index()) ==
            covered_nodes.end()) {
          node_it = literal_table_it->second.erase(node_it);
        } else {
          ++node_it;
        }
      }
      if (literal_table_it->second.empty()) {
        literal_table_it = literal_node_table_.erase(literal_table_it);
      } else {
        ++literal_table_it;
      }
    }
    // check top map
    auto top_table_it = top_node_table_.begin();
    while (top_table_it != top_node_table_.end()) {
      auto node_it = top_table_it->second.begin();
      while (node_it != top_table_it->second.end()) {
        if (covered_nodes.find((*node_it)->node_index()) ==
            covered_nodes.end()) {
          node_it = top_table_it->second.erase(node_it);
        } else {
          ++node_it;
        }
      }
      if (top_table_it->second.empty()) {
        top_table_it = top_node_table_.erase(top_table_it);
      } else {
        ++top_table_it;
      }
    }
  }

private:
  std::unordered_map<
      SddLiteral, std::unordered_set<FPGAPsddNode *, FPGAUniqueTableFunctional,
                                     FPGAUniqueTableFunctional>>
      decision_node_table_;
  std::unordered_map<
      SddLiteral, std::unordered_set<FPGAPsddNode *, FPGAUniqueTableFunctional,
                                     FPGAUniqueTableFunctional>>
      literal_node_table_;
  std::unordered_map<SddLiteral,
                     std::unordered_set<FPGAPsddNode *, FPGAUniqueTableFunctional,
                                        FPGAUniqueTableFunctional>>
      top_node_table_;
};
} // namespace

FPGAPsddUniqueTable *FPGAPsddUniqueTable::GetFPGAPsddUniqueTable() {
  return new FPGAPsddUniqueTableImp();
}
