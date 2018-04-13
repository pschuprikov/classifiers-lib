#ifndef P4T_NATIVE_H
#define P4T_NATIVE_H

#include "common.h"

#include <boost/python.hpp>

namespace p4t {

namespace py = boost::python;

auto min_pmgr(py::object classifier) -> py::object;

auto min_bmgr1_w_expansions(
        py::object classifier, int max_expanded_bits) -> py::object;

auto min_bmgr(py::object classifiers, int max_num_groups) -> py::object;
auto best_subgroup(
        py::object classifier, int max_width, bool only_exact, string algo)
    -> py::object;

void set_num_threads(int num_threads);

void pylog(string msg);

auto try_boolean_minimization(
        py::object classifier, bool use_resolution) -> py::object;

auto split(
        py::object classifier, int capacity, bool use_resolution) -> py::object;

auto calc_obstruction_weights(py::object classifier) -> py::object;

auto incremental_updates(
        py::object new_classifier, py::list lpm_groups, py::object traditional,
        int tcam_size) -> py::tuple;

}

#endif
