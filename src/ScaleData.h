/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do notes that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/
#ifndef LATTICES_SCALEDATA_H
#define LATTICES_SCALEDATA_H

#pragma once

#include <utility>
//==============================================================================
namespace lattices::scaledata
{
// ratio and coordinate types
typedef std::pair<uint64_t, uint64_t> frac_t;
typedef std::pair<int, int> coord_t;

// The baseline from which we compute the ratios and
// lattice coordinates lattice of our 12-note scale
// is the pythagorean chain with 6 fifths up and 5 down.
// Here it is as ratios, fractions, and coordinates:
static constexpr double pyth12ratios[12] = {
    1.0,           256.0 / 243.0, 9.0 / 8.0,    32.0 / 27.0, 81.0 / 64.0, 4.0 / 3.0,
    729.0 / 512.0, 3.0 / 2.0,     128.0 / 81.0, 27.0 / 16.0, 16.0 / 9.0,  243.0 / 128.0};
static constexpr frac_t pyth12fractions[12] = {{1, 1},    {256, 243}, {9, 8},     {32, 27},
                                               {81, 64},  {4, 3},     {729, 512}, {3, 2},
                                               {128, 81}, {27, 16},   {16, 9},    {243, 128}};
static constexpr coord_t pyth12coords[12] = {{0, 0}, {-5, 0}, {2, 0},  {-3, 0}, {4, 0},  {-1, 0},
                                             {6, 0}, {1, 0},  {-4, 0}, {3, 0},  {-2, 0}, {5, 0}};

// We get our Duodene scale, and any visitors from other
// dimensions, by offsetting the above ratios by one of
// the following commas.
// Do not change the order after 1.0
enum CommaNames
{
    none,
    syntonic,
    septimal,
    undecimal,
    tridecimal,
    septendecimal,
    novemdecimal,
    twentythree,
    twentynine,
    thirtyone,
    sevenoverfive,
    elevenoverten,
    thirteenoverten,
    thirteenovereleven,
    fourteenovereleven,
    fourteenoverthirteen,
};

// How a comma affects a scale degree depends if it sits east (major)
// or west (minor) on the pyth chain.
// The root note could be either, so eventually I'll make a
// feature that lets the user flip the first of these. kk
// todo: make the feature that lets you choose which it is.
static bool isDegreeMajor[12] = {false, false, true,  false, true,  false,
                                 true,  true,  false, true,  false, true};

// A comma defined by its name, its ratio as a fraction,
// that fraction and its reciprocal expressed as doubles
// (for major and minor offsets), and a coordinate offset:
struct comma_t
{
    constexpr comma_t()
    {
        commaname = none;
        fraction = {1, 1};
        majorRatio = 1.0;
        minorRatio = 1.0;
        coord = {0, 0};
        nameIndex = 0;
    }
    constexpr comma_t(CommaNames n, frac_t f, coord_t c = {-4, 1})
        : commaname(n), fraction(f), coord(c)
    {
        majorRatio = static_cast<double>(f.first) / static_cast<double>(f.second);
        minorRatio = 1.0 / majorRatio;
        nameIndex = static_cast<int>(n);
    }
    comma_t(const comma_t &) = default;
    comma_t(comma_t &&) noexcept = default;
    comma_t &operator=(const comma_t &) = default;
    comma_t &operator=(comma_t &&) noexcept = default;
    ~comma_t() noexcept = default;

    double getRatio(int degree) const { return isDegreeMajor[degree] ? majorRatio : minorRatio; }
    frac_t getFraction(int degree) const
    {
        if (isDegreeMajor[degree])
            return fraction;

        auto res = std::make_pair(fraction.second, fraction.first);
        return res;
    }
    frac_t getFraction(bool major) const
    {
        if (major)
            return fraction;

        auto res = std::make_pair(fraction.second, fraction.first);
        return res;
    }
    coord_t getCoord(int degree) const
    {
        if (isDegreeMajor[degree])
            return coord;

        coord_t res = {coord.first * -1, coord.second * -1};
        return res;
    }
    int nameIndex{};

  protected:
    CommaNames commaname;
    frac_t fraction;
    double majorRatio;
    double minorRatio;
    coord_t coord;
};
// Here's the commas I've included so far (though we are not yet using them all).
// Any questions about these choices, feel free to open a Github issue.
// To add a comma, add a name at the end of the enum, add an initializer with a
// name and ratio at the end of this array, add a pair of colors in LatticeColours.h,
// and add a control in VisitorsComponent.h
// Do not change the order after 1.0
// TODO: write a test that checks this against the enum
static constexpr comma_t commas[17] = {
    comma_t(),
    comma_t(syntonic, {80, 81}),
    comma_t(septimal, {64, 63}),
    comma_t(undecimal, {32, 33}),
    comma_t(tridecimal, {1024, 1053}),
    comma_t(septendecimal, {2176, 2187}),
    comma_t(novemdecimal, {512, 513}),
    comma_t(twentythree, {736, 729}),
    comma_t(twentynine, {256, 261}),
    comma_t(thirtyone, {248, 243}),
    comma_t(sevenoverfive, {3584, 3645}),
    comma_t(elevenoverten, {44, 45}),
    comma_t(thirteenoverten, {40, 39}),
    comma_t(thirteenovereleven, {352, 351}),
    comma_t(fourteenovereleven, {896, 891}),
    comma_t(fourteenoverthirteen, {1701, 1664}),
};

static constexpr comma_t defaultCommas[12] = {
    comma_t(),
    comma_t(syntonic, {80, 81}),
    comma_t(),
    comma_t(syntonic, {80, 81}),
    comma_t(syntonic, {80, 81}),
    comma_t(),
    comma_t(syntonic, {80, 81}),
    comma_t(),
    comma_t(syntonic, {80, 81}),
    comma_t(syntonic, {80, 81}),
    comma_t(syntonic, {80, 81}),
    comma_t(syntonic, {80, 81}),
};

struct ScaleData
{
    ScaleData(const std::string n, const int *v = nullptr) : ScaleName(n)
    {
        if (v)
        {
            // only used for streaming
            for (int d = 0; d < 12; ++d)
            {
                auto c = static_cast<CommaNames>(v[d]);
                setDegree(d, c);
            }
        }
        else
        {
            resetToDefault();
        }
    }

    virtual void setDegree(const int d, const CommaNames c)
    {
        CC[d] = commas[c];
        CT[d] = pyth12ratios[d] * CC[d].getRatio(d);
        CO[d] = pyth12coords[d];
        auto co = CC[d].getCoord(d);
        CO[d].first += co.first;
        CO[d].second += co.second;
    }

    void resetToDefault()
    {
        for (int d = 0; d < 12; ++d)
        {
            CC[d] = defaultCommas[d];
            CT[d] = pyth12ratios[d] * CC[d].getRatio(d);
            CO[d] = pyth12coords[d];
            auto c = CC[d].getCoord(d);
            CO[d].first += c.first;
            CO[d].second += c.second;
        }
    }

    void setName(const std::string n) { ScaleName = n; }
    std::string ScaleName = "";
    std::array<comma_t, 12> CC; // Current Commas
    std::array<double, 12> CT;  // Current Tuning
    std::array<coord_t, 12> CO; // Current Co-Ordinates
};

struct SyntonicData
{
    void calculateSteps(const int sx, const int sy)
    {
        std::cout << "calcluatilsdgn" << std::endl;
        for (int d = 0; d < 12; ++d)
        {
            CT[d] = 1.0;
            CO[d] = {0, 0};
        }

        auto sxit = sx;
        auto sxsi = sx > 0 ? 1 : -1;
        auto sxr = sx > 0 ? sRatio : 1 / sRatio;
        while (sxit != 0)
        {
            auto shit = (sxsi > 0) ? -1 : 0;
            auto c = ((sxit + shit) % 4 + 4) % 4;
            std::cout << c << std::endl;
            for (int r = 0; r < 3; ++r)
            {
                int d = fDRC(r, c);
                CT[d] *= sxr;
                CO[d].first += 4 * sxsi;
                CO[d].second += -1 * sxsi;
            }
            sxit -= sxsi;
        }

        auto syit = sy;
        auto sysi = sy > 0 ? 1 : -1;
        auto syr = sy > 0 ? dRatio : 1 / dRatio;
        while (syit != 0)
        {
            auto fuck = (sysi > 0) ? -1 : 0;
            auto r = ((fuck + syit) % 3 + 3) % 3;
            std::cout << r << std::endl;
            for (int c = 0; c < 4; ++c)
            {
                int d = fDRC(r, c);
                CT[d] *= syr;
                CO[d].second += 3 * sysi;
            }
            syit -= sysi;
        }
    }

    double getTuning(const int d) { return duoRatios[d] * CT[d]; }
    coord_t getCoord(const int d)
    {
        auto [cx, cy] = duoCoords[d];
        cx += CO[d].first;
        cy += CO[d].second;
        return {cx, cy};
    }

  protected:
    static constexpr double sRatio = 81.0 / 80.0;
    static constexpr double dRatio = 125.0 / 128.0;
    // int priorX{0}, priorY{0};

    std::array<double, 12> CT;  // Current Tuning offsets
    std::array<coord_t, 12> CO; // Current Co-Ordinates

    double duoRatios[12]{1.0,         16.0 / 15.0, 9.0 / 8.0, 6.0 / 5.0, 5.0 / 4.0, 4.0 / 3.0,
                         45.0 / 32.0, 3.0 / 2.0,   8.0 / 5.0, 5.0 / 3.0, 9.0 / 5.0, 15.0 / 8.0};
    coord_t duoCoords[12]{{0, 0}, {-1, -1}, {2, 0},  {1, -1}, {0, 1},  {-1, 0},
                          {2, 1}, {1, 0},   {0, -1}, {-1, 1}, {2, -1}, {1, 1}};

    int DRC[3][4] = {
        {1, 8, 3, 10},
        {5, 0, 7, 2},
        {9, 4, 11, 6},
    };
    // find degree by row/column
    int fDRC(const int r, const int c) const { return DRC[r][c]; }
};

} // namespace lattices::scaledata
#endif // LATTICES_SCALEDATA_H