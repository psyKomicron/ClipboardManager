//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#pragma once
#include <stdint.h>
#include <utility>
#include <stdlib.h>
#include <vector>
#include <fstream>

namespace clip::murmur
{
    using hash = std::pair<uint64_t, uint64_t>;

    inline uint64_t fmix64(uint64_t k)
    {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccd;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53;
        k ^= k >> 33;

        return k;
    }

    hash MurmurHash3_x64_128(std::ifstream& stream, uint64_t seed)
    {
        uint64_t h1 = seed;
        uint64_t h2 = seed;

        const uint64_t c1 = 0x87c37b91114253d5;
        const uint64_t c2 = 0x4cf5ad432745937f;

        //----------
        // body

        uint64_t k1{};
        uint64_t k2{};
        size_t dataSize{};

        // Read the stream as an array of uint64_t:
        while (stream.read(reinterpret_cast<char*>(&k1), sizeof(k1)) 
               && stream.read(reinterpret_cast<char*>(&k2), sizeof(k2)))
        {
            dataSize += 2 * (sizeof(uint64_t) / sizeof(uint8_t));

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
            std::vector<uint8_t> tail{};
            uint8_t c{};
            while (stream.read(reinterpret_cast<char*>(&c), sizeof(c)))
            {
                dataSize++;
                tail.push_back(c);
            }

            // End of MurmurHash:

            k1 = 0;
            k2 = 0;

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
                    k2 *= c2; k2 = _rotl64(k2, 33); k2 *= c1; h2 ^= k2;
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
                    k1 *= c1; k1 = _rotl64(k1, 31); k1 *= c2; h1 ^= k1;
            }
        }

        //----------
        // finalization

        h1 ^= dataSize; 
        h2 ^= dataSize;

        h1 += h2;
        h2 += h1;

        h1 = fmix64(h1);
        h2 = fmix64(h2);

        h1 += h2;
        h2 += h1;

        return hash(h1, h2);
    }
}