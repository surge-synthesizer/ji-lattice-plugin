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
    Visitors(std::string n, const JIMath &jim, int *v = nullptr) : name(n)
    {
        j = &jim;

        if (v != nullptr)
        {
            for (int d = 0; d < 12; ++d)
            {
                setDegree(d, v[d]);
            }
        }
    }

    ~Visitors() { j = nullptr; }

    std::string name = "";
    int vis[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1};
    double tuning[12] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    const JIMath *j;

    void setTuning(int d, int v)
    {
        bool major = (d == 7 || d == 2 || d == 9 || d == 4 || d == 11 || d == 6);

        switch (v)
        {
        case 0:
            tuning[d] = 1.0;
            break;
        case 1:
            tuning[d] = j->comma(j->syntonic, major);
            break;
        case 2:
            tuning[d] = j->comma(j->seven, major);
            break;
        case 3:
            tuning[d] = j->comma(j->eleven, major);
            break;
        case 4:
            tuning[d] = j->comma(j->thirteen, major);
            break;
        case 5:
            tuning[d] = j->comma(j->seventeen, major);
            break;
        case 6:
            tuning[d] = j->comma(j->nineteen, major);
            break;
        case 7:
            tuning[d] = j->comma(j->twentythree, major);
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
        tuning[1] = 1.0;
        tuning[2] = 1.0;
        tuning[3] = 1.0;
        tuning[4] = 1.0;
        tuning[5] = 1.0;
        tuning[6] = 1.0;
        tuning[7] = 1.0;
        tuning[8] = 1.0;
        tuning[9] = 1.0;
        tuning[10] = 1.0;
        tuning[11] = 1.0;
    }
};
