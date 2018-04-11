#include <p4t/utils/python_utils.h>
#include <p4t/utils/timer.h>

#include <boost/python/numpy.hpp>

namespace np = boost::python::numpy;

auto p4t::utils::svmr_entry2filter(py::object const& entry) -> model::Filter {
    using model::Filter;
    using model::Bit;

    np::ndarray value = np::from_object(entry.attr("value"));
    np::ndarray mask = np::from_object(entry.attr("mask"));

    bool * const p_value = reinterpret_cast<bool *>(value.get_data()); 
    bool * const p_mask = reinterpret_cast<bool *>(mask.get_data()); 
    auto const width = len(value);

    Filter result(width);
    for (auto i = 0u; i < width; i++) {
        if (!p_mask[i]) {
            result.set(i, Bit::ANY);
        } else {
            result.set(i, p_value[i] ? Bit::ONE : Bit::ZERO);
        }
    }
    return result;
}

auto p4t::utils::svmr2filters(py::object const& svmr) -> vector<model::Filter> {
    using model::Filter;

    vector<Filter> filters{};
    for (auto i = 0; i < len(svmr); i++) {
        filters.emplace_back(svmr_entry2filter(svmr[i]));
    }

    return filters;
}

auto p4t::utils::svmr2rules(py::object const& svmr) -> vector<model::Rule> {
    using model::Rule;
    using model::Action;

    utils::Timer t{"from python conversion"};
    vector<Rule> rules{};
    for (auto i = 0; i < len(svmr); i++) {
        Action action;
        if (svmr[i].attr("action") == py::object()) {
            action = Action::nop();
        } else {
            action = Action{py::extract<int>(svmr[i].attr("action"))};
        }
        rules.emplace_back(svmr_entry2filter(svmr[i]), action);
    }
    return rules;
}

auto p4t::utils::rules2svmr(vector<model::Rule> const& rules) -> py::object {
    using model::Bit;
    using model::Action;

    utils::Timer t{"to python conversion"};
    py::list result{};
    for (auto i = 0; i < int(rules.size()); i++) {
        np::ndarray mask = np::zeros(
                py::make_tuple(rules[i].filter().size()), 
                np::dtype::get_builtin<bool>());
        np::ndarray value = np::zeros(
                py::make_tuple(rules[i].filter().size()), 
                np::dtype::get_builtin<bool>());
        for (auto j = 0; j < int(rules[i].filter().size()); j++) {
            switch(rules[i].filter()[j]) {
                case Bit::ANY: {
                    mask[j] = false;
                    value[j] = false;
                } break;
                case Bit::ONE: {
                    mask[j] = true;
                    value[j] = true;
                } break;
                case Bit::ZERO: {
                    mask[j] = true;
                    value[j] = false;
                } break;
            }
        }
        auto const action = rules[i].action() != Action::nop() ? 
            py::object(rules[i].action().code()) : py::object();
        result.append(py::make_tuple(value, mask, action));
    }
    return result;
}
