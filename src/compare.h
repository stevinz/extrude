//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#ifndef DR_MATH_H
#define DR_MATH_H

#include <cmath>
#include "../libs/handmade_math.h"

// Forward Declarations
class DrColor;
class DrPoint;
class DrPointF;

// Local Defines
#define EPSILON     0.00001
#define DR_INFINITY 1e300
#define DR_PI       3.141592653589793238463

// Math Enums
enum class Winding_Orientation {
    Clockwise           = 0,
    CounterClockwise    = 1,
    LineSegment         = 2,
};

namespace Dr {


    //####################################################################################
    //##    FuzzyCompare
    //##        Returns true if 'number_a' is within +-'EPSILON' of 'number_b'
    //##    IsCloseTo
    //##        Returns true if 'number_desired' is within +-'tolerance' of 'number_to_check'
    //############################
    template<class T> bool  FuzzyCompare(const T& number_a, const T& number_b) {
        return ( (number_a <= (number_b + EPSILON)) && (number_a >= (number_b - EPSILON)) ); }

    template<class T> bool  IsCloseTo(const T& number_desired, const T& number_to_check, const T& tolerance) {
        return ( (number_to_check <= (number_desired + tolerance)) && (number_to_check >= (number_desired - tolerance)) ); }

    /// @brief: Checks to make sure double is NOT Nan and is NOT infinite
    bool                    RealDouble(double number_to_check);
    /// @brief: Checks to make sure double is NOT Nan and is NOT infinite
    bool                    RealFloat(float number_to_check);
    /// @brief: Checks to make sure integer is NOT Nan and is NOT infinite
    bool                    RealInteger(int number_to_check);


    //####################################################################################
    //##    Comparison Functions
    //############################
    // Returns number_to_check fit to within the bounds of min / max
    template<class T> T     Clamp(const T& number_to_check, const T& min, const T& max) {
        if (number_to_check < min) return min;
        if (number_to_check > max) return max;
        return number_to_check;
    }

    // Return the Max of two values
    template<class T> T     Max(const T& a, const T& b) { return (a > b) ? a : b; }

    // Return the Min of two values
    template<class T> T     Min(const T& a, const T& b) { return (a < b) ? a : b; }

    // Linear Interpolation between two values
    template<class T> T     Lerp(const T& f1, const T& f2, const T& t) { return (f1 * (static_cast<T>(1.0) - t)) + (f2 * t); }

    // Linear Interpolation between two values by no more than d
    template<class T> T     LerpConst(const T& f1, const T& f2, const T& d) { return f1 + Clamp(f2 - f1, -d, d); }

    // Swaps two values
    template<class T> void  Swap(T& number1, T& number2) { T temp = number1; number1 = number2; number2 = temp; }


    //####################################################################################
    //##    Range Functions
    //############################
    // Converts value from one range to another
    template<class T> T     RangeConvert(T value, T old_range_min, T old_range_max, T new_range_min, T new_range_max) {
        if (value < old_range_min) value = old_range_min;
        if (value > old_range_max) value = old_range_max;

        T old_range = (old_range_max - old_range_min);
        T new_range = old_range;
        T new_value = value;
        if (Dr::FuzzyCompare(old_range, static_cast<T>(0))) {
            new_value = new_range_min;
        } else {
            new_range = (new_range_max - new_range_min);
            new_value = (((value - old_range_min) * new_range) / old_range) + new_range_min;
        }
        return new_value;
    }

    /// @brief:     Rounds to nearest multiple of m, so m == 5 rounds to nearest multiple of 5, m == 0.1 rounds to nearst first decimal place
    template<class T> T     RoundToMultiple(T value, double m) {
        double rounded = std::round(static_cast<double>(value) / m) * m;
        return static_cast<T>(rounded);
    }


    //####################################################################################
    //##    Angle Functions
    //############################
    template<class T> T     RadiansToDegrees(const T& rad)      { return (rad * 57.295779513082321);    }       // == (180.0 / 3.141592653589793238463);
    template<class T> T     DegreesToRadians(const T& degrees)  { return degrees * (DR_PI / 180.0);     }

    // Equalizes x, y, and z angles to within 0 to 360
    template<class T> T     EqualizeAngle0to360(const T& angle) {
        T equalized = angle;
        while (equalized <   0) { equalized += 360; }
        while (equalized > 360) { equalized -= 360; }
        return equalized;
    }

    // Finds closest angle within 180 degrees of angle (both angles must be between 0 to 360)
    template<class T> T     FindClosestAngle180(const T& start, const T& angle) {
        T closest = angle;
        if (closest - start > 180)
            closest -= 360;
        else if (start - closest > 180)
            closest += 360;
        return closest;
    }

    double      CalcRotationAngleInDegrees(DrPointF center_point, DrPointF target_point);
    double      Closest90DegreeAngle(double angle, double angle_to_find);
    double      DifferenceBetween2Angles(double angle1, double angle2);
    bool        IsSimilarAngle(double angle1, double angle2, double tolerance = 0.001);
    bool        IsSquare(double check_angle);
    bool        IsRectangle(DrPointF p1, DrPointF p2, DrPointF p3, DrPointF p4);
    DrPointF    RotatePointAroundOrigin(DrPointF point, DrPointF origin, double angle, bool angle_is_in_radians = false);


    //####################################################################################
    //##    Color Helper Functions
    //############################
    bool        IsSameColor(const DrColor &color1, const DrColor &color2, double tolerance);


    //####################################################################################
    //##    Matrix Functions
    //############################
    hmm_m4      IdentityMatrix();


}   // End namespace Dr

#endif // DR_MATH_H







