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
        : horizNum(horizN), horizDen(horizD), diagNum(diagN), diagDen(horizD)
    {
    }

    static constexpr int limit = 9;
    typedef int monzo[limit];

    static constexpr int primes[limit] = {2, 3, 5, 7, 11, 13, 17, 19, 23};
    static constexpr std::pair<uint64_t, uint64_t> Commas[15] = {
        {531441, 524288}, // 3
        {80, 81},         // 5
        {125, 128},       // 5
        {64, 63},         // 7
        {32, 33},         // 11
        {1024, 1053},     // 13
        {2176, 2187},     // 17
        {512, 513},       // 19
        {352, 351},       // 13/11
        {896, 891},       // 14/11
        {40, 39},         // 13/10
        {3584, 3645},     // 7/5
        {1701, 1664},     // 14/13
        {44, 45},         // 11/10
        {736, 729},       // 23
    };

    enum Comma_t
    {
        pyth,
        syntonic,
        diesis,
        seven,
        eleven,
        thirteen,
        seventeen,
        nineteen,
        twentythree
    };

    double comma(Comma_t c, bool major = true)
    {
        uint64_t A = 1;
        uint64_t B = 1;

        switch (c)
        {
        case pyth:
            A = Commas[0].first;
            B = Commas[0].second;
            break;
        case syntonic:
            A = Commas[1].first;
            B = Commas[1].second;
            break;
        case diesis:
            A = Commas[2].first;
            B = Commas[2].second;
            break;
        case seven:
            A = Commas[3].first;
            B = Commas[3].second;
            break;
        case eleven:
            A = Commas[4].first;
            B = Commas[4].second;
            break;
        case thirteen:
            A = Commas[5].first;
            B = Commas[5].second;
            break;
        case seventeen:
            A = Commas[6].first;
            B = Commas[6].second;
            break;
        case nineteen:
            A = Commas[7].first;
            B = Commas[7].second;
            break;
        case twentythree:
            A = Commas[8].first;
            B = Commas[8].second;
            break;
        }

        return (major) ? (double)A / B : (double)B / A;
    }

    // Maybe move these to the tuning library on Tones one day?
    // 3/2 up by 3/2 is 9/4
    std::pair<uint64_t, uint64_t> multiplyRatio(uint64_t N1, uint64_t D1, uint64_t N2, uint64_t D2)
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

    std::pair<uint64_t, uint64_t> divideRatio(uint64_t N1, uint64_t D1, uint64_t N2, uint64_t D2)
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

    void monzoToRatio(monzo m, uint64_t &num, uint64_t &denom)
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

    inline void octaveReduceMonzo(monzo &m)
    {
        uint64_t n{1}, d{1};

        monzoToRatio(m, n, d);
        octaveReduceRatio(n, d);
        ratioToMonzo(n, d, m);
    }

    // ============ Note Name Support

    std::string nameNoteOnLattice(int x, int y)
    {
        auto location = x + y * 4 + 3;

        auto ml = ((location % 7) + 7) % 7;
        std::string name = noteNames[ml];

        while (location >= 7)
        {
            name += "#";
            location -= 7;
        }
        while (location < 0)
        {
            name += "b";
            location += 7;
        }
        auto pom = y;
        while (pom > 0)
        {
            name += "-";
            --pom;
        }
        while (pom < 0)
        {
            name += "+";
            ++pom;
        }

        return name;
    }

  private:
    std::string noteNames[7] = {"F", "C", "G", "D", "A", "E", "B"};
};

#endif // JI_MTS_SOURCE_JIMATH_H
