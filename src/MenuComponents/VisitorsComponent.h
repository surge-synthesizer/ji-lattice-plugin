/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do notes that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include <algorithm>
#include <string>
#include <memory>

#include "Visitors.h"
#include "LatticeComponent.h"

//==============================================================================
struct VisitorsComponent : public juce::Component
{
    VisitorsComponent(int ng, std::string names[])
    {
        juce::Colour n{juce::Colours::transparentWhite};
        juce::Colour o{juce::Colours::ghostwhite.withAlpha(.15f)};

        numGroups = ng;

        circle.addEllipse(0, 0, diameter, diameter);

        for (int i = 0; i < 7; ++i)
        {
            commas.add(new juce::ShapeButton("option" + std::to_string(i), n, o, o));
            commas[i]->setShape(circle, true, true, false);
            addAndMakeVisible(commas[i]);
            commas[i]->setRadioGroupId(2);
            commas[i]->onClick = [this] { selectComma(); };
            commas[i]->setClickingTogglesState(true);
        }

        miniLattice = std::make_unique<lattice_t>(dd, this, 18);
        addAndMakeVisible(*miniLattice);

        plusButton = std::make_unique<juce::TextButton>("+");
        addAndMakeVisible(*plusButton);
        plusButton->onClick = [this] { newGroup(); };

        for (int g = 0; g < numGroups; ++g)
        {
            auto name = (g == 0) ? "None" : std::to_string(g);
            groups.add(new juce::TextButton(name));
            addAndMakeVisible(groups[g]);
            groups[g]->setRadioGroupId(3);
            groups[g]->onClick = [this] { selectGroup(); };
            groups[g]->setClickingTogglesState(true);
        }

        commas[1]->setToggleState(true, juce::dontSendNotification);
    }

    void resized() override
    {
        if (this->isEnabled())
        {
            auto b = this->getLocalBounds();
            miniLattice->setBounds(1, diameter, b.getWidth() - 2, diameter * 4.5);
            miniLattice->setEnabled(numGroups > 1 && selectedGroup != 0);

            for (int i = 0; i < 7; ++i)
            {
                commas[i]->setEnabled(selectedGroup != 0);
                commas[i]->setBounds(5 * (1 + i) + diameter * i, diameter * 5 + boxsize + 5,
                                     diameter, diameter);
            }

            groups[0]->setBounds(5, 5, boxsize * 2, boxsize);
            for (int i = 1; i < numGroups; ++i)
            {
                groups[i]->setBounds(5 + boxsize * (i + 1), 5, boxsize, boxsize);
            }
            plusButton->setBounds(5 + boxsize * (numGroups + 1), 5, boxsize, boxsize);
        }
    }

    void paint(juce::Graphics &g) override
    {
        if (!this->isEnabled())
            return;

        auto b = this->getLocalBounds();

        g.setColour(menuColour);
        g.fillRect(b);

        g.setColour(outlineColour);
        g.drawRect(b, 1);

        if (selectedGroup != 0)
        {
            for (int i = 0; i < 7; ++i)
            {
                int left2 = 5.f * (1 + i) + diameter * i;

                g.setColour(juce::Colours::black);
                g.fillEllipse(left2, diameter * 5 + boxsize + 5, diameter, diameter);

                g.setGradientFill(chooseColour(i, (float)left2, (float)left2 + diameter));
                g.fillEllipse(left2, diameter * 5 + boxsize + 5, diameter, diameter);

                g.setColour(juce::Colours::white);
                g.setFont(stoke);
                g.drawFittedText(names[i], left2 + 2, diameter * 5 + 2 + boxsize + 5, diameter - 4,
                                 diameter - 4, juce::Justification::centred, 1, .05f);

                if (commas[i]->getToggleState())
                {
                    g.setColour(outlineColour);
                    g.drawEllipse(left2, diameter * 5 + boxsize + 5, diameter, diameter, 3.f);
                }
            }
        }
    }

    bool update = false;
    bool reselect = false;
    int dd[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1}; // Dimension of degree
    bool madeNewGroup = false;
    int selectedGroup = 0;
    int selectedNote{0};

    void setGroupData(int *v)
    {
        for (int d = 0; d < 12; ++d)
        {
            dd[d] = v[d];
            miniLattice->updateDegree(d, v[d]);
        }
        int idx{};
        for (int i = 0; i < 7; ++i)
        {
            if (dd[selectedNote] == i)
            {
                idx = i;
                break;
            }
        }
        commas[idx]->setToggleState(true, juce::sendNotification);

        resized();
        repaint();
    }

    void selectNote(int n)
    {
        selectedNote = n;
        commas[dd[n]]->setToggleState(true, juce::sendNotification);
        miniLattice->selectedDegree = n;
        repaint();
    }

  private:
    int numGroups{0};
    static constexpr int radius = 20;
    int diameter = 40;
    int boxsize = 30;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(radius)};

    std::array<std::string, 7> names = {"3", "5", "7", "11", "13", "17", "19"};

    std::unique_ptr<juce::TextButton> plusButton;
    juce::OwnedArray<juce::TextButton> groups;

    using lattice_t = SmallLatticeComponent<VisitorsComponent>;
    std::unique_ptr<lattice_t> miniLattice;

    juce::Path circle;
    juce::OwnedArray<juce::ShapeButton> commas;

    juce::Colour menuColour{.475f, .5f, 0.2f, 1.f};
    juce::Colour outlineColour{juce::Colours::ghostwhite};

    const juce::Colour py1{.5f, .51f, .3f, 1.f};
    const juce::Colour py2{.5277778f, .79f, .41f, .25f};

    const juce::Colour pe1{.35f, .75f, .98f, 1.f};
    const juce::Colour pe2{.5277778f, .79f, .41f, .25f};

    const juce::Colour sep1{.8138889f, 1.f, .8f, 1.f};
    const juce::Colour sep2{.6166667, 1.f, .8f, .1f};

    const juce::Colour und1{.15f, 1.f, 1.f, 1.f};
    const juce::Colour und2{0.f, .84f, 1.f, .02f};

    const juce::Colour trid1{0.f, 1.f, 1.f, 1.f};
    const juce::Colour trid2{.6888889f, 1.f, .96f, .38f};

    const juce::Colour sed1{.4722222f, 1.f, .51f, 1.f};
    const juce::Colour sed2{.1666667f, 1.f, .8f, .71f};

    const juce::Colour nod1{0.f, .84f, 1.f, .02f};
    const juce::Colour nod2{.7361111f, 1.f, 1.f, 1.f};

    //    const juce::Colour vct1{.3833333f, 1.f, .1f, 1.f};
    //    const juce::Colour vct2{.6861111f, .55f, .96f, .48f};

    juce::ColourGradient chooseColour(int c, float left, float right)
    {
        float d = static_cast<float>(diameter);
        juce::Rectangle<float> a{left, right, d, d};

        switch (c)
        {
        case 0:
            return juce::ColourGradient::horizontal(py1, py2, a);
        case 1:
            return juce::ColourGradient::horizontal(pe1, pe2, a);
        case 2:
            return juce::ColourGradient::horizontal(sep1, sep2, a);
        case 3:
            return juce::ColourGradient::horizontal(und1, und2, a);
        case 4:
            return juce::ColourGradient::horizontal(trid1, trid2, a);
        case 5:
            return juce::ColourGradient::horizontal(sed1, sed2, a);
        case 6:
            return juce::ColourGradient::horizontal(nod1, nod2, a);
        default:
            return juce::ColourGradient::horizontal(py1, py2, a);
        }
    }

    void selectComma()
    {
        if (selectedGroup == 0)
            return;

        for (int i = 0; i < 7; ++i)
        {
            if (commas[i]->getToggleState())
            {
                dd[selectedNote] = i;
                miniLattice->updateDegree(selectedNote, i);
                repaint();
                update = true;
                break;
            }
        }
    }

    void selectGroup()
    {
        for (int i = 0; i < numGroups; ++i)
        {
            if (groups[i]->getToggleState())
            {
                selectedGroup = i;
                reselect = true;
                break;
            }
        }
    }

    void newGroup()
    {
        groups.add(new juce::TextButton(std::to_string(numGroups)));
        addAndMakeVisible(groups[numGroups]);
        groups[numGroups]->setRadioGroupId(3);
        groups[numGroups]->onClick = [this] { selectGroup(); };
        groups[numGroups]->setClickingTogglesState(true);
        groups[numGroups]->setToggleState(true, juce::dontSendNotification);

        madeNewGroup = true;
        ++numGroups;
        selectGroup();
    }
};
