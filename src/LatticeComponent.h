/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include "JIMath.h"
#include "LatticesBinary.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <climits>

#include <melatonin_blur/melatonin_blur.h>

//==============================================================================
struct LatticeComponent : juce::Component
{
    LatticeComponent(std::pair<int, int> *c)
    {
        for (int i = 0; i < 12; ++i)
        {
            CoO[i] = c[i];
        }
    }

    void update(std::pair<int, int> *c, int *v)
    {
        for (int i = 0; i < 12; ++i)
        {
            CoO[i] = c[i];
            visitor[i] = v[i];
        }
    }

    void paint(juce::Graphics &g) override
    {
        float ctrDistance{JIRadius * (5.f / 3.f)};

        float vDistance = 2.0f * ctrDistance;
        float hDistance = 2.0f * ctrDistance;

        auto ctrX = getWidth() / 2;
        auto ctrH = getHeight() / 2;

        auto nV = std::ceil(getHeight() / vDistance);
        auto nW = std::ceil(getWidth() / hDistance);

        juce::Image Lines{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image Spheres{juce::Image::ARGB, getWidth(), getHeight(), true};
        {
            juce::Graphics lG(Lines);
            juce::Graphics sG(Spheres);
            for (int v = -nV - 1; v < nV + 1; ++v)
            {
                float off = v * hDistance * 0.5f;
                float y = -v * vDistance + ctrH;
                if (y < 0 || y > getHeight())
                    continue;

                for (int w = -nW - 1; w < nW + 1; ++w)
                {
                    float x = w * hDistance + ctrX + off;

                    if (x < 0 || x > getWidth())
                        continue;

                    std::pair<int, int> C = {w, v};         // current sphere
                    std::pair<int, int> H = {w + 1, v};     // next one over
                    std::pair<int, int> U = {w, v + 1};     // next one up
                    std::pair<int, int> D = {w + 1, v - 1}; // next one down

                    // ok, so how far is this sphere from a lit up one?
                    auto dist = calcDist(C);
                    int degree{0};
                    if (dist == 0)
                    {
                        for (int i = 0; i < 12; ++i)
                        {
                            if (CoO[i] == C)
                            {
                                degree = i;
                                break;
                            }
                        }
                    }

                    // and what about its lines?
                    auto hDist = std::max(dist, calcDist(H));
                    auto uDist = std::max(dist, calcDist(U));
                    auto dDist = std::max(dist, calcDist(D));
                    // those numbers will set this
                    float alpha{};

                    // Horizontal Line
                    alpha = 1.f / (std::sqrt(hDist) + 1);
                    lG.setColour(juce::Colours::white.withAlpha(alpha));
                    juce::Line<float> horiz(x, y, x + hDistance, y);
                    lG.drawLine(horiz, 3.f);

                    // Upward Line
                    alpha = 1.f / (std::sqrt(uDist) + 1);
                    lG.setColour(juce::Colours::white.withAlpha(alpha));
                    juce::Line<float> up(x, y, x + (hDistance * .5f), y - vDistance);
                    float ul[2] = {7.f, 3.f};
                    lG.drawDashedLine(up, ul, 2, 3.f, 1);

                    // Downward Line
                    alpha = 1.f / (std::sqrt(dDist) + 1);
                    lG.setColour(juce::Colours::white.withAlpha(alpha));
                    juce::Line<float> down(x, y, x + (hDistance * .5f), y + vDistance);
                    float dl[2] = {2.f, 3.f};
                    lG.drawDashedLine(down, dl, 2, 3.f, 1);

                    auto ellipseRadius = JIRadius * 1.15;

                    // Spheres
                    juce::Path e{};
                    e.addEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius);
                    // And their shadows
                    juce::Path b{};
                    b.addEllipse(x - ellipseRadius - 1.5, y - JIRadius - 1.5, 2 * ellipseRadius + 3,
                                 2 * JIRadius + 3);

                    // Select gradient colour
                    bool uni = ((w + (v * 4)) % 12 == 0) ? true : false;
                    auto gradient =
                        chooseColour(std::abs(v), x, y, (dist == 0), visitor[degree], uni);

                    alpha = 1.f / (std::sqrt(dist) + 1);
                    whiteShadow.setOpacity(alpha);
                    whiteShadow.render(sG, e);
                    blackShadow.setOpacity(alpha);
                    blackShadow.render(sG, b);
                    sG.setColour(juce::Colours::black);
                    sG.fillPath(b);
                    gradient.multiplyOpacity(alpha);
                    sG.setGradientFill(gradient);
                    sG.fillPath(e);
                    sG.setColour(juce::Colours::white.withAlpha(alpha));
                    sG.drawEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius,
                                   3);

                    // Names or Ratios?
                    auto [n, d] = calculateCell(w, v);
                    if (dist == 0 && visitor[degree] > 1)
                    {
                        reCalculateCell(n, d, visitor[degree], degree);
                    }
                    auto s = std::to_string(n) + "/" + std::to_string(d);
                    // std::string s = jim.nameNoteOnLattice(w, v);
                    sG.setFont(stoke);
                    sG.drawFittedText(s, x - ellipseRadius + 3, y - (JIRadius / 3.f),
                                      2.f * (ellipseRadius - 3), .66667f * JIRadius,
                                      juce::Justification::horizontallyCentred, 1, 0.05f);
                }
            }
        }
        g.drawImageAt(Lines, 0, 0, false);
        g.drawImageAt(Spheres, 0, 0, false);
    }

    int visitor[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1};

  protected:
    static constexpr int JIRadius{26};
    JIMath jim;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(JIRadius)};

    melatonin::DropShadow blackShadow = {juce::Colours::black, 8};
    melatonin::DropShadow whiteShadow = {juce::Colours::antiquewhite, 12};

    std::array<std::pair<int, int>, 12> CoO{}; // currently lit co-ordinates

    inline int calcDist(std::pair<int, int> xy) // how far is a given coordinate from those?
    {
        int res{INT_MAX};

        for (int i = 0; i < 12; ++i)
        {
            int tx = std::abs(xy.first - CoO[i].first);
            int ty = std::abs(xy.second - CoO[i].second);
            int sum = tx + ty;
            if (sum < res)
                res = sum;
        }

        return res;
    }

    const juce::Colour com1{0.f, .84f, 1.f, 1.f}, com2{.961111f, .79f, .41f, .25f},
        py1{.5f, .51f, .3f, 1.f}, py2{.5277778f, .79f, .41f, .25f}, l1c1{.35f, .75f, .98f, 1.f},
        l1c2{.5277778f, .79f, .41f, .25f}, l2c1{.2888889f, .97f, .67f, 1.f},
        l2c2{.6194444f, .71f, 1.f, .25f}, l3c1{.6916667f, .97f, .76f, 1.f},
        l3c2{.4361111f, 1.f, .59f, .61f}, l4c1{.5777778f, .97f, .94f, 58.f},
        l4c2{.8666667f, 1.f, .36f, 1.f}, sep1{.8138889f, 1.f, .8f, 1.f},
        sep2{.6166667, 1.f, .8f, .1f}, und1{.15f, 1.f, 1.f, 1.f}, und2{0.f, .84f, 1.f, .02f},
        trid1{0.f, 1.f, 1.f, 1.f}, trid2{.6888889f, 1.f, .96f, .38f},
        sed1{.4722222f, 1.f, .51f, 1.f}, sed2{.1666667f, 1.f, .8f, .71f},
        nod1{0.f, .84f, 1.f, .02f}, nod2{.7361111f, 1.f, 1.f, 1.f};

    juce::ColourGradient chooseColour(int row, float x, float y, bool lit, int visitor,
                                      bool unison = false)
    {
        auto ellipseRadius = JIRadius * 1.15;

        if (unison)
        {
            return juce::ColourGradient(com1, x - ellipseRadius, y, com2, x + ellipseRadius, y,
                                        false);
        }

        if (lit)
        {
            switch (visitor)
            {
            case 0:
                break;
            case 1:
                break;
            case 2:
                return juce::ColourGradient(sep1, x - ellipseRadius, y, sep2, x + ellipseRadius, y,
                                            false);
            case 3:
                return juce::ColourGradient(und1, x - ellipseRadius, y, und2, x + ellipseRadius, y,
                                            false);
            case 4:
                return juce::ColourGradient(trid1, x - ellipseRadius, y, trid2, x + ellipseRadius,
                                            y, false);
            case 5:
                return juce::ColourGradient(sed1, x - ellipseRadius, y, sed2, x + ellipseRadius, y,
                                            false);
            case 6:
                return juce::ColourGradient(nod1, x - ellipseRadius, y, nod2, x + ellipseRadius, y,
                                            false);
            default:
                break;
            }
        }

        switch (row)
        {
        case 0:
            return juce::ColourGradient(py1, x - ellipseRadius, y, py2, x + ellipseRadius, y,
                                        false);
        case 1:
            return juce::ColourGradient(l1c1, x - ellipseRadius, y, l1c2, x + ellipseRadius, y,
                                        false);
        case 2:
            return juce::ColourGradient(l2c1, x - ellipseRadius, y, l2c2, x + ellipseRadius, y,
                                        false);
        case 3:
            return juce::ColourGradient(l3c1, x - ellipseRadius, y, l3c2, x + ellipseRadius, y,
                                        false);
        default:
            return juce::ColourGradient(l4c1, x - ellipseRadius, y, l4c2, x + ellipseRadius, y,
                                        false);
        }
    }

    inline std::pair<uint64_t, uint64_t> calculateCell(int fifths, int thirds)
    {
        uint64_t n{1}, d{1};

        while (thirds > 0)
        {
            auto [nn, dd] = jim.multiplyRatio(n, d, 5, 4);
            n = nn;
            d = dd;
            thirds--;
        }

        while (thirds < 0)
        {
            auto [nn, dd] = jim.divideRatio(n, d, 5, 4);
            n = nn;
            d = dd;
            thirds++;
        }

        while (fifths > 0)
        {
            auto [nn, dd] = jim.multiplyRatio(n, d, 3, 2);
            n = nn;
            d = dd;
            fifths--;
        }

        while (fifths < 0)
        {
            auto [nn, dd] = jim.divideRatio(n, d, 3, 2);
            n = nn;
            d = dd;
            fifths++;
        }

        auto g = std::gcd(n, d);
        n = n / g;
        d = d / g;

        return {n, d};
    }

    inline void reCalculateCell(uint64_t &n, uint64_t &d, int visitor, int degree)
    {
        int cidx = visitor + 1;

        uint64_t cn{1}, cd{1};

        if (degree == 7 || degree == 2 || degree == 9 || degree == 4 || degree == 11 || degree == 6)
        {
            n *= jim.Commas[1].second;
            d *= jim.Commas[1].first;
            auto g = std::gcd(n, d);
            n = n / g;
            d = d / g;
            cn = jim.Commas[cidx].first;
            cd = jim.Commas[cidx].second;
        }
        else
        {
            n *= jim.Commas[1].first;
            d *= jim.Commas[1].second;
            auto g = std::gcd(n, d);
            n = n / g;
            d = d / g;
            cn = jim.Commas[cidx].second;
            cd = jim.Commas[cidx].first;
        }

        auto [nn, nd] = jim.multiplyRatio(n, d, cn, cd);

        n = nn;
        d = nd;

        auto g = std::gcd(n, d);
        n = n / g;
        d = d / g;
    }
};
