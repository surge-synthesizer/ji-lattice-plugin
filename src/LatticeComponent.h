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
#include "LatticesAssets.h"

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
        
        juce::Image litSpheres{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image unlitSpheres{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image sphereBackgrounds{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image litLines{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image unlitLines{juce::Image::ARGB, getWidth(), getHeight(), true};
        {
            juce::Graphics lS(litSpheres);
            juce::Graphics uS(unlitSpheres);
            juce::Graphics sB(sphereBackgrounds);
            juce::Graphics lL(litLines);
            juce::Graphics uL(unlitLines);
            for (int v = -nV - 1; v < nV + 1; ++v)
            {
                float off = v * hDistance * 0.5f;
                float y = -v * vDistance + ctrH;
                if (y < 0 || y > getHeight())
                    continue;

                for (int w = -nW-1; w < nW + 1; ++w)
                {
                    float x = w * hDistance + ctrX + off;

                    if (x < 0 || x > getWidth())
                        continue;

                    bool sphereLit{false}, horizLit{false}, upLit{false}, downLit{false};
                    std::pair<int, int> C = {w, v}; // current sphere
                    for (int i = 0; i < 12; ++i)
                    {
                        if (C == CoO[i])
                        {
                            sphereLit = true;
                            break;
                        }
                    }
                    if (sphereLit)
                    {
                        std::pair<int, int> H = {w + 1, v}; // next one over
                        std::pair<int, int> U = {w, v + 1}; // next one up
                        std::pair<int, int> D = {w + 1, v - 1}; // next one down
                        for (int i = 0; i < 12; ++i)
                        {
                            if (H == CoO[i]) horizLit = true;
                            if (U == CoO[i]) upLit = true;
                            if (D == CoO[i]) downLit = true;
                        }
                    }
                    
                    // Horizontal Line
                    if (horizLit)
                    {
                        lL.setColour(juce::Colours::white.withAlpha(1.f));
                        juce::Line<float> horiz(x, y, x + hDistance, y);
                        lL.drawLine(horiz, 3.f);
                    }
                    else
                    {
                        uL.setColour(juce::Colours::white.withAlpha(.75f));
                        juce::Line<float> horiz(x, y, x + hDistance, y);
                        uL.drawLine(horiz, 3.f);
                    }
                    
                    // Upward Line
                    if (upLit)
                    {
                        lL.setColour(juce::Colours::white.withAlpha(1.f));
                        juce::Line<float> up(x, y, x + (hDistance * .5f), y - vDistance);
                        float l[2] = {7.f, 3.f};
                        lL.drawDashedLine(up, l, 2, 3.f, 1);
                    }
                    else
                    {
                        uL.setColour(juce::Colours::white.withAlpha(.75f));
                        juce::Line<float> up(x, y, x + (hDistance * .5f), y - vDistance);
                        float l[2] = {7.f, 3.f};
                        uL.drawDashedLine(up, l, 2, 3.f, 1);
                    }

                    // Downward Line
                    if (downLit)
                    {
                        lL.setColour(juce::Colours::white.withAlpha(1.f));
                        juce::Line<float> down(x, y, x + (hDistance * .5f), y + vDistance);
                        float l[2] = {2.f, 3.f};
                        lL.drawDashedLine(down, l, 2, 3.f, 1);
                    }
                    else
                    {
                        uL.setColour(juce::Colours::white.withAlpha(.75f));
                        juce::Line<float> down(x, y, x + (hDistance * .5f), y + vDistance);
                        float l[2] = {2.f, 3.f};
                        uL.drawDashedLine(down, l, 2, 3.f, 1);
                    }
                    
                    // Sphere Backgrounds
                    auto ellipseRadius = JIRadius * 1.15;
                    
                    auto gradient = juce::ColourGradient{};
                     // Select gradient colour
                    if ((w + (v * 4)) % 12 == 0)
                    {
                        gradient = juce::ColourGradient(com1, x - ellipseRadius, y,
                                                        com2, x + ellipseRadius, y, false);
                    }
                    else if (v == 0)
                    {
                        gradient = juce::ColourGradient(p1, x - ellipseRadius, y,
                                                        p2, x + ellipseRadius, y, false);
                    }
                    else if (v == 1 || v == - 1)
                    {
                        gradient = juce::ColourGradient(l1c1, x - ellipseRadius, y,
                                                        l1c2, x + ellipseRadius, y, false);
                    }
                    else if (v == 2 || v == - 2)
                    {
                        gradient = juce::ColourGradient(l2c1, x - ellipseRadius, y,
                                                        l2c2, x + ellipseRadius, y, false);
                    }
                    else if (v == 3 || v == - 3)
                    {
                        gradient = juce::ColourGradient(l3c1, x - ellipseRadius, y,
                                                        l3c2, x + ellipseRadius, y, false);
                    }
                    else
                    {
                        gradient = juce::ColourGradient(l4c1, x - ellipseRadius, y,
                                                        l4c2, x + ellipseRadius, y, false);
                    }
                    
                    // Names or Ratios?
 //                    auto [n,d] = calculateCell(w, v);
 //                    auto s = std::to_string(n) + "/" + std::to_string(d);
                    std::string s = jim.nameNoteOnLattice(w,v);
                    // Spheres
                    juce::Path e{};
                    e.addEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius);
                    // And their shadows
                    juce::Path b{};
                    b.addEllipse(x - ellipseRadius - 1.5, y - JIRadius - 1.5, 2 * ellipseRadius + 3, 2 * JIRadius + 3);
                    
                    
                    if (sphereLit)
                    {
                        whiteShadow.render(lS, e);
                        blackShadow.render(lS, b);
                        lS.setColour(juce::Colours::black);
                        lS.fillPath(b);
                        gradient.multiplyOpacity(1.f);
                        lS.setGradientFill(gradient);
                        lS.fillPath(e);
                        lS.setColour(juce::Colours::white);
                        lS.drawEllipse(x - ellipseRadius,y - JIRadius, 2 * ellipseRadius, 2 * JIRadius, 3);
                        lS.setFont(stoke);
                        lS.drawFittedText(s, x - ellipseRadius + 3, y - (JIRadius / 3.f) , 2.f * (ellipseRadius - 3), .66667f * JIRadius, juce::Justification::horizontallyCentred, 1, 0.05f);
                    }
                    else
                    {
                        whiteShadow.render(uS, e);
                        blackShadow.render(uS, b);
                        uS.setColour(juce::Colours::black);
                        uS.fillPath(b);
                        gradient.multiplyOpacity(.75f);
                        uS.setGradientFill(gradient);
                        uS.fillPath(e);
                        uS.setColour(juce::Colours::white.withAlpha(.75f));
                        uS.drawEllipse(x - ellipseRadius,y - JIRadius, 2 * ellipseRadius, 2 * JIRadius, 3);
//                        auto pst = juce::PathStrokeType(3.f);
//                        uS.strokePath(e, pst);
                        uS.setFont(stoke);
                        uS.drawFittedText(s, x - ellipseRadius + 3, y - (JIRadius / 3.f) , 2.f * (ellipseRadius - 3), .66667f * JIRadius, juce::Justification::horizontallyCentred, 1, 0.05f);
                    }
                }
            }
        }
        g.drawImageAt(blur.render(unlitLines), 0, 0, false);
        g.drawImageAt(litLines, 0, 0, false);
        g.drawImageAt(blur.render(unlitSpheres), 0, 0, false);
        g.drawImageAt(litSpheres, 0, 0, false);
    }

    std::pair<uint64_t, uint64_t> calculateCell(int fifths, int thirds)
    {
        uint64_t n{1}, d{1};

        while(thirds > 0)
        {
            auto [nn,dd] = jim.multiplyRatio(n, d, 5, 4);
            n = nn;
            d = dd;
            thirds--;
        }

        while(thirds < 0)
        {
            auto [nn,dd] = jim.divideRatio(n, d, 5, 4);
            n = nn;
            d = dd;
            thirds++;
        }

        while(fifths > 0)
        {
            auto [nn,dd] = jim.multiplyRatio(n, d, 3, 2);
            n = nn;
            d = dd;
            fifths--;
        }

        while(fifths < 0)
        {
            auto [nn,dd] = jim.divideRatio(n, d, 3, 2);
            n = nn;
            d = dd;
            fifths++;
        }
        
        auto g = std::gcd(n,d);
        n = n / g;
        d = d / g;

        return {n,d};
    }
protected:
    static constexpr int JIRadius{26};
    JIMath jim;
    
    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{ juce::Typeface::createSystemTypefaceFor(LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};
    
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
    
    melatonin::CachedBlur blur{3};
    melatonin::DropShadow blackShadow = {juce::Colours::black, 8};
    melatonin::DropShadow whiteShadow = {juce::Colours::antiquewhite, 12};
    
    std::pair<int, int> CoO[12]
    {
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
        {0,0},
    }; // current co-ordinates
};


