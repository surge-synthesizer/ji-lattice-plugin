/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#ifndef LATTICES_LATTICECOMPONENT_H
#define LATTICES_LATTICECOMPONENT_H

#include "JIMath.h"
#include "LatticesBinary.h"
#include "LatticesProcessor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <climits>
// may need this later
// #include <tuple>

#include <juce_animation/juce_animation.h>
#include <melatonin_blur/melatonin_blur.h>

//==============================================================================
struct LatticeComponent : juce::Component, private juce::MultiTimer
{
    LatticeComponent(LatticesProcessor &p) : proc(&p)
    {
        auto gwc = juce::Colours::ghostwhite;

        homeButton = std::make_unique<juce::ShapeButton>("Home", gwc, gwc, gwc);
        addAndMakeVisible(*homeButton);
        homeButton->onClick = [this] { proc->shift(0); };
        juce::Path circle{};
        circle.addEllipse(0, 0, 24, 24);
        homeButton->setShape(circle, true, true, false);

        westButton = std::make_unique<juce::ArrowButton>("West", .5f, gwc);
        addAndMakeVisible(*westButton);
        westButton->onClick = [this] { proc->shift(1); };

        eastButton = std::make_unique<juce::ArrowButton>("East", 0.f, gwc);
        addAndMakeVisible(*eastButton);
        eastButton->onClick = [this] { proc->shift(2); };

        northButton = std::make_unique<juce::ArrowButton>("North", .75f, gwc);
        addAndMakeVisible(*northButton);
        northButton->onClick = [this] { proc->shift(3); };

        southButton = std::make_unique<juce::ArrowButton>("South", .25f, gwc);
        addAndMakeVisible(*southButton);
        southButton->onClick = [this] { proc->shift(4); };

        zoomOutButton = std::make_unique<juce::TextButton>("-");
        addAndMakeVisible(*zoomOutButton);
        zoomOutButton->onClick = [this] { zoomOut(); };

        zoomInButton = std::make_unique<juce::TextButton>("+");
        addAndMakeVisible(*zoomInButton);
        zoomInButton->onClick = [this] { zoomIn(); };

        updater.addAnimator(follow);
        firstRun = true;
        setWantsKeyboardFocus(true);
        startTimer(0, 50); // for keyboard gestures
        startTimer(1, 50); // for following the highlight around
    }

    LatticeComponent(std::pair<int, int> *c, int *v) : proc(nullptr)
    {
        for (int i = 0; i < 12; ++i)
        {
            CoO[i] = c[i];
            visitor[i] = v[i];
        }
    }

    ~LatticeComponent() { proc = nullptr; }

    void zoomIn()
    {
        if (JIRadius == 42)
            return;

        ++JIRadius;
        ellipseRadius = JIRadius * 1.15;
        stoke.setPointHeight(JIRadius);
        blackShadow.setRadius(JIRadius / 3);
        whiteShadow.setRadius(JIRadius / 2);
        repaint();
    }

    void zoomOut()
    {
        if (JIRadius == 15)
            return;

        --JIRadius;
        ellipseRadius = JIRadius * 1.15;
        stoke.setPointHeight(JIRadius);
        blackShadow.setRadius(JIRadius / 3);
        whiteShadow.setRadius(JIRadius / 2);
        repaint();
    }

    void resized() override
    {
        auto b = this->getLocalBounds();

        homeButton->setBounds(b.getRight() - 72, b.getBottom() - 72, 24, 24);
        westButton->setBounds(b.getRight() - 104, b.getBottom() - 71, 24, 24);
        eastButton->setBounds(b.getRight() - 38, b.getBottom() - 71, 24, 24);
        northButton->setBounds(b.getRight() - 71, b.getBottom() - 104, 24, 24);
        southButton->setBounds(b.getRight() - 71, b.getBottom() - 38, 24, 24);

        zoomOutButton->setBounds(20, b.getBottom() - 55, 35, 35);
        zoomInButton->setBounds(60, b.getBottom() - 55, 35, 35);
    }

    bool keyPressed(const juce::KeyPress &key) override
    {
        if (key == juce::KeyPress::returnKey)
        {
            proc->shift(0);
            return true;
        }

        if (key == juce::KeyPress::leftKey && !westFlag)
        {
            westFlag = true;
            proc->shift(1);
            return true;
        }

        if (key == juce::KeyPress::rightKey && !eastFlag)
        {
            eastFlag = true;
            proc->shift(2);
            return true;
        }

        if (key == juce::KeyPress::upKey && !northFlag)
        {
            northFlag = true;
            proc->shift(3);
            return true;
        }

        if (key == juce::KeyPress::downKey && !southFlag)
        {
            southFlag = true;
            proc->shift(4);
            return true;
        }

        return false;
    }

    void timerCallback(int timerID) override
    {
        if (timerID == 0)
        {
            westFlag = false;
            eastFlag = false;
            northFlag = false;
            southFlag = false;
        }

        if (timerID == 1)
        {
            if (proc->changed || firstRun)
            {
                repaint();
                proc->changed = false;

                int nx = proc->positionX;
                int ny = proc->positionY;

                bool sH = (goalX != nx && nx % 4 == 0);
                bool sV = (goalY != ny && ny % 3 == 0);

                procX = nx;
                procY = ny;

                if (sH || sV)
                {
                    priorX = xShift;
                    priorY = yShift;
                    goalX = procX;
                    goalY = procY;

                    follow.start();
                }
            }
        }
    }

    void paint(juce::Graphics &g) override
    {
        bool enabled = this->isEnabled();

        int shadowSpacing1 = JIRadius / 20;
        int shadowSpacing2 = JIRadius / 10;

        float ctrDistance{JIRadius * (5.f / 3.f)};

        float vhDistance = 2.0f * ctrDistance;

        auto ctrX = getWidth() / 2;
        auto ctrH = getHeight() / 2;

        int yS = static_cast<int>(std::abs(yShift));
        int xS = static_cast<int>(std::abs(xShift));

        int nV = std::ceil(getHeight() / vhDistance) + yS;
        int nW = std::ceil(getWidth() / vhDistance) + xS;

        int dist{0}, hDist{0}, uDist{0}, dDist{0};

        juce::Image Lines{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image Spheres{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image Text{juce::Image::ARGB, getWidth(), getHeight(), true};
        {
            juce::Graphics lG(Lines);
            juce::Graphics sG(Spheres);
            juce::Graphics tG(Text);
            for (int v = -nV; v < nV; ++v)
            {
                float off = v * vhDistance * 0.5f;
                float y = -v * vhDistance + ctrH + yShift;
                if (y < -ctrDistance || y > getHeight() + ctrDistance)
                    continue;

                for (int w = -nW; w < nW; ++w)
                {
                    float x = w * vhDistance + ctrX + off - xShift;

                    if (x < -ctrDistance || x > getWidth() + ctrDistance)
                        continue;

                    int degree{0};
                    if (enabled) // get our bearings so we know how brightly to draw stuff
                    {
                        std::pair<int, int> C = {w, v};         // current sphere
                        std::pair<int, int> H = {w + 1, v};     // next one over
                        std::pair<int, int> U = {w, v + 1};     // next one up
                        std::pair<int, int> D = {w + 1, v - 1}; // next one down

                        // ok, so how far is this sphere from a lit up one?
                        dist = calcDist(C);

                        if (dist == 0)
                        {
                            for (int i = 0; i < 12; ++i)
                            {
                                if (proc->coOrds[i] == C)
                                {
                                    degree = i;
                                    break;
                                }
                            }
                        }
                        // and what about its lines?
                        hDist = std::max(dist, calcDist(H));
                        uDist = std::max(dist, calcDist(U));
                        dDist = std::max(dist, calcDist(D));
                    }
                    else // if we're disabled everything is kinda dim
                    {
                        dist = 2;
                        hDist = 2;
                        uDist = 2;
                        dDist = 2;
                    }

                    // those numbers will set this
                    float alpha{};

                    float thickness = JIRadius / 9.f;

                    // Horizontal Line
                    alpha = 1.f / (std::sqrt(hDist) + 1);
                    lG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    juce::Line<float> horiz(x, y, x + vhDistance, y);
                    lG.drawLine(horiz, thickness);

                    // Upward Line
                    alpha = 1.f / (std::sqrt(uDist) + 1);
                    lG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    juce::Line<float> up(x, y, x + (vhDistance * .5f), y - vhDistance);
                    float ul[2] = {7.f, 3.f};
                    lG.drawDashedLine(up, ul, 2, thickness, 1);

                    // Downward Line
                    alpha = 1.f / (std::sqrt(dDist) + 1);
                    lG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    juce::Line<float> down(x, y, x + (vhDistance * .5f), y + vhDistance);
                    float dl[2] = {2.f, 3.f};
                    lG.drawDashedLine(down, dl, 2, thickness, 1);

                    // Spheres
                    juce::Path e{};
                    e.addEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius);
                    // And their shadows
                    juce::Path b{};
                    b.addEllipse(x - ellipseRadius - shadowSpacing1, y - JIRadius - shadowSpacing1,
                                 2 * ellipseRadius + shadowSpacing2, 2 * JIRadius + shadowSpacing2);

                    // Select gradient colour

                    bool uni = enabled ? ((w + (v * 4)) % 12 == 0) : false;
                    bool lit = enabled ? (dist == 0) : false;
                    auto gradient = chooseColour(std::abs(v), x, y, lit,
                                                 proc->currentVisitors->vis[degree], uni);

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
                    sG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    sG.drawEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius,
                                   thickness);

                    // Names or Ratios?
                    /*
                    auto [n, d] = calculateCell(w, v);

                    if (enabled && (dist == 0 && proc->currentVisitors->vis[degree] > 1))
                    {
                        reCalculateCell(n, d, proc->currentVisitors->vis[degree], degree);
                    }
                    auto s = std::to_string(n) + "/" + std::to_string(d);
                    */

                    auto s = nameNoteOnLattice(w, v, degree, lit);
                    tG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    tG.setFont(stoke);

                    tG.drawFittedText(s, x - ellipseRadius + 3, y - (JIRadius / 3.f),
                                      2.f * (ellipseRadius - 3), .66667f * JIRadius,
                                      juce::Justification::horizontallyCentred, 1, 0.05f);
                }
            }
        }
        g.drawImageAt(Lines, 0, 0, false);
        g.drawImageAt(Spheres, 0, 0, false);
        g.drawImageAt(Text, 0, 0, false);

        auto b = this->getLocalBounds();

        g.setColour(bg);
        g.fillRect(b.getRight() - 110, b.getBottom() - 110, 101, 101);
        g.setColour(juce::Colours::ghostwhite);
        g.drawRect(b.getRight() - 110, b.getBottom() - 110, 101, 101);
    }

  private:
    int syntonicDrift{0}, diesisDrift{0}, procX{0}, procY{0};
    float xShift{0}, yShift{0}, priorX{0}, priorY{0}, goalX{0}, goalY{0};
    bool firstRun{false};

    juce::VBlankAnimatorUpdater updater{this};
    juce::Animator follow =
        juce::ValueAnimatorBuilder{}
            .withEasing(juce::Easings::createEaseInOut())
            .withDurationMs(1000)
            .withValueChangedCallback([this](auto value) { shiftToFollow((float)value); })
            .build();

    void shiftToFollow(const float v)
    {
        float dist = JIRadius * 2.f * (5.f / 3.f);
        auto nv = 1 - v;

        xShift = nv * priorX + v * dist * (goalX + goalY * .5f);
        yShift = nv * priorY + v * dist * goalY;

        repaint();

        // if (follow.isComplete())
    }

    std::string noteNames[7] = {"F", "C", "G", "D", "A", "E", "B"};

    std::string nameNoteOnLattice(int x, int y, int degree, bool lit = false)
    {
        int origin = proc->originNoteName.first + proc->originNoteName.second * 7;
        int location = x + y * 4 + origin;
        int letter = ((location % 7) + 7) % 7;
        std::string name = noteNames[letter];

        while (location >= 7)
        {
            // if it already has a sharp
            if (name.compare(name.size() - 1, 1, "#") == 0)
            {
                name.pop_back(); // replace it
                name += "*";     // with a double sharp
            }
            else
            {
                name += "#";
            }

            location -= 7;
        }
        while (location < 0)
        {
            if (name.compare(name.size() - 1, 1, "b") == 0)
            {
                name.pop_back();
                name += "c";
            }
            else
            {
                name += "b";
            }

            location += 7;
        }

        auto row = y;

        int visitor = proc->currentVisitors->vis[degree];

        if (lit && visitor > 1)
        {
            bool major = (degree == 7 || degree == 2 || degree == 9 || degree == 4 ||
                          degree == 11 || degree == 6);

            auto visAcc = ""; // visitor Accidental

            switch (visitor)
            {
            case 2:
                visAcc = (major) ? ")" : "(";
                break;
            case 3:
                visAcc = (major) ? ">" : "<";
                break;
            case 4:
                visAcc = (major) ? "{" : "}";
                break;
            case 5:
                visAcc = (major) ? ":" : ";";
                break;
            case 6:
                visAcc = (major) ? "," : ".";
                break;
            default:
                break;
            }

            // Remove a plus/minus when adding another accidental
            row += (major) ? -1 : 1;
            name += visAcc;
        }

        while (row > 0)
        {
            name += "-";
            --row;
        }
        while (row < 0)
        {
            name += "+";
            ++row;
        }

        return name;
    }

  protected:
    LatticesProcessor *proc;

    int JIRadius{26};
    int ellipseRadius = JIRadius * 1.15;
    JIMath jim;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};
    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(JIRadius)};

    melatonin::DropShadow blackShadow = {juce::Colours::black, JIRadius / 3};
    melatonin::DropShadow whiteShadow = {juce::Colours::ghostwhite, JIRadius / 2};

    int visitor[12] = {};
    std::array<std::pair<int, int>, 12> CoO{}; // currently lit co-ordinates

    virtual inline int calcDist(std::pair<int, int> xy) // how far is a given coordinate from those?
    {
        int res{INT_MAX};

        for (int i = 0; i < 12; ++i)
        {
            int tx = std::abs(xy.first - proc->coOrds[i].first);
            int ty = std::abs(xy.second - proc->coOrds[i].second);
            int sum = tx + ty;
            if (sum < res)
                res = sum;
        }

        return res;
    }

    juce::Colour bg = juce::Colour{.475f, 1.f, 0.05f, 1.f};

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

  private:
    std::unique_ptr<juce::TextButton> zoomOutButton;
    std::unique_ptr<juce::TextButton> zoomInButton;

    std::unique_ptr<juce::ArrowButton> westButton;
    std::unique_ptr<juce::ArrowButton> eastButton;
    std::unique_ptr<juce::ArrowButton> northButton;
    std::unique_ptr<juce::ArrowButton> southButton;

    bool westFlag{false}, eastFlag{false}, northFlag{false}, southFlag{false};

    std::unique_ptr<juce::ShapeButton> homeButton;
};

// =================================================================================================

template <typename buttonUser> struct SmallLatticeComponent : LatticeComponent
{
    SmallLatticeComponent(int *v, buttonUser *bu, int size = 30)
        : LatticeComponent(initCo, homes), buttonParent(bu)
    {
        JIRadius = size;
        ellipseRadius = JIRadius * 1.15;
        stoke.setPointHeight(JIRadius);

        blackShadow.setRadius(JIRadius / 3);
        whiteShadow.setRadius(JIRadius / 2);

        circleShape.addEllipse(0, 0, ellipseRadius, JIRadius);

        juce::Colour n{juce::Colours::transparentWhite};
        juce::Colour o{juce::Colours::ghostwhite.withAlpha(.15f)};

        for (int d = 0; d < 12; ++d)
        {
            buttons.add(new juce::ShapeButton(std::to_string(d), n, o, o));
            buttons[d]->setShape(circleShape, true, true, false);
            addAndMakeVisible(buttons[d]);
            buttons[d]->setRadioGroupId(1);
            buttons[d]->onClick = [this] { whichNote(); };
            buttons[d]->setClickingTogglesState(true);

            updateDegree(d, v[d]);
        }
        buttons[0]->setToggleState(true, juce::dontSendNotification);
    }

    void updateDegree(int d, int v)
    {
        visitor[d] = v;

        auto vx{0};
        auto vy{0};

        if ((d == 9 || d == 4 || d == 11 || d == 6) && v == 0)
        {
            vx = 4;
            vy = -1;
        }
        if ((d == 5 || d == 0) && v != 0)
        {
            vx = 4;
            vy = -1;
        }
        if ((d == 7 || d == 2) && v != 0)
        {
            vx = -4;
            vy = 1;
        }
        if ((d == 1 || d == 8 || d == 3 || d == 10) && v == 0)
        {
            vx = -4;
            vy = 1;
        }

        CoO[d].first = initCo[d].first + vx;
        CoO[d].second = initCo[d].second + vy;
    }

    void resized() override {}

    void paint(juce::Graphics &g) override
    {

        auto b = this->getLocalBounds();
        g.setColour(juce::Colour{.475f, 1.f, 0.05f, 1.f});
        g.fillRect(b);
        g.setColour(juce::Colours::ghostwhite);
        g.drawRect(b);

        bool enabled = this->isEnabled();
        for (int d = 0; d < 12; ++d)
        {
            buttons[d]->setEnabled(enabled);
        }

        int shadowSpacing1 = JIRadius / 20;
        int shadowSpacing2 = JIRadius / 10;
        auto a = enabled ? 75.f : .5f;

        whiteShadow.setColor(juce::Colours::ghostwhite.withAlpha(a));
        blackShadow.setColor(juce::Colours::black.withAlpha(a));

        float ctrDistance{JIRadius * (5.f / 3.f)};

        float vhDistance = 2.0f * ctrDistance;

        auto ctrX = getWidth() / 2;
        auto ctrH = getHeight() / 2;

        auto nV = std::ceil(getHeight() / vhDistance);
        auto nW = std::ceil(getWidth() / vhDistance);

        juce::Image Lines{juce::Image::ARGB, getWidth(), getHeight(), true};
        juce::Image Spheres{juce::Image::ARGB, getWidth(), getHeight(), true};
        {
            juce::Graphics lG(Lines);
            juce::Graphics sG(Spheres);
            for (int v = -nV; v < nV; ++v)
            {
                float off = v * vhDistance * 0.5f;
                float y = -v * vhDistance + ctrH;
                if (y < 0 || y > getHeight())
                    continue;

                for (int w = -nW; w < nW; ++w)
                {
                    float x = w * vhDistance + ctrX + off;

                    if (x < 0 || x > getWidth())
                        continue;

                    std::pair<int, int> C = {w, v}; // current sphere
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
                        buttons[degree]->setBounds(x - ellipseRadius, y - JIRadius,
                                                   2 * ellipseRadius, 2 * JIRadius);
                    }
                    else
                    {
                        continue;
                    }

                    std::pair<int, int> H = {w + 1, v};     // next sphere over
                    std::pair<int, int> U = {w, v + 1};     // next sphere up
                    std::pair<int, int> D = {w + 1, v - 1}; // next sphere down

                    // in this class we just don't draw the stuff that's not in the mapping
                    bool hLit = calcDist(H) == 0;
                    bool uLit = calcDist(U) == 0;
                    bool dLit = calcDist(D) == 0;
                    float alpha = enabled ? .9f : .5f;

                    float thickness = JIRadius / 9.f;

                    if (hLit) // Horizontal Line
                    {
                        lG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                        juce::Line<float> horiz(x, y, x + vhDistance, y);
                        lG.drawLine(horiz, thickness);
                    }

                    if (uLit) // Upward Line
                    {
                        lG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                        juce::Line<float> up(x, y, x + (vhDistance * .5f), y - vhDistance);
                        float ul[2] = {7.f, 3.f};
                        lG.drawDashedLine(up, ul, 2, thickness, 1);
                    }

                    if (dLit) // Downward Line
                    {
                        lG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                        juce::Line<float> down(x, y, x + (vhDistance * .5f), y + vhDistance);
                        float dl[2] = {2.f, 3.f};
                        lG.drawDashedLine(down, dl, 2, thickness, 1);
                    }

                    // Spheres
                    juce::Path e{};
                    e.addEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius);
                    // And their shadows
                    // TODO: make these dependent on constructor size
                    juce::Path b{};
                    b.addEllipse(x - ellipseRadius - shadowSpacing1, y - JIRadius - shadowSpacing1,
                                 2 * ellipseRadius + shadowSpacing2, 2 * JIRadius + shadowSpacing2);

                    // Select gradient colour
                    auto gradient = chooseWisely(std::abs(v), x, y, visitor[degree]);

                    if (degree == selectedDegree && enabled)
                    {
                        selectedHighlight.render(sG, b);
                    }
                    else
                    {
                        whiteShadow.render(sG, e);
                        blackShadow.render(sG, b);
                    }
                    sG.setColour(juce::Colours::black.withAlpha(1.f));
                    sG.fillPath(b);
                    alpha = (degree == selectedDegree) ? 1.f : .75f;
                    if (!enabled)
                        alpha = .5f;
                    gradient.multiplyOpacity(alpha);
                    sG.setGradientFill(gradient);
                    sG.fillPath(e);
                    sG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    sG.drawEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius,
                                   thickness);

                    auto [n, d] = calculateCell(w, v);
                    if (dist == 0 && visitor[degree] > 1)
                    {
                        reCalculateCell(n, d, visitor[degree], degree);
                    }
                    auto s = std::to_string(n) + "/" + std::to_string(d);

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

    int selectedDegree{0};

  protected:
    buttonUser *buttonParent;
    juce::Path circleShape;
    juce::OwnedArray<juce::ShapeButton> buttons;

    melatonin::DropShadow selectedHighlight = {juce::Colours::ghostwhite, 18};

    int homes[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1};
    std::pair<int, int> initCo[12]{{0, 0}, {-1, -1}, {2, 0},  {1, -1}, {0, 1},  {-1, 0},
                                   {2, 1}, {1, 0},   {0, -1}, {-1, 1}, {2, -1}, {1, 1}};

    inline int calcDist(std::pair<int, int> xy) override
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

    juce::ColourGradient chooseWisely(int row, float x, float y, int visitor)
    {

        if (row == 0)
        {
            return juce::ColourGradient(py1, x - ellipseRadius, y, py2, x + ellipseRadius, y,
                                        false);
        }
        else
        {
            switch (visitor)
            {
            case 0:
                return juce::ColourGradient(py1, x - ellipseRadius, y, py2, x + ellipseRadius, y,
                                            false);
            case 1:
                return juce::ColourGradient(l1c1, x - ellipseRadius, y, l1c2, x + ellipseRadius, y,
                                            false);
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
                return juce::ColourGradient(py1, x - ellipseRadius, y, py2, x + ellipseRadius, y,
                                            false);
            }
        }
    }

    void whichNote()
    {
        int n{};
        for (int i = 0; i < 12; ++i)
        {
            if (buttons[i]->getToggleState())
            {
                n = i;
                break;
            }
        }
        buttonParent->selectNote(n);
    }
};

#endif // LATTICES_LATTICECOMPONENT_H
