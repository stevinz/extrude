//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#ifndef DR_VEC2_H
#define DR_VEC2_H

#include <cmath>



//####################################################################################
//##    DrVec2
//##        2D Vector with the usual operations + - * / overloaded
//############################
class DrVec2
{

public:
    float x;
    float y;

    // Constructors
    DrVec2();
    DrVec2(float f);
    DrVec2(float x_, float y_);
    DrVec2(const DrVec2 &v);

    // Common Vector 2 Types
    static DrVec2   unitX()         { return DrVec2(1.f, 0.f); }
    static DrVec2   unitY()         { return DrVec2(0.f, 1.f); }
    static DrVec2   zero()          { return DrVec2(0.f, 0.f); }
    static DrVec2   one()           { return DrVec2(1.f, 1.f); }

    // Setters
    void set(float x_, float y_)    { x = x_; y = y_; }
    void setX(float x_)             { x = x_; }
    void setY(float y_)             { y = y_; }

    // Overload Operators - Additions
    DrVec2&         operator+=  (const DrVec2 &v_);
    DrVec2&         operator+=  (float f_);
    DrVec2          operator+   (const DrVec2 &v_) const;
    DrVec2          operator+   (float f_) const;
    friend DrVec2   operator+   (const float d_, const DrVec2 &vec);            // Left hand side scalar clockwise addition

    // Overload Operators - Subtractions
    DrVec2&         operator-=  (const DrVec2 &v_);                             // Opposite vector
    DrVec2&         operator-=  (float f_);
    DrVec2          operator-   (const DrVec2 &v_) const;
    DrVec2          operator-   (float f_) const;
    friend DrVec2   operator-   (const float d_, const DrVec2 &vec);            // Left hand side scalar clockwise substraction

    // Overload Operators - Negative
    DrVec2          operator-   () const;

    // Overload Operators - Comparisons
    bool            operator!=  (const DrVec2 &v_) const;
    bool            operator==  (const DrVec2 &d_) const;
    bool            operator<   (const DrVec2 &v_) const;                       // No mathematical meaning, but nice for std::map ordering

    // Overload Operators - Divisions
    DrVec2&         operator/=  (const float d_);
    DrVec2          operator/   (const float d_) const;
    DrVec2          operator/   (const DrVec2 &v_) const;

    // Overload Operators - Multiplication
    DrVec2&         operator*=  (const DrVec2 &d_);
    DrVec2&         operator*=  (const float d_);
    DrVec2          operator*   (const DrVec2 &v_) const;
    DrVec2          operator*   (const float d_) const;                         // Right hand side scalar multiplication
    friend DrVec2   operator*   (const float d_, const DrVec2 &vec);            // Left  hand side scalar multiplication

    // Operators on Vector3
    float           dot(const DrVec2 &v_) const;                                // Dot product
    float           normSquared() const;
    DrVec2          normalized() const;
    float           normalize();
    float           norm() const;

    // Accessors
    const float& operator[]     (int i) const;
    float& operator[]           (int i);
    operator const float*       () const;                                       // Const Pointer
    operator float*             ();                                             // Pointer

};


#endif // DR_VEC2_H














