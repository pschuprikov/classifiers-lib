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
        auto const candidate = std::find_if(it.base(), end(rules),
            [rule=*it] (auto const& other) {
                return Filter::subsums(other.filter(), rule.filter()) 
                    || (Filter::intersect(other.filter(), rule.filter()) 
                            && rule.action() != other.action());
            }
        );
        if (candidate != end(rules) 
                && Filter::subsums(candidate->filter(), it->filter())
                    && candidate->action() == it->action()) {
            active.erase(it.base() - begin(rules) - 1);
        } 
        if (is_default_nop && it->action() == Action::nop() && candidate == end(rules)) {
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

auto p4t::boolean_minimization::perform_boolean_minimization(vector<Rule> rules, bool is_default_nop, bool use_resolution) 
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

        if (use_resolution) {
            rules = try_resolution(rules);
            if (update_size(rules.size(), "resolution")) {
                trying = true;
            }
        } else {
            // One iteration is enough in this case
            break;
        }
    }
    return rules;
}

auto p4t::boolean_minimization::calc_obstruction_weights(vector<Rule> const& rules) 
        -> std::unordered_map<Action, int> {
    std::unordered_map<Action, set<int>> counting_rules;
    for (auto i = begin(rules); i != end(rules); ++i) {
        for (auto j = begin(rules); j != i; ++j) {
            if (j->action() != i->action() && Filter::intersect(i->filter(), j->filter())) {
                counting_rules[i->action()].insert(j - begin(rules));
            }
        }
    }

    std::unordered_map<Action, int> result{};
    for (auto p : counting_rules) {
        result[p.first] = p.second.size();
    }
    return result;
}

