#ifndef BOOLEAN_MINIMIZATION_H
#define BOOLEAN_MINIMIZATION_H

#include "rule.h"

namespace p4t {
    auto perform_boolean_minimization(vector<Rule> rules, bool is_default_nop) 
        -> vector<Rule>;
}

#endif
