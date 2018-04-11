#ifndef CHAIN_ALGOS_H
#define CHAIN_ALGOS_H

#include <p4t/model/support.h>

namespace p4t::opt {

auto find_min_chain_partition(vector<model::Support> const& ss) 
    -> vector<vector<model::Support>>;

auto find_min_bounded_chain_partition(
        vector<vector<model::Support>> const& sss, 
        vector<vector<int>> const& weights, 
        int max_num_chains) -> vector<vector<vector<model::Support>>>;
}

#endif
