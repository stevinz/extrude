//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include <iomanip>

#include "color.h"
#include "../compare.h"
#include "../string.h"


inline constexpr unsigned char operator "" _uc( unsigned long long arg ) noexcept {
    return static_cast<unsigned char>(arg);
}

//####################################################################################
//##    Constructors
//####################################################################################
DrColor::DrColor() {
    r = 0; g = 0; b = 0; a = 255;
}
DrColor::DrColor(const unsigned char &r_, const unsigned char &g_, const unsigned char &b_, const unsigned char &a_) {
    r = Dr::Clamp(static_cast<unsigned char>(r_), 0_uc, 255_uc);
    g = Dr::Clamp(static_cast<unsigned char>(g_), 0_uc, 255_uc);
    b = Dr::Clamp(static_cast<unsigned char>(b_), 0_uc, 255_uc);
    a = Dr::Clamp(static_cast<unsigned char>(a_), 0_uc, 255_uc);
}
DrColor::DrColor(int r_, int g_, int b_, int a_) {
    r = Dr::Clamp(static_cast<unsigned char>(r_), 0_uc, 255_uc);
    g = Dr::Clamp(static_cast<unsigned char>(g_), 0_uc, 255_uc);
    b = Dr::Clamp(static_cast<unsigned char>(b_), 0_uc, 255_uc);
    a = Dr::Clamp(static_cast<unsigned char>(a_), 0_uc, 255_uc);
}
DrColor::DrColor(float r_, float g_, float b_, float a_) {
    r = static_cast<unsigned char>(Dr::Clamp(r_, 0.0f, 1.0f) * 255.0f);
    g = static_cast<unsigned char>(Dr::Clamp(g_, 0.0f, 1.0f) * 255.0f);
    b = static_cast<unsigned char>(Dr::Clamp(b_, 0.0f, 1.0f) * 255.0f);
    a = static_cast<unsigned char>(Dr::Clamp(a_, 0.0f, 1.0f) * 255.0f);
}
DrColor::DrColor(double r_, double g_, double b_, double a_) {
    r = static_cast<unsigned char>(Dr::Clamp(r_, 0.0, 1.0) * 255.0);
    g = static_cast<unsigned char>(Dr::Clamp(g_, 0.0, 1.0) * 255.0);
    b = static_cast<unsigned char>(Dr::Clamp(b_, 0.0, 1.0) * 255.0);
    a = static_cast<unsigned char>(Dr::Clamp(a_, 0.0, 1.0) * 255.0);
}
DrColor::DrColor(unsigned int ui) {
    a = (ui & 0xFF000000) >> 24;
    r = (ui & 0x00FF0000) >> 16;
    g = (ui & 0x0000FF00) >> 8;
    b = (ui & 0x000000FF) >> 0;
}


//####################################################################################
//##    Unsigned Int
//####################################################################################
unsigned int DrColor::rgb() {
    unsigned int color = static_cast<unsigned int>(b) |
                        (static_cast<unsigned int>(g) << 8) |
                        (static_cast<unsigned int>(r) << 16);
    return color;
}
unsigned int DrColor::rgba() {
    unsigned int color = static_cast<unsigned int>(b) |
                        (static_cast<unsigned int>(g) << 8) |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(a) << 24);
    return color;
}
std::string DrColor::name() {
    std::string hex_r = Dr::ToHex(static_cast<int>(r));
    std::string hex_g = Dr::ToHex(static_cast<int>(g));
    std::string hex_b = Dr::ToHex(static_cast<int>(b));

    if (hex_r.length() == 1) hex_r = std::string("0") + hex_r;
    if (hex_g.length() == 1) hex_g = std::string("0") + hex_g;
    if (hex_b.length() == 1) hex_b = std::string("0") + hex_b;
    return std::string("#" + hex_r + hex_g + hex_b);
}


//####################################################################################
//##    Setters
//####################################################################################
void DrColor::setRed(int red)           { r = Dr::Clamp(static_cast<unsigned char>(red),            0_uc, 255_uc); }
void DrColor::setRedF(double red)       { r = Dr::Clamp(static_cast<unsigned char>(red * 255.0),    0_uc, 255_uc); }
void DrColor::setGreen(int green)       { g = Dr::Clamp(static_cast<unsigned char>(green),          0_uc, 255_uc); }
void DrColor::setGreenF(double green)   { g = Dr::Clamp(static_cast<unsigned char>(green * 255.0),  0_uc, 255_uc); }
void DrColor::setBlue(int blue)         { b = Dr::Clamp(static_cast<unsigned char>(blue),           0_uc, 255_uc); }
void DrColor::setBlueF(double blue)     { b = Dr::Clamp(static_cast<unsigned char>(blue * 255.0),   0_uc, 255_uc); }
void DrColor::setAlpha(int alpha)       { a = Dr::Clamp(static_cast<unsigned char>(alpha),          0_uc, 255_uc); }
void DrColor::setAlphaF(double alpha)   { a = Dr::Clamp(static_cast<unsigned char>(alpha * 255.0),  0_uc, 255_uc); }


//####################################################################################
//##    Color Editing
//####################################################################################
// Values are 0 to 255+ range, redistributes overflow of highest value to other values (i.e. DrColor(260r, 0g, 0b) becomes DrColor(255r, 2.5b, 2.5g)
DrColor DrColor::redistributeRgb(double r, double g, double b) {
    double extra = 0.0;
    if (r > 255.0) {
        extra = r - 255.0;
        b += extra * (b / (b + g));
        g += extra * (g / (b + g));
    } else if (g > 255.0) {
        extra = g - 255.0;
        r += extra * (r / (r + b));
        b += extra * (b / (r + b));
    } else if (b > 255.0) {
        extra = b - 255.0;
        r += extra * (r / (r + g));
        g += extra * (g / (r + g));
    }
    int ir = Dr::Clamp(static_cast<int>(r), 0, 255);
    int ig = Dr::Clamp(static_cast<int>(g), 0, 255);
    int ib = Dr::Clamp(static_cast<int>(b), 0, 255);
    return DrColor(ir, ig, ib);
}

// Darkens color by percent
DrColor DrColor::darker(int percent) {
    // Can't process negative percent
    if (percent <= 0) return *this;

    // Convert to multiplier, multiply rgb values, redistribute overflows
    double m = 1.0 / (static_cast<double>(percent) / 100.0);
    return redistributeRgb(m * this->red(), m * this->green(), m * this->blue());
}

// Lightens color by percent
DrColor DrColor::lighter(int percent) {
    // Can't process negative percent
    if (percent <= 0) return (*this);

    // Convert to multiplier, multiply rgb values, redistribute overflows
    double m = static_cast<double>(percent) / 100.0;
    return redistributeRgb(m * this->red(), m * this->green(), m * this->blue());
}


//####################################################################################
//##    Overload Operators
//####################################################################################
DrColor& DrColor::operator=(const DrColor &other) {
    r = other.r;
    g = other.g;
    b = other.b;
    a = other.a;
    return *this;
}

DrColor DrColor::operator+(const DrColor &other) const {
    return DrColor(Dr::Clamp(r + other.r, 0, 255),
                   Dr::Clamp(g + other.g, 0, 255),
                   Dr::Clamp(b + other.b, 0, 255),
                   Dr::Clamp(a + other.a, 0, 255));
}

DrColor DrColor::operator-(const DrColor &other) const {
    return DrColor(Dr::Clamp(r - other.r, 0, 255),
                   Dr::Clamp(g - other.g, 0, 255),
                   Dr::Clamp(b - other.b, 0, 255),
                   Dr::Clamp(a - other.a, 0, 255));
}

DrColor DrColor::operator*(int k) const {
    return DrColor(Dr::Clamp(r * k, 0, 255),
                   Dr::Clamp(g * k, 0, 255),
                   Dr::Clamp(b * k, 0, 255),
                   Dr::Clamp(a * k, 0, 255));
}

DrColor DrColor::operator/(int k) const {
    if (k == 0) return DrColor(255, 255, 255, 255);
    return DrColor(Dr::Clamp(r / k, 0, 255),
                   Dr::Clamp(g / k, 0, 255),
                   Dr::Clamp(b / k, 0, 255),
                   Dr::Clamp(a / k, 0, 255));
}

bool DrColor::operator==(const DrColor &other) const {
    return (r == other.r) && (g == other.g) && (b == other.b) && (a == other.a);
}

bool DrColor::operator!=(const DrColor &other) const {
    return (r != other.r) || (g != other.g) || (b != other.b) || (a != other.a);
}











