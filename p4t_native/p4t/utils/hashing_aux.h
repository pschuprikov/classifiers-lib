#ifndef HASHING_AUX_H
#define HASHING_AUX_H

#include <array>

namespace p4t::utils::hash {

inline auto hash_combine(size_t& result, size_t hash) {
    result ^= hash + 0x9e3779b9 + (result<<6) + (result>>2);
}

template <class T, size_t N>
auto hash_array(std::array<T, N> const& arr) -> size_t {
    std::size_t result = 0;
    for (auto i = 0u; i < N; i++) {
        hash_combine(result, std::hash<T>()(arr[i]));
    }
    return result;
}

template <class T, size_t... idxs>
auto hash_tuple(T const& t, std::index_sequence<idxs...> const&) -> size_t {
    std::size_t result = 0;
    for (auto const& hash: {std::hash<std::tuple_element_t<idxs, T>>()(std::get<idxs>(t))...}) {
        hash_combine(result, hash);
    }
    return result;
}

template<class... Args> 
auto hash_tuple(tuple<Args...> const& t) -> size_t {
    return hash_tuple(t, std::make_index_sequence<sizeof...(Args)>());
}

}

#endif
