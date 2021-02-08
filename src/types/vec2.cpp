//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "../compare.h"
#include "vec2.h"


//####################################################################################
//##    Constructors
//####################################################################################
DrVec2::DrVec2()                        { x = 0.f;                      y = 0.f; }
DrVec2::DrVec2(float f)                 { x = f;                        y = f; }
DrVec2::DrVec2(float x_, float y_)      { x = x_;                       y = y_; }
DrVec2::DrVec2(const DrVec2 &v)         { x = static_cast<float>(v.x);  y = static_cast<float>(v.y); }


//####################################################################################
//##    Overload Operators - Additions
//####################################################################################
DrVec2& DrVec2::operator+=(const DrVec2 &v_) {
    x += v_.x;
    y += v_.y;
    return *this;
}

DrVec2& DrVec2::operator+=(float f_) {
    x += f_;
    y += f_;
    return *this;
}

DrVec2 DrVec2::operator+(const DrVec2 &v_) const {
    return DrVec2(x+v_.x, y+v_.y);
}

DrVec2 DrVec2::operator+(float f_) const {
    return DrVec2(x+f_, y+f_);
}

// Friend function, left hand side addition
DrVec2 operator+(const float d_, const DrVec2 &vec) {
    return DrVec2(d_+vec.x, d_+vec.y);
}


//####################################################################################
//##    Overload Operators - Subtractions
//####################################################################################
DrVec2& DrVec2::operator-=(const DrVec2 &v_) {
    x -= v_.x;
    y -= v_.y;
    return *this;
}

DrVec2& DrVec2::operator-=(float f_) {
    x -= f_;
    y -= f_;
    return *this;
}

// Negative
DrVec2 DrVec2::operator-() const {
    return DrVec2(-x, -y);
}

DrVec2 DrVec2::operator-(const DrVec2 &v_) const {
    return DrVec2(x-v_.x, y-v_.y);
}

DrVec2 DrVec2::operator-(float f_) const { return DrVec2(x-f_, y-f_); }

// Friend function, left hand side subtraction
DrVec2 operator-(const float d_, const DrVec2 &vec) {
    return DrVec2(d_-vec.x, d_-vec.y);
}


//####################################################################################
//##    Overload Operators - Comparisons
//####################################################################################
bool DrVec2::operator!=(const DrVec2 &v_) const {
    return (Dr::IsCloseTo(x, v_.x, 0.001f) == false) || (Dr::IsCloseTo(y, v_.y, 0.001f) == false);
}

bool DrVec2::operator==(const DrVec2 &d_) const {
    return Dr::IsCloseTo(x, d_.x, 0.001f) && Dr::IsCloseTo(y, d_.y, 0.001f);
}

bool DrVec2::operator<(const DrVec2 &v_) const {
    if (Dr::IsCloseTo(x, v_.x, 0.001f) == false)
        return x < v_.x;
    else
        return y < v_.y;
}


//####################################################################################
//##    Overload Operators - Divisions
//####################################################################################
DrVec2& DrVec2::operator/=(const float d_) {
    x /= d_;
    y /= d_;
    return *this;
}

DrVec2 DrVec2::operator/(const float d_) const {
    return DrVec2(x/d_, y/d_);
}

DrVec2 DrVec2::operator/(const DrVec2 &v_) const {
    return DrVec2(x/v_.x, y/v_.y);
}


//####################################################################################
//##    Overload Operators - Divisions
//####################################################################################
DrVec2& DrVec2::operator*=(const DrVec2 &d_) {
    x *= d_.x;
    y *= d_.y;
    return *this;
}

DrVec2& DrVec2::operator*=(const float d_) {
    x *= d_;
    y *= d_;
    return *this;
}

DrVec2 DrVec2::operator*(const DrVec2 &v_) const {
    return DrVec2(x*v_.x, y*v_.y);
}

// Right hand side scalar multiplication
DrVec2 DrVec2::operator*(const float d_) const {
    return DrVec2(x*d_, y*d_);
}

// Left hand side scalar multiplication
DrVec2 operator*(const float d_, const DrVec2 &vec) {
    return DrVec2(d_*vec.x, d_*vec.y);
}

//####################################################################################
//##    Operators on DrVec2
//####################################################################################
// Dot product
float DrVec2::dot(const DrVec2 &v_) const {
    return x*v_.x + y*v_.y;
}

float DrVec2::normSquared() const {
    return dot(*this);
}

DrVec2 DrVec2::normalized() const {
    return (*this) * (1.f / std::sqrt(normSquared()));
}

float DrVec2::normalize() {
    float l = std::sqrt(normSquared());
    float f = 1.f / l;
    x *= f;
    y *= f;
    return l;
}

float DrVec2::norm() const {
    return std::sqrt(normSquared());
}


//####################################################################################
//##    Accessors
//####################################################################################
// Conversion returns the memory address at the index i
const float& DrVec2::operator[](int i) const {
    return (reinterpret_cast<const float*>(this))[i];
}

// Conversion returns the memory address at the index i
float& DrVec2::operator[](int i) {
    return (reinterpret_cast<float*>(this))[i];
}

// Conversion returns the memory address of the vector
// Very convenient to pass a DrVec2 pointer as a parameter to OpenGL:
//      EX:
//          DrVec2 pos, normal;
//          glNormal3fv(normal);
//          glVertex3fv(pos);
DrVec2::operator const float*() const {
    return &x;
}

// Conversion returns the memory address of the vector (non const version)
DrVec2::operator float*() {
    return &x;
}








