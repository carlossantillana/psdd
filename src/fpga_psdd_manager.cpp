//
// Created by Yujia Shen on 3/20/18.
//

#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <psdd/fpga_psdd_manager.h>
#include <psdd/fpga_psdd_unique_table.h>
#include <queue>
#include <sstream>
#include <stack>
#include <unordered_set>

#define DELAY 23
#define II 1

namespace {
using std::unordered_set;

void TagSddVtreeWithPsddVtree(
    const std::vector<Vtree *> &sdd_vtree_serialized,
    const std::vector<Vtree *> &psdd_vtree_serialized) {
  auto vtree_size = sdd_vtree_serialized.size();
  assert(vtree_size == psdd_vtree_serialized.size());
  for (size_t i = 0; i < vtree_size; ++i) {
    sdd_vtree_set_data((void *)psdd_vtree_serialized[i],
                       sdd_vtree_serialized[i]);
  }
}

struct MultiplyFunctional {
  std::size_t operator()(const std::pair<FPGAPsddNode *, FPGAPsddNode *> &arg) const {
    return (std::hash<uintmax_t>{}(arg.first->node_index()) << 1) ^
           (std::hash<uintmax_t>{}(arg.second->node_index()));
  }
  bool operator()(const std::pair<FPGAPsddNode *, FPGAPsddNode *> &arg1,
                  const std::pair<FPGAPsddNode *, FPGAPsddNode *> &arg2) const {
    return (arg1.second == arg2.second) && (arg1.first == arg2.first);
  }
};

class ComputationCache {
public:
  explicit ComputationCache(uint32_t variable_size)
      : cache_(2 * variable_size - 1) {}
  std::pair<FPGAPsddNode *, Probability>
  Lookup(FPGAPsddNode *first_node, FPGAPsddNode *second_node, bool *found) const {
    auto vtree_index = sdd_vtree_position(first_node->vtree_node());
    assert(cache_.size() > static_cast<size_t>(vtree_index));
    const auto &cache_at_vtree = cache_[vtree_index];
    auto node_pair = std::pair<FPGAPsddNode *, FPGAPsddNode *>(first_node, second_node);
    auto lookup_it = cache_at_vtree.find(node_pair);
    if (lookup_it == cache_at_vtree.end()) {
      *found = false;
      return std::make_pair(nullptr, Probability::CreateFromDecimal(0));
    } else {
      *found = true;
      return lookup_it->second;
    }
  }
  void Update(FPGAPsddNode *first, FPGAPsddNode *second,
              const std::pair<FPGAPsddNode *, Probability> &result) {
    auto vtree_index = sdd_vtree_position(first->vtree_node());
    assert(cache_.size() > static_cast<size_t>(vtree_index));
    std::pair<FPGAPsddNode *, FPGAPsddNode *> node_pair(first, second);
    cache_[vtree_index][node_pair] = result;
  }

private:
  std::vector<std::unordered_map<std::pair<FPGAPsddNode *, FPGAPsddNode *>,
                                 std::pair<FPGAPsddNode *, Probability>,
                                 MultiplyFunctional, MultiplyFunctional>>
      cache_;
};

std::pair<FPGAPsddNode *, PsddParameter>
MultiplyWithCache(FPGAPsddNode *first, FPGAPsddNode *second, FPGAPsddManager *manager,
                  uintmax_t flag_index, ComputationCache *cache) {
  bool found = false;
  auto result = cache->Lookup(first, second, &found);
  if (found)
    return result;
  assert(sdd_vtree_position(first->vtree_node()) ==
         sdd_vtree_position(second->vtree_node()));
  if (first->node_type() == DECISION_NODE_TYPE) {
    assert(second->node_type() == DECISION_NODE_TYPE);
    FPGAPsddNode *first_decision_node = first->psdd_decision_node();
    FPGAPsddNode *second_decision_node = second->psdd_decision_node();
    const auto &first_primes = first_decision_node->primes();
    const auto &first_subs = first_decision_node->subs();
    const auto &first_parameters = first_decision_node->parameters();
    const auto &second_primes = second_decision_node->primes();
    const auto &second_subs = second_decision_node->subs();
    const auto &second_parameters = second_decision_node->parameters();
    auto first_element_size = first_primes.size();
    auto second_element_size = second_primes.size();
    std::vector<FPGAPsddNode *> next_primes;
    std::vector<FPGAPsddNode *> next_subs;
    std::vector<PsddParameter> next_parameters;
    PsddParameter partition = PsddParameter::CreateFromDecimal(0);
    for (size_t i = 0; i < first_element_size; ++i) {
      FPGAPsddNode *cur_first_prime = first_primes[i];
      FPGAPsddNode *cur_first_sub = first_subs[i];
      PsddParameter cur_first_param = first_parameters[i];
      for (size_t j = 0; j < second_element_size; ++j) {
        FPGAPsddNode *cur_second_prime = second_primes[j];
        auto cur_prime_result = MultiplyWithCache(
            cur_first_prime, cur_second_prime, manager, flag_index, cache);
        if (cur_prime_result.first == nullptr) {
          continue;
        }
        FPGAPsddNode *cur_second_sub = second_subs[j];
        auto cur_sub_result = MultiplyWithCache(cur_first_sub, cur_second_sub,
                                                manager, flag_index, cache);
        if (cur_sub_result.first == nullptr) {
          continue;
        }
        next_primes.push_back(cur_prime_result.first);
        next_subs.push_back(cur_sub_result.first);
        PsddParameter cur_second_param = second_parameters[j];
        next_parameters.push_back(cur_second_param * cur_first_param *
                                  cur_prime_result.second *
                                  cur_sub_result.second);
        partition = partition + next_parameters.back();
      }
    }
    if (next_primes.empty()) {
      std::pair<FPGAPsddNode *, Probability> comp_result = {
          nullptr, PsddParameter::CreateFromDecimal(0)};
      cache->Update(first, second, comp_result);
      return comp_result;
    }
    for (auto &single_parameter : next_parameters) {
      single_parameter = single_parameter / partition;
      assert(single_parameter != PsddParameter::CreateFromDecimal(0));
    }
    auto new_node = manager->GetConformedFPGAPsddDecisionNode(
        next_primes, next_subs, next_parameters, flag_index);
    std::pair<FPGAPsddNode *, Probability> comp_result = {new_node, partition};
    cache->Update(first, second, comp_result);
    return comp_result;
  } else if (first->node_type() == LITERAL_NODE_TYPE) {
    FPGAPsddNode *first_literal_node = first->psdd_literal_node();
    if (second->node_type() == LITERAL_NODE_TYPE) {
      FPGAPsddNode *second_literal_node = second->psdd_literal_node();
      assert(first_literal_node->variable_index() ==
             second_literal_node->variable_index());
      if (first_literal_node->literal() == second_literal_node->literal()) {
        FPGAPsddNode *new_node = manager->GetFPGAPsddLiteralNode(
            first_literal_node->literal(), flag_index);
        std::pair<FPGAPsddNode *, Probability> comp_result = {
            new_node, Probability::CreateFromDecimal(1)};
        cache->Update(first, second, comp_result);
        return comp_result;
      } else {
        std::pair<FPGAPsddNode *, Probability> comp_result = {
            nullptr, Probability::CreateFromDecimal(0)};
        cache->Update(first, second, comp_result);
        return comp_result;
      }
    } else {
      assert(second->node_type() == TOP_NODE_TYPE);
      FPGAPsddNode *second_top_node = second->psdd_top_node();
      assert(first_literal_node->variable_index() ==
             second_top_node->variable_index());
      if (first_literal_node->sign()) {
        FPGAPsddNode *new_node = manager->GetFPGAPsddLiteralNode(
            first_literal_node->literal(), flag_index);
        std::pair<FPGAPsddNode *, Probability> comp_result = {
            new_node, second_top_node->true_parameter()};
        cache->Update(first, second, comp_result);
        return comp_result;
      } else {
        FPGAPsddNode *new_node = manager->GetFPGAPsddLiteralNode(
            first_literal_node->literal(), flag_index);
        std::pair<FPGAPsddNode *, Probability> comp_result = {
            new_node, second_top_node->false_parameter()};
        cache->Update(first, second, comp_result);
        return comp_result;
      }
    }
  } else {
    assert(first->node_type() == TOP_NODE_TYPE);
    FPGAPsddNode *first_top_node = first->psdd_top_node();
    if (second->node_type() == LITERAL_NODE_TYPE) {
      FPGAPsddNode *second_literal_node = second->psdd_literal_node();
      assert(first_top_node->variable_index() ==
             second_literal_node->variable_index());
      if (second_literal_node->sign()) {
        FPGAPsddNode *new_node = manager->GetFPGAPsddLiteralNode(
            second_literal_node->literal(), flag_index);
        std::pair<FPGAPsddNode *, Probability> comp_result = {
            new_node, first_top_node->true_parameter()};
        cache->Update(first, second, comp_result);
        return comp_result;
      } else {
        FPGAPsddNode *new_node = manager->GetFPGAPsddLiteralNode(
            second_literal_node->literal(), flag_index);
        std::pair<FPGAPsddNode *, Probability> comp_result = {
            new_node, first_top_node->false_parameter()};
        cache->Update(first, second, comp_result);
        return comp_result;
      }
    } else {
      assert(second->node_type() == TOP_NODE_TYPE);
      FPGAPsddNode *second_top_node = second->psdd_top_node();
      assert(first_top_node->variable_index() ==
             second_top_node->variable_index());
      PsddParameter pos_weight =
          first_top_node->true_parameter() * second_top_node->true_parameter();
      PsddParameter neg_weight = first_top_node->false_parameter() *
                                 second_top_node->false_parameter();
      PsddParameter partition = pos_weight + neg_weight;
      FPGAPsddNode *new_node = manager->GetFPGAPsddTopNode(
          first_top_node->variable_index(), flag_index, pos_weight / partition,
          neg_weight / partition);
      assert(new_node->psdd_top_node()->true_parameter() !=
             PsddParameter::CreateFromDecimal(0));
      assert(new_node->psdd_top_node()->false_parameter() !=
             PsddParameter::CreateFromDecimal(0));
      std::pair<FPGAPsddNode *, Probability> comp_result = {new_node, partition};
      cache->Update(first, second, comp_result);
      return comp_result;
    }
  }
}
} // namespace

FPGAPsddManager *FPGAPsddManager::GetFPGAPsddManagerFromSddVtree(
    Vtree *sdd_vtree,
    const std::unordered_map<uint32_t, uint32_t> &variable_mapping) {
  std::vector<Vtree *> serialized_sdd_vtree =
      fpga_vtree_util::SerializeVtree(sdd_vtree);
  for (auto vtree_it = serialized_sdd_vtree.rbegin();
       vtree_it != serialized_sdd_vtree.rend(); ++vtree_it) {
    Vtree *sdd_vtree_node = *vtree_it;
    if (sdd_vtree_is_leaf(sdd_vtree_node)) {
      SddLiteral sdd_variable_index = sdd_vtree_var(sdd_vtree_node);
      auto variable_mapping_it =
          variable_mapping.find((uint32_t)sdd_variable_index);
      assert(variable_mapping_it != variable_mapping.end());
      Vtree *psdd_vtree_node =
          new_leaf_vtree((SddLiteral)variable_mapping_it->second);
      sdd_vtree_set_data((void *)psdd_vtree_node, sdd_vtree_node);
    } else {
      Vtree *sdd_vtree_left_node = sdd_vtree_left(sdd_vtree_node);
      Vtree *sdd_vtree_right_node = sdd_vtree_right(sdd_vtree_node);
      auto psdd_vtree_left_node = (Vtree *)sdd_vtree_data(sdd_vtree_left_node);
      auto psdd_vtree_right_node =
          (Vtree *)sdd_vtree_data(sdd_vtree_right_node);
      Vtree *psdd_vtree_node =
          new_internal_vtree(psdd_vtree_left_node, psdd_vtree_right_node);
      sdd_vtree_set_data((void *)psdd_vtree_node, sdd_vtree_node);
    }
  }
  auto psdd_vtree = (Vtree *)sdd_vtree_data(sdd_vtree);
  set_vtree_properties(psdd_vtree);
  for (Vtree *sdd_vtree_node : serialized_sdd_vtree) {
    sdd_vtree_set_data(nullptr, sdd_vtree_node);
  }
  auto unique_table = FPGAPsddUniqueTable::GetFPGAPsddUniqueTable();
  return new FPGAPsddManager(psdd_vtree, unique_table);
}
FPGAPsddManager::FPGAPsddManager(Vtree *vtree, FPGAPsddUniqueTable *unique_table)
    : vtree_(vtree), fpga_unique_table_(unique_table), node_index_(0),
      leaf_vtree_map_() {
  std::vector<Vtree *> serialized_vtrees = fpga_vtree_util::SerializeVtree(vtree_);
  for (Vtree *cur_v : serialized_vtrees) {
    if (sdd_vtree_is_leaf(cur_v)) {
      leaf_vtree_map_[sdd_vtree_var(cur_v)] = cur_v;
    }
  }
}


FPGAPsddManager::~FPGAPsddManager() {
  fpga_unique_table_->DeleteUnusedFPGAPsddNodes({});
  delete (fpga_unique_table_);
  sdd_vtree_free(vtree_);
}
void FPGAPsddManager::DeleteUnusedFPGAPsddNodes(
    const std::vector<FPGAPsddNode *> &used_nodes) {
  fpga_unique_table_->DeleteUnusedFPGAPsddNodes(used_nodes);
}

FPGAPsddNode *FPGAPsddManager::ConvertSddToPsdd(SddNode *root_node, Vtree *sdd_vtree,
                                        uintmax_t flag_index) {
  if (sdd_node_is_false(root_node)) {
    // nullptr for PsddNode means false
    return nullptr;
  }
  if (sdd_node_is_true(root_node)) {
    return GetTrueFPGANode(vtree_, flag_index);
  }
  std::vector<Vtree *> serialized_psdd_vtrees =
      fpga_vtree_util::SerializeVtree(vtree_);
  std::vector<Vtree *> serialized_sdd_vtrees =
      fpga_vtree_util::SerializeVtree(sdd_vtree);
  TagSddVtreeWithPsddVtree(serialized_sdd_vtrees, serialized_psdd_vtrees);
  std::unordered_map<SddLiteral, FPGAPsddNode *> true_nodes_map;
  SddSize node_size = 0;
  SddNode **serialized_sdd_nodes = sdd_topological_sort(root_node, &node_size);
  std::unordered_map<SddSize, FPGAPsddNode *> node_map;
  for (size_t i = 0; i < node_size; ++i) {
    SddNode *cur_node = serialized_sdd_nodes[i];
    if (sdd_node_is_decision(cur_node)) {
      Vtree *old_vtree_node = sdd_vtree_of(cur_node);
      auto new_vtree_node = (Vtree *)sdd_vtree_data(old_vtree_node);
      std::vector<FPGAPsddNode *> primes;
      std::vector<FPGAPsddNode *> subs;
      SddNode **elements = sdd_node_elements(cur_node);
      SddSize element_size = sdd_node_size(cur_node);
      std::vector<PsddParameter> parameters(
          element_size, PsddParameter::CreateFromDecimal(1));
      for (size_t j = 0; j < element_size; j++) {
        SddNode *cur_prime = elements[2 * j];
        SddNode *cur_sub = elements[2 * j + 1];
        auto node_map_it = node_map.find(sdd_id(cur_prime));
        assert(node_map_it != node_map.end());
        FPGAPsddNode *cur_psdd_prime = node_map_it->second;
        if (sdd_node_is_true(cur_sub)) {
          FPGAPsddNode *cur_normed_psdd_prime =
              NormalizeFPGAPsddNode(sdd_vtree_left(new_vtree_node), cur_psdd_prime,
                                flag_index, &true_nodes_map);
          FPGAPsddNode *cur_normed_psdd_sub = GetTrueFPGANode(
              sdd_vtree_right(new_vtree_node), flag_index, &true_nodes_map);
          primes.push_back(cur_normed_psdd_prime);
          subs.push_back(cur_normed_psdd_sub);
        } else if (sdd_node_is_false(cur_sub)) {
          continue;
        } else {
          // a literal or decision
          auto node_map_sub_it = node_map.find(sdd_id(cur_sub));
          assert(node_map_sub_it != node_map.end());
          FPGAPsddNode *cur_psdd_sub = node_map_sub_it->second;
          FPGAPsddNode *cur_normed_psdd_prime =
              NormalizeFPGAPsddNode(sdd_vtree_left(new_vtree_node), cur_psdd_prime,
                                flag_index, &true_nodes_map);
          FPGAPsddNode *cur_normed_psdd_sub =
              NormalizeFPGAPsddNode(sdd_vtree_right(new_vtree_node), cur_psdd_sub,
                                flag_index, &true_nodes_map);
          primes.push_back(cur_normed_psdd_prime);
          subs.push_back(cur_normed_psdd_sub);
        }
      }
      assert(!primes.empty());
      FPGAPsddNode *new_decn_node = new FPGAPsddNode(
          node_index_, new_vtree_node, flag_index, primes, subs, parameters);
      new_decn_node = fpga_unique_table_->GetUniqueNode(new_decn_node, &node_index_);
      node_map[sdd_id(cur_node)] = new_decn_node;
    } else if (sdd_node_is_literal(cur_node)) {
      Vtree *old_vtree_node = sdd_vtree_of(cur_node);
      auto new_vtree_node = (Vtree *)sdd_vtree_data(old_vtree_node);
      SddLiteral old_literal = sdd_node_literal(cur_node);
      int32_t new_literal = 0;
      if (old_literal > 0) {
        // a positive literal
        new_literal = static_cast<int32_t>(sdd_vtree_var(new_vtree_node));
      } else {
        new_literal = -static_cast<int32_t>(sdd_vtree_var(new_vtree_node));
      }
      FPGAPsddNode *new_literal_node = new FPGAPsddNode(
          node_index_, new_vtree_node, flag_index, new_literal);
      new_literal_node =
          fpga_unique_table_->GetUniqueNode(new_literal_node, &node_index_);
      node_map[sdd_id(cur_node)] = new_literal_node;
    } else {
      // True false node
      continue;
    }
  }
  assert(node_map.find(sdd_id(root_node)) != node_map.end());
  FPGAPsddNode *psdd_root_node = node_map[sdd_id(root_node)];
  FPGAPsddNode *psdd_root_normalized_node =
      NormalizeFPGAPsddNode(vtree_, psdd_root_node, flag_index, &true_nodes_map);
  for (auto sdd_vtree_node : serialized_sdd_vtrees) {
    sdd_vtree_set_data(nullptr, sdd_vtree_node);
  }
  free(serialized_sdd_nodes);
  return psdd_root_normalized_node;
}

FPGAPsddNode *FPGAPsddManager::GetTrueFPGANode(
    Vtree *target_vtree_node, uintmax_t flag_index,
    std::unordered_map<SddLiteral, FPGAPsddNode *> *true_node_map) {
  if (true_node_map->find(sdd_vtree_position(target_vtree_node)) !=
      true_node_map->end()) {
    return true_node_map->find(sdd_vtree_position(target_vtree_node))->second;
  } else {
    std::vector<Vtree *> post_order_vtree_nodes =
        fpga_vtree_util::SerializeVtree(target_vtree_node);
    for (auto it = post_order_vtree_nodes.rbegin();
         it != post_order_vtree_nodes.rend(); it++) {
      Vtree *cur_vtree_node = *it;
      if (sdd_vtree_is_leaf(cur_vtree_node)) {
        FPGAPsddNode *new_true_node =
            GetFPGAPsddTopNode((uint32_t)sdd_vtree_var(cur_vtree_node), flag_index,
                           PsddParameter::CreateFromDecimal(1),
                           PsddParameter::CreateFromDecimal(1));
        true_node_map->insert(
            std::make_pair(sdd_vtree_position(cur_vtree_node), new_true_node));
      } else {
        Vtree *cur_left_node = sdd_vtree_left(cur_vtree_node);
        Vtree *cur_right_node = sdd_vtree_right(cur_vtree_node);
        assert(true_node_map->find(sdd_vtree_position(cur_left_node)) !=
               true_node_map->end());
        assert(true_node_map->find(sdd_vtree_position(cur_right_node)) !=
               true_node_map->end());
        FPGAPsddNode *left_true_node =
            true_node_map->find(sdd_vtree_position(cur_left_node))->second;
        FPGAPsddNode *right_true_node =
            true_node_map->find(sdd_vtree_position(cur_right_node))->second;
        FPGAPsddNode *new_true_node = GetConformedFPGAPsddDecisionNode(
            {left_true_node}, {right_true_node},
            {PsddParameter::CreateFromDecimal(1)}, flag_index);
        true_node_map->insert(
            std::make_pair(sdd_vtree_position(cur_vtree_node), new_true_node));
      }
    }
    assert(true_node_map->find(sdd_vtree_position(target_vtree_node)) !=
           true_node_map->end());
    return true_node_map->find(sdd_vtree_position(target_vtree_node))->second;
  }
}
FPGAPsddNode *FPGAPsddManager::GetTrueFPGANode(Vtree *target_vtree_node,
                                   uintmax_t flag_index) {
  std::unordered_map<SddLiteral, FPGAPsddNode *> true_node_map;
  return GetTrueFPGANode(target_vtree_node, flag_index, &true_node_map);
}


FPGAPsddNode *FPGAPsddManager::NormalizeFPGAPsddNode(
    Vtree *target_vtree_node, FPGAPsddNode *target_psdd_node, uintmax_t flag_index,
    std::unordered_map<SddLiteral, FPGAPsddNode *> *true_node_map) {
  FPGAPsddNode *cur_node = target_psdd_node;
  while (cur_node->vtree_node() != target_vtree_node) {
    Vtree *cur_vtree_node = cur_node->vtree_node();
    Vtree *cur_vtree_parent_node = sdd_vtree_parent(cur_vtree_node);
    assert(cur_vtree_parent_node != nullptr);
    if (sdd_vtree_left(cur_vtree_parent_node) == cur_vtree_node) {
      auto true_node = GetTrueFPGANode(sdd_vtree_right(cur_vtree_parent_node),
                                   flag_index, true_node_map);
      FPGAPsddNode *next_node = new FPGAPsddNode(
          node_index_, cur_vtree_parent_node, flag_index, {cur_node},
          {true_node}, {PsddParameter::CreateFromDecimal(1)});
      next_node = fpga_unique_table_->GetUniqueNode(next_node, &node_index_);
      cur_node = next_node;
    } else {
      assert(sdd_vtree_right(cur_vtree_parent_node) == cur_vtree_node);
      auto true_node = GetTrueFPGANode(sdd_vtree_left(cur_vtree_parent_node),
                                   flag_index, true_node_map);
      FPGAPsddNode *next_node = new FPGAPsddNode(
          node_index_, cur_vtree_parent_node, flag_index, {true_node},
          {cur_node}, {PsddParameter::CreateFromDecimal(1)});
      next_node = fpga_unique_table_->GetUniqueNode(next_node, &node_index_);
      cur_node = next_node;
    }
  }
  return cur_node;
}

Vtree *FPGAPsddManager::vtree() const { return vtree_; }

FPGAPsddManager *FPGAPsddManager::GetFPGAPsddManagerFromVtree(Vtree *psdd_vtree) {
  Vtree *copy_vtree = fpga_vtree_util::CopyVtree(psdd_vtree);
  auto *unique_table = FPGAPsddUniqueTable::GetFPGAPsddUniqueTable();
  return new FPGAPsddManager(copy_vtree, unique_table);
}

FPGAPsddNode *
FPGAPsddManager::GetFPGAPsddTopNode(uint32_t variable_index, uintmax_t flag_index,
                            const PsddParameter &positive_parameter,
                            const PsddParameter &negative_parameter) {
  assert(leaf_vtree_map_.find(variable_index) != leaf_vtree_map_.end());
  Vtree *target_vtree_node = leaf_vtree_map_[variable_index];
  assert(sdd_vtree_is_leaf(target_vtree_node));
  auto next_node = new FPGAPsddNode(node_index_, target_vtree_node, flag_index,
                                   (uint32_t)sdd_vtree_var(target_vtree_node),
                                   positive_parameter, negative_parameter);
  next_node =
      fpga_unique_table_->GetUniqueNode(next_node, &node_index_);
  return next_node;
}

FPGAPsddNode *FPGAPsddManager::GetFPGAPsddLiteralNode(int32_t literal,
                                                 uintmax_t flag_index) {
  assert(leaf_vtree_map_.find(abs(literal)) != leaf_vtree_map_.end());
  Vtree *target_vtree_node = leaf_vtree_map_[abs(literal)];
  assert(sdd_vtree_is_leaf(target_vtree_node));
  auto next_node =
      new FPGAPsddNode(node_index_, target_vtree_node, flag_index, literal);
  next_node = fpga_unique_table_->GetUniqueNode(next_node, &node_index_);
  return next_node;
}

FPGAPsddNode *FPGAPsddManager::GetConformedFPGAPsddDecisionNode(
    const std::vector<FPGAPsddNode *> &primes, const std::vector<FPGAPsddNode *> &subs,
    const std::vector<PsddParameter> &params, uintmax_t flag_index) {
  std::unordered_map<SddLiteral, FPGAPsddNode *> true_node_map;
  Vtree *lca =
      sdd_vtree_lca(primes[0]->vtree_node(), subs[0]->vtree_node(), vtree_);
  assert(lca != nullptr);
  Vtree *left_child = sdd_vtree_left(lca);
  Vtree *right_child = sdd_vtree_right(lca);
  auto element_size = primes.size();
  std::vector<FPGAPsddNode *> conformed_primes;
  std::vector<FPGAPsddNode *> conformed_subs;
  for (size_t i = 0; i < element_size; ++i) {
    FPGAPsddNode *cur_prime = primes[i];
    FPGAPsddNode *cur_sub = subs[i];
    FPGAPsddNode *cur_conformed_prime =
        NormalizeFPGAPsddNode(left_child, cur_prime, flag_index, &true_node_map);
    FPGAPsddNode *cur_conformed_sub =
        NormalizeFPGAPsddNode(right_child, cur_sub, flag_index, &true_node_map);
    conformed_primes.push_back(cur_conformed_prime);
    conformed_subs.push_back(cur_conformed_sub);
  }
  auto next_decn_node = new FPGAPsddNode(
      node_index_, lca, flag_index, conformed_primes, conformed_subs, params);
  next_decn_node = fpga_unique_table_->GetUniqueNode(
      next_decn_node, &node_index_);
  return next_decn_node;
}

// TODO: Use the flag index from the input
FPGAPsddNode *FPGAPsddManager::LoadFPGAPsddNode(Vtree *target_vtree,
                                    FPGAPsddNode *root_psdd_node,
                                    uintmax_t flag_index) {
  std::vector<FPGAPsddNode *> serialized_nodes =
      fpga_psdd_node_util::SerializePsddNodes(root_psdd_node);
  for (auto it = serialized_nodes.rbegin(); it != serialized_nodes.rend();
       ++it) {
    FPGAPsddNode *cur_node = *it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      FPGAPsddNode *cur_literal_node = cur_node->psdd_literal_node();
      FPGAPsddNode *new_node =
          GetFPGAPsddLiteralNode(cur_literal_node->literal(), flag_index);
      cur_literal_node->SetUserData((uintmax_t)new_node);
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      FPGAPsddNode *cur_top_node = cur_node->psdd_top_node();
      FPGAPsddNode *new_top_node = GetFPGAPsddTopNode(
          cur_top_node->variable_index(), flag_index,
          cur_top_node->true_parameter(), cur_top_node->false_parameter());
      cur_top_node->SetUserData((uintmax_t)new_top_node);
    } else {
      assert(cur_node->node_type() == DECISION_NODE_TYPE);
      FPGAPsddNode *cur_decision_node = cur_node->psdd_decision_node();
      const auto &cur_primes = cur_decision_node->primes();
      const auto &cur_subs = cur_decision_node->subs();
      const auto &cur_parameters = cur_decision_node->parameters();
      std::vector<FPGAPsddNode *> new_primes(cur_primes.size(), nullptr);
      std::vector<FPGAPsddNode *> new_subs(cur_subs.size(), nullptr);
      for (size_t i = 0; i < cur_primes.size(); ++i) {
        new_primes[i] = (FPGAPsddNode *)cur_primes[i]->user_data();
        new_subs[i] = (FPGAPsddNode *)cur_subs[i]->user_data();
      }
      FPGAPsddNode *new_decision_node = GetConformedFPGAPsddDecisionNode(
          new_primes, new_subs, cur_parameters, flag_index);
      cur_decision_node->SetUserData((uintmax_t)new_decision_node);
    }
  }
  auto new_root_node = (FPGAPsddNode *)root_psdd_node->user_data();
  std::unordered_map<SddLiteral, FPGAPsddNode *> true_node_map;
  FPGAPsddNode *result_node = NormalizeFPGAPsddNode(target_vtree, new_root_node,
                                            flag_index, &true_node_map);
  for (FPGAPsddNode *cur_node : serialized_nodes) {
    cur_node->SetUserData(0);
  }
  return result_node;
}
FPGAPsddNode *FPGAPsddManager::NormalizeFPGAPsddNode(Vtree *target_vtree_node,
                                         FPGAPsddNode *target_psdd_node,
                                         uintmax_t flag_index) {
  std::unordered_map<SddLiteral, FPGAPsddNode *> true_node_map;
  return NormalizeFPGAPsddNode(target_vtree_node, target_psdd_node, flag_index,
                           &true_node_map);
}

std::pair<FPGAPsddNode *, PsddParameter>
FPGAPsddManager::Multiply(FPGAPsddNode *arg1, FPGAPsddNode *arg2, uintmax_t flag_index) {
  ComputationCache cache((uint32_t)leaf_vtree_map_.size());
  return MultiplyWithCache(arg1, arg2, this, flag_index, &cache);
}


PsddNodeStruct ConvertPsddToStruct(FPGAPsddNode * cur_node, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector,
  int & currentChild, std::vector<ap_fixed<32,8,AP_RND>, aligned_allocator<ap_fixed<32,8,AP_RND>>> &parameter_vector,
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>>& bool_param_vector, int & currentBoolParam){
  PsddNodeStruct PsddStruct;
  PsddStruct.node_type_ = cur_node->node_type_;
  PsddStruct.children_size = cur_node->primes_.size();
  PsddStruct.children_offset = currentChild;
  for (int i = 0; i < cur_node->primes_.size(); i++){
    uint32_t prime = cur_node->primes_[i]->node_index_;
    uint32_t sub = cur_node->subs_[i]->node_index_;
    PsddParameter param = cur_node->parameters_[i];
    prime_vector[currentChild] = prime;
    sub_vector[currentChild] = sub;
    parameter_vector[currentChild] = param.parameter_;
    currentChild++;
  }
  if (int(PsddStruct.node_type_) == TOP_NODE_TYPE){
    bool_param_vector[currentBoolParam] = cur_node->true_parameter_.parameter_;
    currentBoolParam++;
    bool_param_vector[currentBoolParam] = cur_node->false_parameter_.parameter_;
    currentBoolParam++;
  }
  return PsddStruct;
}
FPGAPsddNode *FPGAPsddManager::ReadFPGAPsddFile(const char *psdd_filename, uintmax_t flag_index,
  std::vector<ap_uint<64>,aligned_allocator<ap_uint<64>>> &dram_data,
  std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>> &fpga_node_vector,
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector,
  //std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &new_prime_vector,
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector,
  //std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &new_sub_vector,
  //std::vector<ap_fixed<32,8,AP_RND>, aligned_allocator<ap_fixed<32,8,AP_RND>>> &new_parameter_vector ,
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>> &bool_param_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_index_vector,
  std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_variable_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &top_variable_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &variable_index_vector, std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &children_size_vector,
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &children_offset_vector
 //,std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &new_node_type_vector
 ) {
  std::ifstream psdd_file;
  std::unordered_map<uintmax_t, FPGAPsddNode *> construct_fpga_cache;
  int currentChild = 0;
  int currentBoolParam = 0;
  int currentLiteral = 0;
  int current_index = 0;
  int new_index = 0;
  int currentTopVariable = 0;
  int maxChildren = 0;

  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>>  node_type_vector(PSDD_SIZE);
  std::vector<ap_fixed<32,8,AP_RND>, aligned_allocator<ap_fixed<32,8,AP_RND>>> parameter_vector(TOTAL_CHILDREN);

  std::vector<int,aligned_allocator<int>>  node_update_cycle(PSDD_SIZE); // this array records the last cycle a node prob was updated
  for(int i = 0 ; i < PSDD_SIZE ; i++ ){
    node_update_cycle[i] = -100;
  }
  psdd_file.open(psdd_filename);
  if (!psdd_file) {
    std::cerr << "File " << psdd_filename << " cannot be open.";
    exit(1); // terminate with error
  }
  std::unordered_map<int, int> children;
  std::string line;
  FPGAPsddNode *root_node = nullptr;
  while (std::getline(psdd_file, line)) {
    if (line[0] == 'c') {
      continue;
    }
    if (line[0] == 'p') {
      continue;
    }
    if (line[0] == 'L') {
      std::istringstream iss(line.substr(1, std::string::npos));
      uintmax_t node_index;
      uint32_t vtree_index;
      int32_t literal;
      iss >> node_index >> vtree_index >> literal;
      FPGAPsddNode *cur_node = GetFPGAPsddLiteralNode(literal, flag_index);
      fpga_node_vector[cur_node->node_index_] = ConvertPsddToStruct(cur_node, prime_vector, sub_vector,
         currentChild, parameter_vector, bool_param_vector, currentBoolParam);
      construct_fpga_cache[node_index] = cur_node;
      root_node = cur_node;
      literal_vector[currentLiteral] = literal;
      literal_variable_vector[currentLiteral] = literal > 0 ? static_cast<uint>(literal)
                          : static_cast<uint32_t>(-literal);
      literal_index_vector[currentLiteral++] = current_index;
    	ap_uint<64> dram_fifo_data = 0;
    	dram_fifo_data(62,61) = 1;
    	dram_data[new_index] = dram_fifo_data;
    	new_index++;
      node_type_vector[current_index++] = 1;
    } else if (line[0] == 'T') {
      std::istringstream iss(line.substr(1, std::string::npos));
      uintmax_t node_index;
      uint32_t vtree_index;
      uint32_t variable_index;
      double neg_log_pr;
      double pos_log_pr;
      iss >> node_index >> vtree_index >> variable_index >> neg_log_pr >>
          pos_log_pr;
      FPGAPsddNode *cur_node = GetFPGAPsddTopNode(
          variable_index, flag_index, PsddParameter::CreateFromLog(pos_log_pr),
          PsddParameter::CreateFromLog(neg_log_pr));
      fpga_node_vector[cur_node->node_index_] = ConvertPsddToStruct(cur_node,
         prime_vector, sub_vector, currentChild, parameter_vector, bool_param_vector, currentBoolParam);
      construct_fpga_cache[node_index] = cur_node;
      top_variable_vector[currentTopVariable] = cur_node->variable_index_;
      variable_index_vector[currentTopVariable++] = current_index;
    	ap_uint<64> dram_fifo_data = 0;
    	dram_fifo_data(62,61) = 1; //Don't do anything, and increase node_index
    	dram_data[new_index] = dram_fifo_data;
    	new_index++;
      node_type_vector[current_index++] = 3;
      root_node = cur_node;
    } else {
      assert(line[0] == 'D');
      std::istringstream iss(line.substr(1, std::string::npos));
      uintmax_t node_index;
      int vtree_index;
      uintmax_t element_size;
      iss >> node_index >> vtree_index >> element_size;
      std::vector<FPGAPsddNode *> primes;
      std::vector<FPGAPsddNode *> subs;
      std::vector<PsddParameter> params;
      if (children.find(element_size) == children.end()){
        children.emplace(element_size, 1);
      } else{
        children.at(element_size)++;
      }
      for (size_t j = 0; j < element_size; j++) {
        uintmax_t prime_index;
        uintmax_t sub_index;
        double weight_in_log;
        iss >> prime_index >> sub_index >> weight_in_log;
        assert(construct_fpga_cache.find(prime_index) != construct_fpga_cache.end());
        assert(construct_fpga_cache.find(sub_index) != construct_fpga_cache.end());
        FPGAPsddNode *prime_node = construct_fpga_cache[prime_index];
        FPGAPsddNode *sub_node = construct_fpga_cache[sub_index];
        primes.push_back(prime_node);
        subs.push_back(sub_node);
	      PsddParameter new_params = PsddParameter::CreateFromLog(weight_in_log);
        params.push_back(new_params);
      	while( new_index - node_update_cycle[prime_index] - DELAY/II < 0 || new_index - node_update_cycle[sub_index] - DELAY/II < 0 ){ //Insert bubbles
      		// printf("current_index:%d new_index:%d prime_index:%d sub_index:%d node_update_cycle[prime_index]:%d node_update_cycle[sub_index]:%d \n", current_index, new_index, prime_index, sub_index, node_update_cycle[prime_index], node_update_cycle[sub_index] );
      		ap_uint<64> dram_fifo_data = 0;
        	dram_fifo_data(62,61) = 0; //don't do anything (and don't increase node index either)
      		dram_data[new_index] = dram_fifo_data;
      		new_index++;
      	}
	      ap_uint<64> dram_fifo_data;
        dram_fifo_data(19,0) = prime_index;
        dram_fifo_data(39,20) = sub_index;
	      ap_fixed<21,8,AP_RND > parameter_temp = new_params.parameter_;
        dram_fifo_data(60,40) = *((ap_uint<21>*)(&(parameter_temp)));
        if( j == element_size - 1 ){
	        dram_fifo_data(62,61) = 3; //do computation, and this is the final edge of a node (increase node index)
		      node_update_cycle[current_index] = new_index;	//record the last cycle a node prob is updated
        } else{
        	dram_fifo_data(62,61) = 2; // do computation, and this is NOT the final edge of a node (don't increase node index)
	       }
    	dram_data[new_index] = dram_fifo_data;
    	new_index++;
      }
      FPGAPsddNode *cur_node =
          GetConformedFPGAPsddDecisionNode(primes, subs, params, flag_index);
      maxChildren = maxChildren > cur_node->primes_.size() ? maxChildren : cur_node->primes_.size();
      children_size_vector[current_index] = cur_node->primes_.size();
      children_offset_vector[current_index] = currentChild;
      node_type_vector[current_index++] = 2;
  	if( element_size < 1 ){
  		printf("error. element_size:%d for node %d\n", element_size, current_index);
  		exit(1);
  	}
    fpga_node_vector[cur_node->node_index_] = ConvertPsddToStruct(cur_node,
      prime_vector, sub_vector, currentChild, parameter_vector, bool_param_vector, currentBoolParam);
    construct_fpga_cache[node_index] = cur_node;
    root_node = cur_node;
    }
  }
  printf("currentChild: %u, currentBoolParam: %u, currentLiteral: %u, currentTopVariable: %u, maxChildren: %u\n",
  currentChild, currentBoolParam, currentLiteral, currentTopVariable, maxChildren);
	printf("measured MERGED_LOOP_LEN:%d const MERGED_LOOP_LEN : %d\n", new_index, MERGED_LOOP_LEN);
	if( new_index != MERGED_LOOP_LEN ){
		printf("Above two numbers should match. Exiting...\n");
		exit(1);
	}
  psdd_file.close();
  return root_node;
}

PsddNodeStruct ConvertPsddToStructOldLinear(FPGAPsddNode * cur_node, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector,
  int & currentChild, std::vector<float, aligned_allocator<float>> &parameter_vector,
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>>& bool_param_vector, int & currentBoolParam,
  int &previousPrime, int& previousSub, int current_index){
  PsddNodeStruct PsddStruct;
  PsddStruct.node_type_ = cur_node->node_type_;
  PsddStruct.children_size = cur_node->primes_.size();
  PsddStruct.children_offset = currentChild;
  for (int i = 0; i < cur_node->primes_.size(); i++){
    int prime = cur_node->primes_[i]->node_index_;
    int sub = cur_node->subs_[i]->node_index_;
    PsddParameter param = cur_node->parameters_[i];
    prime_vector[currentChild] = prime ;
    sub_vector[currentChild] = sub ;
    parameter_vector[currentChild] = std::exp(param.parameter_);
    if (currentChild == 770397 ){
      std::cout << "offset: " << 770397 << " node: " << current_index << " param.parameter_: " << param.parameter_ << " std::exp(param.parameter_): " << std::exp(param.parameter_) << std::endl;
    }
    currentChild++;
  }
  if (int(PsddStruct.node_type_) == TOP_NODE_TYPE){
    bool_param_vector[currentBoolParam] = std::exp(cur_node->true_parameter_.parameter_);
    currentBoolParam++;
    bool_param_vector[currentBoolParam] = std::exp(cur_node->false_parameter_.parameter_);
    currentBoolParam++;
  }
  return PsddStruct;
}

PsddNodeStruct ConvertPsddToStructOld(FPGAPsddNode * cur_node, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector,
  int & currentChild, std::vector<float, aligned_allocator<float>> &parameter_vector,
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>>& bool_param_vector, int & currentBoolParam,
  int &previousPrime, int& previousSub){
  PsddNodeStruct PsddStruct;
  PsddStruct.node_type_ = cur_node->node_type_;
  PsddStruct.children_size = cur_node->primes_.size();
  PsddStruct.children_offset = currentChild;
  for (int i = 0; i < cur_node->primes_.size(); i++){
    int prime = cur_node->primes_[i]->node_index_;
    int sub = cur_node->subs_[i]->node_index_;
    PsddParameter param = cur_node->parameters_[i];
    prime_vector[currentChild] = prime ;
    sub_vector[currentChild] = sub ;
    parameter_vector[currentChild] = param.parameter_;
    currentChild++;
  }
  if (int(PsddStruct.node_type_) == TOP_NODE_TYPE){
    bool_param_vector[currentBoolParam] = cur_node->true_parameter_.parameter_;
    currentBoolParam++;
    bool_param_vector[currentBoolParam] = cur_node->false_parameter_.parameter_;
    currentBoolParam++;
  }
  return PsddStruct;
}
FPGAPsddNode *FPGAPsddManager::ReadFPGAPsddFileOld(const char *psdd_filename, uintmax_t flag_index, std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>> &fpga_node_vector,
  std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector, std::vector<float, aligned_allocator<float>> &parameter_vector ,
  std::vector<ap_fixed<32,2,AP_RND>, aligned_allocator<ap_fixed<32,2,AP_RND>>> &bool_param_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_index_vector,
  std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &literal_variable_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &top_variable_vector, std::vector<ap_int<32>,aligned_allocator<ap_int<32>>> &variable_index_vector, std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &children_size_vector,
  std::vector<ap_uint<32>, aligned_allocator<ap_uint<32>>> &children_offset_vector, std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &node_type_vector ) {
  std::ifstream psdd_file;
  std::unordered_map<uintmax_t, FPGAPsddNode *> construct_fpga_cache;
  int currentChild = 0;
  int currentBoolParam = 0;
  int currentLiteral = 0;
  int current_index = 0;
  int currentTopVariable = 0;
  int previousPrime = 0;
  int previousSub = 0;
  int maxChildren = 0;
  int currentDecision = 0;
  psdd_file.open(psdd_filename);
  if (!psdd_file) {
    std::cerr << "File " << psdd_filename << " cannot be open.";
    exit(1); // terminate with error
  }
  std::unordered_map<int, int> children;
  std::string line;
  FPGAPsddNode *root_node = nullptr;
  while (std::getline(psdd_file, line)) {
    if (line[0] == 'c') {
      continue;
    }
    if (line[0] == 'p') {
      continue;
    }
    if (line[0] == 'L') {
      std::istringstream iss(line.substr(1, std::string::npos));
      uintmax_t node_index;
      uint32_t vtree_index;
      int32_t literal;
      iss >> node_index >> vtree_index >> literal;
      FPGAPsddNode *cur_node = GetFPGAPsddLiteralNode(literal, flag_index);
      fpga_node_vector[cur_node->node_index_] = ConvertPsddToStructOldLinear(cur_node, prime_vector, sub_vector,
         currentChild, parameter_vector, bool_param_vector, currentBoolParam,
         previousPrime, previousSub, current_index);
      construct_fpga_cache[node_index] = cur_node;
      root_node = cur_node;
      literal_vector[currentLiteral] = literal;
      literal_variable_vector[currentLiteral] = literal > 0 ? static_cast<uint>(literal)
                          : static_cast<uint32_t>(-literal);
      literal_index_vector[currentLiteral++] = current_index;
      node_type_vector[current_index++] = LITERAL_NODE_TYPE;

    } else if (line[0] == 'T') {
      std::istringstream iss(line.substr(1, std::string::npos));
      uintmax_t node_index;
      uint32_t vtree_index;
      uint32_t variable_index;
      double neg_log_pr;
      double pos_log_pr;
      iss >> node_index >> vtree_index >> variable_index >> neg_log_pr >>
          pos_log_pr;
      FPGAPsddNode *cur_node = GetFPGAPsddTopNode(
          variable_index, flag_index, PsddParameter::CreateFromLog(pos_log_pr),
          PsddParameter::CreateFromLog(neg_log_pr));
      fpga_node_vector[cur_node->node_index_] = ConvertPsddToStructOldLinear(cur_node,
         prime_vector, sub_vector, currentChild, parameter_vector, bool_param_vector, currentBoolParam,
          previousPrime, previousSub, current_index);
      construct_fpga_cache[node_index] = cur_node;
      top_variable_vector[currentTopVariable] = cur_node->variable_index_;
      variable_index_vector[currentTopVariable++] = current_index;
      node_type_vector[current_index++] = TOP_NODE_TYPE;
      root_node = cur_node;
    } else {
      assert(line[0] == 'D');
      std::istringstream iss(line.substr(1, std::string::npos));
      uintmax_t node_index;
      int vtree_index;
      uintmax_t element_size;


      iss >> node_index >> vtree_index >> element_size;
      std::vector<FPGAPsddNode *> primes;
      std::vector<FPGAPsddNode *> subs;
      std::vector<PsddParameter> params;
      if (children.find(element_size) == children.end()){
        children.emplace(element_size, 1);
      } else{
        children.at(element_size)++;
      }
      for (size_t j = 0; j < element_size; j++) {
        uintmax_t prime_index;
        uintmax_t sub_index;
        double weight_in_log;
        iss >> prime_index >> sub_index >> weight_in_log;
        assert(construct_fpga_cache.find(prime_index) != construct_fpga_cache.end());
        assert(construct_fpga_cache.find(sub_index) != construct_fpga_cache.end());
        FPGAPsddNode *prime_node = construct_fpga_cache[prime_index];
        FPGAPsddNode *sub_node = construct_fpga_cache[sub_index];
        primes.push_back(prime_node);
        subs.push_back(sub_node);
        params.push_back(PsddParameter::CreateFromLog(weight_in_log));
      }
      FPGAPsddNode *cur_node =
          GetConformedFPGAPsddDecisionNode(primes, subs, params, flag_index);
          children_size_vector[current_index] = cur_node->primes_.size();
          children_offset_vector[current_index] = currentChild;
          node_type_vector[current_index++] = DECISION_NODE_TYPE;
          currentDecision++;
          maxChildren = maxChildren > cur_node->primes_.size() ? maxChildren : cur_node->primes_.size();
      fpga_node_vector[cur_node->node_index_] = ConvertPsddToStructOldLinear(cur_node,
        prime_vector, sub_vector, currentChild, parameter_vector, bool_param_vector, currentBoolParam,
        previousPrime, previousSub, current_index);
      construct_fpga_cache[node_index] = cur_node;
      root_node = cur_node;
    }
  }
  std::cout << "TOTAL_LITERALS: " << currentLiteral <<  " TOTAL_VARIABLES: " << currentTopVariable << " TOTAL_DECISION: " << currentDecision << " TOTAL_CHILDREN: "  << currentChild << " PSDD_SIZE: " << current_index << " MAX_CHILDREN: " << maxChildren << std::endl;
  psdd_file.close();
  return root_node;
}

std::vector<FPGAPsddNode *> FPGAPsddManager::SampleParametersForMultipleFPGAPsdds(
    RandomDoubleGenerator *generator,
    const std::vector<FPGAPsddNode *> &root_psdd_nodes, uintmax_t flag_index) {
  std::vector<FPGAPsddNode *> serialized_psdd_nodes =
      fpga_psdd_node_util::SerializePsddNodes(root_psdd_nodes);
  for (auto node_it = serialized_psdd_nodes.rbegin();
       node_it != serialized_psdd_nodes.rend(); ++node_it) {
    FPGAPsddNode *cur_node = *node_it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      FPGAPsddNode *new_node = GetFPGAPsddLiteralNode(
          cur_node->psdd_literal_node()->literal(), flag_index);
      cur_node->SetUserData((uintmax_t)new_node);
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      double pos_num = generator->generate();
      double neg_num = generator->generate();
      double sum = pos_num + neg_num;
      auto true_parameter = PsddParameter::CreateFromDecimal(pos_num / sum);
      auto false_parameter = PsddParameter::CreateFromDecimal(neg_num / sum);
      double sum_lg = (true_parameter + false_parameter).parameter();
      assert(std::abs(sum_lg) <= 0.0001);
      FPGAPsddNode *new_node =
          GetFPGAPsddTopNode(cur_node->psdd_top_node()->variable_index(),
                         flag_index, true_parameter, false_parameter);
      cur_node->SetUserData((uintmax_t)new_node);
    } else {
      assert(cur_node->node_type() == DECISION_NODE_TYPE);
      FPGAPsddNode *cur_decn_node = cur_node->psdd_decision_node();
      const auto &primes = cur_decn_node->primes();
      const auto &subs = cur_decn_node->subs();
      auto element_size = primes.size();
      std::vector<FPGAPsddNode *> next_primes(element_size, nullptr);
      std::vector<FPGAPsddNode *> next_subs(element_size, nullptr);
      std::vector<PsddParameter> sampled_number(element_size);
      PsddParameter sum = PsddParameter::CreateFromDecimal(0);
      for (size_t i = 0; i < element_size; ++i) {
        double cur_num = generator->generate();
        sampled_number[i] = PsddParameter::CreateFromDecimal(cur_num);
        sum = sum + sampled_number[i];
        next_primes[i] = (FPGAPsddNode *)primes[i]->user_data();
        next_subs[i] = (FPGAPsddNode *)subs[i]->user_data();
      }
      std::vector<PsddParameter> next_parameters(element_size);
      for (size_t i = 0; i < element_size; ++i) {
        next_parameters[i] = sampled_number[i] / sum;
      }
      FPGAPsddNode *new_node = GetConformedFPGAPsddDecisionNode(
          next_primes, next_subs, next_parameters, flag_index);
      cur_node->SetUserData((uintmax_t)new_node);
    }
  }
  auto root_psdd_nodes_size = root_psdd_nodes.size();
  std::vector<FPGAPsddNode *> new_root_nodes(root_psdd_nodes_size, nullptr);
  for (size_t i = 0; i < root_psdd_nodes_size; ++i) {
    new_root_nodes[i] = (FPGAPsddNode *)root_psdd_nodes[i]->user_data();
  }
  for (FPGAPsddNode *cur_node : serialized_psdd_nodes) {
    cur_node->SetUserData(0);
  }
  return new_root_nodes;
}
FPGAPsddNode *FPGAPsddManager::SampleParameters(RandomDoubleGenerator *generator,
                                        FPGAPsddNode *target_root_node,
                                        uintmax_t flag_index) {
  return SampleParametersForMultipleFPGAPsdds(generator, {target_root_node},
                                          flag_index)[0];
}

FPGAPsddNode *FPGAPsddManager::FromSdd(
    SddNode *root_node, Vtree *sdd_vtree, uintmax_t flag_index,
    const std::unordered_set<SddLiteral> &used_psdd_variables) {
  Vtree *sub_psdd_vtree =
      fpga_vtree_util::SubVtreeByVariables(vtree_, used_psdd_variables);
  return FromSdd(root_node, sdd_vtree, flag_index, sub_psdd_vtree);
}

FPGAPsddNode *FPGAPsddManager::FromSdd(SddNode *root_node, Vtree *sdd_vtree,
                               uintmax_t flag_index, Vtree *sub_psdd_vtree) {
  if (sdd_node_is_false(root_node)) {
    // nullptr for PsddNode means false
    return nullptr;
  }
  if (sdd_node_is_true(root_node)) {
    return GetTrueFPGANode(sub_psdd_vtree, flag_index);
  }
  std::vector<Vtree *> serialized_psdd_vtrees =
      fpga_vtree_util::SerializeVtree(sub_psdd_vtree);
  std::vector<Vtree *> serialized_sdd_vtrees =
      fpga_vtree_util::SerializeVtree(sdd_vtree);
  assert(serialized_psdd_vtrees.size() == serialized_sdd_vtrees.size());
  TagSddVtreeWithPsddVtree(serialized_sdd_vtrees, serialized_psdd_vtrees);
  std::unordered_map<SddLiteral, FPGAPsddNode *> true_nodes_map;
  SddSize node_size = 0;
  SddNode **serialized_sdd_nodes = sdd_topological_sort(root_node, &node_size);
  std::unordered_map<SddSize, FPGAPsddNode *> node_map;
  for (size_t i = 0; i < node_size; ++i) {
    SddNode *cur_node = serialized_sdd_nodes[i];
    if (sdd_node_is_decision(cur_node)) {
      Vtree *old_vtree_node = sdd_vtree_of(cur_node);
      auto new_vtree_node = (Vtree *)sdd_vtree_data(old_vtree_node);
      std::vector<FPGAPsddNode *> primes;
      std::vector<FPGAPsddNode *> subs;
      SddNode **elements = sdd_node_elements(cur_node);
      SddSize element_size = sdd_node_size(cur_node);
      std::vector<PsddParameter> parameters(
          element_size, PsddParameter::CreateFromDecimal(1));
      for (size_t j = 0; j < element_size; j++) {
        SddNode *cur_prime = elements[2 * j];
        SddNode *cur_sub = elements[2 * j + 1];
        auto node_map_it = node_map.find(sdd_id(cur_prime));
        assert(node_map_it != node_map.end());
        FPGAPsddNode *cur_psdd_prime = node_map_it->second;
        if (sdd_node_is_true(cur_sub)) {
          FPGAPsddNode *cur_normed_psdd_prime =
              NormalizeFPGAPsddNode(sdd_vtree_left(new_vtree_node), cur_psdd_prime,
                                flag_index, &true_nodes_map);
          FPGAPsddNode *cur_normed_psdd_sub = GetTrueFPGANode(
              sdd_vtree_right(new_vtree_node), flag_index, &true_nodes_map);
          primes.push_back(cur_normed_psdd_prime);
          subs.push_back(cur_normed_psdd_sub);
        } else if (sdd_node_is_false(cur_sub)) {
          continue;
        } else {
          // a literal or decision
          auto node_map_sub_it = node_map.find(sdd_id(cur_sub));
          assert(node_map_sub_it != node_map.end());
          FPGAPsddNode *cur_psdd_sub = node_map_sub_it->second;
          FPGAPsddNode *cur_normed_psdd_prime =
              NormalizeFPGAPsddNode(sdd_vtree_left(new_vtree_node), cur_psdd_prime,
                                flag_index, &true_nodes_map);
          FPGAPsddNode *cur_normed_psdd_sub =
              NormalizeFPGAPsddNode(sdd_vtree_right(new_vtree_node), cur_psdd_sub,
                                flag_index, &true_nodes_map);
          primes.push_back(cur_normed_psdd_prime);
          subs.push_back(cur_normed_psdd_sub);
        }
      }
      assert(!primes.empty());
      FPGAPsddNode *new_decn_node = new FPGAPsddNode(
          node_index_, new_vtree_node, flag_index, primes, subs, parameters);
      new_decn_node = fpga_unique_table_->GetUniqueNode(new_decn_node, &node_index_);
      node_map[sdd_id(cur_node)] = new_decn_node;
    } else if (sdd_node_is_literal(cur_node)) {
      Vtree *old_vtree_node = sdd_vtree_of(cur_node);
      auto new_vtree_node = (Vtree *)sdd_vtree_data(old_vtree_node);
      SddLiteral old_literal = sdd_node_literal(cur_node);
      int32_t new_literal = 0;
      if (old_literal > 0) {
        // a positive literal
        new_literal = static_cast<int32_t>(sdd_vtree_var(new_vtree_node));
      } else {
        new_literal = -static_cast<int32_t>(sdd_vtree_var(new_vtree_node));
      }
      FPGAPsddNode *new_literal_node = new FPGAPsddNode(
          node_index_, new_vtree_node, flag_index, new_literal);
      new_literal_node =
          fpga_unique_table_->GetUniqueNode(new_literal_node, &node_index_);
      node_map[sdd_id(cur_node)] = new_literal_node;
    } else {
      // True false node
      continue;
    }
  }
  assert(node_map.find(sdd_id(root_node)) != node_map.end());
  FPGAPsddNode *psdd_root_node = node_map[sdd_id(root_node)];
  FPGAPsddNode *psdd_root_normalized_node = NormalizeFPGAPsddNode(
      sub_psdd_vtree, psdd_root_node, flag_index, &true_nodes_map);
  for (auto sdd_vtree_node : serialized_sdd_vtrees) {
    sdd_vtree_set_data(nullptr, sdd_vtree_node);
  }
  free(serialized_sdd_nodes);
  return psdd_root_normalized_node;
}

FPGAPsddNode *FPGAPsddManager::LearnPsddParameters(
    FPGAPsddNode *target_structure,
    const std::unordered_map<int32_t, BatchedPsddValue> &examples,
    size_t data_size, PsddParameter alpha, uintmax_t flag_index) {
  std::vector<FPGAPsddNode *> serialized_psdd_nodes =
      fpga_psdd_node_util::SerializePsddNodes(target_structure);
  assert(serialized_psdd_nodes[0] == target_structure);
  if (data_size != 0) {
    for (auto node_it = serialized_psdd_nodes.rbegin();
         node_it != serialized_psdd_nodes.rend(); ++node_it) {
      FPGAPsddNode *cur_node = *node_it;
      // Initialize the context value
      BatchedPsddValue initial_context_values(data_size, false);
      cur_node->SetBatchedPsddContextValue(std::move(initial_context_values));
      if (cur_node->node_type() == LITERAL_NODE_TYPE) {
        FPGAPsddNode *cur_literal_node = cur_node->psdd_literal_node();
        auto example_it = examples.find(cur_literal_node->variable_index());
        assert(example_it != examples.end());
        BatchedPsddValue cur_batched_value = example_it->second;
        assert(cur_batched_value.size() == data_size);
        if (!cur_literal_node->sign()) {
          auto data_size = cur_batched_value.size();
          for (size_t i = 0; i < data_size; ++i) {
            cur_batched_value[i] = !cur_batched_value[i];
          }
        }
        cur_literal_node->SetBatchedPsddValue(std::move(cur_batched_value));
        continue;
      }
      if (cur_node->node_type() == TOP_NODE_TYPE) {
        BatchedPsddValue cur_batched_value(data_size, true);
        cur_node->SetBatchedPsddValue(std::move(cur_batched_value));
        continue;
      }
      if (cur_node->node_type() == DECISION_NODE_TYPE) {
        FPGAPsddNode *cur_decn_node = cur_node->psdd_decision_node();
        const auto &cur_primes = cur_decn_node->primes();
        const auto &cur_subs = cur_decn_node->subs();
        size_t element_size = cur_primes.size();
        BatchedPsddValue cur_batched_value(data_size, false);
        for (size_t i = 0; i < element_size; ++i) {
          const auto &cur_prime_value = cur_primes[i]->batched_psdd_value();
          const auto &cur_sub_value = cur_subs[i]->batched_psdd_value();
          for (size_t cur_data_index = 0; cur_data_index < data_size;
               ++cur_data_index) {
            if (cur_prime_value[cur_data_index] &&
                cur_sub_value[cur_data_index]) {
              cur_batched_value[cur_data_index] = true;
            }
          }
        }
        cur_decn_node->SetBatchedPsddValue(std::move(cur_batched_value));
        continue;
      }
    }
    // Initialize the context value for the root node.
    target_structure->MutableBatchedPsddContextValue()->flip();
    for (FPGAPsddNode *cur_node : serialized_psdd_nodes) {
      const auto &cur_contexts = cur_node->batched_psdd_context_value();
      if (cur_node->node_type() == TOP_NODE_TYPE) {
        FPGAPsddNode *cur_top_node = cur_node->psdd_top_node();
        int32_t variable_index = cur_top_node->variable_index();
        auto example_it = examples.find(variable_index);
        assert(example_it != examples.end());
        uintmax_t pos_data_count = 0;
        uintmax_t neg_data_count = 0;
        for (size_t i = 0; i < data_size; ++i) {
          if (cur_contexts[i]) {
            if (example_it->second[i]) {
              ++pos_data_count;
            } else {
              ++neg_data_count;
            }
          }
        }
        cur_top_node->IncrementTrueDataCount(pos_data_count);
        cur_top_node->IncrementFalseDataCount(neg_data_count);
      } else if (cur_node->node_type() == DECISION_NODE_TYPE) {
        FPGAPsddNode *cur_decn_node = cur_node->psdd_decision_node();
        const auto &primes = cur_decn_node->primes();
        const auto &subs = cur_decn_node->subs();
        size_t element_size = primes.size();
        for (size_t i = 0; i < element_size; ++i) {
          const auto &cur_prime_value = primes[i]->batched_psdd_value();
          const auto &cur_sub_value = subs[i]->batched_psdd_value();
          for (size_t j = 0; j < data_size; ++j) {
            if (cur_contexts[j] && cur_prime_value[j] && cur_sub_value[j]) {
              cur_decn_node->IncrementDataCount(i, 1);
              primes[i]->MutableBatchedPsddContextValue()->at(j) = true;
              subs[i]->MutableBatchedPsddContextValue()->at(j) = true;
            }
          }
        }
      }
      cur_node->MutableBatchedPsddContextValue()->clear();
      cur_node->MutableBatchedPsddValue()->clear();
    }
  }
  // Calculate local probability
  for (auto it = serialized_psdd_nodes.rbegin();
       it != serialized_psdd_nodes.rend(); ++it) {
    FPGAPsddNode *cur_node = *it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      FPGAPsddNode *cur_lit_node = cur_node->psdd_literal_node();
      FPGAPsddNode *new_node =
          GetFPGAPsddLiteralNode(cur_lit_node->literal(), flag_index);
      cur_lit_node->SetUserData((uintmax_t)new_node);
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      FPGAPsddNode *cur_top_node = cur_node->psdd_top_node();
      // Calculates laplacian smoothed data counts
      const auto true_data_count =
          PsddParameter::CreateFromDecimal(
              static_cast<double>(cur_top_node->true_data_count())) +
          alpha;
      const auto false_data_count =
          PsddParameter::CreateFromDecimal(
              static_cast<double>(cur_top_node->false_data_count())) +
          alpha;
      const auto total_data_count = true_data_count + false_data_count;
      FPGAPsddNode *new_top_node =
          GetFPGAPsddTopNode(cur_top_node->variable_index(), flag_index,
                         true_data_count / total_data_count,
                         false_data_count / total_data_count);
      cur_top_node->SetUserData((uintmax_t)new_top_node);
    } else {
      assert(cur_node->node_type() == DECISION_NODE_TYPE);
      FPGAPsddNode *cur_decision_node = cur_node->psdd_decision_node();
      std::vector<PsddParameter> parameters;
      const auto &primes = cur_decision_node->primes();
      const auto &subs = cur_decision_node->subs();
      size_t element_size = primes.size();
      PsddParameter total_data_counts = PsddParameter::CreateFromDecimal(0);
      const auto &data_counts = cur_decision_node->data_counts();
      std::vector<FPGAPsddNode *> new_primes;
      std::vector<FPGAPsddNode *> new_subs;
      for (size_t i = 0; i < element_size; ++i) {
        parameters.push_back(PsddParameter::CreateFromDecimal(
                                 static_cast<double>(data_counts[i])) +
                             alpha);
        total_data_counts = total_data_counts + parameters.back();
        new_primes.push_back((FPGAPsddNode *)primes[i]->user_data());
        new_subs.push_back((FPGAPsddNode *)subs[i]->user_data());
      }
      for (size_t i = 0; i < element_size; ++i) {
        parameters[i] = parameters[i] / total_data_counts;
      }
      FPGAPsddNode *new_decn_node = GetConformedFPGAPsddDecisionNode(
          new_primes, new_subs, parameters, flag_index);
      cur_decision_node->SetUserData((uintmax_t)new_decn_node);
    }
  }
  FPGAPsddNode *result_node = (FPGAPsddNode *)target_structure->user_data();
  // Clear learning states
  for (FPGAPsddNode *cur_node : serialized_psdd_nodes) {
    cur_node->ResetDataCount();
    cur_node->SetUserData(0);
  }
  return result_node;
}
