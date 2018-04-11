#ifndef INTERSECTIONS_OPT_H
#define INTERSECTIONS_OPT_H

#include <p4t/model/rule.h>

#include <unordered_set>
#include <unordered_map>
#include <limits>

namespace std {

template<class BitChunkT, size_t WidthT, class... Other>
struct hash<tuple<p4t::utils::PackedBitArray<BitChunkT, WidthT>, Other...>> {
    template<class T>
    size_t operator()(T const& pba) const {
        return p4t::utils::hash::hash_tuple(pba);
    }
};

}

namespace p4t::opt::boolean_minimization {

using id_t = uint32_t;
static auto const id_none = std::numeric_limits<id_t>::max();

class PreprocessingData {
public:
    using Mask = model::Filter::BitArray;

public:
    PreprocessingData(PreprocessingData&&) = default;

    auto& masks() const {
        return mid_to_mask_;
    }

    auto get_rule_mid(id_t rid) const {
        return rid_to_mid_[rid];
    }

    static auto build(vector<model::Rule> const& rules) -> PreprocessingData;

private:
    PreprocessingData() = default;

    auto add_rule(model::Rule const& rule) -> size_t;
    auto get_mid(Mask const& m) -> size_t;

private:
    vector<size_t> rid_to_mid_;
    vector<Mask> mid_to_mask_;
    std::unordered_map<Mask, size_t> mask_to_mid_;
};

}

#endif
