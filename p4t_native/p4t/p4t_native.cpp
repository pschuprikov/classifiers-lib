#include <iostream>
#include <limits>
#include <numeric>
#include <map>
#include <omp.h>
#include <unordered_set>

#include <p4t/model/filter.h>
#include <p4t/model/support.h>
#include <p4t/utils/python_utils.h>

#include <p4t/opt/chain_algos.h>
#include <p4t/opt/oi_algos.h>
#include <p4t/opt/expansion_algos.h>
#include <p4t/opt/distribution_algos.h>
#include <p4t/opt/boolean_minimization.h>
#include <p4t/opt/updates.h>

#include "p4t_native.h"

namespace {

using namespace p4t;
using namespace p4t::model;
using namespace p4t::utils;
namespace py = p4t::py;

auto map_partition_indices(
        vector<vector<Support>> const& partition, 
        vector<Support> const & supports) -> vector<vector<int>> {

    support_map<int> support2partition{};
    for (auto i = 0u; i < partition.size(); i++) {
        for (auto const& s : partition[i]) {
            support2partition[s] = i;
        }
    }

    vector<vector<int>> result(partition.size());
    for (auto i = 0u; i < supports.size(); i++) {
        if (support2partition.count(supports[i])) {
            result[support2partition[supports[i]]].emplace_back(i);
        }
    }

    return result;
}


auto svmrs2supports(py::object svmrs) {
    vector<vector<Support>> sss(len(svmrs));
    for (auto i = 0; i < len(svmrs); ++i) {
        sss[i] = to_supports(svmr2filters(svmrs[i]));
    }
    return sss;
}

} // namespace 

auto p4t::min_pmgr(py::object svmr) -> py::object {
    if (py::len(svmr) == 0) {
        return py::object();
    }

    auto const filters = svmr2filters(svmr);

    auto const supports = to_supports(filters);
    auto const supports_unique = select_unique(supports);

    auto const partition = opt::find_min_chain_partition(supports_unique);
    auto const partition_indices = map_partition_indices(partition, supports);

    return py::make_tuple(to_python(partition), to_python(partition_indices));
}

auto p4t::min_bmgr(py::object svmrs, int max_num_groups) -> py::object {
    auto const n_supports = svmrs2supports(svmrs);

    vector<vector<Support>> n_unique_supports(n_supports.size());
    vector<vector<int>> n_weights(n_supports.size());
    for (auto i = 0u; i < n_supports.size(); ++i) {
        tie(n_unique_supports[i], n_weights[i]) = select_unique_n_weight(n_supports[i]);
    }
    
    auto const partitions = opt::find_min_bounded_chain_partition(
        n_unique_supports, n_weights, max_num_groups
    );

    vector<vector<vector<int>>> n_partition_indices(len(svmrs));
    for (auto i = 0; i < len(svmrs); i++) {
        n_partition_indices[i] = map_partition_indices(partitions[i], n_supports[i]);
    }

    return py::make_tuple(to_python(partitions), to_python(n_partition_indices));
}

auto p4t::best_subgroup(
        py::object svmr, int l, bool only_exact, 
        string algo, string max_oi_algo) -> py::object {
    auto const filters = svmr2filters(svmr);
    auto const max_oi_mode = max_oi_algo == "top_down" ?
        opt::OIMode::TOP_DOWN : opt::OIMode::MIN_DEGREE;

    if (algo == "min_similarity") {
        auto const bits = opt::best_min_similarity_bits(filters, l);
        auto const result = opt::find_maximal_oi_subset(
            filters, opt::bits_to_mask(bits), max_oi_mode
        );
        return py::make_tuple(to_python(bits), to_python(result));
    } else  if (algo == "icnp_oi" || algo == "icnp_blockers") {
        auto const minme_mode = algo == "icnp_oi" ? 
            opt::MinMEMode::MAX_OI : opt::MinMEMode::BLOCKERS;
        auto const bits_n_result = opt::best_to_stay_minme(
            filters, l, minme_mode, max_oi_mode, only_exact
        );

        return py::make_tuple(
            to_python(bits_n_result.first), to_python(bits_n_result.second)
        );
    } else {
        return py::object();
    }
}

void p4t::set_num_threads(int num_threads) {
    omp_set_dynamic(false);
    omp_set_num_threads(num_threads);
}

auto p4t::min_bmgr1_w_expansions(py::object classifier, int max_expanded_bits) -> py::object {
    if (len(classifier) == 0) {
        return py::object();
    }
    auto const supports = to_supports(svmr2filters(classifier));
    vector<vector<Support>> unique_supports(1); 
    vector<vector<int>> weights(1);
    tie(unique_supports[0], weights[0]) = select_unique_n_weight(supports);

    auto const chain = opt::find_min_bounded_chain_partition(
        unique_supports, weights, 1
    )[0][0];

    auto [chain_w_expansions, expansions] = opt::try_expand_chain(
            chain, unique_supports[0], weights[0], max_expanded_bits);


    support_set in_chain(begin(chain_w_expansions), end(chain_w_expansions));
    vector<int> indices;
    vector<Support> exps;
    for (auto i = 0; i < int(supports.size()); i++) {
        if (in_chain.count(expansions[supports[i]])) {
            indices.push_back(i);
            exps.push_back(expansions[supports[i]]);
        }
    }

    return py::make_tuple(to_python(chain_w_expansions), to_python(indices), to_python(exps));
}

void p4t::pylog(string msg) {
    python_log()->info(msg);
}

auto p4t::split(py::object classifier, int capacity, bool use_resolution) -> py::object {
    auto const rules = svmr2rules(classifier);

    auto [success, here, there] = opt::perform_best_splitting(
        rules, capacity, use_resolution
    );

    if (!success) {
        return py::make_tuple(py::object(), rules2svmr(rules));
    } 

    return py::make_tuple(rules2svmr(here), rules2svmr(there));
}

auto p4t::try_boolean_minimization(py::object classifier, bool use_resolution) -> py::object {
    auto const rules = svmr2rules(classifier);
    auto const result = opt::boolean_minimization::perform_boolean_minimization(
        rules, true, use_resolution
    );
    return rules2svmr(result);
}

auto p4t::calc_obstruction_weights(py::object classifier) -> py::object {
    auto const rules = utils::svmr2rules(classifier);
    auto const weights = opt::boolean_minimization::calc_obstruction_weights(
        rules
    );
    py::dict result{}; 
    for (auto p : weights) {
        result[p.first.code()] = p.second;
    }
    return result;
}

auto p4t::incremental_updates(
        py::object new_classifier, py::list lpm_groups, py::object traditional,
        int tcam_size) -> py::tuple {
    auto const new_classifier_int = utils::svmr2filters(new_classifier);
    vector<pair<vector<Filter>, Support>> lpm_groups_int{};
    for (auto i = 0u; i < len(lpm_groups); ++i) {
        auto bits = lpm_groups[i].attr("bits");
        lpm_groups_int.emplace_back(
            utils::svmr2filters(lpm_groups[i]), 
            Support(py::stl_input_iterator<int>(bits), py::stl_input_iterator<int>())
        );
    }
    auto const traditional_int = utils::svmr2filters(traditional);
    auto const [num_added_groups, num_added_traditional] = 
        opt::incremental_oi_lpm(new_classifier_int, lpm_groups_int, 
                                traditional_int, tcam_size);
    return py::make_tuple(num_added_groups, num_added_traditional);
}
