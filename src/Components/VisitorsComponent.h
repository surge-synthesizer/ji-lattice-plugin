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
#include <vector>

#include "ScaleData.h"
#include "LatticeComponent.h"
#include "LatticesProcessor.h"
#include "LatticesColours.h"

//==============================================================================
struct VisitorsComponent : public juce::Component
{
    VisitorsComponent(LatticesProcessor &p) : proc(&p), Gradients(diameter / 2)
    {
        juce::Colour n{juce::Colours::transparentWhite};
        juce::Colour o{juce::Colours::ghostwhite.withAlpha(.15f)};

        circle.addEllipse(0, 0, diameter, diameter);

        for (int i = 0; i < 7; ++i)
        {
            commaButtons.push_back(
                std::make_unique<juce::ShapeButton>("option" + std::to_string(i), n, o, o));
            commaButtons[i]->setShape(circle, true, true, false);
            addAndMakeVisible(*commaButtons[i]);
            commaButtons[i]->setRadioGroupId(2);
            commaButtons[i]->onClick = [this] { selectComma(); };
            commaButtons[i]->setClickingTogglesState(true);
        }

        miniLattice = std::make_unique<lattice_t>(this, p, 18);
        addAndMakeVisible(*miniLattice);

        leftButton = std::make_unique<juce::TextButton>("<-");
        addAndMakeVisible(*leftButton);
        leftButton->onClick = [this] { scroll(false); };

        rightButton = std::make_unique<juce::TextButton>("->");
        addAndMakeVisible(*rightButton);
        rightButton->onClick = [this] { scroll(true); };

        for (int g = 0; g < proc->numVisitorGroups; ++g)
        {
            groups.push_back(std::make_unique<juce::TextButton>(std::to_string(g)));
            addAndMakeVisible(*groups[g]);
            groups[g]->setRadioGroupId(3);
            groups[g]->onClick = [this] { selectGroup(); };
            groups[g]->setClickingTogglesState(true);
        }

        addButton = std::make_unique<juce::TextButton>("Add");
        addAndMakeVisible(*addButton);
        addButton->onClick = [this] { newGroup(); };

        deleteButton = std::make_unique<juce::TextButton>("Delete");
        addAndMakeVisible(*deleteButton);
        deleteButton->onClick = [this] { deleteGroup(); };

        resetButton = std::make_unique<juce::TextButton>("Reset");
        addAndMakeVisible(*resetButton);
        resetButton->onClick = [this]
        {
            proc->resetVisitorGroup();
            setGroupData();
        };

        groups[selectedGroup]->setToggleState(true, juce::dontSendNotification);
        commaButtons[1]->setToggleState(true, juce::dontSendNotification);
    }

    void resized() override
    {
        auto b = this->getLocalBounds();
        miniLattice->setBounds(1, diameter, b.getWidth() - 2, diameter * 4.5);
        miniLattice->setEnabled(proc->numVisitorGroups > 1 && selectedGroup != 0);

        for (int i = 0; i < 7; ++i)
        {
            commaButtons[i]->setEnabled(selectedGroup != 0);
            commaButtons[i]->setBounds(5 * (1 + i) + diameter * i, diameter * 5 + 30 + 5, diameter,
                                       diameter);
        }

        int num = proc->numVisitorGroups;

        leftButton->setEnabled(num > 16 && scrollPosition != 0);
        leftButton->setBounds(5, 5, boxHeight, boxHeight);
        rightButton->setEnabled(num > 16 && scrollPosition != 16 && scrollPosition < num - 17);
        rightButton->setBounds(b.getRight() - boxHeight - 5, 5, boxHeight, boxHeight);

        int L = scrollPosition;
        int R = L + 16;

        for (int i = 0; i < proc->numVisitorGroups; ++i)
        {
            if (i >= L && i <= R)
            {
                groups[i]->setEnabled(true);
                groups[i]->setVisible(true);
                groups[i]->setBounds(5 + boxHeight + boxWidth * (i - L), 5, boxWidth, boxHeight);
            }
            else
            {
                groups[i]->setEnabled(false);
                groups[i]->setVisible(false);
            }
        }

        addButton->setBounds(5, diameter + 5, 50, 25);
        addButton->setEnabled(proc->numVisitorGroups < 33);
        deleteButton->setBounds(60, diameter + 5, 50, 25);
        deleteButton->setEnabled(proc->numVisitorGroups > 1 && selectedGroup != 0);
        resetButton->setBounds(5, diameter + 35, 50, 25);
        resetButton->setEnabled(proc->numVisitorGroups > 1 && selectedGroup != 0);
    }

    void visibilityChanged() override
    {
        if (isVisible())
        {
            proc->preventVisitorChangesFromProcessor(true);
            proc->selectVisitorGroup(selectedGroup);
        }
        else
        {
            proc->preventVisitorChangesFromProcessor(false);
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
                g.fillEllipse(left2, diameter * 5 + 30 + 5, diameter, diameter);

                g.setGradientFill(Gradients.commaGrad(i, left2 + diameter / 2));
                g.fillEllipse(left2, diameter * 5 + 30 + 5, diameter, diameter);

                g.setColour(juce::Colours::white);
                g.setFont(stoke);
                g.drawFittedText(names[i], left2 + 2, diameter * 5 + 2 + 30 + 5, diameter - 4,
                                 diameter - 4, juce::Justification::centred, 1, .05f);

                if (commaButtons[i]->getToggleState())
                {
                    g.setColour(outlineColour);
                    g.drawEllipse(left2, diameter * 5 + boxHeight + 5, diameter, diameter, 3.f);
                }
            }
        }
    }

    void reset()
    {
        groups.clear();

        for (int g = 0; g < proc->numVisitorGroups; ++g)
        {
            groups.push_back(std::make_unique<juce::TextButton>(std::to_string(g)));
            addAndMakeVisible(*groups[g]);
            groups[g]->setRadioGroupId(3);
            groups[g]->onClick = [this] { selectGroup(); };
            groups[g]->setClickingTogglesState(true);
        }

        for (int i = 0; i < proc->numVisitorGroups; ++i)
        {
            if (proc->currentVisitors == &proc->visitorGroups[i])
            {
                selectedGroup = i;
                break;
            }
        }

        groups[selectedGroup]->setToggleState(true, juce::sendNotification);
        setGroupData();
    }

    void selectNote(int n)
    {
        selectedNote = n;
        commaButtons[proc->currentVisitors->CC[n].nameIndex]->setToggleState(
            true, juce::sendNotification);
        miniLattice->selectedDegree = n;
        repaint();
    }

    int selectedGroup{0};
    int selectedNote{0};

  private:
    LatticesProcessor *proc;

    static constexpr int radius = 20;
    int diameter = 40;
    int boxHeight = 30;
    int boxWidth = 40;

    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(radius)};

    std::array<std::string, 7> names = {"3", "5", "7", "11", "13", "17", "19"};

    std::vector<std::unique_ptr<juce::TextButton>> groups;

    std::unique_ptr<juce::TextButton> deleteButton;
    std::unique_ptr<juce::TextButton> resetButton;
    std::unique_ptr<juce::TextButton> addButton;

    std::unique_ptr<juce::TextButton> leftButton;
    std::unique_ptr<juce::TextButton> rightButton;

    uint8_t scrollPosition{0};

    using lattice_t = SmallLatticeComponent<VisitorsComponent>;
    std::unique_ptr<lattice_t> miniLattice;

    juce::Path circle;
    std::vector<std::unique_ptr<juce::ShapeButton>> commaButtons;

    juce::Colour menuColour{.475f, .5f, 0.2f, 1.f};
    juce::Colour outlineColour{juce::Colours::ghostwhite};

    lattices::colours::GradientProvider Gradients;

    void setGroupData()
    {
        commaButtons[proc->currentVisitors->CC[selectedNote].nameIndex]->setToggleState(
            true, juce::sendNotification);

        resized();
        repaint();
    }

    void selectComma()
    {
        if (selectedGroup == 0)
            return;

        for (int i = 0; i < 7; ++i)
        {
            if (commaButtons[i]->getToggleState())
            {
                proc->updateVisitor(selectedNote, i);
                repaint();
                break;
            }
        }
    }

    void selectGroup()
    {
        for (int i = 0; i < proc->numVisitorGroups; ++i)
        {
            if (groups[i]->getToggleState())
            {
                selectedGroup = i;
                proc->selectVisitorGroup(i);

                if (selectedGroup > 16 + scrollPosition)
                    scrollPosition = selectedGroup - 16;
                break;
            }
        }
        setGroupData();
    }

    void newGroup()
    {
        if (proc->newVisitorGroup())
        {
            int newidx = groups.size();
            groups.push_back(std::make_unique<juce::TextButton>(std::to_string(newidx)));
            addAndMakeVisible(*groups[newidx]);
            groups[newidx]->setRadioGroupId(3);
            groups[newidx]->onClick = [this] { selectGroup(); };
            groups[newidx]->setClickingTogglesState(true);
            groups[newidx]->setToggleState(true, juce::sendNotification);
            selectGroup();
        }
    }

    void deleteGroup()
    {
        if (selectedGroup == 0)
            return;

        proc->deleteVisitorGroup(selectedGroup);

        for (int i = selectedGroup; i < groups.size(); ++i)
        {
            groups[i]->setButtonText(std::to_string(i - 1));
        }
        groups.erase(groups.begin() + selectedGroup);

        if (scrollPosition != 0)
        {
            scrollPosition--;
        }
        groups[selectedGroup - 1]->setToggleState(true, juce::sendNotification);
        selectGroup();
    }

    void scroll(bool right)
    {
        if (right)
        {
            scrollPosition++;
        }
        else
        {
            scrollPosition--;
        }

        int L = scrollPosition;
        int R = L + 16;

        if (selectedGroup < L)
        {
            groups[selectedGroup + 1]->setToggleState(true, juce::sendNotification);
            selectGroup();
        }
        else if (selectedGroup > R)
        {
            groups[selectedGroup - 1]->setToggleState(true, juce::sendNotification);
            selectGroup();
        }
        else
            resized();
    }
};
