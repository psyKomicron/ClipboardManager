#pragma once

// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain.

namespace clip::murmur
{
    template<typename _Elem, typename _Traits>
    inline hash_result MurmurHash3::hash(std::basic_istream<_Elem, _Traits>& stream, const std::optional<uint64_t>& seed) const
    {
        constexpr uint64_t c1 = 0x87c37b91114253d5;
        constexpr uint64_t c2 = 0x4cf5ad432745937f;
        uint64_t h1 = seed.value_or(0);
        uint64_t h2 = seed.value_or(0);
        uint64_t k1{};
        uint64_t k2{};
        size_t dataSize{};

        // Read the stream as an array of uint64_t:
        while (read<_Elem, _Traits>(stream, k1, k2))
        {
            dataSize += 2 * (sizeof(uint64_t) / sizeof(uint8_t));
            rotate(k1, k2, h1, h2, c1, c2);
        }

        /*If the stream is not read through(EOF while reading one of the two uint64_t),
        * go back to where it failed and read the remaining char/uint8_t to construct the
        * tail.
        */
        if (stream.gcount() != 8 && stream.eof())
        {
            // Rewind the stream:
            stream.clear();
            stream.seekg(0 - stream.gcount(), std::ios_base::end);

            // Create the tail:
            std::vector<uint8_t> tailVect{};
            uint8_t c{};
            while (stream.read(reinterpret_cast<_Elem*>(&c), sizeof(c)))
            {
                dataSize++;
                tailVect.push_back(c);
            }

            // End of MurmurHash:
            k1 = 0;
            k2 = 0;
            tail(tailVect, k1, k2, h1, h2, c1, c2);
        }

        finalize(h1, h2, dataSize);

        return hash_result(h1, h2);
    }

    template<typename _Elem, concepts::is_iterable<_Elem> _Iterable>
    inline hash_result MurmurHash3::hash(const _Iterable& data, const uint64_t& seed) const
    {
        constexpr uint64_t c1 = 0x87c37b91114253d5;
        constexpr uint64_t c2 = 0x4cf5ad432745937f;
        const size_t nblocks = data.size() / 16;
        const uint64_t* blocks = (const uint64_t*)(&data[0]);
        uint64_t h1 = seed;
        uint64_t h2 = seed;

        for (size_t i = 0; i < nblocks; i++)
        {
            uint64_t k1 = blocks[i * 2 + 0];
            uint64_t k2 = blocks[i * 2 + 1];

            rotate(k1, k2, h1, h2, c1, c2);
        }

        uint64_t k1 = 0;
        uint64_t k2 = 0;
        tail(data, k1, k2, h1, h2, c1, c2);

        finalize(h1, h2, data.size());

        return hash_result(h1, h2);
    }


    template<concepts::is_iterable<uint8_t> T>
    inline void MurmurHash3::tail(T& tail, uint64_t& k1, uint64_t& k2, uint64_t& h1, uint64_t& h2, const uint64_t& c1, const uint64_t& c2) const
    {
        switch (tail.size())
        {
            case 15: 
                k2 ^= ((uint64_t)tail[14]) << 48;
                [[fallthrough]];
            case 14: 
                k2 ^= ((uint64_t)tail[13]) << 40;
                [[fallthrough]];
            case 13: 
                k2 ^= ((uint64_t)tail[12]) << 32;
                [[fallthrough]];
            case 12: 
                k2 ^= ((uint64_t)tail[11]) << 24;
                [[fallthrough]];
            case 11: 
                k2 ^= ((uint64_t)tail[10]) << 16;
                [[fallthrough]];
            case 10: 
                k2 ^= ((uint64_t)tail[9]) << 8;
                [[fallthrough]];
            case  9: 
                k2 ^= ((uint64_t)tail[8]) << 0;
                k2 *= c2; 
                k2 = _rotl64(k2, 33); 
                k2 *= c1; 
                h2 ^= k2;
                [[fallthrough]];
            case  8: 
                k1 ^= ((uint64_t)tail[7]) << 56;
                [[fallthrough]];
            case  7: 
                k1 ^= ((uint64_t)tail[6]) << 48;
                [[fallthrough]];
            case  6: 
                k1 ^= ((uint64_t)tail[5]) << 40;
                [[fallthrough]];
            case  5:
                k1 ^= ((uint64_t)tail[4]) << 32;
                [[fallthrough]];
            case  4:
                k1 ^= ((uint64_t)tail[3]) << 24;
                [[fallthrough]];
            case  3: 
                k1 ^= ((uint64_t)tail[2]) << 16;
                [[fallthrough]];
            case  2: 
                k1 ^= ((uint64_t)tail[1]) << 8;
                [[fallthrough]];
            case  1: 
                k1 ^= ((uint64_t)tail[0]) << 0;
                k1 *= c1; 
                k1 = _rotl64(k1, 31); 
                k1 *= c2; 
                h1 ^= k1;
        }
    }

    template<typename _Elem, typename _Traits>
    inline bool MurmurHash3::read(std::basic_istream<_Elem, _Traits>& stream, uint64_t& k1, uint64_t& k2) const
    {
        return stream.read(reinterpret_cast<_Elem*>(&k1), sizeof(k1))
            && stream.read(reinterpret_cast<_Elem*>(&k2), sizeof(k2));
    }

    inline void MurmurHash3::rotate(uint64_t& k1, uint64_t& k2, uint64_t& h1, uint64_t& h2, const uint64_t& c1, const uint64_t& c2) const
    {
        k1 *= c1;
        k1 = _rotl64(k1, 31);
        k1 *= c2;
        h1 ^= k1;

        h1 = _rotl64(h1, 27);
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = _rotl64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

        h2 = _rotl64(h2, 31);
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    inline constexpr void MurmurHash3::finalize(uint64_t& h1, uint64_t& h2, const size_t& dataSize) const
    {
        h1 ^= dataSize; 
        h2 ^= dataSize;

        h1 += h2;
        h2 += h1;

        h1 = fmix64(h1);
        h2 = fmix64(h2);

        h1 += h2;
        h2 += h1;
    }

    inline constexpr uint64_t MurmurHash3::fmix64(uint64_t k) const
    {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccd;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53;
        k ^= k >> 33;

        return k;
    }
}