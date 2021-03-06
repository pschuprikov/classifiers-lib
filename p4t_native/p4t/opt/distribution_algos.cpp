#include "distribution_algos.h"

#include <p4t/opt/boolean_minimization.h>


auto p4t::opt::perform_best_splitting(
        vector<model::Rule> const& rules, int capacity, bool use_resolution) 
    -> tuple<bool, vector<model::Rule>, vector<model::Rule>> {
    using namespace model;

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
        if (rules[i].action() == model::Action::nop()) {
            continue;
        }
        vector<Rule> rules_here;

        auto l = i;
        auto r = std::min(i + capacity, int(rules.size()));

        while (r - l > 1) {
            auto m = (r + l) / 2;
            log()->info(" trying j = {:d}", m);

            rules_here.assign(begin(rules), begin(rules) + m);
            std::for_each(begin(rules_here), begin(rules_here) + i, 
                    [](auto& x) { x.action() = Action::nop(); });
            rules_here = boolean_minimization::perform_boolean_minimization(rules_here, true, use_resolution);

            if (int(rules_here.size()) > capacity) {
                r = m;
            } else {
                l = m;
            }
        }

        if (l == i) {
            continue;
        }

        vector<Rule> rules_there(begin(rules), end(rules));
        
        std::for_each(begin(rules_there) + i, begin(rules_there) + l, 
                [](auto& x) { x.action() = Action::nop(); });
        rules_there = boolean_minimization::perform_boolean_minimization(rules_there, true, use_resolution);

        if (best_value < 0 || rules_there.size() == 0 
                ||  double(l - i) / rules_there.size() > best_value) {
            best_rules_here = rules_here;
            best_rules_there = rules_there;

            if (rules_there.size() == 0) {
                return make_tuple(true, rules_here, rules_there); 
            }

            best_value = double(l - i) / rules_there.size();
        }
    }

    if (best_value < 0) {
        return make_tuple(false, vector<Rule>{}, vector<Rule>{});
    }
    
    return make_tuple(true, best_rules_here, best_rules_there);
}
