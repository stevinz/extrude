//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "compare.h"
#include "types/color.h"
#include "types/pointf.h"

namespace Dr {


//####################################################################################
//##
//##    Number Comparision Functions
//##
//############################
// Checks to make sure double is NOT Nan and is NOT infinite
bool RealDouble(double number_to_check) {
    return ((std::isnan(number_to_check) == false) && (std::isinf(number_to_check) == false));
}

// Checks to make sure double is NOT Nan and is NOT infinite
bool RealFloat(float number_to_check) {
    return ((std::isnan(number_to_check) == false) && (std::isinf(number_to_check) == false));
}

// Checks to make sure integer is NOT Nan and is NOT infinite
bool RealInteger(int number_to_check) {
    return ((std::isnan(number_to_check) == false) && (std::isinf(number_to_check) == false));
}


//####################################################################################
//##
//##    Angle Comparision Functions
//##
//####################################################################################
//    Calculates angle from a center point to any target point, 0 = Up
//        #KEYWORD: "angleTo", "angleFrom", "angle between", "angleBetween"
//              0
//              |
//        270 --+-- 90
//              |
//             180
double CalcRotationAngleInDegrees(DrPointF center_point, DrPointF target_point) {
    // Calculate the angle in RADIANS from the deltaY and deltaX values (atan2 returns radians values from [-PI, PI]), 0 currently points EAST
    // #NOTE: By preserving Y and X param order to atan2, we are expecting a CLOCKWISE angle direction
    ///double theta = qAtan2(target_point.y() - center_point.y(), target_point.x() - center_point.x());
    double angle = std::atan2(target_point.y - center_point.y, target_point.x - center_point.x);
           angle = RadiansToDegrees( angle );

    // Rotate the returned angle clockwise by 90 degrees (this rotates the possible results counter-clockwise and makes 0 point NORTH)
    // #NOTE: adding to an angle rotates it clockwise, subtracting would rotate it counter-clockwise
    angle += 90.0;      // in radians 3.141592653589793238463 / 2.0;

    // Convert to positive range (0-360) since we want to prevent negative angles, adjust them now
    // We can assume that atan2 will not return a negative value greater than one partial rotation
    if (angle < 0) { angle += 360; }

    return angle;
}

// Returns 'angle' rounded to the nearest 90 multiple of 'angle_to_find'
double Closest90DegreeAngle(double angle, double angle_to_find) {
    double closest_angle =    0.0;
    double distance_apart = 180.0;
    for (double d = angle_to_find - 1080; d < angle_to_find + 1080; d += 90.0) {
        if (abs(d - angle) < distance_apart) {
            closest_angle = d;
            distance_apart = abs(d - angle);
        }
    }
    return closest_angle;
}

// Returns difference between two angles between 0 and 180
double DifferenceBetween2Angles(double angle1, double angle2) {
    while (angle1 <    0) { angle1 += 360; }
    while (angle2 <    0) { angle2 += 360; }
    while (angle1 >= 360) { angle1 -= 360; }
    while (angle2 >= 360) { angle2 -= 360; }

    double diff;
    if (angle1 > angle2) {
        diff = angle1 - angle2;
        if (diff > 180) diff = angle2 - (angle1 - 360);
    } else {
        diff = angle2 - angle1;
        if (diff > 180) diff = angle1 - (angle2 - 360);
    }
    return diff;
}

// Returns true of the two angles are parrallel or perpendicular
bool IsSimilarAngle(double angle1, double angle2, double tolerance) {
    while (angle1 >= 90) { angle1 -= 90; }
    while (angle1 <   0) { angle1 += 90; }
    while (angle2 >= 90) { angle2 -= 90; }
    while (angle2 <   0) { angle2 += 90; }
    return IsCloseTo(angle1, angle2, tolerance);
}

// Returns true is 'check_angle' in equal to 0, 90, 180, or 270, i.e. "square" angle
bool IsSquare(double check_angle) {
    check_angle = abs(check_angle);
    while (check_angle >= 360) check_angle -= 360;
    if (FuzzyCompare(check_angle, 0.0))   return true;
    if (FuzzyCompare(check_angle, 90.0))  return true;
    if (FuzzyCompare(check_angle, 180.0)) return true;
    if (FuzzyCompare(check_angle, 270.0)) return true;
    return false;
}

// Returns true if 4 points make up a rectangle
bool IsRectangle(DrPointF p1, DrPointF p2, DrPointF p3, DrPointF p4) {
    double cx, cy;
    double dd1, dd2, dd3, dd4;

    cx = (p1.x + p2.x + p3.x + p4.x) / 4;
    cy = (p1.y + p2.y + p3.y + p4.y) / 4;

    dd1 = pow(cx - p1.x, 2.0) + pow(cy - p1.y, 2.0);
    dd2 = pow(cx - p2.x, 2.0) + pow(cy - p2.y, 2.0);
    dd3 = pow(cx - p3.x, 2.0) + pow(cy - p3.y, 2.0);
    dd4 = pow(cx - p4.x, 2.0) + pow(cy - p4.y, 2.0);
    return (IsCloseTo(dd1, dd2, 0.001) && IsCloseTo(dd1, dd3, 0.001) && IsCloseTo(dd1, dd4, 0.001));
}

DrPointF RotatePointAroundOrigin(DrPointF point, DrPointF origin, double angle, bool angle_is_in_radians) {
    if (angle_is_in_radians == false) angle = DegreesToRadians(angle);

    double s = sin(angle);
    double c = cos(angle);

    // Translate point to origin
    point.x -= origin.x;
    point.y -= origin.y;

    // Rotate point
    double x = point.x * c - point.y * s;
    double y = point.x * s + point.y * c;

    // Translate point back
    point.x = x + origin.x;
    point.y = y + origin.y;
    return point;
}


//####################################################################################
//##    Color Helper Functions
//####################################################################################
// Compares 2 colors, returns true if they are the same
bool IsSameColor(const DrColor &color1, const DrColor &color2, double tolerance) {
    return ( Dr::IsCloseTo(color1.redF(),   color2.redF(),   tolerance) &&
             Dr::IsCloseTo(color1.greenF(), color2.greenF(), tolerance) &&
             Dr::IsCloseTo(color1.blueF(),  color2.blueF(),  tolerance) &&
             Dr::IsCloseTo(color1.alphaF(), color2.alphaF(), tolerance) );
}


//####################################################################################
//##
//##    Matrix Functions
//##
//############################
hmm_m4 IdentityMatrix() {
    hmm_m4 m;
    m.Elements[0][0] = 1.0;
    m.Elements[0][1] = 0.0;
    m.Elements[0][2] = 0.0;
    m.Elements[0][3] = 0.0;

    m.Elements[1][0] = 0.0;
    m.Elements[1][1] = 1.0;
    m.Elements[1][2] = 0.0;
    m.Elements[1][3] = 0.0;

    m.Elements[2][0] = 0.0;
    m.Elements[2][1] = 0.0;
    m.Elements[2][2] = 1.0;
    m.Elements[2][3] = 0.0;

    m.Elements[3][0] = 0.0;
    m.Elements[3][1] = 0.0;
    m.Elements[3][2] = 0.0;
    m.Elements[3][3] = 1.0;
    return m;
}



}   // End namespace Dr








