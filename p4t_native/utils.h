#ifndef UTILS_H
#define UTILS_H

#include "filter.h"

namespace p4t {


inline auto svmr2filters(py::object const& svmr) {
    vector<Filter> filters{};
    for (auto i = 0; i < len(svmr); i++) {
        filters.emplace_back(Filter(svmr[i]));
    }

    return filters;
}

inline auto svmr2actions(py::object const& svmr) {
    vector<int> actions{};
    for (auto i = 0; i < len(svmr); i++) {
        if (svmr[i].attr("action") == py::object()) {
            actions.emplace_back(-1);
        } else {
            actions.emplace_back(py::extract<int>(svmr[i].attr("action")));
        }
    }
    return actions;
}

inline auto filters_n_actions2svmr(
        vector<Filter> const& filters, 
        vector<int> const& actions
        ) {
    assert(filters.size() == actions.size());

    py::list result{};
    for (auto i = 0; i < int(filters.size()); i++) {
        py::list mask{};
        py::list value{};
        for (auto j = 0; j < int(filters[i].size()); j++) {
            switch(filters[i][j]) {
                case Bit::ANY: {
                    mask.append(false);
                    value.append(false);
                } break;
                case Bit::ONE: {
                    mask.append(true);
                    value.append(true);
                } break;
                case Bit::ZERO: {
                    mask.append(true);
                    value.append(false);
                } break;
            }
        }
        result.append(py::make_tuple(value, mask, actions[i]));
    }
    return result;
}

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
