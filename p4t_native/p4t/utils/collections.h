#ifndef COLLECTIONS_H
#define COLLECTIONS_H

namespace p4t::utils {

template<class Container, class Indices> 
auto subset(Container const& c, Indices const& indices) {
    Container result;
    for (auto i : indices) {
        result.emplace_back(c[i]);
    }
    return result;
}

template<class It, class IdxIt, class Out1, class Out2> 
auto partition_copy(
        It first, It beyond, IdxIt id_first, IdxIt idx_beyond,
        Out1 out_true, Out2 out_false
    ) {
    auto idx_it = id_first;
    for (auto it = first; it != beyond; ++it) {
        if (idx_it != idx_beyond && (it - first) == *idx_it) {
            idx_it++;
            out_true++ = *it;
        } else {
            out_false++ = *it;
        }
    }
}

}

#endif
