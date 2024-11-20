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
        
        grid(g, ctrDistance, vDistance, hDistance,ctrX, ctrH);
        sphereBackgrounds(g, ctrDistance, vDistance, hDistance,ctrX, ctrH);

        auto nV = std::ceil(getHeight() / vDistance);
        auto nW = std::ceil(getWidth() / hDistance);
        
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

                bool lit{false};
                for (int i = 0; i < 12; ++i) // are we currently lit?
                {
                    std::pair<int, int> C = {w,v}; // current sphere

                    if (C == CoO[i])
                    {
                        lit = true;
                        break;
                    }
                }
                
                auto ellipseRadius = JIRadius * 1.15;
                
                auto alpha = (lit) ? 1.f : .5f;
                
                if ((w + (v * 4)) % 12 == 0)
                {
                    auto gradient = juce::ColourGradient(com1, x-(ellipseRadius), y,
                                                         com2, x+(ellipseRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 0)
                {
                    auto gradient = juce::ColourGradient(p1, x-(ellipseRadius), y,
                                                         p2, x+(ellipseRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 1 || v == - 1)
                {
                    auto gradient = juce::ColourGradient(l1c1, x-(ellipseRadius), y,
                                                         l1c2, x+(ellipseRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 2 || v == - 2)
                {
                    auto gradient = juce::ColourGradient(l2c1, x-(ellipseRadius), y,
                                                         l2c2, x+(ellipseRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 3 || v == - 3)
                {
                    auto gradient = juce::ColourGradient(l3c1, x-(ellipseRadius), y,
                                                         l3c2, x+(ellipseRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else
                {
                    auto gradient = juce::ColourGradient(l4c1, x-(ellipseRadius), y,
                                                         l4c2, x+(ellipseRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                g.fillEllipse(x - (ellipseRadius),y - JIRadius, 2 * ellipseRadius, 2 * JIRadius);
                
                g.setColour(juce::Colours::white.withAlpha(alpha));
                g.drawEllipse(x - ellipseRadius,y - JIRadius, 2 * ellipseRadius, 2 * JIRadius, 3);

//                auto [n,d] = calculateCell(w, v);
//                auto s = std::to_string(n) + "/" + std::to_string(d);
                
                
                
                std::string s = jim.nameNoteOnLattice(w,v);
                g.setFont(stoke);
                g.drawFittedText(s, x - ellipseRadius + 3, y - (JIRadius / 3.f) , 2.f * (ellipseRadius - 3), .66667f * JIRadius, juce::Justification::horizontallyCentred, 1, 0.05f);
            }
        }
    }
    
    void grid(juce::Graphics &g, float ctrDistance, float vDistance, float hDistance, float ctrX, float ctrH)
    {
        auto nV = std::ceil(getHeight() / vDistance);
        auto nW = std::ceil(getWidth() / hDistance);
        
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
                for (int i = 0; i < 12; ++i)
                {
                    std::pair<int, int> C = {w, v}; // current sphere
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
                
                juce::Image hLine{juce::Image::ARGB, (int)hDistance, (int)vDistance, true};
                {
                    juce::Graphics tg(hLine);
                    tg.setColour(juce::Colours::white);
                    juce::Line<float> horiz(0, vDistance * .5, hDistance, vDistance * .5);
                    tg.drawLine(horiz, 4.f);
                }
                
                if (horizLit)
                {
                    g.setOpacity(1.f);
                    g.drawImageAt(hLine, x, y - vDistance * .5, false);
                }
                else
                {
                    g.setOpacity(.5f);
                    g.drawImageAt(hBlur.render(hLine), x, y - vDistance * .5, false);
                }
                
                juce::Image uLine{juce::Image::ARGB, (int)hDistance, (int)vDistance, true};
                {
                    juce::Graphics tg(uLine);
                    tg.setColour(juce::Colours::white);
                    juce::Line<float> up(0, vDistance, hDistance * .5f, 0);
                    float l[2] = {7.f, 3.f};
                    tg.drawDashedLine(up, l, 2, 4.f, 1);
                }
                if (upLit)
                {
                    g.setOpacity(1.f);
                    g.drawImageAt(uLine, x, y - vDistance, false);
                }
                else
                {
                    g.setOpacity(.85f);
                    g.drawImageAt(uBlur.render(uLine), x, y - vDistance, false);
                }
                
                juce::Image dLine{juce::Image::ARGB, (int)hDistance, (int)vDistance, true};
                {
                    juce::Graphics tg(dLine);
                    tg.setColour(juce::Colours::white);
                    juce::Line<float> down(0, 0, hDistance * .5f, vDistance);
                    float le[2] = {2.f, 3.f};
                    tg.drawDashedLine(down, le, 2, 4.f, 1);
                }
                if (downLit)
                {
                    g.setOpacity(1.f);
                    g.drawImageAt(dLine, x, y, false);
                }
                else
                {
                    g.setOpacity(.85f);
                    g.drawImageAt(dBlur.render(dLine), x, y, false);
                }
            }
        }
    }
    
    void sphereBackgrounds(juce::Graphics &g, float ctrDistance, float vDistance, float hDistance, float ctrX, float ctrH)
    {
        auto nV = std::ceil(getHeight() / vDistance);
        auto nW = std::ceil(getWidth() / hDistance);
        
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
                
                auto ellipseRadius = JIRadius * 1.15;
                
                juce::Path e{};
                e.addEllipse(x - ellipseRadius - 1.5, y - JIRadius - 1.5, 2 * ellipseRadius + 3, 2 * JIRadius + 3);
//                shadow.render(g, e);
                
                g.setColour(juce::Colours::black.withAlpha(1.f));
                g.fillPath(e);
            }
        }
    }

    std::pair<uint64_t, uint64_t> calculateCell(int fifths, int thirds)
    {
        uint64_t n{1}, d{1};

        while(thirds > 0)
        {
            auto [nn,dd] = jim.multiplyRatio(n, d, 5, 4);
            n = nn;
            d = dd;
            thirds --;
        }

        while(thirds < 0)
        {
            auto [nn,dd] = jim.divideRatio(n, d, 5, 4);
            n = nn;
            d = dd;
            thirds ++;
        }

        while(fifths > 0)
        {
            auto [nn,dd] = jim.multiplyRatio(n, d, 3, 2);
            n = nn;
            d = dd;
            fifths --;
        }

        while(fifths < 0)
        {
            auto [nn,dd] = jim.divideRatio(n, d, 3, 2);
            n = nn;
            d = dd;
            fifths ++;
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
    
    melatonin::CachedBlur hBlur{6};
    
    melatonin::CachedBlur uBlur{5};
    melatonin::CachedBlur dBlur{3};
    
    melatonin::DropShadow shadow = {juce::Colours::black, 15};
    
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


