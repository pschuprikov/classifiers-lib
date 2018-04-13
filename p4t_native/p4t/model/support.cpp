#include "support.h"

auto p4t::model::select_unique(vector<Support> supports) -> vector<Support> {
    std::sort(begin(supports), end(supports));
    auto last = std::unique(begin(supports), end(supports));
    supports.erase(last, end(supports));

    return supports;
}


auto p4t::model::weight(vector<Support> const& unique_supports, 
        vector<Support> const& all_supports) -> vector<int> {
    support_map<int> support_cnt{};

    for (auto const& support : all_supports) {
        support_cnt[support]++;
    }

    vector<int> result(unique_supports.size());
    for (auto i = 0u; i < unique_supports.size(); i++) {
        result[i] = support_cnt[unique_supports[i]];
    }

    return result;
}

auto p4t::model::select_unique_n_weight(vector<Support> const& supports) 
    -> pair<vector<Support>, vector<int>> {
    auto const unique = select_unique(supports);
    auto const weights = weight(unique, supports);
    return make_pair(unique, weights);
}
