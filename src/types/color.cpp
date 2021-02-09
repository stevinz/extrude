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

#include "../compare.h"
#include "../string.h"
#include "color.h"


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

void DrColor::setRgbF(double red, double green, double blue, double alpha) {
    r = Dr::Clamp(static_cast<unsigned char>(red * 255.0),      0_uc, 255_uc);
    g = Dr::Clamp(static_cast<unsigned char>(green * 255.0),    0_uc, 255_uc); 
    b = Dr::Clamp(static_cast<unsigned char>(blue * 255.0),     0_uc, 255_uc); 
    a = Dr::Clamp(static_cast<unsigned char>(alpha * 255.0),    0_uc, 255_uc); 
}

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



//####################################################################################
//##    Rgb / Hsv Conversion Functions
//##
//##    Source:
//##        http://web.mit.edu/GRAPHICS/src/tilemkrs/tile.c
//##        "fractile 1.1, placed in public domain by Steve Kirkendall, 8 April 1996"
//##
//####################################################################################
DrHsv DrColor::getHsv() {
	DrHsv	hsv;	                                                /* the resulting HSV value */
	double	rd, gd, bd;                                             /* the RGB components, in range 0.0 - 1.0 */
	double	max, min;                                               /* extremes from r, g, b */
	double	delta;	                                                /* difference between max and min */

	/* extract the RGB components from rgb */
	rd = redF();
	gd = greenF();
	bd = blueF();

	/* find max and min */
	if (rd > gd) {
		max = (bd > rd) ? bd : rd;
		min = (gd > bd) ? bd : gd;
    } else {
		max = (bd > gd) ? bd : gd;
		min = (rd > bd) ? bd : rd;
	}

	hsv.value = max;                                                /* compute "value" */
	hsv.saturation = (max > 0.0) ? (max - min) / max : 0;           /* compute "saturation" */

	/* compute "hue".  This is the hard one */
	delta = max - min;
	if (delta <= 0.001) {
		hsv.hue = 0.0;                                              /* gray - any hue will work */
	} else {
		/* divide hexagonal color wheel into three sectors */
		if (max == rd)
			hsv.hue = (gd - bd) / delta;                            /* color is between yellow and magenta */
		else if (max == gd)
			hsv.hue = 2.0 + (bd - rd) / delta;                      /* color is between cyan and yellow */
		else /* max == b */
			hsv.hue = 4.0 + (rd - gd) / delta;                      /* color is between magenta and cyan */
		
		hsv.hue *= 60.0;                                            /* convert hue to degrees */

		/* make sure hue is not negative */
		if (hsv.hue < 0.0) hsv.hue += 360.0;
	}
	
	return hsv;
}

void DrColor::setFromHsv(DrHsv hsv) {
	DrRgb	rgb;	    /* the new rgb value */
	double	h;	        /* copy of the "hsv.hue" */
	double	i, f;	    /* integer and fractional parts of "h" */
	int	    p, q, t;    /* permuted RGB values, in integer form */
	int	    v;	        /* "hsv.value", in integer form */

	if (hsv.saturation < 0.01) {
		/* simple gray conversion */
		rgb.red = rgb.green = rgb.blue = (int)(hsv.value * 256.0);
	} else {
		/* convert hue to range [0,6) */
		h = hsv.hue / 60.0;
		if (h >= 6.0)
			h -= 6.0;

		/* break "h" down into integer and fractional parts. */
		i = floor(h);
		f = h - i;

		/* compute the permuted RGB values */
		v = (int)(hsv.value * 256);
		p = (int)((hsv.value * (1.0 - hsv.saturation)) * 256.0);
		q = (int)((hsv.value * (1.0 - (hsv.saturation * f))) * 256.0);
		t = (int)((hsv.value * (1.0 - (hsv.saturation * (1.0 - f)))) * 256.0);

		/* map v, p, q, and t into red, green, and blue values */
		switch ((int)i) {
		  case 0:   rgb.red = v, rgb.green = t, rgb.blue = p;	break;
		  case 1:   rgb.red = q, rgb.green = v, rgb.blue = p;	break;
		  case 2:   rgb.red = p, rgb.green = v, rgb.blue = t;	break;
		  case 3:   rgb.red = p, rgb.green = q, rgb.blue = v;	break;
		  case 4:   rgb.red = t, rgb.green = p, rgb.blue = v;	break;
		  case 5:   rgb.red = v, rgb.green = p, rgb.blue = q;	break;
		}
	}

	setRed(rgb.red);
    setGreen(rgb.green);
    setBlue(rgb.blue);
}







