//
// Description:     Polyline Simplification, 2D implementation of the Ramer-Douglas-Peucker algorithm
// Author:          Tim Sheerman-Chase
// License:         CC0 1.0 Universal (CC0 1.0) Public Domain Dedication
// Source(s):       https://gist.github.com/TimSC/0813573d77734bcb6f2cd2cf6cc7aa51
//                  https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
//
// Copyright (C) 2016 by Tim Sheerman-Chase
//
//
// #KEYWORD: "low poly"
//
//
#ifndef POLYLINE_SIMPLIFICATION_H
#define POLYLINE_SIMPLIFICATION_H

#include <vector>

// Forward Declarations
class DrPointF;

//####################################################################################
//##    PolylineSimplification
//##        Decimates a curve composed of line segments to a similar curve with fewer points
//##        (that is to say, reduces number of points in a polygon)...
//############################
class PolylineSimplification
{

public:
    PolylineSimplification();

    static std::vector<DrPointF> RamerDouglasPeucker(const std::vector<DrPointF> &point_list, double epsilon);
};


#endif // POLYLINE_SIMPLIFICATION_H





