//
// Created by Yujia Shen on 10/19/17.
//

#include <cstddef>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
#include <gmp.h>
#include <iostream>
#include <psdd/fpga_psdd_node.h>
#include <psdd/psdd_node.h>
#include <queue>
#include <random>
#include <stack>
#include <unordered_set>

namespace {
Vtree *
SubVtreeByVariablesHelper(Vtree *root,
                          const std::unordered_set<SddLiteral> &variables) {
  if (sdd_vtree_is_leaf(root)) {
    SddLiteral vtree_var = sdd_vtree_var(root);
    auto vtree_var_it = variables.find(vtree_var);
    if (vtree_var_it != variables.end()) {
      return root;
    } else {
      return nullptr;
    }
  } else {
    Vtree *left_child_result =
        SubVtreeByVariablesHelper(sdd_vtree_left(root), variables);
    Vtree *right_child_result =
        SubVtreeByVariablesHelper(sdd_vtree_right(root), variables);
    if (left_child_result == nullptr) {
      return right_child_result;
    } else if (right_child_result == nullptr) {
      return left_child_result;
    } else {
      if (left_child_result == sdd_vtree_left(root) &&
          right_child_result == sdd_vtree_right(root)) {
        return root;
      } else {
        return nullptr; // error. variables does not form a sub vtree.
      }
    }
  }
}

void LeftToRightLeafTraverseHelper(std::vector<SddLiteral> *literal_vector,
                                   Vtree *cur_root) {
  if (sdd_vtree_is_leaf(cur_root)) {
    literal_vector->push_back(sdd_vtree_var(cur_root));
  } else {
    LeftToRightLeafTraverseHelper(literal_vector, sdd_vtree_left(cur_root));
    LeftToRightLeafTraverseHelper(literal_vector, sdd_vtree_right(cur_root));
  }
}
} // namespace

namespace fpga_vtree_util {
std::vector<Vtree *> SerializeVtree(Vtree *root) {
  std::vector<Vtree *> serialized_vtree;
  std::queue<Vtree *> vtree_queue;
  vtree_queue.push(root);
  while (!vtree_queue.empty()) {
    Vtree *front_node = vtree_queue.front();
    vtree_queue.pop();
    serialized_vtree.push_back(front_node);
    if (!sdd_vtree_is_leaf(front_node)) {
      vtree_queue.push(sdd_vtree_left(front_node));
      vtree_queue.push(sdd_vtree_right(front_node));
    }
  }
  return serialized_vtree;
}
Vtree *CopyVtree(Vtree *root) {
  std::vector<Vtree *> orig_vtrees = SerializeVtree(root);
  auto orig_vtree_size = orig_vtrees.size();
  for (int64_t i = (int64_t)orig_vtree_size - 1; i >= 0; --i) {
    Vtree *orig_vtree = orig_vtrees[i];
    Vtree *new_node = nullptr;
    if (sdd_vtree_is_leaf(orig_vtree)) {
      new_node = new_leaf_vtree(sdd_vtree_var(orig_vtree));
    } else {
      Vtree *orig_left = sdd_vtree_left(orig_vtree);
      Vtree *orig_right = sdd_vtree_right(orig_vtree);
      new_node = new_internal_vtree((Vtree *)sdd_vtree_data(orig_left),
                                    (Vtree *)sdd_vtree_data(orig_right));
      sdd_vtree_set_data(nullptr, orig_left);
      sdd_vtree_set_data(nullptr, orig_right);
    }
    sdd_vtree_set_data((void *)new_node, orig_vtree);
  }
  auto new_vtree = (Vtree *)sdd_vtree_data(root);
  sdd_vtree_set_data(nullptr, root);
  set_vtree_properties(new_vtree);
  return new_vtree;
}
std::vector<SddLiteral> VariablesUnderVtree(Vtree *root) {
  std::vector<SddLiteral> variables;
  std::vector<Vtree *> vtree_serialized = SerializeVtree(root);
  for (Vtree *cur_vtree : vtree_serialized) {
    if (sdd_vtree_is_leaf(cur_vtree)) {
      variables.push_back(sdd_vtree_var(cur_vtree));
    }
  }
  return variables;
}
Vtree *ProjectVtree(Vtree *orig_vtree,
                    const std::vector<SddLiteral> &variables) {
  std::unordered_set<SddLiteral> variable_set;
  for (SddLiteral variable_index : variables) {
    variable_set.insert(variable_index);
  }
  std::vector<Vtree *> serialized_vtrees = SerializeVtree(orig_vtree);
  for (auto vit = serialized_vtrees.rbegin(); vit != serialized_vtrees.rend();
       ++vit) {
    Vtree *cur_vtree_node = *vit;
    if (sdd_vtree_is_leaf(cur_vtree_node)) {
      SddLiteral cur_variable_index = sdd_vtree_var(cur_vtree_node);
      if (variable_set.find(cur_variable_index) != variable_set.end()) {
        Vtree *new_vtree_node = new_leaf_vtree(cur_variable_index);
        sdd_vtree_set_data((void *)new_vtree_node, cur_vtree_node);
      } else {
        sdd_vtree_set_data(nullptr, cur_vtree_node);
      }
    } else {
      Vtree *left_child = sdd_vtree_left(cur_vtree_node);
      Vtree *right_child = sdd_vtree_right(cur_vtree_node);
      auto new_left_child = (Vtree *)sdd_vtree_data(left_child);
      sdd_vtree_set_data(nullptr, left_child);
      auto new_right_child = (Vtree *)sdd_vtree_data(right_child);
      sdd_vtree_set_data(nullptr, right_child);
      if (new_left_child && new_right_child) {
        Vtree *new_vtree_node =
            new_internal_vtree(new_left_child, new_right_child);
        sdd_vtree_set_data((void *)new_vtree_node, cur_vtree_node);
      } else if (new_left_child || new_right_child) {
        Vtree *new_vtree_node =
            new_left_child != nullptr ? new_left_child : new_right_child;
        sdd_vtree_set_data((void *)new_vtree_node, cur_vtree_node);
      } else {
        sdd_vtree_set_data(nullptr, cur_vtree_node);
      }
    }
  }
  auto new_vtree_root = (Vtree *)sdd_vtree_data(orig_vtree);
  sdd_vtree_set_data(nullptr, orig_vtree);
  set_vtree_properties(new_vtree_root);
  return new_vtree_root;
}
Vtree *
CopyVtree(Vtree *root,
          const std::unordered_map<SddLiteral, SddLiteral> &variable_map) {
  std::vector<Vtree *> orig_vtrees = SerializeVtree(root);
  auto orig_vtree_size = orig_vtrees.size();
  for (int64_t i = (int64_t)orig_vtree_size - 1; i >= 0; --i) {
    Vtree *orig_vtree = orig_vtrees[i];
    Vtree *new_node = nullptr;
    if (sdd_vtree_is_leaf(orig_vtree)) {
      assert(variable_map.find(sdd_vtree_var(orig_vtree)) !=
             variable_map.end());
      new_node =
          new_leaf_vtree(variable_map.find(sdd_vtree_var(orig_vtree))->second);
    } else {
      Vtree *orig_left = sdd_vtree_left(orig_vtree);
      Vtree *orig_right = sdd_vtree_right(orig_vtree);
      new_node = new_internal_vtree((Vtree *)sdd_vtree_data(orig_left),
                                    (Vtree *)sdd_vtree_data(orig_right));
      sdd_vtree_set_data(nullptr, orig_left);
      sdd_vtree_set_data(nullptr, orig_right);
    }
    sdd_vtree_set_data((void *)new_node, orig_vtree);
  }
  auto new_vtree = (Vtree *)sdd_vtree_data(root);
  sdd_vtree_set_data(nullptr, root);
  set_vtree_properties(new_vtree);
  return new_vtree;
}

Vtree *SubVtreeByVariables(Vtree *root,
                           const std::unordered_set<SddLiteral> &variables) {
  Vtree *result_candidate = SubVtreeByVariablesHelper(root, variables);
  if (result_candidate == nullptr) {
    return nullptr;
  }
  SddLiteral variable_size_under_candidate =
      sdd_vtree_var_count(result_candidate);
  if (variable_size_under_candidate !=
      static_cast<SddLiteral>(variables.size())) {
    return nullptr;
  } else {
    return result_candidate;
  }
}
std::vector<SddLiteral> LeftToRightLeafTraverse(Vtree *root) {
  std::vector<SddLiteral> result;
  LeftToRightLeafTraverseHelper(&result, root);
  return result;
}
} // namespace vtree_util
namespace fpga_psdd_node_util {

SddNode *ConvertPsddNodeToSddNode(
    const std::vector<FPGAPsddNode *> &serialized_psdd_nodes,
    const std::unordered_map<SddLiteral, SddLiteral> &variable_map,
    SddManager *sdd_manager) {
  for (auto node_it = serialized_psdd_nodes.rbegin();
       node_it != serialized_psdd_nodes.rend(); ++node_it) {
    FPGAPsddNode *cur_node = *node_it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      FPGAPsddNode *cur_literal = cur_node->psdd_literal_node();
      uint32_t psdd_variable_index = cur_literal->variable_index();
      assert(variable_map.find((SddLiteral)psdd_variable_index) !=
             variable_map.end());
      SddLiteral sdd_variable_index =
          variable_map.find((SddLiteral)psdd_variable_index)->second;
      SddLiteral sdd_literal = sdd_variable_index;
      if (!cur_literal->sign()) {
        sdd_literal = -sdd_literal;
      }
      SddNode *cur_lit = sdd_manager_literal(sdd_literal, sdd_manager);
      cur_node->SetUserData((uintmax_t)cur_lit);
    } else if (cur_node->node_type() == DECISION_NODE_TYPE) {
      FPGAPsddNode *cur_decn_node = cur_node->psdd_decision_node();
      const auto &decn_primes = cur_decn_node->primes();
      const auto &decn_subs = cur_decn_node->subs();
      auto element_size = decn_primes.size();
      SddNode *cur_logic = sdd_manager_false(sdd_manager);
      for (size_t i = 0; i < element_size; ++i) {
        FPGAPsddNode *cur_prime = decn_primes[i];
        FPGAPsddNode *cur_sub = decn_subs[i];
        SddNode *cur_partition =
            sdd_conjoin((SddNode *)cur_prime->user_data(),
                        (SddNode *)cur_sub->user_data(), sdd_manager);
        cur_logic = sdd_disjoin(cur_logic, cur_partition, sdd_manager);
      }
      cur_node->SetUserData((uintmax_t)cur_logic);
    } else {
      assert(cur_node->node_type() == TOP_NODE_TYPE);
      cur_node->SetUserData((uintmax_t)sdd_manager_true(sdd_manager));
    }
  }
  auto root_logic = (SddNode *)serialized_psdd_nodes[0]->user_data();
  for (FPGAPsddNode *cur_node : serialized_psdd_nodes) {
    cur_node->SetUserData(0);
  }
  return root_logic;
}

// parents appear before children
std::vector<FPGAPsddNode *> SerializePsddNodes(FPGAPsddNode *root) {
  return SerializePsddNodes(std::vector<FPGAPsddNode *>({root}));
}

std::vector<FPGAPsddNode *>
SerializePsddNodes(const std::vector<FPGAPsddNode *> &root_nodes) {
  std::unordered_set<uintmax_t> node_explored;
  std::vector<FPGAPsddNode*> result;
  for (const auto cur_root_node : root_nodes) {
    if (node_explored.find(cur_root_node->node_index()) ==
        node_explored.end()) {
      result.push_back(cur_root_node);
      node_explored.insert(cur_root_node->node_index());
    }
  }
  uintmax_t explore_index = 0;
  while (explore_index != result.size()) {
    FPGAPsddNode *cur_psdd_node = result[explore_index];
    if (cur_psdd_node->node_type() == 2) {
      // auto cur_decn_node = static_cast<PsddDecisionNode *>(cur_psdd_node);
      const std::vector<FPGAPsddNode *> &primes = cur_psdd_node->primes();
      const std::vector<FPGAPsddNode *> &subs = cur_psdd_node->subs();
      for (const auto cur_prime : primes) {
        if (node_explored.find(cur_prime->node_index()) ==
            node_explored.end()) {
          node_explored.insert(cur_prime->node_index());
          result.push_back(cur_prime);
        }
      }
      for (const auto cur_sub : subs) {
        if (node_explored.find(cur_sub->node_index()) == node_explored.end()) {
          node_explored.insert(cur_sub->node_index());
          result.push_back(cur_sub);
        }
      }
    }
    ++explore_index;
  }
  return result;
}

std::vector<uint32_t> SerializePsddNodesEvaluate(uint32_t root_node,  std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>> &fpga_node_vector
                                                ,std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector,
                                              std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector) {
  return SerializePsddNodesEvaluate(std::vector<uint32_t>({root_node}), fpga_node_vector, prime_vector, sub_vector);
}

std::vector<uint32_t> SerializePsddNodesEvaluate(const std::vector<uint32_t> &root_nodes, std::vector<PsddNodeStruct,aligned_allocator<PsddNodeStruct>> &fpga_node_vector
                                                ,std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &prime_vector,
                                              std::vector<ap_uint<32>,aligned_allocator<ap_uint<32>>> &sub_vector) {
  std::unordered_set<uintmax_t> node_explored;
  std::vector<uint32_t> result;
  for (int i = 0 ; i < root_nodes.size() ; i++ ) {
    uint32_t cur_root_node_idx = root_nodes[i];
    if (node_explored.find(cur_root_node_idx) ==
        node_explored.end()) {
      result.push_back(cur_root_node_idx);
      node_explored.insert(cur_root_node_idx);
    }
  }
  uintmax_t explore_index = 0;
  while (explore_index != result.size()) {
    uint32_t cur_psdd_node_idx = result[explore_index];
    if (fpga_node_vector[cur_psdd_node_idx].node_type_ == 2) {
      for (int i = 0 ; i < fpga_node_vector[cur_psdd_node_idx].children_size; i++ ) {
        uint32_t cur_prime_idx = prime_vector[i + fpga_node_vector[cur_psdd_node_idx].children_offset];
        if (node_explored.find(cur_prime_idx) ==
            node_explored.end()) {
          node_explored.insert(cur_prime_idx);
          result.push_back(cur_prime_idx);
        }
      }
      for (int i = 0 ; i < fpga_node_vector[cur_psdd_node_idx].children_size ; i++ ) {
        uint32_t cur_sub_idx = sub_vector[i + fpga_node_vector[cur_psdd_node_idx].children_offset];
        if (node_explored.find(cur_sub_idx) == node_explored.end()) {
          node_explored.insert(cur_sub_idx);
          result.push_back(cur_sub_idx);
        }
      }
    }
    ++explore_index;
  }
  return result;

 }


std::unordered_map<uintmax_t, FPGAPsddNode *>
GetCoveredPsddNodes(const std::vector<FPGAPsddNode *> &root_nodes) {
  std::unordered_map<uintmax_t, FPGAPsddNode *> covered_nodes;
  std::queue<FPGAPsddNode *> front_nodes;
  for (const auto cur_root_node : root_nodes) {
    if (covered_nodes.find(cur_root_node->node_index()) ==
        covered_nodes.end()) {
      front_nodes.push(cur_root_node);
      covered_nodes[cur_root_node->node_index()] = cur_root_node;
    }
  }
  while (!front_nodes.empty()) {
    FPGAPsddNode *cur_psdd_node = front_nodes.front();
    front_nodes.pop();
    if (cur_psdd_node->node_type() == 2) {
      auto cur_decn_node = cur_psdd_node;
      const std::vector<FPGAPsddNode *> &primes = cur_decn_node->primes();
      const std::vector<FPGAPsddNode *> &subs = cur_decn_node->subs();
      for (const auto cur_prime : primes) {
        if (covered_nodes.find(cur_prime->node_index()) ==
            covered_nodes.end()) {
          covered_nodes[cur_prime->node_index()] = cur_prime;
          front_nodes.push(cur_prime);
        }
      }
      for (const auto cur_sub : subs) {
        if (covered_nodes.find(cur_sub->node_index()) == covered_nodes.end()) {
          covered_nodes[cur_sub->node_index()] = cur_sub;
          front_nodes.push(cur_sub);
        }
      }
    }
  }
  return covered_nodes;
}
void SetActivationFlag(const std::bitset<MAX_VAR> &evidence,
                       const std::vector<FPGAPsddNode *> &serialized_psdd_nodes) {
  for (auto node_it = serialized_psdd_nodes.rbegin();
       node_it != serialized_psdd_nodes.rend(); ++node_it) {
    FPGAPsddNode *cur_node = *node_it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      // literal
      auto cur_literal_node = cur_node->psdd_literal_node();
      if (evidence[cur_literal_node->variable_index()] ==
          cur_literal_node->sign()) {
        cur_literal_node->SetActivationFlag();
      }
    } else if (cur_node->node_type() == DECISION_NODE_TYPE) {
      auto cur_decn_node = cur_node->psdd_decision_node();
      const auto &cur_primes = cur_decn_node->primes();
      const auto &cur_subs = cur_decn_node->subs();
      assert(cur_primes.size() == cur_subs.size());
      auto element_size = cur_primes.size();
      for (size_t k = 0; k < element_size; ++k) {
        if (cur_primes[k]->activation_flag() &&
            cur_subs[k]->activation_flag()) {
          cur_decn_node->SetActivationFlag();
          break;
        }
      }
    } else {
      // cur_node->node_type() == 3 (TOP node)
      cur_node->SetActivationFlag();
    }
  }
}

std::pair<std::bitset<MAX_VAR>, Probability>
GetMPESolution(const std::vector<FPGAPsddNode *> &serialized_psdd_nodes) {
  for (auto it = serialized_psdd_nodes.rbegin();
       it != serialized_psdd_nodes.rend(); ++it) {
    FPGAPsddNode *cur_node = *it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      auto *cache_pair = new std::pair<PsddParameter, uintmax_t>(
          PsddParameter::CreateFromDecimal(1.0), 0);
      cur_node->SetUserData((uintmax_t)cache_pair);
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      // PsddGopNode *cur_top_node = cur_node->top_node();
      Probability cur_value;
      std::pair<PsddParameter, uintmax_t> *cache_pair;
      if (cur_node->true_parameter() > cur_node->false_parameter()) {
        cache_pair = new std::pair<PsddParameter, uintmax_t>(
            cur_node->true_parameter(), 1);
      } else {
        cache_pair = new std::pair<PsddParameter, uintmax_t>(
            cur_node->false_parameter(), 0);
      }
      cur_node->SetUserData((uintmax_t)cache_pair);
    } else {
      assert(cur_node->node_type() == DECISION_NODE_TYPE);
      // PsddDecisionNode *cur_decn_node = cur_node->psdd_decision_node();
      Probability max_product = Probability::CreateFromDecimal(0);
      uintmax_t max_index = 0;
      const auto &cur_primes = cur_node->primes();
      const auto &cur_subs = cur_node->subs();
      const auto &cur_params = cur_node->parameters();
      uintmax_t element_size = cur_node->primes().size();
      for (uintmax_t i = 0; i < element_size; ++i) {
        FPGAPsddNode *cur_prime_node = cur_primes[i];
        FPGAPsddNode *cur_sub_node = cur_subs[i];
        PsddParameter cur_parameter = cur_params[i];
        auto cur_prime_comp_cache =
            (std::pair<PsddParameter, uintmax_t> *)cur_prime_node->user_data();
        auto cur_sub_comp_cache =
            (std::pair<PsddParameter, uintmax_t> *)cur_sub_node->user_data();
        Probability cur_product = cur_prime_comp_cache->first *
                                  cur_sub_comp_cache->first * cur_parameter;
        if (cur_product > max_product) {
          max_product = cur_product;
          max_index = i;
        }
      }
      auto *cache_pair =
          new std::pair<PsddParameter, uintmax_t>(max_product, max_index);
      cur_node->SetUserData((uintmax_t)cache_pair);
    }
  }
  // Extract solution;
  std::queue<FPGAPsddNode *> node_queue;
  node_queue.push(serialized_psdd_nodes[0]);
  std::bitset<MAX_VAR> max_instantiation;
  auto *cache_pair =
      (std::pair<PsddParameter, uintmax_t> *)serialized_psdd_nodes[0]
          ->user_data();
  auto max_prob = cache_pair->first;
  while (!node_queue.empty()) {
    FPGAPsddNode *cur_node = node_queue.front();
    node_queue.pop();
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      // PsddLiteralNode *cur_literal_node = cur_node->psdd_literal_node();
      if (cur_node->sign()) {
        max_instantiation.set(cur_node->variable_index());
      }
    } else if (cur_node->node_type() == DECISION_NODE_TYPE) {
      // PsddDecisionNode *cur_decn_node = cur_node->psdd_decision_node();
      cache_pair = (std::pair<PsddParameter, uintmax_t> *)cur_node->user_data();
      node_queue.push(cur_node->primes()[cache_pair->second]);
      node_queue.push(cur_node->subs()[cache_pair->second]);
    } else {
      assert(cur_node->node_type() == TOP_NODE_TYPE);
      // PsddTopNode *cur_top_node = cur_node->psdd_top_node();
      cache_pair =
          (std::pair<PsddParameter, uintmax_t> *)cur_node->user_data();
      if (cache_pair->second) {
        max_instantiation.set(cur_node->variable_index());
      }
    }
  }
  for (FPGAPsddNode *cur_node : serialized_psdd_nodes) {
    cache_pair = (std::pair<PsddParameter, uintmax_t> *)cur_node->user_data();
    delete (cache_pair);
    cur_node->SetUserData(0);
  }
  return {max_instantiation, max_prob};
}

std::pair<std::bitset<MAX_VAR>, Probability>
GetMPESolution(FPGAPsddNode *psdd_node) {
  auto serialized_psdd_nodes = fpga_psdd_node_util::SerializePsddNodes(psdd_node);
  return GetMPESolution(serialized_psdd_nodes);
}
mpz_class ModelCount(const std::vector<FPGAPsddNode *> &serialized_nodes) {
  std::unordered_map<uintmax_t, mpz_class> count_cache;
  for (auto node_it = serialized_nodes.rbegin();
       node_it != serialized_nodes.rend(); ++node_it) {
    FPGAPsddNode *cur_node = *node_it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      count_cache[cur_node->node_index()] = 1;
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      count_cache[cur_node->node_index()] = 2;
    } else {
      assert(cur_node->node_type() == DECISION_NODE_TYPE);
      FPGAPsddNode *cur_decn_node = cur_node->psdd_decision_node();
      const auto &primes = cur_decn_node->primes();
      const auto &subs = cur_decn_node->subs();
      auto element_size = primes.size();
      mpz_class total_count = 0;
      for (size_t i = 0; i < element_size; ++i) {
        FPGAPsddNode *cur_prime = primes[i];
        FPGAPsddNode *cur_sub = subs[i];
        const mpz_class &a = count_cache[cur_prime->node_index()];
        const mpz_class &b = count_cache[cur_sub->node_index()];
        mpz_class product = 0;
        mpz_mul(product.get_mpz_t(), a.get_mpz_t(), b.get_mpz_t());
        mpz_add(total_count.get_mpz_t(), total_count.get_mpz_t(),
                product.get_mpz_t());
      }
      count_cache[cur_node->node_index()] = total_count;
    }
  }
  return count_cache[serialized_nodes[0]->node_index()];
}
Probability Evaluate(const std::bitset<MAX_VAR> &variables,
                     const std::bitset<MAX_VAR> &instantiation,
                     const std::vector<FPGAPsddNode *> &serialized_nodes) {
  std::unordered_map<uintmax_t, Probability> evaluation_cache;
  bool first = true;
  for (auto node_it = serialized_nodes.rbegin();
       node_it != serialized_nodes.rend(); ++node_it) {
    FPGAPsddNode *cur_node = *node_it;
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      // PsddLiteralNode *cur_lit = cur_node->psdd_literal_node();
      if (variables[cur_node->variable_index()]) {
        if (instantiation[cur_node->variable_index()] == cur_node->sign()) {
          evaluation_cache[cur_node->node_index()] =
              Probability::CreateFromDecimal(1);
        } else {
          evaluation_cache[cur_node->node_index()] =
              Probability::CreateFromDecimal(0);
        }
      } else {
        evaluation_cache[cur_node->node_index()] =
            Probability::CreateFromDecimal(1);
      }
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      // PsddTopNode *cur_top = cur_node->psdd_top_node();
      if (variables[cur_node->variable_index()]) {
        if (instantiation[cur_node->variable_index()]) {
          evaluation_cache[cur_node->node_index()] = cur_node->true_parameter();
        } else {
          evaluation_cache[cur_node->node_index()] = cur_node->false_parameter();
        }
      } else {
        evaluation_cache[cur_node->node_index()] =
            Probability::CreateFromDecimal(1);
      }
    } else {
      auto element_size = cur_node->primes().size();
      Probability cur_prob = Probability::CreateFromDecimal(0);
      for (size_t i = 0; i < element_size; ++i) {
        FPGAPsddNode *cur_prime = cur_node->primes()[i];
        FPGAPsddNode *cur_sub = cur_node->subs()[i];
        cur_prob = cur_prob + evaluation_cache[cur_prime->node_index()] *
                                  evaluation_cache[cur_sub->node_index()] *
                                  cur_node->parameters()[i];
      }
      evaluation_cache[cur_node->node_index()] = cur_prob;
    }
  }
  return evaluation_cache[serialized_nodes[0]->node_index()];
}

Probability Evaluate(const std::bitset<MAX_VAR> &variables,
                     const std::bitset<MAX_VAR> &instantiation,
                     FPGAPsddNode *root_node) {
  std::vector<FPGAPsddNode *> serialized_nodes = SerializePsddNodes(root_node);
  return Evaluate(variables, instantiation, serialized_nodes);
}

void WritePsddToFile(FPGAPsddNode *root_node, const char *output_filename) {
  auto serialized_psdds = SerializePsddNodes(root_node);
  std::string psdd_content =
      "c ids of psdd nodes start at 0\nc psdd nodes appear bottom-up, children "
      "before parents\nc file syntax:\nc psdd count-of-psdd-nodes\nc L "
      "id-of-literal-sdd-node id-of-vtree literal\nc T id-of-trueNode-sdd-node "
      "id-of-vtree variable log(neg_prob) log(pos_prob)\nc D "
      "id-of-decomposition-sdd-node id-of-vtree number-of-elements "
      "{id-of-prime id-of-sub log(elementProb)}*\nc\n";
  psdd_content += "psdd " + std::to_string(serialized_psdds.size()) + "\n";
  uintmax_t node_index = 0;
  for (auto it = serialized_psdds.rbegin(); it != serialized_psdds.rend();
       ++it) {
    FPGAPsddNode *cur = *it;
    if (cur->node_type() == LITERAL_NODE_TYPE) {
      // PsddLiteralNode *cur_literal = cur->psdd_literal_node();
      psdd_content +=
          "L " + std::to_string(node_index) + " " +
          std::to_string(sdd_vtree_position(cur->vtree_node())) + " " +
          std::to_string(cur->literal()) + "\n";
    } else if (cur->node_type() == TOP_NODE_TYPE) {
      // PsddTopNode *cur_top_node = cur->psdd_top_node();
      psdd_content +=
          "T " + std::to_string(node_index) + " " +
          std::to_string(sdd_vtree_position(cur->vtree_node())) + " " +
          std::to_string(cur->variable_index()) + " " +
          std::to_string(cur->false_parameter().parameter()) + " " +
          std::to_string(cur->true_parameter().parameter()) + "\n";
    } else {
      assert(cur->node_type() == DECISION_NODE_TYPE);
      // PsddDecisionNode *cur_decision_node = cur->psdd_decision_node();
      psdd_content +=
          "D " + std::to_string(node_index) + " " +
          std::to_string(sdd_vtree_position(cur->vtree_node())) +
          " " + std::to_string(cur->primes().size());
      const auto &primes = cur->primes();
      const auto &subs = cur->subs();
      const auto &params = cur->parameters();
      auto element_size = primes.size();
      for (size_t i = 0; i < element_size; ++i) {
        psdd_content += " " + std::to_string(primes[i]->user_data());
        psdd_content += " " + std::to_string(subs[i]->user_data());
        psdd_content += " " + std::to_string(params[i].parameter());
      }
      psdd_content += "\n";
    }
    cur->SetUserData(node_index);
    node_index += 1;
  }
  std::ofstream output_file;
  output_file.open(output_filename);
  output_file << psdd_content;
  output_file.close();
  for (FPGAPsddNode *cur_node : serialized_psdds) {
    cur_node->SetUserData(0);
  }
}
std::unordered_map<uint32_t, std::pair<Probability, Probability>>
GetMarginals(const std::vector<FPGAPsddNode *> &serialized_nodes) {
  // first is false second is true
  std::unordered_map<uint32_t, std::pair<Probability, Probability>> marginals;
  std::vector<Probability> derivatives(serialized_nodes.size(),
                                       Probability::CreateFromDecimal(0));
  auto index = 0;
  for (FPGAPsddNode *cur_node : serialized_nodes) {
    cur_node->SetUserData((uintmax_t)index);
    index++;
  }
  std::cout << "index size: " << index << std::endl;
  derivatives[0] = Probability::CreateFromDecimal(1);
  std::cout << "derivatives[0]: " << derivatives[0].parameter_ << std::endl;
  std::cout << "derivatives[1]: " << derivatives[1].parameter_ << std::endl;
  std::cout << "serialized_nodes.size(): " << serialized_nodes.size() << std::endl;

  for (FPGAPsddNode *cur_node : serialized_nodes) {
    if (cur_node->node_type() == LITERAL_NODE_TYPE) {
      auto cur_lit = cur_node->psdd_literal_node();
      auto marginal_it = marginals.find(cur_lit->variable_index());
      if (marginal_it == marginals.end()) {
        marginals[cur_lit->variable_index()] =
            std::make_pair(PsddParameter::CreateFromDecimal(0),
                           PsddParameter::CreateFromDecimal(0));
        marginal_it = marginals.find(cur_lit->variable_index());
      }
      if (cur_lit->sign()) {
        marginal_it->second.second =
            marginal_it->second.second + derivatives[cur_lit->user_data()];
      } else {
        marginal_it->second.first =
            marginal_it->second.first + derivatives[cur_lit->user_data()];
      }
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      auto cur_top = cur_node->psdd_top_node();
      auto marginal_it = marginals.find(cur_top->variable_index());
      if (marginal_it == marginals.end()) {
        marginals[cur_top->variable_index()] =
            std::make_pair(PsddParameter::CreateFromDecimal(0),
                           PsddParameter::CreateFromDecimal(0));
        marginal_it = marginals.find(cur_top->variable_index());
      }
      marginal_it->second.first =
          marginal_it->second.first +
          derivatives[cur_top->user_data()] * cur_top->false_parameter();
      marginal_it->second.second =
          marginal_it->second.second +
          derivatives[cur_top->user_data()] * cur_top->true_parameter();
    } else {
      auto cur_decn_node = cur_node->psdd_decision_node();
      const auto &primes = cur_decn_node->primes();
      const auto &subs = cur_decn_node->subs();
      const auto &params = cur_decn_node->parameters();
      Probability cur_derivative = derivatives[cur_decn_node->user_data()];
      auto element_size = primes.size();
      for (size_t i = 0; i < element_size; ++i) {
        derivatives[primes[i]->user_data()] =
            derivatives[primes[i]->user_data()] + cur_derivative * params[i];
        derivatives[subs[i]->user_data()] =
            derivatives[subs[i]->user_data()] + cur_derivative * params[i];
      }
    }
  }
  for (FPGAPsddNode *cur_node : serialized_nodes) {
    cur_node->SetUserData(0);
  }
  for (auto &cur_marginal : marginals) {
    Probability partition =
        cur_marginal.second.first + cur_marginal.second.second;
    cur_marginal.second.first = cur_marginal.second.first / partition;
    cur_marginal.second.second = cur_marginal.second.second / partition;
  }
  return marginals;
}
uintmax_t GetPsddSize(FPGAPsddNode *root_node) {
  uintmax_t psdd_size = 0;
  auto serialized_psdds = SerializePsddNodes(root_node);
  for (FPGAPsddNode *cur_node : serialized_psdds) {
    if (cur_node->node_type() == DECISION_NODE_TYPE) {
      auto cur_decision_node = cur_node->psdd_decision_node();
      psdd_size += cur_decision_node->primes().size() - 1;
    } else if (cur_node->node_type() == TOP_NODE_TYPE) {
      psdd_size += 1;
    }
  }
  return psdd_size;
}
} // namespace psdd_node_util
FPGAPsddNode::FPGAPsddNode(PsddLiteralNode * literal){
  node_type_ = 1;
  node_index_ = literal->node_index();
  vtree_node_ = literal->vtree_node();
  flag_index_ = literal->flag_index();
  activation_flag_ = literal->activation_flag();
  hash_value_ = literal->hash_value();
  user_data_ =literal->user_data();
  literal_ = literal->literal();
}
FPGAPsddNode::FPGAPsddNode(PsddDecisionNode * decision){
  node_type_ = 2;
  node_index_ = decision->node_index();
  vtree_node_ = decision->vtree_node();
  flag_index_ = decision->flag_index();
  activation_flag_ = decision->activation_flag();
  hash_value_ = decision->hash_value();
  parameters_ = decision->parameters();
  data_counts_ = decision->data_counts();
}
FPGAPsddNode::FPGAPsddNode(PsddTopNode * top){
  node_type_ = 3;
  node_index_ = top->node_index();
  vtree_node_ = top->vtree_node();
  flag_index_ = top->flag_index();
  activation_flag_ = top->activation_flag();
  hash_value_ = top->hash_value();
  user_data_ =top->user_data();
  variable_index_ = top->variable_index();
  true_parameter_ = top->true_parameter();
  false_parameter_ = top->false_parameter();
  false_data_count_ = top->false_data_count();
  true_data_count_ = top->true_data_count();
}

FPGAPsddNode::~FPGAPsddNode() {}

FPGAPsddNode::FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node)
    : FPGAPsddNode(node_index, vtree_node, 0) {}

FPGAPsddNode::FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node,
                   uintmax_t flag_index)
    : node_index_(node_index), vtree_node_(vtree_node), user_data_(0),
      flag_index_(flag_index), activation_flag_(false) {}

uintmax_t FPGAPsddNode::node_index() const { return node_index_; }

uintmax_t FPGAPsddNode::flag_index() const { return flag_index_; }

std::size_t FPGAPsddNode::hash_value() const { return hash_value_; }

void FPGAPsddNode::set_hash_value(std::size_t hash_value) {
  hash_value_ = hash_value;
}
Vtree *FPGAPsddNode::vtree_node() const { return vtree_node_; }

bool FPGAPsddNode::activation_flag() const { return activation_flag_; }

void FPGAPsddNode::SetActivationFlag() { activation_flag_ = true; }

void FPGAPsddNode::SetNodeType(int newNodeType) { node_type_ = newNodeType; }

void FPGAPsddNode::ResetActivationFlag() { activation_flag_ = false; }

uintmax_t FPGAPsddNode::user_data() const { return user_data_; }
void FPGAPsddNode::SetUserData(uintmax_t user_data) { user_data_ = user_data; }
//start of literal node
FPGAPsddNode::FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node,
                                 uintmax_t flag_index, int32_t literal)
    : node_index_(node_index), vtree_node_(vtree_node), user_data_(0),
      flag_index_(flag_index), activation_flag_(false),
       literal_(literal), node_type_(LITERAL_NODE_TYPE),
       variable_index_( literal > 0 ? static_cast<uint32_t>(literal)
                           : static_cast<uint32_t>(-literal)) {
  CalculateHashValue();
}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                                 uintmax_t flag_index, int32_t literal)
    : FPGAPsddNode(*node_index, vtree_node, flag_index, literal) {
  *node_index += 1;
}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                                 int32_t literal)
    : FPGAPsddNode(node_index, vtree_node, 0, literal) {}
//made more dynamic
bool FPGAPsddNode::operator==(const FPGAPsddNode &other) const {
  if (node_type_ == LITERAL_NODE_TYPE){
    return literal_ == other.literal() && flag_index() == other.flag_index();
  } else if (node_type_ == DECISION_NODE_TYPE){
      if (primes_.size() != other.primes_.size()) {
        return false;
      }
      if (flag_index() != other.flag_index()) {
        return false;
      }
      auto element_size = primes_.size();
      for (size_t i = 0; i < element_size; i++) {
        if (primes_[i]->node_index() != other.primes_[i]->node_index()) {
          return false;
        }
        if (subs_[i]->node_index() != other.subs_[i]->node_index()) {
          return false;
        }
        if (parameters_[i] != other.parameters_[i]) {
          return false;
        }
      }
      return true;
  } else if (node_type_ == TOP_NODE_TYPE) {
    return variable_index_ == other.variable_index_ &&
          flag_index() == other.flag_index() &&
          true_parameter_ == other.true_parameter_;
  }
}
//made more dynamic
int FPGAPsddNode::node_type() const { return node_type_; }

bool FPGAPsddNode::sign() const { if (node_type() == 1) return literal_ > 0; }

//made more dynamic
uint32_t FPGAPsddNode::variable_index() const {
  if (node_type_ == LITERAL_NODE_TYPE){
    return literal_ > 0 ? static_cast<uint32_t>(literal_)
                        : static_cast<uint32_t>(-literal_);
  } else if (node_type_ == TOP_NODE_TYPE){
    return variable_index_;
  }
}

int32_t FPGAPsddNode::literal() const { if (node_type() == 1) return literal_; }

//made more dynamic
void FPGAPsddNode::CalculateHashValue() {
  if (node_type_ == LITERAL_NODE_TYPE){
    std::size_t hash_value = std::hash<int32_t>{}(literal_);
    hash_value ^= (std::hash<uintmax_t>{}(flag_index()) << 1);
    set_hash_value(hash_value);
  } else if (node_type_ == DECISION_NODE_TYPE){
    std::size_t hash_value = std::hash<uintmax_t>{}(flag_index());
    auto element_size = primes_.size();
    for (size_t i = 0; i < element_size; i++) {
      hash_value ^= (std::hash<uintmax_t>{}(primes_[i]->node_index()) << i);
      hash_value ^= (std::hash<uintmax_t>{}(subs_[i]->node_index()) << i);
      hash_value ^= (parameters_[i].hash_value() << i);
    }
    set_hash_value(hash_value);
  } else if (node_type_ == TOP_NODE_TYPE){
      std::size_t hash_value = std::hash<uint32_t>{}(variable_index_);
      hash_value ^= true_parameter_.hash_value() << 1;
      hash_value ^= std::hash<uintmax_t>{}(flag_index()) << 2;
      set_hash_value(hash_value);
  }
}

//made more dynamic
void FPGAPsddNode::ResetDataCount() {
  if (node_type_ == DECISION_NODE_TYPE){
    auto element_size = primes_.size();
    for (size_t i = 0; i < element_size; ++i) {
      data_counts_[i] = 0;
    }
  } else if (node_type_ == TOP_NODE_TYPE){
      true_data_count_ = 0;
      false_data_count_ = 0;
  }
}

//made more dynamic
void FPGAPsddNode::DirectSample(
    std::bitset<MAX_VAR> *instantiation,
    RandomDoubleFromUniformGenerator *generator) {
  if (node_type_ == LITERAL_NODE_TYPE){
    if (literal_ > 0) {
      instantiation->set((size_t)literal_);
    }
  } else if (node_type_ == DECISION_NODE_TYPE){
      PsddParameter uniform_rand = PsddParameter::CreateFromDecimal(
        (generator->generate() - generator->min()) /
        (generator->max() - generator->min()));
    PsddParameter acc = PsddParameter::CreateFromDecimal(0);
    auto element_size = primes_.size();
    for (size_t i = 0; i < element_size; ++i) {
      PsddParameter cur_parameter = parameters_[i];
      acc = acc + cur_parameter;
      if (uniform_rand < acc) {
        // Use this partition
        primes_[i]->DirectSample(instantiation, generator);
        subs_[i]->DirectSample(instantiation, generator);
        return;
      }
    }
    assert(false);
  } else if (node_type_ == TOP_NODE_TYPE){
        PsddParameter uniform_rand = PsddParameter::CreateFromDecimal(
        (generator->generate() - generator->min()) /
        (generator->max() - generator->min()));
    if (uniform_rand < true_parameter_) {
      instantiation->set(variable_index_);
    }
  }
}
//start of decision
FPGAPsddNode::FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node,
                                   uintmax_t flag_index,
                                   const std::vector<FPGAPsddNode *> &primes,
                                   const std::vector<FPGAPsddNode *> &subs,
                                   const std::vector<PsddParameter> &parameters)
    : node_index_(node_index), vtree_node_(vtree_node), user_data_(0),
      flag_index_(flag_index), activation_flag_(false),
      data_counts_(primes.size(), 0), node_type_(DECISION_NODE_TYPE) {
  std::vector<std::pair<uintmax_t, uintmax_t>> indexes;
  for (const auto &cur_prime : primes) {
    indexes.emplace_back(
        std::make_pair(indexes.size(), cur_prime->node_index()));
  }
  std::sort(
      indexes.begin(), indexes.end(),
      [](const std::pair<uintmax_t, uintmax_t> &lhs, const std::pair<uintmax_t, uintmax_t> &rhs) { return lhs.second < rhs.second; });
  auto partition_size = primes.size();
  assert(partition_size == subs.size());
  primes_.resize(partition_size, nullptr);
  subs_.resize(partition_size, nullptr);
  parameters_.resize(partition_size);
  for (size_t i = 0; i < partition_size; i++) {
    primes_[i] = primes[indexes[i].first];
    subs_[i] = subs[indexes[i].first];
    if (!parameters.empty()) {
      parameters_[i] = parameters[indexes[i].first];
    }
  }
  CalculateHashValue();
}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                                   uintmax_t flag_index,
                                   const std::vector<FPGAPsddNode *> &primes,
                                   const std::vector<FPGAPsddNode *> &subs,
                                   const std::vector<PsddParameter> &parameters)
    : FPGAPsddNode(*node_index, vtree_node, flag_index, primes, subs,
                       parameters) {
  *node_index += 1;
}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                                   uintmax_t flag_index,
                                   const std::vector<FPGAPsddNode *> &primes,
                                   const std::vector<FPGAPsddNode *> &subs)
    : FPGAPsddNode(node_index, vtree_node, flag_index, primes, subs, {}) {}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                                   const std::vector<FPGAPsddNode *> &primes,
                                   const std::vector<FPGAPsddNode *> &subs)
    : FPGAPsddNode(node_index, vtree_node, 0, primes, subs) {}


const std::vector<FPGAPsddNode *> &FPGAPsddNode::primes() const {
  if (node_type_ == DECISION_NODE_TYPE)
    return primes_;
}

const std::vector<FPGAPsddNode *> &FPGAPsddNode::subs() const {
  if (node_type_ == DECISION_NODE_TYPE) return subs_;
  }

const std::vector<PsddParameter> &FPGAPsddNode::parameters() const {
  if (node_type_ == DECISION_NODE_TYPE)
    return parameters_;
}

void FPGAPsddNode::IncrementDataCount(uintmax_t index,
                                          uintmax_t increment_size) {
  if (node_type_ == DECISION_NODE_TYPE)
    data_counts_[index] += increment_size;
}

const std::vector<uintmax_t> &FPGAPsddNode::data_counts() const {
  if (node_type_ == DECISION_NODE_TYPE)
    return data_counts_;
}
//start of top node
FPGAPsddNode::FPGAPsddNode(uintmax_t node_index, Vtree *vtree_node,
                         uintmax_t flag_index, uint32_t variable_index,
                         PsddParameter true_parameter,
                         PsddParameter false_parameter)
    : node_index_(node_index), vtree_node_(vtree_node), user_data_(0),
      flag_index_(flag_index), activation_flag_(false),
      variable_index_(variable_index), true_parameter_(true_parameter),
      false_parameter_(false_parameter), true_data_count_(0),
      false_data_count_(0), node_type_(TOP_NODE_TYPE) {
  CalculateHashValue();
}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                         uintmax_t flag_index, uint32_t variable_index)
    : FPGAPsddNode(*node_index, vtree_node, flag_index, variable_index,
                  PsddParameter(), PsddParameter()) {
  *node_index += 1;
}

FPGAPsddNode::FPGAPsddNode(uintmax_t *node_index, Vtree *vtree_node,
                         uint32_t variable_index)
    : FPGAPsddNode(node_index, vtree_node, 0, variable_index) {}


void FPGAPsddNode::IncrementTrueDataCount(uintmax_t increment_size) {
  if (node_type_ == TOP_NODE_TYPE)
    true_data_count_ += increment_size;
}

void FPGAPsddNode::IncrementFalseDataCount(uintmax_t increment_size) {
  if (node_type_ == TOP_NODE_TYPE)
    false_data_count_ += increment_size;
}

PsddParameter FPGAPsddNode::true_parameter() const {
  if (node_type_ == TOP_NODE_TYPE)
  return true_parameter_;
}
PsddParameter FPGAPsddNode::false_parameter() const {
  if (node_type_ == TOP_NODE_TYPE)
  return false_parameter_;
}

uintmax_t FPGAPsddNode::true_data_count() const {
  if (node_type_ == TOP_NODE_TYPE)
  return true_data_count_;
}
uintmax_t FPGAPsddNode::false_data_count() const {
  if (node_type_ == TOP_NODE_TYPE)
  return false_data_count_;
}
