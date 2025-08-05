#include <Regex/NFA.h>

NFA &&NFA::concat(NFA &&other) && {
  assert(this != &other);
  concat(std::move(other));
  return std::move(*this);
}

NFA &NFA::concat(NFA &&other) & {
  assert(this != &other);
  // other is empty
  if (!other.start_state)
    return *this;

  if (!start_state) {
    start_state = std::move(other.start_state);
  } else {
    for (auto &state : accept_states) {
      state->out1 = other.start_state;
    }
  }
  accept_states = std::move(other.accept_states);
  return *this;
}

NFA &&NFA::concat(std::shared_ptr<State> state) && {
  assert(state);
  concat(state);
  return std::move(*this);
}

NFA &NFA::concat(std::shared_ptr<State> state) & {
  assert(state);
  if (!start_state) {
    start_state = state;
  } else {
    for (auto &accept_state : accept_states) {
      accept_state->out1 = state;
    }
  }
  accept_states = {state};
  return *this;
}

NFA &&NFA::prepend(NFA &&other) && {
  assert(this != &other);
  prepend(std::move(other));
  return std::move(*this);
}

NFA &NFA::prepend(NFA &&other) & {
  assert(this != &other);
  // other is empty
  if (!other.start_state)
    return *this;

  if (!start_state) {
    accept_states = std::move(other.accept_states);
  } else {
    for (auto &state : other.accept_states) {
      state->out1 = start_state;
    }
  }
  start_state = std::move(other.start_state);
  return *this;
}

NFA &&NFA::prepend(std::shared_ptr<State> state) && {
  assert(state);
  prepend(state);
  return std::move(*this);
}

NFA &NFA::prepend(std::shared_ptr<State> state) & {
  assert(state);
  if (start_state) {
    accept_states = {state};
  } else {
    state->out1 = start_state;
  }
  start_state = state;
  return *this;
}

NFA &&NFA::apply_star() && {
  apply_star();
  return std::move(*this);
}

NFA &NFA::apply_star() & {
  auto state = make_epsilon_state();
  state->out2 = start_state;
  for (auto &accept_state : accept_states) {
    accept_state->out1 = state;
  }
  start_state = state;
  accept_states = {state};
  return *this;
};

NFA &&NFA::apply_plus() && {
  apply_plus();
  return std::move(*this);
}

NFA &NFA::apply_plus() & {
  auto state = make_epsilon_state();
  state->out2 = start_state;
  for (auto &accept_state : accept_states) {
    accept_state->out1 = state;
  }
  accept_states = {state};
  return *this;
}

NFA &&NFA::apply_optional() && {
  apply_optional();
  return std::move(*this);
}

NFA &NFA::apply_optional() & {
  auto state = make_epsilon_state();
  state->out2 = start_state;
  start_state = state;
  accept_states.push_back(state);
  return *this;
}

NFA &&NFA::alternate(NFA &&other) && {
  assert(this != &other);
  alternate(std::move(other));
  return std::move(*this);
}

NFA &NFA::alternate(NFA &&other) & {
  assert(this != &other);
  auto state = make_epsilon_state();
  if (start_state) {
    state->out1 = start_state;
  }
  if (other.start_state) {
    state->out2 = other.start_state;
  }
  start_state = state;
  accept_states.insert(accept_states.end(), other.accept_states.begin(),
                       other.accept_states.end());
  return *this;
}

NFA NFA::clone() const {
  std::unordered_set<std::shared_ptr<State>> visited;
  std::unordered_map<std::shared_ptr<State>, std::shared_ptr<State>> clones;
  std::vector<std::shared_ptr<State>> stack;
  if (start_state) {
    stack.push_back(start_state);
  }
  while (stack.size()) {
    auto current = stack.back();
    stack.pop_back();
    if (visited.find(current) != visited.end()) {
      continue;
    }
    visited.insert(current);
    auto clone = current->clone();
    clones[current] = clone;
    if (current->out1) {
      stack.push_back(current->out1);
    }
    if (current->out2) {
      stack.push_back(current->out2);
    }
  }
  NFA nfa;
  nfa.start_state = clones[start_state];
  for (const auto &accept_state : accept_states) {
    nfa.accept_states.push_back(clones[accept_state]);
  }
  for (const auto &clone : clones) {
    if (clone.first->out1)
      clone.second->out1 = clones[clone.first->out1];
    if (clone.first->out2)
      clone.second->out2 = clones[clone.first->out2];
  }

  return nfa;
}

NFA &&NFA::apply_interval(std::tuple<size_t, size_t, bool> quants) && {
  apply_interval(quants);
  return std::move(*this);
}

NFA &NFA::apply_interval(std::tuple<size_t, size_t, bool> quants) & {
  size_t min = std::get<0>(quants);
  size_t max = std::get<1>(quants);
  bool unbounded = std::get<2>(quants);

  NFA original = this->clone();

  if (min == 0) {
    this->apply_optional();
    min = 1;
  }

  for (size_t i = 1; i < min; i++) {
    this->concat(original.clone());
  }
  if (unbounded) {
    this->concat(original.clone().apply_star());
  } else {
    NFA st = original.clone();
    st.apply_optional();
    size_t lim = max - min;
    for (size_t i = 0; i < lim; i++) {
      this->concat(st.clone());
    }
  }
  return *this;
}

void NFA::print() const {
  std::vector<std::shared_ptr<State>> stack;
  std::unordered_set<std::shared_ptr<State>> visited;
  if (start_state) {
    stack.push_back(start_state);
  }
  while (stack.size()) {
    auto current = stack.back();
    stack.pop_back();
    if (visited.find(current) != visited.end()) {
      continue;
    }
    visited.insert(current);
    current->print();
    if (current->out1) {
      stack.push_back(current->out1);
    }
    if (current->out2) {
      stack.push_back(current->out2);
    }
  }
  for (const auto &accept_state : accept_states) {
    std::cout << "Accept NFAState id: " << accept_state->did << std::endl;
  }
  std::cout << "Start NFAState id: " << (start_state ? start_state->did : -1)
            << std::endl;
}
