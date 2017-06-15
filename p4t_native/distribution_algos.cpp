#include "boolean_minimization.h"

#include "distribution_algos.h"

auto p4t::perform_best_splitting(vector<Rule> const& rules, int capacity) 
    -> tuple<bool, vector<Rule>, vector<Rule>> {

    if (int(rules.size()) <= capacity) {
        return make_tuple(true, rules, vector<Rule>{});
    }

    auto best_value = -1.0;

    vector<Rule> best_rules_here{};
    vector<Rule> best_rules_there{};

    auto first_non_nop = -1;
    for (auto i = 0; i < int(rules.size()); i++) {
        if (rules[i].action() != Action::nop()) {
            first_non_nop = i;
            break;
        }
    }


    for (auto i = first_non_nop; i <= std::min(first_non_nop, int(rules.size())); i++) {
        if (rules[i].action() == Action::nop()) {
            continue;
        }
        for (auto j = i + capacity / 2; j <= std::min(i + capacity, int(rules.size())); j++) {
            if (rules[j - 1].action() == Action::nop()) {
                continue;
            }
            vector<Rule> rules_here(begin(rules), begin(rules) + j);

            std::for_each(begin(rules_here), begin(rules_here) + i, 
                    [](auto& x) { x.action() = Action::nop(); });
            rules_here = perform_boolean_minimization(rules_here, true);

            if (int(rules_here.size()) > capacity) {
                continue;
            }

            vector<Rule> rules_there(begin(rules), end(rules));
            
            std::for_each(begin(rules_there) + i, begin(rules_there) + j, 
                    [](auto& x) { x.action() = Action::nop(); });
            rules_there = perform_boolean_minimization(rules_there, true);

            if (best_value < 0 || rules_there.size() == 0 
                    ||  double(j - i) / rules_there.size() > best_value) {
                best_rules_here = rules_here;
                best_rules_there = rules_there;

                if (rules_there.size() == 0) {
                    return make_tuple(true, rules_here, rules_there); 
                }

                best_value = double(j - i) / rules_there.size();
            }
        }
    }

    if (best_value < 0) {
        return make_tuple(false, vector<Rule>{}, vector<Rule>{});
    }
    
    return make_tuple(true, best_rules_here, best_rules_there);
}
