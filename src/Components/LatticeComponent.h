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
#include "ScaleData.h"
#include "LatticesColours.h"
#include "LatticesBinary.h"
#include "LatticesProcessor.h"

#include <algorithm>
#include <numeric>
#include <array>
#include <cmath>
#include <climits>
#include <string>

#include <juce_animation/juce_animation.h>
#include <melatonin_blur/melatonin_blur.h>

//==============================================================================
struct LatticeComponent : juce::Component, private juce::MultiTimer
{
    LatticeComponent(LatticesProcessor &p) : proc(&p), Gradients(JIRadius)
    {
        auto gwc = juce::Colours::ghostwhite;

        homeButton = std::make_unique<juce::ShapeButton>("Home", gwc, gwc, gwc);
        addAndMakeVisible(*homeButton);
        homeButton->onClick = [this] { proc->shift(0); };
        juce::Path circle{};
        circle.addEllipse(0, 0, 24, 24);
        homeButton->setShape(circle, true, true, false);
        homeButton->setWantsKeyboardFocus(false);

        arrowButtons.push_back(std::make_unique<juce::ArrowButton>("West", .5f, gwc));
        arrowButtons.push_back(std::make_unique<juce::ArrowButton>("East", 0.f, gwc));
        arrowButtons.push_back(std::make_unique<juce::ArrowButton>("North", .75f, gwc));
        arrowButtons.push_back(std::make_unique<juce::ArrowButton>("South", .25f, gwc));
        for (int i = 0; i < 4; ++i)
        {
            addAndMakeVisible(*arrowButtons[i]);
            arrowButtons[i]->onClick = [this, i] { proc->shift(i + 1); };
            arrowButtons[i]->setWantsKeyboardFocus(false);
        }

        zoomOutButton = std::make_unique<juce::TextButton>("-");
        addAndMakeVisible(*zoomOutButton);
        zoomOutButton->onClick = [this] { zoomOut(); };
        zoomOutButton->setWantsKeyboardFocus(false);

        zoomInButton = std::make_unique<juce::TextButton>("+");
        addAndMakeVisible(*zoomInButton);
        zoomInButton->onClick = [this] { zoomIn(); };
        zoomInButton->setWantsKeyboardFocus(false);

        for (int i = 0; i < 32; ++i)
        {
            visButtons.emplace_back(std::make_unique<juce::TextButton>(std::to_string(i + 1)));
            addAndMakeVisible(*visButtons[i]);
            visButtons[i]->setClickingTogglesState(true);
            visButtons[i]->onClick = [this, i] { proc->selectVisitorGroup(i + 1, true); };
            visButtons[i]->setWantsKeyboardFocus(false);
        }

        updater.addAnimator(follow);
        setWantsKeyboardFocus(true);
        startTimer(0, 50); // for keyboard gestures
        startTimer(1, 50); // for following the highlight around
        repaint();
    }

    LatticeComponent(LatticesProcessor &p, bool smol) : proc(&p), Gradients(JIRadius) {}

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
        Gradients.setSize(JIRadius);
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
        Gradients.setSize(JIRadius);
        repaint();
    }

    void resized() override
    {
        auto b = this->getLocalBounds();

        homeButton->setBounds(b.getRight() - 72, b.getBottom() - 72, 24, 24);
        arrowButtons[0]->setBounds(b.getRight() - 104, b.getBottom() - 71, 24, 24);
        arrowButtons[1]->setBounds(b.getRight() - 38, b.getBottom() - 71, 24, 24);
        arrowButtons[2]->setBounds(b.getRight() - 71, b.getBottom() - 104, 24, 24);
        arrowButtons[3]->setBounds(b.getRight() - 71, b.getBottom() - 38, 24, 24);

        for (int i = 0; i < visButtons.size(); ++i)
        {
            visButtons[i]->setBounds(10 + i * 40, b.getBottom() - 45, 35, 35);
        }

        zoomOutButton->setBounds(b.getRight() - 90, 45, 35, 35);
        zoomInButton->setBounds(b.getRight() - 50, 45, 35, 35);
    }

    bool keyPressed(const juce::KeyPress &key) override
    {
        if (key == juce::KeyPress::returnKey && !homeFlag)
        {
            homeFlag = true;
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

        auto k = std::to_wstring(key.getTextCharacter());
        auto n = std::stoi(k) - 48;
        if (n >= 0 && n <= 9 && !visitorFlag)
        {
            visitorFlag = true;
            if (n == 0)
                n += 10;
            proc->selectVisitorGroup(n, true);
            return true;
        }

        return false;
    }

    void timerCallback(int timerID) override
    {
        if (timerID == 0)
        {
            homeFlag = false;
            westFlag = false;
            eastFlag = false;
            northFlag = false;
            southFlag = false;
            visitorFlag = false;
        }

        if (timerID == 1)
        {
            if (proc->changed)
            {
                repaint();
                proc->changed = false;

                int nx = proc->positionXY.first;
                int ny = proc->positionXY.second;

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

        homeButton->setEnabled(enabled);
        homeButton->setVisible(enabled);
        for (const auto &a : arrowButtons)
        {
            a->setEnabled(enabled);
            a->setVisible(enabled);
        }
        zoomInButton->setEnabled(enabled);
        zoomInButton->setVisible(enabled);
        zoomOutButton->setEnabled(enabled);
        zoomOutButton->setVisible(enabled);
        if (enabled)
        {
            int cv = proc->getCurrentVisitorGroupIndex();
            int idx{1};
            int nv = static_cast<int>(proc->visitorGroups.size());
            for (const auto &v : visButtons)
            {
                v->setToggleState(idx == cv, juce::dontSendNotification);
                v->setEnabled(idx < nv);
                v->setVisible(idx < nv);
                idx++;
            }
        }
        else
        {
            for (const auto &v : visButtons)
            {
                v->setEnabled(false);
                v->setVisible(false);
            }
        }

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
        float thickness = JIRadius / 9.f;
        float alpha{0.f};

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

                    int degree = ((7 * w + 4 * v) % 12 + 12) % 12;
                    int degreeTransposed{0};
                    int vis{0}, hVis{0}, uVis{0}, dVis{0};
                    if (enabled) // get our bearings so we know how brightly to draw stuff
                    {
                        std::pair<int, int> C = {w, v};         // current sphere
                        std::pair<int, int> H = {w + 1, v};     // next one over
                        std::pair<int, int> U = {w, v + 1};     // next one up
                        std::pair<int, int> D = {w + 1, v - 1}; // next one down

                        for (int d = 0; d < 12; ++d)
                        {
                            auto dco = proc->coOrds[d];
                            if (C == dco)
                            {
                                degreeTransposed = d;
                                vis = proc->currentVisitors->CC[d].nameIndex;
                                continue;
                            }
                            if (H == dco)
                            {
                                hVis = proc->currentVisitors->CC[d].nameIndex;
                                continue;
                            }
                            if (U == dco)
                            {
                                uVis = proc->currentVisitors->CC[d].nameIndex;
                                continue;
                            }
                            if (D == dco)
                            {
                                dVis = proc->currentVisitors->CC[d].nameIndex;
                            }
                        }
                        // ok, so how far is this sphere from a lit up one?
                        dist = calcDist(C);
                        // and what about its neighbors?
                        hDist = std::max(dist, calcDist(H));
                        uDist = std::max(dist, calcDist(U));
                        dDist = std::max(dist, calcDist(D));

                        // If NOT (this sphere and its neighbor are both normal,
                        // or they have the same visitor).
                        // In other words: If one of these has a visitor from
                        // some dimension, and the other does not.
                        // We will then draw the lines dimmer to emphasize the disconnect.
                        if (!((vis < 2 && hVis < 2) || vis == hVis))
                        {
                            hDist += 5;
                        }
                        if (!((vis < 2 && uVis < 2) || vis == uVis))
                        {
                            uDist += 5;
                        }
                        if (!((vis < 2 && dVis < 2) || vis == dVis))
                        {
                            dDist += 5;
                        }
                    }
                    else // if we're disabled everything is kinda dim
                    {
                        dist = 3;
                        hDist = 3;
                        uDist = 3;
                        dDist = 3;
                    }

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
                    juce::ColourGradient gradient{};

                    bool rcs = (vis > 1 && vis != 17);
                    if (degree == 0)
                    {
                        gradient = Gradients.rootGrad(x);
                    }
                    else if (enabled && dist == 0 && rcs)
                    {
                        gradient = Gradients.commaGrad(vis, x);
                    }
                    else
                    {
                        gradient = Gradients.latticeGrad(v, x);
                    }

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

                    // auto [n, d] = calculateCell(w, v);
                    // if (rcs && dist == 0)
                    // {
                    //     reCalculateCell(n, d, degree);
                    // }
                    // auto s = std::to_string(n) + "/" + std::to_string(d);

                    auto s = nameNoteOnLattice(w, v, degreeTransposed, dist == 0);
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

        if (enabled)
        {
            auto b = this->getLocalBounds();

            g.setColour(lattices::colours::background);
            g.fillRect(b.getRight() - 110, b.getBottom() - 110, 101, 101);
            g.setColour(juce::Colours::ghostwhite);
            g.drawRect(b.getRight() - 110, b.getBottom() - 110, 101, 101);
        }
    }

  protected:
    LatticesProcessor *proc;
    JIMath jim;

    int JIRadius{26};
    int ellipseRadius = JIRadius * 1.15;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};
    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(JIRadius)};

    lattices::colours::GradientProvider Gradients;
    melatonin::DropShadow blackShadow = {juce::Colours::black, JIRadius / 3};
    melatonin::DropShadow whiteShadow = {juce::Colours::ghostwhite, JIRadius / 2};

    // how far is a given coordinate from the currently active ones?
    virtual int calcDist(std::pair<int, int> xy)
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

    std::pair<uint64_t, uint64_t> calculateCell(int fifths, int thirds)
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

    virtual void reCalculateCell(uint64_t &n, uint64_t &d, int degree)
    {
        auto major = lattices::scaledata::isDegreeMajor[degree];

        // take out one syntonic comma, add in the visiting, reduce by GDC
        auto synt = lattices::scaledata::commas[lattices::scaledata::syntonic].getFraction(!major);
        n *= synt.first;
        d *= synt.second;
        auto vc = proc->currentVisitors->CC[degree].getFraction(degree);
        auto [nn, nd] = jim.multiplyRatio(n, d, vc.first, vc.second);
        auto gcd = std::gcd(nn, nd);

        n = nn / gcd;
        d = nd / gcd;
    }

  private:
    int syntonicDrift{0}, diesisDrift{0}, procX{0}, procY{0};
    float xShift{0}, yShift{0}, priorX{0}, priorY{0}, goalX{0}, goalY{0};

    bool homeFlag{false}, westFlag{false}, eastFlag{false}, northFlag{false}, southFlag{false},
        visitorFlag{false};

    const std::string noteNames[7] = {"F", "C", "G", "D", "A", "E", "B"};

    std::unique_ptr<juce::TextButton> zoomOutButton;
    std::unique_ptr<juce::TextButton> zoomInButton;

    std::unique_ptr<juce::ShapeButton> homeButton;
    std::vector<std::unique_ptr<juce::ArrowButton>> arrowButtons;
    std::vector<std::unique_ptr<juce::TextButton>> visButtons;

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

        int visitor = proc->currentVisitors->CC[degree].nameIndex;

        if (lit && visitor > 1)
        {
            bool major = lattices::scaledata::isDegreeMajor[degree];

            // Remove a plus/minus, to be replaced by another accidental
            row += (major) ? -1 : 1;

            auto visAcc = ""; // visitor accidental

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
};

// =================================================================================================

template <typename buttonUser> struct SmallLatticeComponent : LatticeComponent
{
    SmallLatticeComponent(buttonUser *bu, LatticesProcessor &p, int size = 30)
        : buttonParent(bu), LatticeComponent(p, true)
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
            buttons.push_back(std::make_unique<juce::ShapeButton>(std::to_string(d), n, o, o));
            buttons[d]->setShape(circleShape, true, true, false);
            addAndMakeVisible(*buttons[d]);
            buttons[d]->setRadioGroupId(1);
            buttons[d]->onClick = [this] { whichNote(); };
            buttons[d]->setClickingTogglesState(true);
        }
        buttons[0]->setToggleState(true, juce::dontSendNotification);
    }

    void resized() override {}

    void paint(juce::Graphics &g) override
    {
        auto bounds = this->getLocalBounds();
        g.setColour(juce::Colour{.475f, 1.f, 0.05f, 1.f});
        g.fillRect(bounds);
        g.setColour(juce::Colours::ghostwhite);
        g.drawRect(bounds);

        bool enabled = this->isEnabled();
        for (int d = 0; d < 12; ++d)
        {
            buttons[d]->setEnabled(enabled);
        }

        int shadowSpacing1 = JIRadius / 20;
        int shadowSpacing2 = JIRadius / 10;
        auto a = enabled ? .75f : .5f;

        whiteShadow.setColor(juce::Colours::ghostwhite.withAlpha(a));
        blackShadow.setColor(juce::Colours::black.withAlpha(a));

        float ctrDistance{JIRadius * (5.f / 3.f)};

        float vhDistance = 2.0f * ctrDistance;

        auto ctrX = getWidth() / 2 - ctrDistance;
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
                            if (proc->currentVisitors->CO[i] == C)
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

                    auto vis = proc->currentVisitors->CC[degree].nameIndex;
                    // Select gradient colour
                    auto gradient = Gradients.commaGrad(vis, x);

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
                    gradient.multiplyOpacity(alpha);
                    sG.setGradientFill(gradient);
                    sG.fillPath(e);
                    sG.setColour(juce::Colours::ghostwhite.withAlpha(alpha));
                    sG.drawEllipse(x - ellipseRadius, y - JIRadius, 2 * ellipseRadius, 2 * JIRadius,
                                   thickness);

                    uint64_t n{1}, d{1};
                    reCalculateCell(n, d, degree);
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
    std::vector<std::unique_ptr<juce::ShapeButton>> buttons;

    melatonin::DropShadow selectedHighlight = {juce::Colours::ghostwhite, 18};

    int calcDist(std::pair<int, int> xy) override
    {
        int res{INT_MAX};

        for (int d = 0; d < 12; ++d)
        {
            int tx = std::abs(xy.first - proc->currentVisitors->CO[d].first);
            int ty = std::abs(xy.second - proc->currentVisitors->CO[d].second);
            int sum = tx + ty;
            if (sum < res)
                res = sum;
        }

        return res;
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

    void reCalculateCell(uint64_t &n, uint64_t &d, int degree) override
    {
        auto [tn, td] = lattices::scaledata::pyth12fractions[degree];
        auto [cn, cd] = proc->currentVisitors->CC[degree].getFraction(degree);
        tn *= cn;
        td *= cd;
        auto gcd = std::gcd(tn, td);
        n = tn / gcd;
        d = td / gcd;
    }
};

#endif // LATTICES_LATTICECOMPONENT_H
