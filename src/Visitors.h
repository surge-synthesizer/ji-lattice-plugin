/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do notes that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

//==============================================================================
struct Visitors
{
    Visitors(std::string n, int *v = nullptr) : name(n)
    {
        if (v != nullptr)
        {
            for (int i = 0; i < 12; ++i)
            {
                dimensions[i] = v[i];
            }
        }
    }

    int dimensions[12] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1};
    std::string name = "";

    void setName(std::string n) { name = n; }

    void setDegree(int deg, int dim) { dimensions[deg] = dim; }

    void saveGroup(int *dim, std::string &n)
    {
        n = name;

        for (int i = 0; i < 12; ++i)
        {
            dim[i] = dimensions[i];
        }
    }
    void loadGroup(int *dim, std::string n)
    {
        name = n;

        for (int i = 0; i < 12; ++i)
        {
            dimensions[i] = dim[i];
        }
    }
};
