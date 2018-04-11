#ifndef BOOLEAN_MINIMIZATION_H
#define BOOLEAN_MINIMIZATION_H

#include <p4t/model/rule.h>

namespace p4t::opt::boolean_minimization {
    auto perform_boolean_minimization(
        vector<model::Rule> rules, bool is_default_nop, bool use_resolution) 
        -> vector<model::Rule>;
    auto calc_obstruction_weights(
        vector<model::Rule> const& rules) 
        -> std::unordered_map<model::Action, int>;
}

#endif
