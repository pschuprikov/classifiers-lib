#ifndef OI_ALGOS_H
#define OI_ALGOS_H

#include <p4t/common.h>
#include <p4t/model/filter.h>

namespace p4t::opt {

enum MinMEMode {
    MAX_OI, BLOCKERS
};

struct BitStats {
    explicit BitStats(size_t n) 
        : dontcare(n), zeros(n), ones(n), exact_bits{} {
    }

    vector<int> dontcare;
    vector<int> zeros;
    vector<int> ones;
    vector<int> exact_bits;
};


auto calc_bit_stats(vector<model::Filter> const& filters) -> BitStats;

auto best_min_similarity_bits(vector<model::Filter> const& filters, size_t l) 
    -> vector<int>;

auto best_to_stay_minme(
    vector<model::Filter> filters, size_t l, MinMEMode mode, bool only_exact) 
    -> pair<vector<int>, vector<int>>;

auto find_maximal_oi_subset(
    vector<model::Filter> const& filters, model::Filter::BitArray const& mask) 
    -> vector<int>;

auto find_maximal_oi_subset_indices(
    vector<model::Filter> const& filters, vector<int> const& indices, 
    model::Filter::BitArray const& mask) 
    -> vector<int>;

auto bits_to_mask(vector<int> const& bits) -> model::Filter::BitArray;

}

#endif
