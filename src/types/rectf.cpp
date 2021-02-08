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
#include "point.h"
#include "pointf.h"
#include "rect.h"
#include "rectf.h"


//####################################################################################
//##    Constructors
//####################################################################################
DrRectF::DrRectF() {
    x =         0.0;
    y =         0.0;
    width =     0.0;
    height =    0.0;
}
DrRectF::DrRectF(double x_, double y_, double width_, double height_) {
    x =         x_;
    y =         y_;
    width =     width_;
    height =    height_;
}
DrRectF::DrRectF(const DrPointF &top_left, const DrPointF &bottom_right) {
    x =         top_left.x;
    y =         top_left.y;
    width =     (bottom_right.x - top_left.x);
    height =    (bottom_right.y - top_left.y);
}
DrRectF::DrRectF(const DrRect &r) {
    x =         r.x;
    y =         r.y;
    width =     r.width;
    height =    r.height;
}
DrRectF::DrRectF(const DrRectF &r) {
    x =         r.x;
    y =         r.y;
    width =     r.width;
    height =    r.height;
}


//####################################################################################
//##    Conversion
//####################################################################################
DrRect DrRectF::toRect()                { return DrRect(*this); }


//####################################################################################
//##    Helper Functions
//####################################################################################
bool DrRectF::contains(const DrPoint point) { return contains(DrPointF(point)); }
bool DrRectF::contains(const DrPointF pointf) {
    if (pointf.x > left() && pointf.x < right()) {
        if (pointf.y > top() && pointf.y < bottom())
            return true;
    }
    return false;
}


//####################################################################################
//##    Getters
//####################################################################################
double  DrRectF::left()                 { return (width > 0) ? (x) : (x + width + 1); }
double  DrRectF::right()                { return (width > 0) ? (x + width - 1) : (x); }

double  DrRectF::top()                  { return (height > 0) ? (y) : (y + height + 1); }
double  DrRectF::bottom()               { return (height > 0) ? (y + height - 1) : (y); }

DrPointF DrRectF::topLeft()             { return DrPointF(left(),    top()); }
DrPointF DrRectF::topRight()            { return DrPointF(right(),   top()); }
DrPointF DrRectF::bottomLeft()          { return DrPointF(left(),    bottom()); }
DrPointF DrRectF::bottomRight()         { return DrPointF(right(),   bottom()); }






