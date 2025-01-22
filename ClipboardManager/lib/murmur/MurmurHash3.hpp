#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <istream>
#include <ranges>
#include <optional>
#include <winrt/Windows.Storage.Streams.h>

namespace clip::murmur::concepts
{
    template<typename T, typename _Elem>
    concept is_iterable = std::ranges::range<T> 
        && std::convertible_to<std::ranges::range_value_t<T>, _Elem>
        && requires(T t)
    {
        { t.size() } -> std::convertible_to<std::size_t>;
    };
}

namespace clip::murmur
{
    using hash_result = std::pair<uint64_t, uint64_t>;

    class MurmurHash3
    {
    public:
        MurmurHash3() = default;

        template<typename _Elem, typename _Traits>
        inline hash_result hash(std::basic_istream<_Elem, _Traits>& stream, const std::optional<uint64_t>& seed = {}) const;

        template<typename _Elem, concepts::is_iterable<_Elem> _Iterable>
        inline hash_result hash(const _Iterable& data, const uint64_t& seed) const;

    private:
        uint64_t seed = 0;

        template<concepts::is_iterable<uint8_t> T>
        inline void tail(T& tail, uint64_t& k1, uint64_t& k2, uint64_t& h1, uint64_t& h2, 
                         const uint64_t& c1, const uint64_t& c2) const;
        /*inline void tail(const uint8_t* tail, const std::size_t& size, uint64_t& k1, uint64_t& k2, 
                         uint64_t& h1, uint64_t& h2, const uint64_t& c1, const uint64_t& c2) const;*/

        template<typename _Elem, typename _Traits>
        inline bool read(std::basic_istream<_Elem, _Traits>& stream, uint64_t& k1, uint64_t& k2) const;

        inline void rotate(uint64_t& k1, uint64_t& k2, uint64_t& h1, uint64_t& h2, const uint64_t& c1, const uint64_t& c2) const;
        inline constexpr uint64_t fmix64(uint64_t k) const;
        inline constexpr void finalize(uint64_t& h1, uint64_t& h2, const size_t& dataSize) const;
    };
}

#include "lib/implementation/murmurhash3_implementation.hpp"