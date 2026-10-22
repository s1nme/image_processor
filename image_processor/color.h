#pragma once

struct Color {
    double red;
    double green;
    double blue;
    Color();
    Color(double r, double g, double b);
    void Clamp();
};
