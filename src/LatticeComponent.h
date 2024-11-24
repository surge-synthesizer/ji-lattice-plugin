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

    void update(std::pair<int, int> *c)
    {
        for (int i = 0; i < 12; ++i)
        {
            CoO[i] = c[i];
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
                    // and what about its lines?
                    auto hDist = std::max(dist, calcDist(H));
                    auto uDist = std::max(dist, calcDist(U));
                    auto dDist = std::max(dist, calcDist(D));
                    // those numbers will set this
                    float alpha{};

                    // Horizontal Line
                    alpha = 1.f / (std::powf(hDist, .5f) + 1);
                    lG.setColour(juce::Colours::white.withAlpha(alpha));
                    juce::Line<float> horiz(x, y, x + hDistance, y);
                    lG.drawLine(horiz, 3.f);

                    // Upward Line
                    alpha = 1.f / (std::powf(uDist, .5f) + 1);
                    lG.setColour(juce::Colours::white.withAlpha(alpha));
                    juce::Line<float> up(x, y, x + (hDistance * .5f), y - vDistance);
                    float ul[2] = {7.f, 3.f};
                    lG.drawDashedLine(up, ul, 2, 3.f, 1);

                    // Downward Line
                    alpha = 1.f / (std::powf(dDist, .5f) + 1);
                    lG.setColour(juce::Colours::white.withAlpha(alpha));
                    juce::Line<float> down(x, y, x + (hDistance * .5f), y + vDistance);
                    float dl[2] = {2.f, 3.f};
                    lG.drawDashedLine(down, dl, 2, 3.f, 1);

                    auto ellipseRadius = JIRadius * 1.15;
                    auto gradient = juce::ColourGradient{};
                    // Select gradient colour
                    if ((w + (v * 4)) % 12 == 0)
                    {
                        gradient = juce::ColourGradient(com1, x - ellipseRadius, y, com2,
                                                        x + ellipseRadius, y, false);
                    }
                    else if (v == 0)
                    {
                        gradient = juce::ColourGradient(p1, x - ellipseRadius, y, p2,
                                                        x + ellipseRadius, y, false);
                    }
                    else if (v == 1 || v == -1)
                    {
                        gradient = juce::ColourGradient(l1c1, x - ellipseRadius, y, l1c2,
                                                        x + ellipseRadius, y, false);
                    }
                    else if (v == 2 || v == -2)
                    {
                        gradient = juce::ColourGradient(l2c1, x - ellipseRadius, y, l2c2,
                                                        x + ellipseRadius, y, false);
                    }
                    else if (v == 3 || v == -3)
                    {
                        gradient = juce::ColourGradient(l3c1, x - ellipseRadius, y, l3c2,
                                                        x + ellipseRadius, y, false);
                    }
                    else
                    {
                        gradient = juce::ColourGradient(l4c1, x - ellipseRadius, y, l4c2,
                                                        x + ellipseRadius, y, false);
                    }

                    // Spheres
                    juce::Path e{};
                    e.addEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius);
                    // And their shadows
                    juce::Path b{};
                    b.addEllipse(x - ellipseRadius - 1.5, y - JIRadius - 1.5, 2 * ellipseRadius + 3,
                                 2 * JIRadius + 3);

                    alpha = 1.f / (std::powf(dist, .5f) + 1);
                    // draw those
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
                    // auto [n,d] = calculateCell(w, v);
                    // auto s = std::to_string(n) + "/" + std::to_string(d);
                    std::string s = jim.nameNoteOnLattice(w, v);
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

  protected:
    static constexpr int JIRadius{26};
    JIMath jim;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(JIRadius)};

    juce::Colour com1{0.f, .84f, 1.f, 1.f};
    juce::Colour com2{.961111f, .79f, .41f, .25f};

    juce::Colour p1{.5f, .51f, .3f, 1.f};
    juce::Colour p2{.5277778f, .79f, .41f, .25f};

    juce::Colour l1c1{.35f, .75f, .98f, 1.f};
    juce::Colour l1c2{.5277778f, .79f, .41f, .25f};

    juce::Colour l2c1{.2888889f, .97f, .67f, 1.f};
    juce::Colour l2c2{.6194444f, .71f, 1.f, .25f};

    juce::Colour l3c1{.6916667f, .97f, .76f, 1.f};
    juce::Colour l3c2{.4361111f, 1.f, .59f, .61f};

    juce::Colour l4c1{.5777778f, .97f, .94f, 58.f};
    juce::Colour l4c2{.8666667f, 1.f, .36f, 1.f};

    melatonin::DropShadow blackShadow = {juce::Colours::black, 8};
    melatonin::DropShadow whiteShadow = {juce::Colours::antiquewhite, 12};

    std::array<std::pair<int, int>, 12> CoO{}; // currently lit co-ordinates

    inline int calcDist(std::pair<int, int> xy) // how far is a given coordinate from those?
    {
        auto inside = std::find(CoO.begin(), CoO.end(), xy);
        int xDist{CoO[0].first};
        int yDist{CoO[0].second};

        if (inside != CoO.end()) // we're on a lit note
        {
            return 0;
        }
        else // if we aren't, find out how far
        {
            // find X distance first
            if (xy.first < CoO[5].first)
            {
                xDist = CoO[5].first - xy.first;
            }
            else if (xy.first > CoO[2].first)
            {
                xDist = xy.first - CoO[2].first;
            }
            else
            {
                xDist = 0; // We're over or under a lit note
                // we need to know which column we're in for Syntonic mode.
                auto x = xy.first;

                std::array<int, 3> ys;
                int it{0};

                for (int i = 0; i < 12; ++i)
                {
                    if (CoO[i].first == x)
                    {
                        ys[it] = CoO[i].second;
                        ++it;
                    }
                }
                // highest and lowest Y of this column
                int top = *std::max_element(ys.begin(), ys.end());
                int bottom = *std::min_element(ys.begin(), ys.end());

                // so now find y dist
                if (xy.second < bottom)
                {
                    yDist = bottom - xy.second;
                }
                else if (xy.second > top)
                {
                    yDist = xy.second - top;
                }

                auto res = std::max(xDist, yDist);
                return res;
            }

            // ok if we made it here we're off to the sides somewhere
            int top = CoO[4].second;
            int bottom = CoO[8].second;

            if (xy.second < bottom)
            {
                yDist = bottom - xy.second;
            }
            else if (xy.second > top)
            {
                yDist = xy.second - top;
            }
        }

        auto res = std::max(xDist, yDist);
        return res;
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
};
