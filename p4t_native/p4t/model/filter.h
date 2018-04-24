#ifndef FILTER_H
#define FILTER_H

#include <p4t/common.h>
#include <p4t/utils/bit_array.h>
#include <p4t/utils/hashing_aux.h>

#include <iterator>
#include <algorithm>
#include <ios>

namespace p4t::model {

enum class Bit {
    ONE, ZERO, ANY 
};

inline auto& operator<<(std::ostream& out, Bit b) {
    switch (b) {
        case Bit::ONE: return out << "1";
        case Bit::ZERO: return out << "0";
        case Bit::ANY: return out << "*";
        default: abort();
    }
}

class Filter {
public:
    using BitArray = utils::PackedBitArray<uint64_t, MAX_WIDTH>;

public:
    explicit Filter(size_t width) 
        : value_{}, mask_{}, width_{width} {
        assert(width_ <= MAX_WIDTH);
    }

    auto size() const {
        return width_;
    } 
    auto operator[](size_t i) const -> Bit {
        return mask_.get(i) ? (value_.get(i) ? Bit::ONE : Bit::ZERO) : Bit::ANY;
    }

    void set(size_t i, Bit value) {
        switch(value) {
            case Bit::ANY: {
                mask_.set(i, false);
            } break;
            case Bit::ONE: {
                mask_.set(i, true);
                value_.set(i, true);
            } break;
            case Bit::ZERO: {
                mask_.set(i, true);
                value_.set(i, false);
            } break;
        }
    }

    auto has_any(BitArray const& mask) const -> bool {
        using utils::num::testz;
        for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
            if (!testz(mask.chunk(i) & ~mask_.chunk(i))) {
                return true;
            }
        }
        return false;
    }

    auto const value() const {
        return value_;
    }

    auto const mask() const {
        return mask_;
    }

public:
    static auto fast_blocker(
            Filter const& f1, Filter const& f2, BitArray const& mask)
        -> pair<bool, int>;

    static auto fast_blocker(
            Filter const& f1, Filter const& f2)
        -> pair<bool, int>;


    static auto intersect(
            Filter const& lhs, Filter const& rhs, BitArray const& mask) 
        -> bool;

    static auto intersect(Filter const& lhs, Filter const& rhs) -> bool;
    static auto subsums(Filter const& lhs, Filter const& rhs) -> bool;

private:
    BitArray value_;
    BitArray mask_;
    size_t width_;
};

auto inline Filter::fast_blocker(
    Filter const& f1, Filter const& f2, BitArray const& mask) 
    -> pair<bool, int> {
    namespace num = utils::num;

    auto first_difference = -1;
    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        auto const diff = 
            (f1.value_.chunk(i) ^ f2.value_.chunk(i)) 
            & (mask.chunk(i) & f1.mask_.chunk(i) & f2.mask_.chunk(i));
        if (!num::testz(diff)) {
            if (first_difference != -1) {
                return {false, first_difference};
            }
            auto const idx = num::ctz(diff);
            first_difference = i * BitArray::BITS_PER_CHUNK + idx;
            if (!num::testz(diff & ~num::kth_bit<BitArray::BitChunk>::value(idx))) {
                return {false, first_difference};
            }
        }
    }
    return {true, first_difference};
}

// TODO: deduplicate
auto inline Filter::fast_blocker(
    Filter const& f1, Filter const& f2) 
    -> pair<bool, int> {
    namespace num = utils::num;

    auto first_difference = -1;
    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        auto const diff = 
            (f1.value_.chunk(i) ^ f2.value_.chunk(i)) 
            & (f1.mask_.chunk(i) & f2.mask_.chunk(i));
        if (!num::testz(diff)) {
            if (first_difference != -1) {
                return {false, first_difference};
            }
            auto const idx = num::ctz(diff);
            first_difference = i * BitArray::BITS_PER_CHUNK + idx;
            if (!num::testz(diff & ~num::kth_bit<BitArray::BitChunk>::value(idx))) {
                return {false, first_difference};
            }
        }
    }
    return {true, first_difference};
}

auto inline Filter::intersect(
    Filter const& lhs, Filter const& rhs, BitArray const& mask) 
    -> bool {
    namespace num = utils::num;

    assert(lhs.size() == rhs.size());

    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        if (!num::testz((lhs.value_.chunk(i) ^ rhs.value_.chunk(i)) 
                & mask.chunk(i) & lhs.mask_.chunk(i) & rhs.mask_.chunk(i))) {
            return false;
        }
    }

    return true;
}

// TODO: deduplicate
auto inline Filter::intersect(Filter const& lhs, Filter const& rhs) -> bool {
    namespace num = utils::num;

    assert(lhs.size() == rhs.size());

    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        if (!num::testz((lhs.value_.chunk(i) ^ rhs.value_.chunk(i)) 
                & lhs.mask_.chunk(i) & rhs.mask_.chunk(i))) {
            return false;
        }
    }

    return true;
}

auto inline Filter::subsums(Filter const& lhs, Filter const& rhs) -> bool {
    namespace num = utils::num;

    assert(lhs.size() == rhs.size());

    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        if (!num::testz(((lhs.value_.chunk(i) ^ rhs.value_.chunk(i)) | ~rhs.mask_.chunk(i)) 
                & lhs.mask_.chunk(i))) {
            return false;
        }
    }

    return true;
}

}

namespace std {

template<>
struct hash<p4t::model::Filter> {
    size_t operator()(p4t::model::Filter const& f) const {
        using namespace p4t::utils::hash;
        size_t result = std::hash<p4t::model::Filter::BitArray>()(f.mask());
        hash_combine(result, std::hash<p4t::model::Filter::BitArray>()(f.value()));
        return result;
    }
};

}
#endif
