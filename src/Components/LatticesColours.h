//
// Created by Andreya Ek Frisk on 2025-10-10.
//

#ifndef LATTICESCOLOURS_H
#define LATTICESCOLOURS_H

namespace lattices::colours
{

static constexpr int diameter = 40;

static  juce::Colour py1{.5f, .51f, .3f, 1.f};
static  juce::Colour py2{.5277778f, .79f, .41f, .25f};

static  juce::Colour pe1{.35f, .75f, .98f, 1.f};
static  juce::Colour pe2{.5277778f, .79f, .41f, .25f};

static  juce::Colour sep1{.8138889f, 1.f, .8f, 1.f};
static  juce::Colour sep2{.6166667, 1.f, .8f, .1f};

static  juce::Colour und1{.15f, 1.f, 1.f, 1.f};
static  juce::Colour und2{0.f, .84f, 1.f, .02f};

static  juce::Colour trid1{0.f, 1.f, 1.f, 1.f};
static  juce::Colour trid2{.6888889f, 1.f, .96f, .38f};

static  juce::Colour sed1{.4722222f, 1.f, .51f, 1.f};
static  juce::Colour sed2{.1666667f, 1.f, .8f, .71f};

static  juce::Colour nod1{0.f, .84f, 1.f, .02f};
static  juce::Colour nod2{.7361111f, 1.f, 1.f, 1.f};

/*
*/

static juce::ColourGradient sphereColour(int c, float left, float right)
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

}
#endif //LATTICESCOLOURS_H
