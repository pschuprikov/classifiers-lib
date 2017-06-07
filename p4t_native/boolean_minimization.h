#ifndef BOOLEAN_MINIMIZATION_H
#define BOOLEAN_MINIMIZATION_H

#include "filter.h"

namespace p4t {
    auto perform_boolean_minimization(
            vector<Filter> filters, vector<int> actions, bool is_default_nop) 
        -> pair<vector<Filter>, vector<int>>;
}

#endif
