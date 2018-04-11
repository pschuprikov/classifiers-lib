#ifndef PYTHON_UTILS_H
#define PYTHON_UTILS_H

#include <p4t/model/rule.h>
#include <p4t/model/filter.h>

#include <boost/python.hpp>
#include <ctime>

namespace p4t {

namespace py = boost::python;

}

namespace p4t::utils {

auto svmr_entry2filter(py::object const& entry) -> model::Filter;
auto svmr2filters(py::object const& svmr) -> vector<model::Filter>;
auto svmr2rules(py::object const& svmr) -> vector<model::Rule>; 
auto rules2svmr(vector<model::Rule> const& rules) -> py::object;

}

namespace std { // Need std for ADL

template<class T>
auto to_python(T const& x) -> T {
    return x;
}

template<class T1, class T2>
auto to_python(pair<T1, T2> const& p) -> boost::python::tuple {
    return boost::python::make_tuple(to_python(p.first), to_python(p.second));
}

template<class T>
auto to_python(vector<T> const& xs) -> boost::python::list {
    boost::python::list result{};
    for (auto const& x : xs) {
        result.append(to_python(x));
    }
    return result;
}

template<class T>
auto to_python(set<T> const& xs) -> boost::python::list {
    boost::python::list result{};
    for (auto const& x : xs) {
        result.append(to_python(x));
    }
    return result;
}

}

#endif
