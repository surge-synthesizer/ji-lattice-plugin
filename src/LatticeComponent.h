
#pragma once

#include "JIMath.h"


//==============================================================================
struct LatticeComponent : juce::Component
{
    LatticeComponent()
    {
        update(0,0);
    }
    
    void update(int x) // Pyth mode
    {
        CC[0].first = x;
        CC[1].first = x - 5;
        CC[2].first = x + 2;
        CC[3].first = x - 3;
        CC[4].first = x + 4;
        CC[5].first = x - 1;
        CC[6].first = x + 6;
        CC[7].first = x + 1;
        CC[8].first = x - 4;
        CC[9].first = x + 3;
        CC[10].first = x - 2;
        CC[11].first = x + 5;
        
        for (int i = 0; i < 12; ++i)
        {
            CC[i].second = 0;
        }
    }
    
    void update(std::pair<int, int> *c) // Syntonic mode
    {
        for (int i = 0; i < 12; ++i)
        {
            CC[i] = c[i];
        }
    }
    
    void update(int x, int y) // Duodene mode
    {
        CC[0].first = x;
        CC[0].second = y;
        
        CC[1].first = x - 1;
        CC[1].second = y - 1;
        
        CC[2].first = x + 2;
        CC[2].second = y;
        
        CC[3].first = x + 1;
        CC[3].second = y - 1;
        
        CC[4].first = x;
        CC[4].second = y + 1;
        
        CC[5].first = x - 1;
        CC[5].second = y;
        
        CC[6].first = x + 2;
        CC[6].second = y + 1;
        
        CC[7].first = x + 1;
        CC[7].second = y;
        
        CC[8].first = x;
        CC[8].second = y - 1;
        
        CC[9].first = x - 1;
        CC[9].second = y + 1;
        
        CC[10].first = x + 2;
        CC[10].second = y - 1;
        
        CC[11].first = x + 1;
        CC[11].second = y + 1;
    }
    
    juce::Colour colour1 { juce::Colours::blue }, colour2 { juce::Colours::green }, colour3 { juce::Colours::red };
    
    void paint(juce::Graphics &g) override
    {
        int JIRadius{26};
        float ctrDistance{JIRadius * (5.f / 3.f)};
        
        float vDistance = 2.0f * ctrDistance;
        float hDistance = 2.0f * ctrDistance;
        
        auto ctrX = getWidth() / 2;
        auto ctrH = getHeight() / 2;

        auto nV = std::ceil(getHeight() / vDistance);
        auto nW = std::ceil(getWidth() / hDistance);


        // this overdoes it a bit
        for (int v = -nV - 1; v < nV + 1; ++v)
        {
            float off = (v * hDistance * 0.5f);
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
                    
                    if (C == CC[i])
                    {
                        lit = true;
                        break;
                    }
                }
                
                auto alpha = (lit) ? 1.f : .5f;
                
                if ((w + (v * 4)) % 12 == 0)
                {
                    auto gradient = juce::ColourGradient(colour3, x-(1 * JIRadius), y,
                                                         colour1, x+(1 * JIRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }
                else
                {
                    auto gradient = juce::ColourGradient(colour1, x-(1 * JIRadius), y,
                                                         colour2, x+(1 * JIRadius), y, false);
                    gradient.multiplyOpacity(alpha);
                    g.setGradientFill(gradient);
                }

                g.fillEllipse(x-JIRadius,y-JIRadius,2*JIRadius, 2*JIRadius);
                g.setColour(juce::Colours::white.withAlpha(alpha));
                g.drawEllipse(x-JIRadius,y-JIRadius,2*JIRadius, 2*JIRadius, 3);
                
                auto fifths = w;
                auto thirds = v;
                
                auto [n,d] = calculateCell(fifths, thirds);
                auto s = std::to_string(n) + "/" + std::to_string(d);
                g.setFont(JIRadius * (2.f / 3.f));
                g.drawText(s, x - JIRadius + 3, y - 9, 2 * (JIRadius - 3), 20, juce::Justification::horizontallyCentred, true);
                
                bool lineLit{false};
                for (int i = 0; i < 12; ++i) // next horizontal line lit?
                {
                    std::pair<int, int> C = {w + 1,v};
                    if (C == CC[i])
                    {
                        lineLit = true;
                        break;
                    }
                }
                alpha = (lineLit && lit) ? 1.f : .5f;
                g.setColour(juce::Colours::white.withAlpha(alpha));
                juce::Line<float> horiz(x + JIRadius, y, x + hDistance - JIRadius, y);
                g.drawLine(horiz, 3.f);
                
                
                // something like sqrt(.1 * slope), sqrt(1 - .1 * slope)
                float magicNumbers[4] = {0.44721f, 0.89443f, 1.21945f, 2.43891f};
                // tbh I just opened desmos, f'ed around and found out
                
                for (int i = 0; i < 12; ++i) // next upward diagonal line lit?
                {
                    std::pair<int, int> C = {w,v + 1};
                    if (C == CC[i])
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
                juce::Line<float> up(x + JIRadius * magicNumbers[0], y - JIRadius * magicNumbers[1], x + JIRadius * magicNumbers[2], y - JIRadius * magicNumbers[3]);
                float l[2] = {7.f, 3.f};
                g.drawDashedLine(up, l, 2, 3.f, 1);
                
                for (int i = 0; i < 12; ++i) // next downward diagonal line lit?
                {
                    std::pair<int, int> C = {w + 1,v - 1};
                    if (C == CC[i])
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
                juce::Line<float> down(x + JIRadius * magicNumbers[0], y + JIRadius * magicNumbers[1], x + JIRadius * magicNumbers[2], y + JIRadius * magicNumbers[3]);
                float le[2] = {2.f, 3.f};
                g.drawDashedLine(down, le, 2, 3.f, 1);
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
    std::pair<int, int> CC[12]
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


