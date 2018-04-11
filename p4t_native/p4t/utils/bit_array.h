#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H


#include <p4t/utils/bits_aux.h>
#include <p4t/utils/hashing_aux.h>

#include <array>

namespace p4t::utils {

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
            bits_[i / BITS_PER_CHUNK] |= num::kth_bit<BitChunkT>::value(i % BITS_PER_CHUNK);
        } else {
            bits_[i / BITS_PER_CHUNK] &= ~(num::kth_bit<BitChunkT>::value(i % BITS_PER_CHUNK));
        }
    }

    auto get(size_t i) const -> bool {
        return num::testz(
            bits_[i / BITS_PER_CHUNK] & (num::kth_bit<BitChunkT>::value(i % BITS_PER_CHUNK))
        );
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
inline auto operator&(
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
inline auto operator-(
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
inline auto operator~(PackedBitArray<BitChunkT, WidthT> const& arr)
        -> PackedBitArray<BitChunkT, WidthT> {
    PackedBitArray<BitChunkT, WidthT> result{};
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        result.chunk(i) = ~arr.chunk(i); 
    }

    return result;
}

template<class BitChunkT, size_t WidthT> 
inline auto operator^(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs)
        -> PackedBitArray<BitChunkT, WidthT> {
    PackedBitArray<BitChunkT, WidthT> result{};
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        result.chunk(i) = lhs.chunk(i) ^ rhs.chunk(i); 
    }

    return result;
}

template<class BitChunkT, size_t WidthT> 
inline auto operator|(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs)
        -> PackedBitArray<BitChunkT, WidthT> {
    PackedBitArray<BitChunkT, WidthT> result{};
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        result.chunk(i) = lhs.chunk(i) | rhs.chunk(i); 
    }

    return result;
}



template<class BitChunkT, size_t WidthT> 
inline auto is_zero(PackedBitArray<BitChunkT, WidthT> const& arr) -> bool {
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        if (!num::testz(arr.chunk(i))) {
            return false;
        }
    }
    return true;
}

template<class BitChunkT, size_t WidthT> 
inline auto operator==(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs) -> bool {
    return is_zero(lhs ^ rhs);
}

template<class BitChunkT, size_t WidthT> 
inline auto operator!=(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs) -> bool {
    return !(lhs == rhs);
}

template<class BitChunkT, size_t WidthT> 
inline auto operator<(
        PackedBitArray<BitChunkT, WidthT> const& lhs, 
        PackedBitArray<BitChunkT, WidthT> const& rhs) -> bool {
    for (auto i = 0u; i < PackedBitArray<BitChunkT, WidthT>::NUM_CHUNKS; i++) {
        if (num::less(lhs.chunk(i), rhs.chunk(i))) {
            return true;
        } 
        if (!num::testz(lhs.chunk(i) ^ rhs.chunk(i))) {
            return false;
        }
    }
    return false;
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
struct hash<p4t::utils::PackedBitArray<BitChunkT, WidthT>> {
    size_t operator()(p4t::utils::PackedBitArray<BitChunkT, WidthT> const& pba) const {
        return p4t::utils::hash::hash_array(pba.bits_);
    }
};

}

#endif // BIT_ARRAY_H
