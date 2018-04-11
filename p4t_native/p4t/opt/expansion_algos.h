#ifndef EXPANSION_AGLOS_H
#define EXPANSION_AGLOS_H

#include <p4t/model/support.h>
#include <p4t/common.h>

namespace p4t::opt {

auto try_expand_chain(
        vector<model::Support> chain, 
        vector<model::Support> const& unique_supports, 
        vector<int> const& weights,
        int max_bits) 
    -> pair<vector<model::Support>, model::support_map<model::Support>>;
    
}

#endif // EXPANSION_ALGOS
