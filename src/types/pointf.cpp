//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include <math.h>

#include "../compare.h"
#include "point.h"
#include "pointf.h"


//####################################################################################
//##    Constructors
//####################################################################################
DrPointF::DrPointF() {
    x = 0;
    y = 0;
}

DrPointF::DrPointF(double x_, double y_) {
    x = x_;
    y = y_;
}

DrPointF::DrPointF(const DrPoint point) {
    x = static_cast<double>(point.x);
    y = static_cast<double>(point.y);
}

//####################################################################################
//##    Overload Operators
//####################################################################################
DrPointF& DrPointF::operator=(const DrPointF &other) {
    x = other.x;
    y = other.y;
    return *this;
}

DrPointF DrPointF::operator+(const DrPointF &other) const {
    return DrPointF(x + other.x, y + other.y);
}

DrPointF DrPointF::operator-(const DrPointF &other) const {
    return DrPointF(x - other.x, y - other.y);
}

DrPointF DrPointF::operator*(double k) const {
    return DrPointF(x * k, y * k);
}

DrPointF DrPointF::operator/(double k) const {
    return DrPointF(x / k, y / k);
}

DrPointF& DrPointF::operator*=(double k) {
    x *= k;
    y *= k;
    return *this;
}

bool DrPointF::operator==(const DrPointF &other) const {
    return Dr::FuzzyCompare(x, other.x) && Dr::FuzzyCompare(y, other.y);
}

//####################################################################################
//##    Point Functions
//####################################################################################
double DrPointF::dotProduct(const DrPointF &other) const {
    return x * other.x + y * other.y;
}

double DrPointF::distanceSquared(const DrPointF &to) const {
    return static_cast<double>( ((to.x - x) * (to.x - x) + (to.y - y) * (to.y - y)) );
}

double DrPointF::distance(const DrPointF &to) const {
    return sqrt(distanceSquared(to));
}

double DrPointF::distance(const DrPointF &segment_start, const DrPointF &segment_end) const {
    const double l2 = segment_start.distanceSquared(segment_end);
    if (l2 == 0.0) {
        return distance(segment_start);          // v == w case
    }

    // Consider the line extending the segment, parameterized as v + t (w - v)
    // We find projection of DrPointF p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    const double t = ((*this - segment_start).dotProduct(segment_end - segment_end)) / l2;
    if (t < 0.0) {
        return distance(segment_start);         // Beyond the 'v' end of the segment
    } else if (t > 1.0) {
        return distance(segment_end);           // Beyond the 'w' end of the segment
    }

    // Projection falls on the segment
    DrPointF projection = segment_start + (segment_end - segment_start) * t;
    return distance(projection);
}

double DrPointF::decisionDistance(const std::vector<DrPointF> &DrPointFs) const {
    DrPointF result = DrPointFs[0];
    double dst = distance(DrPointFs[0]);
    for (std::size_t i = 1; i < DrPointFs.size(); i++) {
        DrPointF cur = DrPointFs[i];
        double curDistance = distance(cur);
        if (curDistance < dst) {
            result = cur;
            dst = curDistance;
        }
    }
    return dst;
}


//####################################################################################
//##    Conversions
//####################################################################################
DrPoint DrPointF::toPoint() {
    return DrPoint( x, y );
}








