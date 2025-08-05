#include <Regex/NFA.h>

std::shared_ptr<NFA::DFAState> NFA::Iterator::next_on(char32_t c) {
  if (!current_state) {
    return nullptr;
  }
  auto prev_state = current_state;

  // check if this state already computed the transition for this character
  if (current_state->transitions.find(c) != current_state->transitions.end()) {
    current_state = current_state->transitions[c];
    return prev_state;
  }

  // build the next state
  auto next_state = std::make_shared<NFA::DFAState>();
  // transition on c from each NFA state in current_state, collecting all
  // epsilon closures
  for (const auto &state : current_state->states) {
    if (state->transitions_on(c)) {
      auto next_closure = epsilon_closure(state->out1);
      next_state->states.insert(next_closure.begin(), next_closure.end());
    }
  }
  // if next_state has no states, it is a reject state
  if (next_state->states.empty()) {
    current_state = nullptr;
    return prev_state;
  }
  if (dfa_states.find(next_state) == dfa_states.end()) {
    // if this state not in dfa_states, insert it
    dfa_states.insert(next_state);
  } else {
    // if it is already in dfa_states, use that pointer instead
    next_state = *dfa_states.find(next_state);
  }
  current_state->transitions[c] = next_state;
  current_state = next_state;

  if (dfa_states.size() > dfa_threshold) {
    flush_dfa_states(prev_state);
  }

  return prev_state;
}
bool NFA::Iterator::is_accept() const {
  if (is_reject()) {
    return false;
  }
  for (const auto &state : current_state->states) {
    if (nfa.accept_states.end() !=
        std::find(nfa.accept_states.begin(), nfa.accept_states.end(), state)) {
      return true;
    }
  }
  return false;
}

std::set<std::shared_ptr<NFA::State>>
NFA::Iterator::epsilon_closure(const std::shared_ptr<NFA::State> &state) {
  // if the closure was already computed, return it
  if (epsilon_closures.find(state) != epsilon_closures.end()) {
    return epsilon_closures.at(state);
  }

  // set of nfa states reachable from this state using epsilon transitions
  std::set<std::shared_ptr<NFA::State>> closure = {state};
  if (state->out1 && state->transitions_on(0)) {
    auto out1_closure = epsilon_closure(state->out1);
    closure.insert(out1_closure.begin(), out1_closure.end());
  }
  if (state->out2 && state->transitions_on(0)) {
    auto out2_closure = epsilon_closure(state->out2);
    closure.insert(out2_closure.begin(), out2_closure.end());
  }

  // store and return
  epsilon_closures[state] = closure;
  return closure;
}

std::shared_ptr<NFA::DFAState> NFA::Iterator::start_dfa_state() {
  auto dfa_start = std::make_shared<DFAState>(epsilon_closure(nfa.start_state));
  if (dfa_states.find(dfa_start) == dfa_states.end()) {
    dfa_states.insert(dfa_start);
    return dfa_start;
  } else {
    return *dfa_states.find(dfa_start);
  }
}

void NFA::Iterator::flush_dfa_states(const std::shared_ptr<DFAState> &keep) {
  return;
  std::cout << "Flushing DFA" << std::endl;
  for (auto it = dfa_states.begin(); it != dfa_states.end();) {
    if (*it == keep || *it == current_state) {
      (*it)->transitions.clear(); // transitions invalidated
      it++;
    } else {
      it = dfa_states.erase(it);
    }
  }
}
