/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do notes that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include "JIMath.h"

//==============================================================================
struct Visitors
{
    Visitors(std::string n, const JIMath &j, const int *v = nullptr) : name(n)
    {
        jim = &j;

        if (v != nullptr)
        {
            for (int d = 0; d < 12; ++d)
            {
                setDegree(d, v[d]);
            }
        }
    }

    ~Visitors() { jim = nullptr; }

    std::string name = "";
    int vis[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1};
    double tuning[12] = {1.0, 81.0 / 80.0, 1.0,         81.0 / 80.0, 80.0 / 81.0, 1.0, 80.0 / 81.0,
                         1.0, 81.0 / 80.0, 80.0 / 81.0, 81.0 / 80.0, 80.0 / 81.0};
    const JIMath *jim;

    void setTuning(int d, int v)
    {
        bool major = (d == 7 || d == 2 || d == 9 || d == 4 || d == 11 || d == 6);

        switch (v)
        {
        case 0:
            tuning[d] = 1.0;
            break;
        case 1:
            tuning[d] = jim->comma(jim->syntonic, major);
            break;
        case 2:
            tuning[d] = jim->comma(jim->seven, major);
            break;
        case 3:
            tuning[d] = jim->comma(jim->eleven, major);
            break;
        case 4:
            tuning[d] = jim->comma(jim->thirteen, major);
            break;
        case 5:
            tuning[d] = jim->comma(jim->seventeen, major);
            break;
        case 6:
            tuning[d] = jim->comma(jim->nineteen, major);
            break;
        case 7:
            tuning[d] = jim->comma(jim->twentythree, major);
            break;
        }
    }

    void setName(std::string n) { name = n; }

    void setDegree(int d, int v)
    {
        vis[d] = v;
        setTuning(d, v);
    }

    void resetToDefault()
    {
        vis[0] = 0;
        vis[1] = 1;
        vis[2] = 0;
        vis[3] = 1;
        vis[4] = 1;
        vis[5] = 0;
        vis[6] = 1;
        vis[7] = 0;
        vis[8] = 1;
        vis[9] = 1;
        vis[10] = 1;
        vis[11] = 1;

        tuning[0] = 1.0;
        tuning[1] = 81.0 / 80.0;
        tuning[2] = 1.0;
        tuning[3] = 81.0 / 80.0;
        tuning[4] = 80.0 / 81.0;
        tuning[5] = 1.0;
        tuning[6] = 80.0 / 81.0;
        tuning[7] = 1.0;
        tuning[8] = 81.0 / 80.0;
        tuning[9] = 80.0 / 81.0;
        tuning[10] = 81.0 / 80.0;
        tuning[11] = 80.0 / 81.0;
    }

protected:
    // this pythagorean scale is the baseline we use to calculate
    double pyth12[12]{1.0,
                  (double)256 / 243,
                  (double)9 / 8,
                  (double)32 / 27,
                  (double)81 / 64,
                  (double)4 / 3,
                  (double)729 / 512,
                  (double)3 / 2,
                  (double)128 / 81,
                  (double)27 / 16,
                  (double)16 / 9,
                  (double)243 / 128};
};
