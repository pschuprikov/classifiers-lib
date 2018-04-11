#ifndef DISTRIBUTION_ALGOS_H
#define DISTRIBUTION_ALGOS_H

#include <p4t/model/rule.h>

namespace p4t::opt {

auto perform_best_splitting(
        vector<model::Rule> const& rules, int capacity, bool use_resolution) 
    -> tuple<bool, vector<model::Rule>, vector<model::Rule>>;

}

#endif
