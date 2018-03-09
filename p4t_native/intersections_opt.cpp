#include "intersections_opt.h"
#include "timer.h"

namespace p4t::boolean_minimization {

auto PreprocessingData::add_rule(Rule const& rule) -> size_t {
    auto const mid = get_mid(rule.filter().mask());
    rid_to_mid_.emplace_back(mid);
    return mid;
}

auto PreprocessingData::get_mid(Mask const& m) -> size_t {
    auto mid_it = mask_to_mid_.find(m);
    if (mid_it == end(mask_to_mid_)) {
        auto new_mid = mask_to_mid_.size();
        mask_to_mid_[m] = new_mid;
        mid_to_mask_.emplace_back(m);
        mid_counts_.emplace_back(1);
        return new_mid;
    } else {
        mid_counts_[mid_it->second]++;
        return mid_it->second;
    }
}

auto PreprocessingData::build(vector<Rule> const& rules) 
        -> pair<PreprocessingData, MutableData> {
    Timer t("building preprocessed data");
    PreprocessingData result{};
    for (auto const& rule : rules) {
        result.add_rule(rule);
    }
    auto mut_data = MutableData(result.masks().size());
    for (auto i = 0u; i < mut_data.size(); i++) {
        mut_data[i].rehash(result.get_mid_counts()[i]);
    }
    return make_pair(std::move(result), std::move(mut_data));
}

}
