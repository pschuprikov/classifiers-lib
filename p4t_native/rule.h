#ifndef RULE_H
#define RULE_H

#include "filter.h"

namespace p4t {

class Action {
public: 
    Action(int code = -1) : code_{code} {
    }

    auto code() const {
        return code_;
    }

public:
    static Action nop() {
        return Action{};
    }

private:
    int code_; 
};


inline auto operator==(Action const& lhs, Action const& rhs) {
    return rhs.code() == lhs.code();
}

inline auto operator!=(Action const& lhs, Action const& rhs) {
    return !(rhs == lhs);
}

inline auto operator<(Action const& lhs, Action const& rhs) {
    return lhs.code() < rhs.code();
}

class Rule {
public:
    Rule() = default;

    Rule(Filter filter, Action action)
        : filter_{filter}, action_{action} {
    }

    auto& action() {
        return action_;
    }

    auto& filter() {
        return filter_;
    }

    auto action() const {
        return action_;
    }

    auto filter() const {
        return filter_;
    }

private:
    Filter filter_;
    Action action_;
};

}

namespace std {

template<>
struct hash<p4t::Action> {
    size_t operator()(p4t::Action const& action) const {
        return static_cast<size_t>(action.code());
    }
};

}

#endif // RULE_H

