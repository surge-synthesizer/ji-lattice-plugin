
#pragma once

#include "JIMath.h"


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
                
                auto ewr = JIRadius * 1.25;
                auto ew = JIRadius * 2.5;
                
                auto alpha = (lit) ? 1.f : .5f;
                if ((w + (v * 4)) % 12 == 0)
                {
                    auto gradient = juce::ColourGradient(com1, x-(ewr), y,
                                                         com2, x+(ewr), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 0)
                {
                    auto gradient = juce::ColourGradient(p1, x-(ewr), y,
                                                         p2, x+(ewr), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 1 || v == - 1)
                {
                    auto gradient = juce::ColourGradient(l1c1, x-(ewr), y,
                                                         l1c2, x+(ewr), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 2 || v == - 2)
                {
                    auto gradient = juce::ColourGradient(l2c1, x-(ewr), y,
                                                         l2c2, x+(ewr), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else if (v == 3 || v == - 3)
                {
                    auto gradient = juce::ColourGradient(l3c1, x-(ewr), y,
                                                         l3c2, x+(ewr), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else
                {
                    auto gradient = juce::ColourGradient(l4c1, x-(ewr), y,
                                                         l4c2, x+(ewr), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                g.fillEllipse(x - (ewr),y - JIRadius, ew, 2 * JIRadius);
                
                g.setColour(juce::Colours::white.withAlpha(alpha));
                g.drawEllipse(x - ewr,y - JIRadius, ew, 2 * JIRadius, 3);

                auto [n,d] = calculateCell(w, v);
                auto s = std::to_string(n) + "/" + std::to_string(d);
                g.setFont(stoke);
                g.drawFittedText(s, x - (1.25f * JIRadius) + 4, y - (JIRadius / 3.f) , 2.5f * (JIRadius - 3), .66667f * JIRadius, juce::Justification::horizontallyCentred, 1, 0.05f);
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
                
                bool lineLit{false};
                for (int i = 0; i < 12; ++i) // next horizontal line lit?
                {
                    std::pair<int, int> C = {w + 1,v};
                    if (C == CoO[i])
                    {
                        lineLit = true;
                        break;
                    }
                }
                
                auto alpha = (lineLit && lit) ? 1.f : .5f;
                g.setColour(juce::Colours::white.withAlpha(alpha));
                juce::Line<float> horiz(x, y, x + hDistance, y);
                g.drawLine(horiz, 3.f);
                
                for (int i = 0; i < 12; ++i) // next upward diagonal line lit?
                {
                    std::pair<int, int> C = {w,v + 1};
                    if (C == CoO[i])
                    {
                        lineLit = true;
                        break;
                    }
                    else if (i == 11)
                    {
                        lineLit = false;
                    }
                }
                
                alpha = (lineLit && lit) ? 1.f : .5f;
                g.setColour(juce::Colours::white.withAlpha(alpha));
                juce::Line<float> up(x, y, x + (hDistance * .5f), y - vDistance);
                float l[2] = {7.f, 3.f};
                g.drawDashedLine(up, l, 2, 3.f, 1);
                
                for (int i = 0; i < 12; ++i) // next downward diagonal line lit?
                {
                    std::pair<int, int> C = {w + 1,v - 1};
                    if (C == CoO[i])
                    {
                        lineLit = true;
                        break;
                    }
                    else if (i == 11)
                    {
                        lineLit = false;
                    }
                }
                alpha = (lineLit && lit) ? 1.f : .5f;
                g.setColour(juce::Colours::white.withAlpha(alpha));
                juce::Line<float> down(x, y, x + (hDistance * .5f), y + vDistance);
                float le[2] = {2.f, 3.f};
                g.drawDashedLine(down, le, 2, 3.f, 1);
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
                
                g.setColour(juce::Colours::black.withAlpha(1.f));
                g.fillEllipse(x - (JIRadius * 1.25) - 1.5, y - (JIRadius) - 1.5, 2.5 * JIRadius + 3, 2*JIRadius + 3);
            }
        }
    }

    std::pair<uint64_t, uint64_t> calculateCell(int fifths, int thirds)
    {
        JIMath jim;
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
    
    juce::FontOptions fo{"Stoke", JIRadius, juce::Font::plain};
    juce::Font stoke{fo};

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


