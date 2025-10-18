//
// Created by Andreya Ek Frisk on 2025-10-10.
//

#ifndef LATTICESCOLOURS_H
#define LATTICESCOLOURS_H

#include <utility>

namespace lattices::colours
{
using JC_t = juce::Colour;
using JCG_t = juce::ColourGradient;

static std::vector<std::pair<JC_t, JC_t>> basic = {
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.35f, .75f, .98f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.6916667f, .97f, .76f, 1.f}, JC_t{.4361111f, 1.f, .59f, .61f}),
    std::make_pair(JC_t{.2888889f, .97f, .67f, 1.f}, JC_t{.6194444f, .71f, 1.f, .25f}),
    std::make_pair(JC_t{0.f, .84f, 1.f, 1.f}, JC_t{.961111f, .79f, .41f, .25f})};

static std::vector<std::pair<JC_t, JC_t>> comma = {
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.35f, .75f, .98f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.8138889f, 1.f, .8f, 1.f}, JC_t{.6166667, 1.f, .8f, .1f}),
    std::make_pair(JC_t{.15f, 1.f, 1.f, 1.f}, JC_t{0.f, .84f, 1.f, .02f}),
    std::make_pair(JC_t{0.f, 1.f, 1.f, 1.f}, JC_t{.6888889f, 1.f, .96f, .38f}),
    std::make_pair(JC_t{.4722222f, 1.f, .51f, 1.f}, JC_t{.1666667f, 1.f, .8f, .71f}),
    std::make_pair(JC_t{0.f, .84f, 1.f, .02f}, JC_t{.7361111f, 1.f, 1.f, 1.f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
    std::make_pair(JC_t{.5f, .51f, .3f, 1.f}, JC_t{.5277778f, .79f, .41f, .25f}),
};

struct GradientProvider
{
    GradientProvider(int r) : radius(r), size(r * 2.f) {}

    void setSize(int r)
    {
        radius = r;
        size = r * 2.f;
    }

    JCG_t commaGrad(int idx, float p)
    {
        juce::Rectangle a{p - radius, p + radius, size, size};

        return JCG_t::horizontal(comma[idx].first, comma[idx].second, a);
    }

    JCG_t latticeGrad(int row, float p)
    {
        juce::Rectangle a{p - radius, p + radius, size, size};
        auto r = std::abs(row % 3);
        return JCG_t::horizontal(basic[r].first, basic[r].second, a);
    }
    JCG_t rootGrad(float p)
    {
        juce::Rectangle a{p - radius, p + radius, size, size};
        return JCG_t::horizontal(basic[4].first, basic[4].second, a);
    }

  protected:
    int radius;
    float size;
};
} // namespace lattices::colours
#endif // LATTICESCOLOURS_H
