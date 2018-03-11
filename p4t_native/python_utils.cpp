#include "python_utils.h"

auto p4t::svmr_entry2filter(py::object const& entry) -> Filter {
    auto const width = len(entry.attr("value"));
    Filter result(width);
    for (auto i = 0u; i < width; i++) {
        if (!entry.attr("mask")[i]) {
            result.set(i, Bit::ANY);
        } else {
            result.set(i, entry.attr("value")[i] ? Bit::ONE : Bit::ZERO);
        }
    }
    return result;
}

auto p4t::svmr2filters(py::object const& svmr) -> vector<Filter> {
    vector<Filter> filters{};
    for (auto i = 0; i < len(svmr); i++) {
        filters.emplace_back(svmr_entry2filter(svmr[i]));
    }

    return filters;
}

auto p4t::svmr2rules(py::object const& svmr) -> vector<Rule> {
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

auto p4t::rules2svmr(vector<Rule> const& rules) -> py::object {
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
