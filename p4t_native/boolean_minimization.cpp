#include "boolean_minimization.h"

namespace {

using namespace p4t;

template<class Container, class Indices> 
auto subset(Container const& c, Indices const& indices) {
    Container result;
    for (auto i : indices) {
        result.emplace_back(c[i]);
    }
    return result;
}

auto try_forward_subsumption(vector<Rule> const& rules)  {
    set<int> active;
    for (auto i = 0; i < int(rules.size()); i++) {
        active.insert(i);
    }

    for (auto i = begin(rules); i != end(rules); ++i) {
        if (active.count(i - begin(rules)) == 0) {
            continue;
        }

        for (auto j = i + 1; j != end(rules); ++j) {
            if (Filter::subsums(i->filter(), j->filter())) {
                active.erase(j - begin(rules));
            }
        }
    }

    return subset(rules, active);
}

template<class RuleIt>
auto find_first_intersection(
        Rule const& rule, RuleIt first, RuleIt beyond) {
    return std::find_if(first, beyond,
        [&rule](auto const& other) {
            return Filter::intersect(rule.filter(), other.filter()) && rule.action() != other.action();
        }
    );
}

template<class RuleIt>
auto find_first_subsums(
        Rule const& rule, RuleIt first, RuleIt beyond) {
    return std::find_if(first, beyond,
        [&rule](auto const& other) {
            return Filter::subsums(other.filter(), rule.filter());
        }
    );
}

template<class RuleIt>
auto check_no_intersection_with(
        Rule const& rule, RuleIt first, RuleIt beyond) -> bool {
    return find_first_intersection(rule, first, beyond) != beyond;
}

auto try_backward_subsumption(vector<Rule> const& rules, bool is_default_nop) {
    set<int> active;
    for (auto i = 0; i < int(rules.size()); ++i) {
        active.insert(i);
    }

    for (auto it = rbegin(rules); it != rend(rules); ++it) {
        auto const isect = find_first_intersection(*it, it.base(), end(rules));
        auto const subsum = find_first_subsums(*it, it.base(), end(rules));
        if (std::distance(subsum, isect) > 0) {
            active.erase(it.base() - begin(rules) - 1);
        }
        if (is_default_nop && it->action() == Action::nop() && isect == end(rules)) {
            active.erase(it.base() - begin(rules) - 1);
        }
    }

    return subset(rules, active);
}


auto try_resolution(vector<Rule> const& rules) {
    set<int> active;
    for (auto i = 0; i < int(rules.size()); ++i) {
        active.insert(i);
    }

    for (auto i = begin(rules); i != end(rules); ++i) {
        if (active.count(i - begin(rules)) == 0) {
            continue;
        }

        for (auto j = i + 1; j != end(rules); ++j) {
            if (active.count(j - begin(rules)) == 0) {
                continue;
            } 
            bool only_diff;
            int diff_bit;
            std::tie(only_diff, diff_bit) = Filter::fast_blocker(i->filter(), j->filter());

            if (i->action() == j->action() && only_diff && diff_bit >= 0 &&
                    check_no_intersection_with(*i, i + 1, j)) {
                i->filter().set(diff_bit, Bit::ANY);
                active.erase(j - begin(rules));
            }
        }
    }

    return subset(rules, active);
}


} // namespace

auto p4t::perform_boolean_minimization(vector<Rule> rules, bool is_default_nop) 
        -> vector<Rule> {
    
    auto previous_size = rules.size(); 
    auto const update_size = [&previous_size] (auto new_size, auto method) -> bool {
        if (new_size < previous_size) {
            log()->info("{} saved {} rules", method, previous_size - new_size);
            previous_size = new_size;
            return true;
        }
        return false;
    };

    auto trying = true;
    while (trying) {
        trying = false;
        rules = try_forward_subsumption(rules);
        if (update_size(rules.size(), "forward subsumption")) {
            trying = true;
        }

        rules = try_backward_subsumption(rules, is_default_nop);
        if (update_size(rules.size(), "backward subsumption")) {
            trying = true;
        }

        //rules = try_resolution(rules);
        //if (update_size(rules.size(), "resolution")) {
        //    trying = true;
        //}
    }
    return rules;
}

