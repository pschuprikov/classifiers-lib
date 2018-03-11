#ifndef BITS_AUX_H
#define BITS_AUX_H

#include "common.h"
#include "hashing_aux.h"

typedef long long int u2 __attribute__ ((vector_size(16)));

namespace std {

template<>
struct hash<u2> {
    auto operator()(u2 const& x) const -> size_t {
        using comp = decltype(std::declval<u2>()[0]);
        size_t result = 0;
        p4t::hash::hash_combine(result, std::hash<comp>()(x[0]));
        p4t::hash::hash_combine(result, std::hash<comp>()(x[1]));
        return result;
    }
};

}

namespace p4t {

namespace num {

inline auto testz(__uint128_t const &x) -> bool {
    return x == 0;
}

inline auto ctz(__uint128_t const& x) -> bool {
    return __builtin_ctz(x);
}

inline auto less(__uint128_t const& lhs, __uint128_t const& rhs) -> bool {
    return lhs < rhs;
}

inline auto testz(u2 const &x) -> bool {
    const u2 zero = {0, 0};
    return __builtin_ia32_ptestz128(x, ~zero);
}

inline auto ctz(u2 const &x) -> size_t {
    if (x[0] == 0) {
        return sizeof(x[0]) * 8 + __builtin_ctz(x[1]);
    } else {
        return __builtin_ctz(x[0]);
    }
}

inline auto testz(uint64_t const &x) -> bool {
    return x == 0;
}

inline auto ctz(uint64_t const& x) -> bool {
    return __builtin_ctz(x);
}

inline auto less(uint64_t x, uint64_t y) -> bool {
    return x < y;
}

}

template<class T> struct kth_bit {
    static auto value(size_t k) -> T = delete;
};

template<>
struct kth_bit<u2> {
    static auto value(size_t k) -> u2 {
        using comp = decltype(std::declval<u2>()[0]);
        if (k < sizeof(comp) * 8) {
            u2 const result = {0, comp(1) << k};
            return result;
        } else {
            u2 const result = {comp(1) << (k - sizeof(comp) * 8), 0};
            return result;
        }
    }
};

template<>
struct kth_bit<__uint128_t> {
    static auto value(size_t k) -> __uint128_t {
        return __uint128_t(1) << k;
    }
};

template<>
struct kth_bit<uint64_t> {
    static auto value(size_t k) -> uint64_t {
        return uint64_t(1) << k;
    }
};


}

#endif
