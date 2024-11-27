/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do notes that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include <algorithm>

#include "Visitors.h"

//==============================================================================
struct VisitorsComponent : public juce::Component
{
    VisitorsComponent(int *v, int ng, std::string names[])
    {
        numGroups = ng;
        circle.addEllipse(0, 0, diameter, diameter);

        juce::Colour n = juce::Colours::white.withAlpha(0.f);
        juce::Colour o = juce::Colours::white.withAlpha(.15f);
        juce::Colour d = juce::Colours::white.withAlpha(.5f);

        for (int i = 0; i < 12; ++i)
        {
            dd[i] = v[i];

            notes.add(new juce::ShapeButton(std::to_string(i + 1), n, o, o));
            notes[i]->setShape(circle, true, true, false);
            addAndMakeVisible(notes[i]);
            notes[i]->setRadioGroupId(1);
            notes[i]->onClick = [this] { selectNote(); };
            notes[i]->setClickingTogglesState(true);

            if (i < 7)
            {
                commas.add(new juce::ShapeButton("option" + std::to_string(i), n, o, o));
                commas[i]->setShape(circle, true, true, false);
                addAndMakeVisible(commas[i]);
                commas[i]->setRadioGroupId(2);
                commas[i]->onClick = [this] { selectComma(); };
                commas[i]->setClickingTogglesState(true);
            }
        }

        for (int g = 0; g < numGroups; ++g)
        {
            groups.add(new juce::ShapeButton(names[g], n, o, o));
            groups[g]->setShape(circle, true, true, false);
            addAndMakeVisible(groups[g]);
            groups[g]->setRadioGroupId(3);
            groups[g]->onClick = [this] { selectGroup(); };
            groups[g]->setClickingTogglesState(true);
        }

        notes[4]->setToggleState(true, juce::dontSendNotification);
        selectedNote = 4;
        commas[1]->setToggleState(true, juce::dontSendNotification);
    }

    void resized() override
    {
        int hOffs = diameter + 55;
        int vOffs = 5;
        int place = 0;
        for (int i = 0; i < 12; ++i)
        {
            if (i == 4)
            {
                hOffs -= radius;
                vOffs += diameter + 5;
                place = 0;
            }

            if (i == 8)
            {
                hOffs -= radius;
                vOffs += diameter + 5;
                place = 0;
            }

            int n = map[i];

            notes[n]->setBounds(hOffs + place, vOffs, diameter, diameter);

            place += diameter + 5;

            if (i < 7)
            {
                commas[i]->setBounds(5 * (1 + i) + diameter * i, diameter * 4, diameter, diameter);
            }
        }
    }

    void paint(juce::Graphics &g) override
    {
        auto b = this->getLocalBounds();

        g.setColour(findColour(juce::TextEditor::backgroundColourId));
        g.fillRect(b);

        g.setColour(findColour(juce::TextEditor::outlineColourId));
        g.drawRect(b, 3);

        int hOffs = diameter + 55;
        int vOffs = 5;
        int place = 0;
        for (int i = 0; i < 12; ++i)
        {
            if (i == 4)
            {
                hOffs -= radius;
                vOffs += diameter + 5;
                place = 0;
            }

            if (i == 8)
            {
                hOffs -= radius;
                vOffs += diameter + 5;
                place = 0;
            }

            int n = map[i];

            int left1 = hOffs + place;

            g.setColour(juce::Colours::black);
            g.fillEllipse(left1, vOffs, diameter, diameter);

            g.setGradientFill(chooseColour(dd[n], (float)left1, (float)left1 + diameter));
            g.fillEllipse(left1, vOffs, diameter, diameter);

            if (notes[n]->getToggleState())
            {
                g.setColour(juce::Colours::white);
                g.drawEllipse(left1, vOffs, diameter, diameter, 3.f);
            }

            place += diameter + 5;

            if (i < 7)
            {
                int left2 = 5.f * (1 + i) + diameter * i;

                g.setColour(juce::Colours::black);
                g.fillEllipse(left2, diameter * 4, diameter, diameter);

                g.setGradientFill(chooseColour(i, (float)left2, (float)left2 + diameter));
                g.fillEllipse(left2, diameter * 4, diameter, diameter);

                g.setColour(juce::Colours::white);
                g.setFont(stoke);
                g.drawFittedText(names[i], left2 + 2, diameter * 4 + 2, diameter - 4, diameter - 4,
                                 juce::Justification::centred, 1, .05f);

                if (commas[i]->getToggleState())
                {
                    g.setColour(juce::Colours::white);
                    g.drawEllipse(left2, diameter * 4, diameter, diameter, 3.f);
                }
            }
        }
    }

    bool update = false;
    bool reselect = false;
    int dd[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1}; // Dimension of degree
    bool madeNewGroup = false;
    int selectedGroup = 0;

    void setGroupData(int *v)
    {
        for (int i = 0; i < 12; ++i)
        {
            dd[i] = v[i];
        }
        repaint();
    }

  private:
    int numGroups{0};

    static constexpr int radius = 20;
    int diameter = 40;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(radius)};

    std::array<std::string, 7> names = {"3", "5", "7", "11", "13", "13/11", "13/10"};

    juce::Path circle;

    juce::OwnedArray<juce::ShapeButton> notes;
    juce::OwnedArray<juce::ShapeButton> commas;
    juce::OwnedArray<juce::ShapeButton> groups;

    int selectedNote{-1};

    const int map[12] = {9, 4, 11, 6, 5, 0, 7, 2, 1, 8, 3, 10};

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

    void selectNote()
    {
        for (int i = 0; i < 12; ++i)
        {
            if (notes[i]->getToggleState())
            {
                selectedNote = i;
                commas[dd[i]]->setToggleState(true, juce::sendNotification);
                repaint();
                break;
            }
        }
    }

    void selectComma()
    {
        for (int i = 0; i < 7; ++i)
        {
            if (commas[i]->getToggleState())
            {
                dd[selectedNote] = i;
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
        juce::Colour n = juce::Colours::white.withAlpha(0.f);
        juce::Colour o = juce::Colours::white.withAlpha(.15f);
        juce::Colour d = juce::Colours::white.withAlpha(.5f);

        groups.add(new juce::ShapeButton("new", n, o, o));
        groups[numGroups]->setShape(circle, true, true, false);
        addAndMakeVisible(groups[numGroups]);
        groups[numGroups]->setRadioGroupId(3);
        groups[numGroups]->onClick = [this] { selectGroup(); };
        groups[numGroups]->setClickingTogglesState(true);
        groups[numGroups]->setToggleState(true, juce::dontSendNotification);

        ++numGroups;
        selectGroup();
    }
};
