#ifndef SUPPORT_H
#define SUPPORT_H

#include <unordered_map>
#include <unordered_set>
#include <boost/functional/hash.hpp>

#include <p4t/common.h>
#include <p4t/model/filter.h>

namespace p4t::model { 
using Support = vector<int>;

template<class T>
using support_map = std::unordered_map<Support, T, boost::hash<Support>>;
using support_set = std::unordered_set<Support, boost::hash<Support>>;

auto select_unique(vector<Support> supports) -> vector<Support>;

auto weight(vector<Support> const& unique_supports, 
            vector<Support> const& all_supports) -> vector<int>;

auto select_unique_n_weight(vector<Support> const& supports) 
    -> pair<vector<Support>, vector<int>>;

inline auto to_support(Filter const& filter) -> Support {
    Support result{};
    for (auto i = 0; i < int(filter.size()); i++) {
        if (filter[i] != Bit::ANY) {
            result.emplace_back(i);
        }
    }
    return result;
}

inline auto to_supports(vector<Filter> const& filters) -> vector<Support> {
    vector<Support> supports{};
    transform(begin(filters), end(filters), back_inserter(supports), to_support);
    return supports;
}

inline auto is_subset(Support const& lhs, Support const& rhs) {
    return std::includes(begin(rhs), end(rhs), begin(lhs), end(lhs)); 
}

inline auto is_in_conflict(Support const& lhs, Support const& rhs) {
    return !is_subset(lhs, rhs) && !is_subset(rhs, lhs);
}

inline auto get_intersection(Support const& lhs, Support const& rhs) {
    Support result{};
    std::set_intersection(
        begin(lhs), end(lhs), begin(rhs), end(rhs), back_inserter(result)
    );
    return result;
}

inline auto get_union(Support const& rhs, Support const& lhs) -> Support {
    Support result{};
    set_union(begin(rhs), end(rhs), begin(lhs), end(lhs), back_inserter(result));
    return result;
}

inline auto get_difference(Support const& rhs, Support const& lhs) -> Support {
    Support result{};
    set_difference(begin(rhs), end(rhs), begin(lhs), end(lhs), back_inserter(result));
    return result;
}

template<class OStream>
OStream& operator<<(OStream& os, Support const& s) {
    os << "{";
    for (auto it = begin(s); next(it) != end(s); ++it) {
        os << (*it) << ", ";
    }
    os << s.back() << "}";
    return os;
}

}

#endif
