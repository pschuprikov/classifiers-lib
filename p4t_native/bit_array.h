#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

#include <array>

#include <boost/functional/hash.hpp>

namespace p4t {

template<class BitChunkT, size_t WidthT>
struct PackedBitArray {
    using BitChunk = BitChunkT;

    static auto constexpr WIDTH = WidthT;
    static auto constexpr BITS_PER_CHUNK = 8 * sizeof(BitChunk);
    static auto constexpr NUM_CHUNKS = (WIDTH + BITS_PER_CHUNK - 1) / BITS_PER_CHUNK;

public:
    PackedBitArray() = default;

    void set(size_t i, bool x) {
        if (x) {
            bits_[i / BITS_PER_CHUNK] |= (static_cast<BitChunk>(1) << (i % BITS_PER_CHUNK));
        } else {
            bits_[i / BITS_PER_CHUNK] &= ~(static_cast<BitChunk>(1) << (i % BITS_PER_CHUNK));
        }
    }

    auto get(size_t i) const -> bool {
        return bits_[i / BITS_PER_CHUNK] & (static_cast<BitChunk>(1) << (i % BITS_PER_CHUNK));
    }

    auto chunk(size_t i) const {
        return bits_[i];
    }

    auto& chunk(size_t i) {
        return bits_[i];
    }

    friend class std::hash<PackedBitArray>;

private:
    std::array<BitChunk, NUM_CHUNKS> bits_; 
};

template<class BitChunkT, size_t WidthT> 
inline auto get_intersection(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs)
        -> PackedBitArray<BitChunkT, WidthT> {
    PackedBitArray<BitChunkT, WidthT> result{};
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        result.chunk(i) = lhs.chunk(i) & rhs.chunk(i); 
    }

    return result;
}

template<class BitChunkT, size_t WidthT> 
inline auto get_diff(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs)
        -> PackedBitArray<BitChunkT, WidthT> {
    PackedBitArray<BitChunkT, WidthT> result{};
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        result.chunk(i) = lhs.chunk(i) & (~rhs.chunk(i)); 
    }

    return result;
}

template<class BitChunkT, size_t WidthT> 
inline auto is_zero(PackedBitArray<BitChunkT, WidthT> const& arr) -> bool {
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        if (arr.chunk(i)) {
            return false;
        }
    }
    return true;
}

template<class BitChunkT, size_t WidthT> 
inline auto operator==(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs) -> bool {
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        if (lhs.chunk(i) != rhs.chunk(i)) {
            return false;
        }
    }
    return true;
}

template<class BitChunkT, size_t WidthT>
inline auto popcount(PackedBitArray<BitChunkT, WidthT> const& arr) -> size_t {
    size_t result = 0;
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        result += __builtin_popcount(arr.chunk(i));
    }
    return result;
}

}

namespace std {

template<class BitChunkT, size_t WidthT>
struct hash<p4t::PackedBitArray<BitChunkT, WidthT>> {
    size_t operator()(p4t::PackedBitArray<BitChunkT, WidthT> const& pba) const {
        return boost::hash<decltype(pba.bits_)>()(pba.bits_);
    }
};

}

#endif // BIT_ARRAY_H
