
#pragma once

#include "JIMath.h"


//==============================================================================
struct JIComponent : juce::Component
{
    JIComponent(int px, int py)
    {
        currentX = px;
        currentY = py;
        updateLocation(currentX, currentY);
    }
    
    
    void updateLocation(int x, int y)
    {
        std::cout << "updated location to " << x << " , " << y << std::endl;
        currentX = x;
        currentY = y;
    }
    
    void paint(juce::Graphics &g) override
    {
        int JIRadius{30};
        float ctrDistance{JIRadius * (5.f / 3.f)};
        
        float vDistance = 2.0f * ctrDistance;
        float hDistance = 2.0f * ctrDistance;
        
        auto ctrX = getWidth() / 2;
        auto ctrH = getHeight() / 2;

        auto nV = std::ceil(getHeight() / vDistance);
        auto nW = std::ceil(getWidth() / hDistance);
        
        juce::Colour colour1 { juce::Colours::blue }, colour2 { juce::Colours::green };

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
                
                if (w == -nW-1)
                {
                    
                }
                    
                if (x < 0 || x > getWidth())
                    continue;
                
                auto alpha = 1.f;
                
                if (w < currentX - 1 || w > currentX + 2 || v < currentY - 1 || v > currentY + 1)
                {
                    alpha = .5f;
                }
                
                auto gradient = juce::ColourGradient(colour1, x-(1 * JIRadius), y,
                                                     colour2, x+(1 * JIRadius), y, false);
                gradient.multiplyOpacity(alpha);
                
                g.setGradientFill(gradient);
                g.fillEllipse(x-JIRadius,y-JIRadius,2*JIRadius, 2*JIRadius);
                

                g.setColour(juce::Colours::white.withAlpha(alpha));
                g.drawEllipse(x-JIRadius,y-JIRadius,2*JIRadius, 2*JIRadius, 3);
                
                auto fifths = w;
                auto thirds = v;
                
                auto [n,d] = calculateCell(fifths, thirds);
                auto s = std::to_string(n) + "/" + std::to_string(d);
                // auto s = std::to_string(fifths) + "F " + std::to_string(thirds) + "T";
                g.setFont(20.f);
                g.drawText(s, x - JIRadius + 3, y - 9, 2 * (JIRadius - 3), 20, juce::Justification::horizontallyCentred, true);
                
                if (w == currentX + 2)
                {
                    alpha = .5f;
                }
                g.setColour(juce::Colours::white.withAlpha(alpha));
                juce::Line<float> horiz(x + JIRadius, y, x + hDistance - JIRadius, y);
                g.drawLine(horiz, 3.f);
                
                if (w < currentX - 1 || w > currentX + 2 || v < currentY - 1 || v > currentY)
                {
                    alpha = .5f;
                }
                else
                {
                    alpha = 1.f;
                }
                // something like sqrt(.1 * slope), sqrt(1 - .1 * slope)
                float magicNumbers[4] = {0.44721f, 0.89443f, 1.21945f, 2.43891f};
                // IDK... I just fed around in desmos and found out tbh
                
                g.setColour(juce::Colours::white.withAlpha(alpha));
                juce::Line<float> up(x + JIRadius * magicNumbers[0], y - JIRadius * magicNumbers[1], x + JIRadius * magicNumbers[2], y - JIRadius * magicNumbers[3]);
                float l[2] = {7.f, 3.f};
                g.drawDashedLine(up, l, 2, 3.f, 1);
                
                if (w < currentX - 1 || w > currentX + 1 || v < currentY || v > currentY + 1)
                {
                    alpha = .5f;
                }
                else
                {
                    alpha = 1.f;
                }
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
    int currentX{}, currentY{};
};


