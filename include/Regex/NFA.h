#pragma once
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static size_t debug_id = 0;

class NFA {
public:
  NFA() : start_state(nullptr) {}

  struct State;
  struct LiteralState;
  struct CharClassState;
  struct EpsilonState;
  struct RejectState;

  class Iterator;

  explicit NFA(std::shared_ptr<State> start_state)
      : start_state(start_state), accept_states({start_state}) {}

  NFA &&apply_star() &&;
  NFA &&apply_plus() &&;
  NFA &&apply_optional() &&;
  NFA &&concat(NFA &&other) &&;
  NFA &&concat(std::shared_ptr<State> state) &&;
  NFA &&prepend(NFA &&other) &&;
  NFA &&prepend(std::shared_ptr<State> state) &&;
  NFA &&alternate(NFA &&other) &&;
  NFA &&apply_interval(std::tuple<size_t, size_t, bool> quants) &&;
  NFA &apply_star() &;
  NFA &apply_plus() &;
  NFA &apply_optional() &;
  NFA &concat(NFA &&other) &;
  NFA &concat(std::shared_ptr<State> state) &;
  NFA &prepend(std::shared_ptr<State> state) &;
  NFA &prepend(NFA &&other) &;
  NFA &alternate(NFA &&other) &;
  NFA &apply_interval(std::tuple<size_t, size_t, bool> quants) &;
  NFA clone() const;
  bool empty() const { return start_state == nullptr; }
  void print() const;

  static std::shared_ptr<State> make_literal_state(char32_t c);
  static std::shared_ptr<State>
  make_char_class_state(std::unordered_set<char32_t> &&char_class,
                        bool negated = false);
  static std::shared_ptr<State>
  make_char_class_state(const std::unordered_set<char32_t> &char_class,
                        bool negated = false);
  static std::shared_ptr<State> make_epsilon_state();
  static std::shared_ptr<State> make_reject_state();
  static std::shared_ptr<State> make_wildcard_state();
  static std::shared_ptr<State> make_space_state(bool negated);
  static std::shared_ptr<State> make_word_state(bool negated);
  static std::shared_ptr<State> make_digit_state(bool negated);

private:
  struct DFAState {
    explicit DFAState() {}
    explicit DFAState(std::set<std::shared_ptr<State>> &&states)
        : states(std::move(states)) {}
    std::set<std::shared_ptr<State>> states;
    std::unordered_map<char32_t, std::shared_ptr<DFAState>> transitions;
  };
  struct DFAHash {
    size_t operator()(const std::shared_ptr<DFAState> &state) const {
      size_t hash = 0;
      for (const auto &s : state->states) {
        hash ^= std::hash<State *>()(s.get());
      }
      return hash;
    }
  };

  std::shared_ptr<State> start_state;
  std::vector<std::shared_ptr<State>> accept_states;
};

struct NFA::State {
  explicit State() : did(debug_id++) {}
  std::shared_ptr<State> out1;
  std::shared_ptr<State> out2;
  size_t did;
  virtual bool transitions_on(char32_t c) const = 0;
  virtual std::shared_ptr<State> clone() const = 0;
  virtual void print() const = 0;
};

struct NFA::LiteralState : public NFA::State {
  char32_t literal;
  explicit LiteralState(char32_t c) : State(), literal(c) {}
  bool transitions_on(char32_t c) const override;
  std::shared_ptr<State> clone() const override;
  void print() const override;
};

struct NFA::CharClassState : public NFA::State {
  std::unordered_set<char32_t> char_class;
  bool negated;
  explicit CharClassState(std::unordered_set<char32_t> char_class, bool negated)
      : State(), char_class(char_class), negated(negated) {}
  bool transitions_on(char32_t c) const override;
  std::shared_ptr<State> clone() const override;
  void print() const override;
};

struct NFA::EpsilonState : public NFA::State {
  bool transitions_on(char32_t c) const override { return c == 0; }
  std::shared_ptr<State> clone() const override;
  void print() const override;
};

struct NFA::RejectState : public NFA::State {
  bool transitions_on(char32_t) const override { return false; }
  std::shared_ptr<State> clone() const override { return nullptr; }
  void print() const override;
};

class NFA::Iterator {
public:
  explicit Iterator(const NFA &nfa)
      : nfa(nfa), dfa_states(), epsilon_closures(),
        current_state(start_dfa_state()), dfa_threshold(5000) {}
  Iterator(const NFA &nfa, size_t dfa_threshold)
      : nfa(nfa), dfa_states(), epsilon_closures(),
        current_state(start_dfa_state()), dfa_threshold(dfa_threshold) {}
  std::shared_ptr<DFAState> next_on(char32_t c);
  void reset() { current_state = start_dfa_state(); }
  bool is_accept() const;
  bool is_reject() const { return current_state == nullptr; }

private:
  const NFA &nfa;
  std::unordered_set<std::shared_ptr<DFAState>, DFAHash> dfa_states;
  std::unordered_map<std::shared_ptr<State>, std::set<std::shared_ptr<State>>>
      epsilon_closures;
  std::shared_ptr<DFAState> current_state;
  size_t dfa_threshold;

  std::set<std::shared_ptr<State>>
  epsilon_closure(const std::shared_ptr<State> &state);
  std::shared_ptr<DFAState> start_dfa_state();
  void flush_dfa_states(const std::shared_ptr<DFAState> &state);
};
