/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#ifndef LATTICES_JIMATH_H
#define LATTICES_JIMATH_H

#include <utility>
#include <string>
#include <cmath>

struct JIMath
{
    uint64_t horizNum{3}, horizDen{2}, diagNum{5}, diagDen{4};

    JIMath(uint64_t horizN = 3, uint64_t horizD = 2, uint64_t diagN = 5, uint64_t diagD = 4)
        : horizNum(horizN), horizDen(horizD), diagNum(diagN), diagDen(diagD)
    {
    }

    // Maybe move these to the tuning library on Tones one day?
    // 3/2 up by 3/2 is 9/4
    static std::pair<uint64_t, uint64_t> multiplyRatio(uint64_t N1, uint64_t D1, uint64_t N2,
                                                       uint64_t D2)
    {
        auto nR = N1 * N2;
        auto dR = D1 * D2;

        // Take out octaves until we are within [1,2]
        while (nR > 2 * dR)
        {
            dR *= 2;
        }
        return {nR, dR};
    }

    static std::pair<uint64_t, uint64_t> divideRatio(uint64_t N1, uint64_t D1, uint64_t N2,
                                                     uint64_t D2)
    {
        auto nR = N1 * D2;
        auto dR = D1 * N2;

        // Add in octaves until we are within [1,2]
        while (nR < dR)
        {
            nR *= 2;
        }
        return {nR, dR};
    }

    inline void octaveReduceRatio(uint64_t &num, uint64_t &denom)
    {
        while (num < denom)
        {
            num *= 2;
        }
        while (num > denom * 2)
        {
            denom *= 2;
        }
    }

    // Monzo support

    static constexpr int limit = 9;
    typedef int monzo[limit];

    static constexpr int primes[limit] = {2, 3, 5, 7, 11, 13, 17, 19, 23};

    inline void monzoToRatio(monzo m, uint64_t &num, uint64_t &denom)
    {
        for (int i = 0; i < limit; ++i)
        {
            if (m[i] > 0)
            {
                num *= m[i] * primes[i];
            }
            else
            {
                denom *= -1 * m[i] * primes[i];
            }
        }
    }

    inline void ratioToMonzo(const uint64_t num, const uint64_t denom, monzo &m)
    {
        auto n = num;
        auto d = denom;

        for (int i = 0; i < limit; ++i)
        {
            while (n % primes[i] == 0)
            {
                m[i]++;
                n /= primes[i];
            }
            while (d % primes[i] == 0)
            {
                m[i]--;
                d /= primes[i];
            }
        }
    }

    inline void octaveReduceMonzo(monzo &m)
    {
        uint64_t n{1}, d{1};

        monzoToRatio(m, n, d);
        octaveReduceRatio(n, d);
        ratioToMonzo(n, d, m);
    }
};

#endif // JI_MTS_SOURCE_JIMATH_H
