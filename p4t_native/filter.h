#ifndef FILTER_H
#define FILTER_H


#include "common.h"
#include "bit_array.h"

#include <iterator>
#include <algorithm>

namespace p4t {


enum class Bit {
    ONE, ZERO, ANY 
};

class Filter {
public:
    using BitArray = PackedBitArray<u2, MAX_WIDTH>;

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
        for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
            if (!num::testz(mask.chunk(i) & ~mask_.chunk(i))) {
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

}


auto inline p4t::Filter::fast_blocker(
        Filter const& f1, Filter const& f2, BitArray const& mask) -> pair<bool, int> {

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
            if (!num::testz(diff & ~kth_bit<BitArray::BitChunk>::value(idx))) {
                return {false, first_difference};
            }
        }
    }
    return {true, first_difference};
}

// TODO: deduplicate
auto inline p4t::Filter::fast_blocker(
        Filter const& f1, Filter const& f2) -> pair<bool, int> {

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
            if (!num::testz(diff & ~kth_bit<BitArray::BitChunk>::value(idx))) {
                return {false, first_difference};
            }
        }
    }
    return {true, first_difference};
}

auto inline p4t::Filter::intersect(
        Filter const& lhs, Filter const& rhs, BitArray const& mask) -> bool {

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
auto inline p4t::Filter::intersect(Filter const& lhs, Filter const& rhs) -> bool {

    assert(lhs.size() == rhs.size());

    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        if (!num::testz((lhs.value_.chunk(i) ^ rhs.value_.chunk(i)) 
                & lhs.mask_.chunk(i) & rhs.mask_.chunk(i))) {
            return false;
        }
    }

    return true;
}

auto inline p4t::Filter::subsums(Filter const& lhs, Filter const& rhs) -> bool {
    assert(lhs.size() == rhs.size());

    for (auto i = 0u; i < BitArray::NUM_CHUNKS; i++) {
        if (!num::testz(((lhs.value_.chunk(i) ^ rhs.value_.chunk(i)) | ~rhs.mask_.chunk(i)) 
                & lhs.mask_.chunk(i))) {
            return false;
        }
    }

    return true;
}

#endif
