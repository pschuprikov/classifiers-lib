#include "intersections_opt.h"

namespace p4t::opt::boolean_minimization {

auto PreprocessingData::add_rule(model::Rule const& rule) -> size_t {
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
        return new_mid;
    } else {
        return mid_it->second;
    }
}

auto PreprocessingData::build(vector<model::Rule> const& rules) 
    -> PreprocessingData {
    PreprocessingData result{};
    for (auto const& rule : rules) {
        result.add_rule(rule);
    }
    return std::move(result);
}

}
