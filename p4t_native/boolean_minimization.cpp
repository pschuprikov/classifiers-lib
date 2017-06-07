#include "boolean_minimization.h"

namespace p4t {
namespace {

template<class Container, class Indices> 
auto subset(Container const& c, Indices const& indices) {
    Container result;
    for (auto i : indices) {
        result.emplace_back(c[i]);
    }
    return result;
}

auto try_forward_subsumption(vector<Filter> const& filters, vector<int> const& actions) 
        -> pair<vector<Filter>, vector<int>> {
    set<int> active;
    for (auto i = 0; i < int(filters.size()); i++) {
        active.insert(i);
    }

    for (auto i = 0; i < int(filters.size()); i++) {
        if (active.count(i) == 0) {
            continue;
        }

        for (auto j = i + 1; j < int(filters.size()); j++) {
            if (Filter::subsums(filters[i], filters[j])) {
                active.erase(j);
            }
        }
    }

    return make_pair(subset(filters, active), subset(actions, active));
}

template<class FilterIt, class ActionIt>
auto check_no_intersection_with(
        Filter const& filter, int action,
        FilterIt f_first, FilterIt f_beyond, 
        ActionIt a_first, ActionIt a_beyond) -> bool {

    for (; f_first != f_beyond && a_first != a_beyond; ++f_first, ++a_first) {
        if (Filter::intersect(filter, *f_first) && action != *a_first) {
            return false;
        }
    }
    return true;
}

auto try_backward_subsumption(
        vector<Filter> const& filters, vector<int> const& actions, 
        bool is_default_nop) {
    set<int> active;
    for (auto i = 0; i < int(filters.size()); i++) {
        active.insert(i);
    }

    for (auto i = 0; i < int(filters.size()); i++) {
        for (auto j = i + 1; j < int(filters.size()); j++) {
            if (actions[i] == actions[j] && Filter::subsums(filters[j], filters[i]) &&
                    check_no_intersection_with(
                        filters[i], actions[i],
                        begin(filters) + i + 1, begin(filters) + j,
                        begin(actions) + i + 1, begin(actions) + j
                        )
                    ) {
                active.erase(i);
            }
        }
        if (is_default_nop && actions[i] < 0 && 
                check_no_intersection_with(
                    filters[i], actions[i],
                    begin(filters) + i + 1, end(filters),
                    begin(actions) + i + 1, end(actions)
                    )
                ) {
            active.erase(i);
        }
    }

    return make_pair(subset(filters, active), subset(actions, active));
}


auto try_resolution(vector<Filter> filters, vector<int> const& actions) {
    set<int> active;
    for (auto i = 0; i < int(filters.size()); i++) {
        active.insert(i);
    }
    for (auto i = 0; i < int(filters.size()); i++) {
    if (active.count(i) == 0) {
            continue;
        }

        for (auto j = i + 1; j < int(filters.size()); j++) {
            if (active.count(j) == 0) {
                continue;
            } 
            bool only_diff;
            int diff_bit;
            std::tie(only_diff, diff_bit) = Filter::fast_blocker(filters[i], filters[j]);
            if (actions[i] == actions[j] && only_diff && diff_bit >= 0 &&
                    check_no_intersection_with(
                        filters[i], actions[i],
                        begin(filters) + i + 1, begin(filters) + j,
                        begin(actions) + i + 1, begin(actions) + j
                        )) {
                filters[i].set(diff_bit, Bit::ANY);
                active.erase(j);
            }
        }
    }

    return make_pair(subset(filters, active), subset(actions, active));
}


} // namespace
} // namespace p4t

auto p4t::perform_boolean_minimization(
        vector<Filter> filters, vector<int> actions, bool is_default_nop) 
        -> pair<vector<Filter>, vector<int>> {
    auto old_size = filters.size();
    while (true) {
        std::tie(filters, actions) = try_forward_subsumption(filters, actions);
        std::tie(filters, actions) = try_backward_subsumption(filters, actions, is_default_nop);
        std::tie(filters, actions) = try_resolution(filters, actions);

        if (filters.size() == old_size) {
            break;
        }
    
        old_size = filters.size();
    }
    return make_pair(std::move(filters), std::move(actions));
}

