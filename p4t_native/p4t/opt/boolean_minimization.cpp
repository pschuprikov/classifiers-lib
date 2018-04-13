#include "boolean_minimization.h"

#include <p4t/utils/collections.h>
#include <p4t/utils/timer.h>
#include <p4t/opt/intersections_opt.h>

#include <unordered_map>

namespace {

using namespace p4t;
using namespace p4t::opt::boolean_minimization;
using namespace p4t::model;

auto try_new_forward_subsumption(vector<Rule> const& rules)  {
    utils::Timer t("new forward subsumption");
    set<int> active;
    for (auto i = 0; i < int(rules.size()); i++) {
        active.insert(i);
    }

    auto data = PreprocessingData::build(rules);

    auto total_checked = 0ll; 
    for (auto base_mask: data.masks()) {
        std::vector<vector<tuple<PreprocessingData::Mask, id_t>>> isectors(data.masks().size()); 

        std::vector<tuple<Rule, id_t>> bases{};

        for (auto it = begin(rules); it != end(rules); ++it) {
            total_checked++;

            if (!is_zero(it->filter().mask() - base_mask)) {
                continue;
            }

            auto const cur_rid = it - begin(rules);
            
            isectors[data.get_rule_mid(cur_rid)].emplace_back(
                it->filter().value() & it->filter().mask() & base_mask, cur_rid
            );

            if (it->filter().mask() == base_mask) {
                bases.emplace_back(*it, cur_rid);
            }
        }

        for (auto& res : isectors) {
            std::sort(begin(res), end(res));
        }

        for (auto const& [r, cur_rid] : bases) {
            auto const cur_val = r.filter().value() & base_mask;

            auto subsummed = false;
            for (auto mid = 0u; mid < data.masks().size(); ++mid) {
                total_checked++;
                auto const o_mask = data.masks()[mid];
                auto const o_val = o_mask & cur_val;

                auto isect_it = std::lower_bound(
                    begin(isectors[mid]), end(isectors[mid]),
                    make_tuple(o_val, cur_rid)
                );

                if (isect_it != begin(isectors[mid]) && std::get<0>(*(isect_it - 1)) == o_val) {
                    subsummed = true;
                    break;
                }
            }

            if (subsummed) {
                active.erase(cur_rid);
            }
        }
    }

    return utils::subset(rules, active);
}

[[maybe_unused]]
auto try_forward_subsumption(vector<Rule> const& rules)  {
    utils::Timer t("forward subsumption");
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

    return utils::subset(rules, active);
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


[[maybe_unused]]
auto try_backward_subsumption(vector<Rule> const& rules, bool is_default_nop) {
    utils::Timer t("backward subsumption");

    set<int> active;
    for (auto i = 0; i < int(rules.size()); ++i) {
        active.insert(i);
    }

    auto total_checked = 0ll; 
    for (auto it = rbegin(rules); it != rend(rules); ++it) {
        auto const candidate = std::find_if(it.base(), end(rules),
            [rule=*it] (auto const& other) {
                return Filter::subsums(other.filter(), rule.filter()) 
                    || (Filter::intersect(other.filter(), rule.filter()) 
                            && rule.action() != other.action());
            }
        );
        total_checked += distance(it.base(), candidate);
        if (candidate != end(rules) && candidate->action() == it->action()) {
            active.erase(it.base() - begin(rules) - 1);
        } 
        if (is_default_nop && it->action() == Action::nop() && candidate == end(rules)) {
            active.erase(it.base() - begin(rules) - 1);
        }
    }
    log()->info("total checked: {}", total_checked);

    return utils::subset(rules, active);
}

[[maybe_unused]]
auto try_new_backward_subsumption(vector<Rule> const& rules, bool is_default_nop) {
    utils::Timer t("new backward subsumption");

    set<int> active;
    for (auto i = 0; i < int(rules.size()); ++i) {
        active.insert(i);
    }

    auto data = PreprocessingData::build(rules);

    auto total_checked = 0ll; 

    for (auto base_mask: data.masks()) {
        std::vector<vector<tuple<Filter::BitArray, id_t, Action>>> isectors(data.masks().size()); 
        auto const ignore_action_cmp = [](auto const& a, auto const& b) {
            return make_tuple(std::get<0>(a), std::get<1>(a))
                < make_tuple(std::get<0>(b), std::get<1>(b));
        };

        std::vector<tuple<Rule, id_t>> bases{};

        for (auto it = begin(rules); it != end(rules); ++it) {
            total_checked++;
            auto const cur_rid = it - begin(rules);
            isectors[data.get_rule_mid(cur_rid)].emplace_back(
                it->filter().value() & it->filter().mask() & base_mask, 
                cur_rid, it->action()
            );
            if (it->filter().mask() == base_mask) {
                bases.emplace_back(*it, cur_rid);
            }
        }
        for (auto& res : isectors) {
            std::sort(begin(res), end(res), ignore_action_cmp);
        }

        for (auto const& [r, cur_rid] : bases) {
            auto const cur_val = r.filter().value() & base_mask;

            auto conflicting = id_none;
            auto subsumming = id_none;
            for (auto mid = 0u; mid < data.masks().size(); ++mid) {
                total_checked++;
                auto const o_mask = data.masks()[mid];
                auto const o_val = o_mask & cur_val;
                auto isect_it = std::upper_bound(
                    begin(isectors[mid]), end(isectors[mid]),
                    make_tuple(o_val, cur_rid, 0), ignore_action_cmp
                );
                if (isect_it != end(isectors[mid]) && std::get<0>(*isect_it) == o_val) {
                    if (is_zero(o_mask - base_mask) && std::get<2>(*isect_it) == r.action()) {
                        subsumming = std::min(std::get<1>(*isect_it), subsumming);
                    } else {
                        isect_it = std::find_if(isect_it, end(isectors[mid]), [&] (auto const& x) {
                            return std::get<0>(x) != o_val || std::get<2>(x) != r.action();
                        });
                        if (isect_it != end(isectors[mid]) && std::get<0>(*isect_it) == o_val) {
                            conflicting = std::min(std::get<1>(*isect_it), conflicting);
                        };
                    }
                }
            }

            if (subsumming != id_none 
                    && (conflicting == id_none || conflicting > subsumming)) {
                active.erase(cur_rid);
            }
            if (is_default_nop && r.action() == Action::nop() 
                    && conflicting == id_none) {
                active.erase(cur_rid);
            }
        }
    }
    log()->info("total checked: {}", total_checked);

    return utils::subset(rules, active);
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

    return utils::subset(rules, active);
}


} // namespace

auto p4t::opt::boolean_minimization::perform_boolean_minimization(
        vector<Rule> rules, bool is_default_nop, bool use_resolution) 
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
        rules = try_new_forward_subsumption(rules);
        if (update_size(rules.size(), "forward subsumption")) {
            trying = true;
        }

        rules = try_new_backward_subsumption(rules, is_default_nop);
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

auto p4t::opt::boolean_minimization::calc_obstruction_weights(
        vector<Rule> const& rules) -> std::unordered_map<Action, int> {
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
