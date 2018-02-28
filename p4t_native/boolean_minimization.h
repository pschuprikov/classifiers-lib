#ifndef BOOLEAN_MINIMIZATION_H
#define BOOLEAN_MINIMIZATION_H

#include "rule.h"

namespace p4t {
namespace boolean_minimization {
    auto perform_boolean_minimization(vector<Rule> rules, bool is_default_nop, bool use_resolution) 
        -> vector<Rule>;
    auto calc_obstruction_weights(vector<Rule> const& rules) -> std::unordered_map<Action, int>;
}
}

#endif
