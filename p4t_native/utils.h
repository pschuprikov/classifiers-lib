#ifndef UTILS_H
#define UTILS_H

#include "rule.h"
#include "filter.h"

namespace p4t {


inline auto svmr2filters(py::object const& svmr) {
    vector<Filter> filters{};
    for (auto i = 0; i < len(svmr); i++) {
        filters.emplace_back(Filter(svmr[i]));
    }

    return filters;
}

inline auto svmr2rules(py::object const& svmr) {
    vector<Rule> rules{};
    for (auto i = 0; i < len(svmr); i++) {
        Action action;
        if (svmr[i].attr("action") == py::object()) {
            action = Action::nop();
        } else {
            action = Action{py::extract<int>(svmr[i].attr("action"))};
        }
        rules.emplace_back(Filter(svmr[i]), action);
    }
    return rules;
}

inline auto rules2svmr(vector<Rule> const& rules) {
    py::list result{};
    for (auto i = 0; i < int(rules.size()); i++) {
        py::list mask{};
        py::list value{};
        for (auto j = 0; j < int(rules[i].filter().size()); j++) {
            switch(rules[i].filter()[j]) {
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
        auto const action = rules[i].action() != Action::nop() ? 
            py::object(rules[i].action().code()) : py::object();
        result.append(py::make_tuple(value, mask, action));
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
