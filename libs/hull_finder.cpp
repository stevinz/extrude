//
// Description:     Concave Hull Finder, returned from a collection of points
// Author:          Sam Hocevar
// License:         Licensed under WTFPL 2.0
// Source(s):       http://sam.zoy.org
//                  https://bitbucket.org/vostreltsov/concave-hull/src/default/
//
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                    Version 2, December 2004
//
// Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
//
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
//
//  0. You just DO WHAT THE FUCK YOU WANT TO.
//
//
#include "hull_finder.h"
#include "../src/containers.h"
#include "../src/compare.h"
#include "../src/types/pointf.h"


double HullFinder::IsLeft(DrPointF p0, DrPointF p1, DrPointF p2) {
    return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

bool HullFinder::IsPointInsidePolygon(DrPointF v, const std::vector<DrPointF> &polygon) {
    bool result = false;
    size_t j = polygon.size() - 1;
    for (size_t i = 0; i < polygon.size(); i++) {
        if ((polygon[i].y < v.y && polygon[j].y > v.y) || (polygon[j].y < v.y && polygon[i].y > v.y)) {
            if (polygon[i].x + (v.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < v.x) {
                result = !result;
            }
        }
        j = i;
    }
    return result;
}

bool HullFinder::CheckEdgeIntersection(const DrPointF &p0, const DrPointF &p1, const DrPointF &p2, const DrPointF &p3) {
    double s1_x = p1.x - p0.x;
    double s1_y = p1.y - p0.y;
    double s2_x = p3.x - p2.x;
    double s2_y = p3.y - p2.y;
    double s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / (-s2_x * s1_y + s1_x * s2_y);
    double t = ( s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / (-s2_x * s1_y + s1_x * s2_y);
    return (s > 0 && s < 1 && t > 0 && t < 1);
}

bool HullFinder::CheckEdgeIntersection(const std::vector<DrPointF> &hull, DrPointF curEdgeStart, DrPointF curEdgeEnd, DrPointF checkEdgeStart, DrPointF checkEdgeEnd) {
    for (int i = 0; i < static_cast<int>(hull.size()) - 2; i++) {
        int e1 = i;
        int e2 = i + 1;
        DrPointF p1 = hull[e1];
        DrPointF p2 = hull[e2];

        if (curEdgeStart == p1 && curEdgeEnd == p2) {
            continue;
        }

        if (CheckEdgeIntersection(checkEdgeStart, checkEdgeEnd, p1, p2)) {
            return true;
        }
    }
    return false;
}

DrPointF HullFinder::NearestInnerPoint(DrPointF edgeStart, DrPointF edgeEnd, const std::vector<DrPointF> &DrPointFs, const std::vector<DrPointF> &hull, bool *found) {
    DrPointF result;
    double   distance = 0;
    *found = false;

    for (auto &p : DrPointFs) {
        // Skip DrPointFs that are already in he hull
        if (Dr::VectorContains(hull, p)) {
            continue;
        }
        /*if (!IsPointInsidePolygon(p, hull)) {
            continue;
        }*/

        double d = p.distance(edgeStart, edgeEnd);
        bool skip = false;
        for (size_t i = 0; !skip && i < hull.size() - 1; i++) {
            double dTmp = p.distance(hull[i], hull[i + 1]);
            skip |= dTmp < d;
        }
        if (skip) {
            continue;
        }

        if (!(*found) || distance > d) {
            result = p;
            distance = d;
            *found = true;
        }
    }
    return result;
}

std::vector<DrPointF> HullFinder::FindConvexHull(const std::vector<DrPointF> &DrPointFs) {
    std::vector<DrPointF> P = DrPointFs;
    std::vector<DrPointF> H;

    // Sort P by x and y
    for (size_t i = 0; i < P.size(); i++) {
        for (size_t j = i + 1; j < P.size(); j++) {
            if (P[j].x < P[i].x || (Dr::FuzzyCompare(P[j].x, P[i].x) && P[j].y < P[i].y)) {
                DrPointF tmp = P[i];
                P[i] = P[j];
                P[j] = tmp;
            }
        }
    }

    // the output array H[] will be used as the stack
    int i;                                                                          // array scan index

    // Get the indices of Points with min x-coord and min|max y-coord
    int minmin = 0, minmax;
    double xmin = P[0].x;
    for (i = 1; i < static_cast<int>(P.size()); i++) {
        if (Dr::FuzzyCompare(P[i].x, xmin) == false) break;
    }
    minmax = i - 1;
    if (minmax == static_cast<int>(P.size()) - 1) {                                                   // degenerate case: all x-coords == xmin
        H.push_back(P[minmin]);
        if (Dr::FuzzyCompare(P[minmax].y, P[minmin].y) == false)                    // a nontrivial segment
            H.push_back(P[minmax]);
        H.push_back(P[minmin]);                                                     // add polygon end point
        return H;
    }

    // Get the indices of Points with max x-coord and min|max y-coord
    int maxmin, maxmax = static_cast<int>(P.size()) - 1;
    double xmax = P.back().x;
    for (i = static_cast<int>(P.size()) - 2; i >= 0; i--) {
        if (Dr::FuzzyCompare(P[i].x, xmax) == false) break;
    }
    maxmin = i+1;

    // Compute the lower hull on the stack H
    H.push_back(P[minmin]);                                                         // push  minmin point onto stack
    i = minmax;
    while (++i <= maxmin) {
        // the lower line joins P[minmin]  with P[maxmin]
        if (IsLeft(P[minmin], P[maxmin], P[i]) >= 0 && i < maxmin)
            continue;                                                               // ignore P[i] above or on the lower line

        while (H.size() > 1) {                                                      // there are at least 2 points on the stack
            // test if  P[i] is left of the line at the stack top
            if (IsLeft(H[H.size() - 2], H.back(), P[i]) > 0)
                break;                                                              // P[i] is a new hull  vertex
            H.pop_back();                                                           // pop top point off  stack
        }
        H.push_back(P[i]);                                                          // push P[i] onto stack
    }

    // Next, compute the upper hull on the stack H above  the bottom hull
    if (maxmax != maxmin)                                                           // if  distinct xmax points
         H.push_back(P[maxmax]);                                                    // push maxmax point onto stack
    int bot = static_cast<int>(H.size());                                           // the bottom point of the upper hull stack
    i = maxmin;
    while (--i >= minmax) {
        // the upper line joins P[maxmax]  with P[minmax]
        if (IsLeft( P[maxmax], P[minmax], P[i]) >= 0 && i > minmax)
            continue;                                                               // ignore P[i] below or on the upper line

        while (static_cast<int>(H.size()) > bot)                                    // at least 2 points on the upper stack
        {
            // test if  P[i] is left of the line at the stack top
            if (IsLeft(H[H.size() - 2], H.back(), P[i]) > 0)
                break;                                                              // P[i] is a new hull  vertex
            H.pop_back();                                                           // pop top point off stack
        }
        H.push_back(P[i]);                                                          // push P[i] onto stack
    }
    if (minmax != minmin)
        H.push_back(P[minmin]);                                                     // push joining end point onto stack

    return H;
}

std::vector<DrPointF> HullFinder::FindConcaveHull(const std::vector<DrPointF> &DrPointFs, double N) {
    std::vector<DrPointF> concaveList = FindConvexHull(DrPointFs);

    for (size_t i = 0; i < concaveList.size() - 1; i++) {
        // Find the nearest inner point pk ∈ G from the edge (ci1, ci2);
        DrPointF ci1 = concaveList[i];
        DrPointF ci2 = concaveList[i + 1];

        bool found;
        DrPointF pk = NearestInnerPoint(ci1, ci2, DrPointFs, concaveList, &found);
        if (!found || Dr::VectorContains(concaveList, pk)) {
            continue;
        }

        double eh = ci1.distance(ci2);                              // the length of the edge
        std::vector<DrPointF> tmp;
        tmp.push_back(ci1);
        tmp.push_back(ci2);
        double dd = pk.decisionDistance(tmp);

        if (eh / dd > N) {
            // Check that new candidate edge will not intersect existing edges.
            bool intersects = CheckEdgeIntersection(concaveList, ci1, ci2, ci1, pk);
            intersects |= CheckEdgeIntersection(concaveList, ci1, ci2, pk, ci2);
            if (!intersects) {
                concaveList.insert(concaveList.begin() + static_cast<long>(i) + 1, pk);
                i--;
            }
        }
    }
    return concaveList;
}

//####################################################################################
//##    Makes sure points are in the desired Winding Orientation
//####################################################################################
void HullFinder::EnsureWindingOrientation(std::vector<DrPointF> &points, Winding_Orientation direction_desired) {
    Winding_Orientation winding = HullFinder::FindWindingOrientation(points);
    if ((winding == Winding_Orientation::Clockwise        && direction_desired == Winding_Orientation::CounterClockwise) ||
        (winding == Winding_Orientation::CounterClockwise && direction_desired == Winding_Orientation::Clockwise))
    {
        std::reverse(points.begin(), points.end());
    }
}

//####################################################################################
//##    Returns winding direction of points
//####################################################################################
Winding_Orientation HullFinder::FindWindingOrientation(const std::vector<DrPointF> &points) {
    size_t i1, i2;
    double area = 0;
    for (i1 = 0; i1 < points.size(); i1++) {
        i2 = i1 + 1;
        if (i2 == points.size()) i2 = 0;
        area += points[i1].x * points[i2].y - points[i1].y * points[i2].x;
    }
    if (area > 0) return Winding_Orientation::CounterClockwise;
    if (area < 0) return Winding_Orientation::Clockwise;
    return Winding_Orientation::LineSegment;
}












