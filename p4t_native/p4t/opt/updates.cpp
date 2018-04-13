#include "updates.h"
#include <sstream>


namespace {

using namespace p4t::model;

auto subset_filter(Filter const& f, Support const& bits) -> Filter {
    Filter result(bits.size());
    for (auto i = 0u; i < bits.size(); i++) {
        result.set(i, f[bits[i]]);
    }
    return result;
}

auto is_prefix(Filter const& f) -> bool {
    bool seen_any = false;
    for (auto i = 0u; i < f.size(); i++) {
        if (f[i] == Bit::ANY) {
            seen_any = true;
        } else if (seen_any) {
            return false;
        }
    }
    return true;
}

}

auto p4t::opt::incremental_oi_lpm(
        vector<model::Filter> const& new_filters,
        vector<pair<vector<model::Filter>, model::Support>> groups,
        vector<model::Filter> traditional,
        size_t max_traditional_size) -> pair<size_t, size_t> {

    auto num_added_traditional = 0;
    auto num_added_groups = 0;
    for (auto const& f : new_filters) {
        bool added = false;
        for (auto& group : groups) {
            auto r_f = subset_filter(f, group.second);
            
            if (!is_prefix(r_f)) {
                continue;
            }
            if (!all_of(begin(group.first), end(group.first), 
                        [r_f] (auto const& of) { 
                            return !Filter::intersect(of, r_f); 
                        }
                    )) {
                continue;
            }
            group.first.emplace_back(r_f);
            added = true;
            num_added_groups++;
            break;
        }
        if (!added && traditional.size() < max_traditional_size) {
            traditional.emplace_back(f);
            added = true;
            num_added_traditional++;
        }
        
        if (!added) {
            break;
        }
    }

    return {num_added_groups, num_added_traditional};
}
