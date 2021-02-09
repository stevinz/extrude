//
// Description:     Polyline Simplification, 2D implementation of the Ramer-Douglas-Peucker algorithm
// Author:          Tim Sheerman-Chase
// License:         Released under CC0
// Source(s):       https://gist.github.com/TimSC/0813573d77734bcb6f2cd2cf6cc7aa51
//                  https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
//
// Copyright (C) 2016 by Tim Sheerman-Chase
//
//
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "../types/pointf.h"
#include "polyline_simplification.h"


double PerpendicularDistance(const DrPointF &pt, const DrPointF &line_start, const DrPointF &line_end) {
    double dx = line_end.x - line_start.x;
    double dy = line_end.y - line_start.y;

    // Normalise
    double mag = pow(pow(dx,2.0) + pow(dy,2.0), 0.5);
    if (mag > 0.0) {
        dx /= mag;
        dy /= mag;
    }

    double pvx = pt.x - line_start.x;
    double pvy = pt.y - line_start.y;

    // Get dot product (project pv onto normalized direction)
    double pvdot = dx * pvx + dy * pvy;

    // Scale line direction vector
    double dsx = pvdot * dx;
    double dsy = pvdot * dy;

    // Subtract this from pv
    double ax = pvx - dsx;
    double ay = pvy - dsy;

    return pow(pow(ax,2.0) + pow(ay,2.0), 0.5);
}


std::vector<DrPointF> PolylineSimplification::RamerDouglasPeucker(const std::vector<DrPointF> &point_list, double epsilon) {
    std::vector<DrPointF> simplified;

    if (point_list.size() < 2) {
        throw std::invalid_argument("Not enough points to simplify");
        return point_list;
    }

    // Find the point with the maximum distance from line between start and end
    double dmax = 0.0;
    size_t index = 0;
    size_t end = point_list.size() - 1;
    for (size_t i = 1; i < end; i++) {
        double d = PerpendicularDistance(point_list[i], point_list[0], point_list[end]);
        if (d > dmax) {
            index = i;
            dmax = d;
        }
    }

    // If max distance is greater than epsilon, recursively simplify
    if (dmax > epsilon) {
        // Recursive call
        std::vector<DrPointF> first_line { point_list.begin(), point_list.begin() + static_cast<long>(index) + 1 };
        std::vector<DrPointF> last_line  { point_list.begin() + static_cast<long>(index), point_list.end() };
        std::vector<DrPointF> recursive_results1 = RamerDouglasPeucker(first_line, epsilon);
        std::vector<DrPointF> recursive_results2 = RamerDouglasPeucker(last_line,  epsilon);

        // Build the result list
        simplified.assign(recursive_results1.begin(), recursive_results1.end() - 1);
        simplified.insert(simplified.end(), recursive_results2.begin(), recursive_results2.end());
        if (simplified.size() < 2) {
            throw std::runtime_error("Problem assembling output for Polyline Simplification...");
            return point_list;
        }

    } else {
        //Just return start and end points
        simplified.clear();
        simplified.push_back(point_list[0]);
        simplified.push_back(point_list[end]);
    }
    return simplified;
}




