//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "../../libs/handmade_math.h"
#include "../compare.h"
#include "vec3.h"


//####################################################################################
//##    Constructors
//####################################################################################
DrVec3::DrVec3()                                { x = 0.f;                      y = 0.f;                        z = 0.f; }
DrVec3::DrVec3(float f)                         { x = f;                        y = f;                          z = f; }
DrVec3::DrVec3(float  x_, float  y_, float  z_) { x = x_;                       y = y_;                         z = z_; }
DrVec3::DrVec3(double x_, double y_, double z_) { x = static_cast<float>(x_);   y = static_cast<float>(y_);     z = static_cast<float>(z_);  }
DrVec3::DrVec3(int    x_, int    y_, int    z_) { x = static_cast<float>(x_);   y = static_cast<float>(y_);     z = static_cast<float>(z_);  }
DrVec3::DrVec3(const DrVec3 &v)                 { x = static_cast<float>(v.x);  y = static_cast<float>(v.y);    z = static_cast<float>(v.z); }


//####################################################################################
//##    Vector 3 Functions
//####################################################################################
// Calculates triangle normal from three points of triangle
DrVec3 DrVec3::triangleNormal(const DrVec3 &point_1, const DrVec3 &point_2, const DrVec3 &point_3) {
    // glm::vec3 n = glm::triangleNormal(glm::vec3(point_1.x, point_1.y, point_1.z),
    //                                   glm::vec3(point_2.x, point_2.y, point_2.z),
    //                                   glm::vec3(point_3.x, point_3.y, point_3.z));

    // Cross product of two lines on plane
    DrVec3 n = (point_1 - point_2) % (point_2 - point_3); 
    n.normalize();

    return DrVec3(n.x, n.y, n.z);
}



//####################################################################################
//##    Overload Operators - Additions
//####################################################################################
DrVec3& DrVec3::operator+=(const DrVec3 &v_) {
    x += v_.x;
    y += v_.y;
    z += v_.z;
    return *this;
}

DrVec3& DrVec3::operator+=(float f_) {
    x += f_;
    y += f_;
    z += f_;
    return *this;
}

DrVec3 DrVec3::operator+(const DrVec3 &v_) const {
    return DrVec3(x+v_.x, y+v_.y, z+v_.z);
}

DrVec3 DrVec3::operator+(float f_) const {
    return DrVec3(x+f_, y+f_, z+f_);
}

// Friend function, left hand side addition
DrVec3 operator+(const float d_, const DrVec3 &vec) {
    return DrVec3(d_+vec.x, d_+vec.y, d_+vec.z);
}


//####################################################################################
//##    Overload Operators - Subtractions
//####################################################################################
DrVec3& DrVec3::operator-=(const DrVec3 &v_) {
    x -= v_.x;
    y -= v_.y;
    z -= v_.z;
    return *this;
}

DrVec3& DrVec3::operator-=(float f_) {
    x -= f_;
    y -= f_;
    z -= f_;
    return *this;
}

// Negative
DrVec3 DrVec3::operator-() const {
    return DrVec3(-x, -y, -z);
}

DrVec3 DrVec3::operator-(const DrVec3 &v_) const {
    return DrVec3(x-v_.x, y-v_.y, z-v_.z);
}

DrVec3 DrVec3::operator-(float f_) const { return DrVec3(x-f_, y-f_, z-f_); }

// Friend function, left hand side subtraction
DrVec3 operator-(const float d_, const DrVec3 &vec) {
    return DrVec3(d_-vec.x, d_-vec.y, d_-vec.z);
}


//####################################################################################
//##    Overload Operators - Comparisons
//####################################################################################
bool DrVec3::operator!=(const DrVec3 &v_) const {
    return (Dr::IsCloseTo(x, v_.x, 0.001f) == false) || (Dr::IsCloseTo(y, v_.y, 0.001f) == false) || (Dr::IsCloseTo(z, v_.z, 0.001f) == false);
}

bool DrVec3::operator==(const DrVec3 &d_) const {
    return Dr::IsCloseTo(x, d_.x, 0.001f) && Dr::IsCloseTo(y, d_.y, 0.001f) && Dr::IsCloseTo(z, d_.z, 0.001f);
}

bool DrVec3::operator<(const DrVec3 &v_) const {
    if (Dr::IsCloseTo(x, v_.x, 0.001f) == false)
        return x < v_.x;
    else if (Dr::IsCloseTo(y, v_.y, 0.001f) == false)
        return y < v_.y;
    else
        return z < v_.z;
}


//####################################################################################
//##    Overload Operators - Divisions
//####################################################################################
DrVec3& DrVec3::operator/=(const float d_) {
    x /= d_;
    y /= d_;
    z /= d_;
    return *this;
}

DrVec3 DrVec3::operator/(const float d_) const {
    return DrVec3(x/d_, y/d_, z/d_);
}

DrVec3 DrVec3::operator/(const DrVec3 &v_) const {
    return DrVec3(x/v_.x, y/v_.y, z/v_.z);
}


//####################################################################################
//##    Overload Operators - Divisions
//####################################################################################
DrVec3& DrVec3::operator*=(const DrVec3 &d_) {
    x *= d_.x;
    y *= d_.y;
    z *= d_.z;
    return *this;
}

DrVec3& DrVec3::operator*=(const float d_) {
    x *= d_;
    y *= d_;
    z *= d_;
    return *this;
}

DrVec3 DrVec3::operator*(const DrVec3 &v_) const {
    return DrVec3(x*v_.x, y*v_.y, z*v_.z);
}

// Right hand side scalar multiplication
DrVec3 DrVec3::operator*(const float d_) const {
    return DrVec3(x*d_, y*d_, z*d_);
}

// Left hand side scalar multiplication
DrVec3 operator*(const float d_, const DrVec3 &vec) {
    return DrVec3(d_*vec.x, d_*vec.y, d_*vec.z);
}

// Left  hand side (lhs) matrix multiplication
// DrVec3 operator*(const glm::mat4 &matrix, const DrVec3 &vec) {
//     glm::vec3 multiplied = glm::vec3(matrix * glm::vec4(glm::vec3(vec.x, vec.y, vec.z), 1.f));
//     return DrVec3(multiplied);
// }


//####################################################################################
//##    Operators on DrVec3
//####################################################################################
// Cross product
DrVec3 DrVec3::cross(const DrVec3 &v_) const {
    return DrVec3(y*v_.z - z*v_.y, 
                  z*v_.x - x*v_.z, 
                  x*v_.y - y*v_.x);
}
// Cross product
DrVec3 DrVec3::operator%(const DrVec3 &rhs) const {
    return DrVec3(y*rhs.z - z*rhs.y, 
                  z*rhs.x - x*rhs.z, 
                  x*rhs.y - y*rhs.x);
}

// Dot product
float DrVec3::dot(const DrVec3 &v_) const {
    return x*v_.x + y*v_.y + z*v_.z;
}

// Compute the cotangent (i.e. 1/tan) between 'this' and v_
float DrVec3::cotan(const DrVec3 &v_) const {
    return (this->dot(v_)) / (this->cross(v_)).norm();
}

float DrVec3::normSquared() const {
    return dot(*this);
}

DrVec3 DrVec3::normalized() const {
    return (*this) * (1.f / std::sqrt(normSquared()));
}

float DrVec3::normalize() {
    float l = std::sqrt(normSquared());
    float f = 1.f / l;
    x *= f;
    y *= f;
    z *= f;
    return l;
}

float DrVec3::norm() const {
    return std::sqrt(normSquared());
}


//####################################################################################
//##    Accessors
//####################################################################################
// Conversion returns the memory address at the index i
const float& DrVec3::operator[](int i) const {
    return (reinterpret_cast<const float*>(this))[i];
}

// Conversion returns the memory address at the index i
float& DrVec3::operator[](int i) {
    return (reinterpret_cast<float*>(this))[i];
}

// Conversion returns the memory address of the vector
// Very convenient to pass a DrVec3 pointer as a parameter to OpenGL, Example:
//          DrVec3 pos, normal;
//          glNormal3fv(normal);
//          glVertex3fv(pos);
DrVec3::operator const float*() const {
    return &x;
}

// Conversion returns the memory address of the vector (non const version)
DrVec3::operator float*() {
    return &x;
}
















