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
auto check_no_intersection_with(
        Rule const& rule, RuleIt first, RuleIt beyond) -> bool {

    for (; first != beyond; ++first) {
        if (Filter::intersect(rule.filter(), first->filter()) && rule.action() != first->action()) {
            return false;
        }
    }
    return true;
}

auto try_backward_subsumption(vector<Rule> const& rules, bool is_default_nop) {
    set<int> active;
    for (auto i = 0; i < int(rules.size()); ++i) {
        active.insert(i);
    }

    for (auto i = begin(rules); i != end(rules); ++i) {
        // for (auto j = i + 1; j != end(rules); ++j) {
        //     if (i->action() == j->action() && Filter::subsums(j->filter(), i->filter()) &&
        //             check_no_intersection_with(*i, i + 1, j))
        //             {
        //         active.erase(i - begin(rules));
        //     }
        // }
        if (is_default_nop && i->action() == Action::nop() && 
                check_no_intersection_with(*i, i + 1, end(rules))) {
            active.erase(i - begin(rules));
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

