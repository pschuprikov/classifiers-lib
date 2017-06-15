#ifndef DISTRIBUTION_AGLOS_H
#define DISTRIBUTION_ALGOS_H

#include "rule.h"

namespace p4t {

auto perform_best_splitting(vector<Rule> const& rules, int capacity) 
    -> tuple<bool, vector<Rule>, vector<Rule>>;

}

#endif
