//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include <algorithm>
#include <iostream>
#include <limits>

#include "3rd_party/delaunator.h"
#include "3rd_party/poly_partition.h"
#include "3rd_party/polyline_simplification.h"
#include "compare.h"
#include "imaging.h"
#include "types/color.h"
#include "types/image.h"
#include "types/point.h"
#include "types/pointf.h"
#include "vertex_data.h"


//####################################################################################
//##    Builds an Extruded DrImage Model
//####################################################################################
void DrEngineVertexData::initializeExtrudedImage(DrImage *image, bool wireframe) {

    std::cout << "Extruding, number of polygons: " << static_cast<int>(image->m_poly_list.size()) << std::endl;

    for (int poly_number = 0; poly_number < static_cast<int>(image->m_poly_list.size()); poly_number++) {
        if (image->getBitmap().width < 1 || image->getBitmap().height < 1) continue;

        // ***** Triangulate Concave Hull
        std::vector<DrPointF>              &points =    image->m_poly_list[poly_number];
        std::vector<std::vector<DrPointF>> &hole_list = image->m_hole_list[poly_number];

        double alpha_tolerance = (image->m_outline_processed) ? c_alpha_tolerance : 0.0;
        triangulateFace(points, hole_list, image->getBitmap(), wireframe, Trianglulation::Ear_Clipping, alpha_tolerance);
        //triangulateFace(points, hole_list, image->getBitmap(), wireframe, Trianglulation::Monotone, alpha_tolerance);
        //triangulateFace(points, hole_list, image->getBitmap(), wireframe, Trianglulation::Delaunay, alpha_tolerance);

        // ***** Add extruded triangles from Hull and Holes
        // int slices = wireframe ? 2 : 1;
        // extrudeFacePolygon(points, image->getBitmap().width, image->getBitmap().height, slices);
        // for (auto &hole : hole_list) {
        //     extrudeFacePolygon(hole, image->getBitmap().width, image->getBitmap().height, slices);
        // }
    }

}


//####################################################################################
//##    Smooth / Curve a collection of points representing a 2D outline
//####################################################################################
// Returns a point from a vector with wrap around coverage of vector indices
const DrPointF& pointAt(const std::vector<DrPointF> &point_list, int index) {
    if (index < 0)
        return point_list[index + point_list.size()];
    else if (index >= static_cast<int>(point_list.size()))
        return point_list[index - point_list.size()];
    else
        return point_list[index];
}

const double c_sharp_angle =        110.0;
const double c_smooth_min_size =     50.0;

// Smooths points, neighbors is in each direction (so 1 is index +/- 1 more point in each direction
std::vector<DrPointF> DrEngineVertexData::smoothPoints(const std::vector<DrPointF> &outline_points, int neighbors, double neighbor_distance, double weight) {
    std::vector<DrPointF> smooth_points { };
    if (outline_points.size() < 1) return outline_points;

    // Check size of polygon, accommodate smoothing for small images
    double x_min = outline_points[0].x;
    double x_max = outline_points[0].x;
    double y_min = outline_points[0].y;
    double y_max = outline_points[0].y;
    for (int i = 0; i < static_cast<int>(outline_points.size()); i++) {
        if (outline_points[i].x < x_min) x_min = outline_points[i].x;
        if (outline_points[i].x > x_max) x_max = outline_points[i].x;
        if (outline_points[i].y < y_min) y_min = outline_points[i].y;
        if (outline_points[i].y > y_max) y_max = outline_points[i].y;
    }
    double x_size = x_max - x_min;
    double y_size = y_max - y_min;
    if (x_size < c_smooth_min_size) {
        neighbor_distance /= (c_smooth_min_size / x_size);
    } else if (y_size < c_smooth_min_size) {
        neighbor_distance /= (c_smooth_min_size / y_size);
    }

    // If not enough neighbors, just return starting polygon
    if (static_cast<int>(outline_points.size()) <= (neighbors * 2)) {
        return outline_points;
    }

    // Go through and smooth the points (simple average), don't smooth angles less than c_sharp_angle degrees
    for (int i = 0; i < static_cast<int>(outline_points.size()); ++i) {
        // Current Point
        DrPointF this_point = outline_points[i];
        double total_used = 1.0;
        double x = this_point.x;
        double y = this_point.y;

        // Check if current point is a sharp angle, if so add to list and continue
        DrPointF start_check_point =   pointAt(outline_points, i);
        double   start_check_angle_1 = Dr::CalcRotationAngleInDegrees(start_check_point, pointAt(outline_points, i - 1));
        double   start_check_angle_2 = Dr::CalcRotationAngleInDegrees(start_check_point, pointAt(outline_points, i + 1));
        double   start_diff =          Dr::DifferenceBetween2Angles(start_check_angle_1, start_check_angle_2);
        if (start_diff <= c_sharp_angle) {
            smooth_points.push_back( this_point );
            continue;
        }

        // Check neighbors in both directions for sharp angles, don't include these neighbors for averaging,
        // This allows us to keep sharper corners on square objects
        int average_from = i - neighbors;
        int average_to =   i + neighbors;
        for (int j = i - 1; j >= i - neighbors; j--) {
            DrPointF check_point = pointAt(outline_points, j);
            double check_angle_1 = Dr::CalcRotationAngleInDegrees(check_point, pointAt(outline_points, j - 1));
            double check_angle_2 = Dr::CalcRotationAngleInDegrees(check_point, pointAt(outline_points, j + 1));
            double diff =          Dr::DifferenceBetween2Angles(check_angle_1, check_angle_2);
            if (diff <= c_sharp_angle) { average_from = j + 0 /*1*/; break; }
        }
        for (int j = i + 1; j <= i + neighbors; j++) {
            DrPointF check_point = pointAt(outline_points, j);
            double check_angle_1 = Dr::CalcRotationAngleInDegrees(check_point, pointAt(outline_points, j - 1));
            double check_angle_2 = Dr::CalcRotationAngleInDegrees(check_point, pointAt(outline_points, j + 1));
            double diff =          Dr::DifferenceBetween2Angles(check_angle_1, check_angle_2);
            if (diff <= c_sharp_angle) { average_to =   j - 0 /*1*/; break; }
        }

        // Smooth point
        for (int j = average_from; j <= average_to; j++) {
            // Skip point we're on from adding into average, already added it
            if (j != i) {
                DrPointF check_point = pointAt(outline_points, j);
                if (this_point.distance(check_point) < neighbor_distance) {
                    double weight_reduction = 1.0 / static_cast<double>(abs(j - i));
                    x += (check_point.x * weight * weight_reduction);
                    y += (check_point.y * weight * weight_reduction);
                    total_used +=         weight * weight_reduction;
                }
            }
        }

        // Add to array
        smooth_points.push_back(DrPointF(x / total_used, y / total_used));
    }
    return smooth_points;
}


//####################################################################################
//##    Inserts extra points in between set of points
//####################################################################################
std::vector<DrPointF> DrEngineVertexData::insertPoints(const std::vector<DrPointF> &outline_points) {
    std::vector<DrPointF> insert_points { };

    // Don't insert extra points for simple shapes
    if (outline_points.size() <= 10) return outline_points;

    insert_points.push_back(outline_points[0]);
    for (int i = 1; i < static_cast<int>(outline_points.size()); i++) {

        // Insert extra points for distances greater than 10 pixels
        if (outline_points[i-1].distance(outline_points[i]) > 10.0) {
            DrPointF average = (outline_points[i-1] + outline_points[i]) / 2.0;
            insert_points.push_back(average);
        }
        insert_points.push_back(outline_points[i]);
    }

    return insert_points;
}



//####################################################################################
//##    Triangulate Face and add Triangles to Vertex Data
//####################################################################################
// Finds average number of pixels in a small grid that are transparent
DrColor getRoundedPixel(const DrBitmap &bitmap, const DrPointF &at_point) {
    int px = Dr::Clamp(static_cast<int>(round(at_point.x)), 0, (bitmap.width -  1));
    int py = Dr::Clamp(static_cast<int>(round(at_point.y)), 0, (bitmap.height - 1));
    return bitmap.getPixel(px, py);
}

// Finds average number of pixels in a small grid that are transparent
double averageTransparentPixels(const DrBitmap &bitmap, const DrPointF &at_point, const double &alpha_tolerance) {
    int px = Dr::Clamp(static_cast<int>(round(at_point.x)), 0, (bitmap.width -  1));
    int py = Dr::Clamp(static_cast<int>(round(at_point.y)), 0, (bitmap.height - 1));
    int x_start = (px > 0) ?                 px - 1 : 0;
    int x_end =   (px < bitmap.width - 1)  ? px + 1 : bitmap.width  - 1;
    int y_start = (py > 0) ?                 py - 1 : 0;
    int y_end =   (py < bitmap.height - 1) ? py + 1 : bitmap.height - 1;
    double total_count       = 0;
    double transparent_count = 0;
    for (int x = x_start; x <= x_end; ++x) {
        for (int y = y_start; y <= y_end; ++y) {
            if (bitmap.getPixel(x, y).alphaF() < alpha_tolerance) transparent_count++;
            total_count++;
        }
    }
    return (transparent_count / total_count);
}

void DrEngineVertexData::triangulateFace(const std::vector<DrPointF> &outline_points, const std::vector<std::vector<DrPointF>> &hole_list,
                                         const DrBitmap &image, bool wireframe, Trianglulation type, double alpha_tolerance) {
    int width =  image.width;
    int height = image.height;
    double w2d = width  / 2.0;
    double h2d = height / 2.0;

    // ***** Copy DrPointFs into TPPLPoly
    if (outline_points.size() < 3) return;
    std::list<TPPLPoly> testpolys, result;
    TPPLPoly poly; poly.Init(outline_points.size());
    for (int i = 0; i < static_cast<int>(outline_points.size()); i++) {
        poly[i].x = outline_points[i].x;
        poly[i].y = outline_points[i].y;
    }
    testpolys.push_back( poly );

    // ***** Run triangulation
    TPPLPartition pp;
    switch (type) {
        case Trianglulation::Ear_Clipping:      pp.Triangulate_EC(  &testpolys, &result);       break;
        case Trianglulation::Monotone:          pp.Triangulate_MONO(&testpolys, &result);       break;
        case Trianglulation::Delaunay: {
            result.push_back( poly );
            ///pp.ConvexPartition_OPT(&(*testpolys.begin()), &result);
            ///pp.ConvexPartition_HM(&(*testpolys.begin()), &result);
            break;
        }
    }

    // ***** Add triangulated convex hull to vertex data
    if (type != Trianglulation::Delaunay) {
        for (auto poly : result) {
            float x1 = static_cast<float>(         poly[0].x - w2d);
            float y1 = static_cast<float>(height - poly[0].y - h2d);
            float x2 = static_cast<float>(         poly[1].x - w2d);
            float y2 = static_cast<float>(height - poly[1].y - h2d);
            float x3 = static_cast<float>(         poly[2].x - w2d);
            float y3 = static_cast<float>(height - poly[2].y - h2d);

            float tx1 = static_cast<float>(      poly[0].x / width);
            float ty1 = static_cast<float>(1.0 - poly[0].y / height);
            float tx2 = static_cast<float>(      poly[1].x / width);
            float ty2 = static_cast<float>(1.0 - poly[1].y / height);
            float tx3 = static_cast<float>(      poly[2].x / width);
            float ty3 = static_cast<float>(1.0 - poly[2].y / height);

            triangle( x1, y1, tx1, ty1,
                      x3, y3, tx3, ty3,
                      x2, y2, tx2, ty2);
        }


    // ***** After splitting concave hull into convex polygons, add Delaunay Triangulation to vertex data
    } else {

        // Copy Outline Points into coordinate list
        std::vector<double> coords;
        for (auto poly : result) {
            for (int i = 0; i < poly.GetNumPoints(); i++) {
                coords.push_back(poly[i].x);
                coords.push_back(poly[i].y);
            }
        }

        // Copy Hole Points coordinate list
        for (auto hole : hole_list) {
            for (auto point : hole) {
                coords.push_back(point.x);
                coords.push_back(point.y);
            }
        }

        // Add some uniform points, 4 points looks great and keeps triangles low
        if (wireframe) {
            int x_add, y_add;
            x_add = width /  4;
            y_add = height / 4;
            if (x_add < 1) x_add = 1;
            if (y_add < 1) y_add = 1;
            for (int i = (x_add / 2); i < width; i += x_add) {
                for (int j = (y_add / 2); j < height; j += y_add) {
                    if (image.getPixel(i, j).alphaF() >= alpha_tolerance) {
                        coords.push_back( i );
                        coords.push_back( j );
                    }
                }
            }
        }

        // Check list for duplicates before running triangulation (run with STEP of 2 as coords is stored in x, y pairs)
        std::vector<double> no_duplicates;
        for (std::size_t i = 0; i < coords.size(); i += 2) {
            bool has_it = false;
            for (std::size_t j = i + 2; j < coords.size(); j += 2) {
                if (Dr::FuzzyCompare(coords[i], coords[j]) && Dr::FuzzyCompare(coords[i+1], coords[j+1])) {
                    has_it = true;
                    break;
                }
            }
            if (!has_it) {
                no_duplicates.push_back(coords[i]);
                no_duplicates.push_back(coords[i+1]);
            }
        }
        if (no_duplicates.size() < 6) return;                                           // We need at least 3 points!!


        // Run triangulation, add triangles to vertex data
        Delaunator d(no_duplicates);

        // Delaunay Trianglulation returns a collection of triangles filling a convex hull of a collection of points.
        // So no we have to go through the triangles returned and remove any that are over transparent areas of our object.
        for (size_t i = 0; i < d.triangles.size(); i += 3) {
            double x1 = d.coords[2 * d.triangles[i + 0]];
            double y1 = d.coords[2 * d.triangles[i + 0] + 1];
            double x2 = d.coords[2 * d.triangles[i + 1]];
            double y2 = d.coords[2 * d.triangles[i + 1] + 1];
            double x3 = d.coords[2 * d.triangles[i + 2]];
            double y3 = d.coords[2 * d.triangles[i + 2] + 1];

            // Check each triangle to see if mid-points lie outside concave hull by comparing object image
            DrPoint mid12((x1 + x2) / 2.0, (y1 + y2) / 2.0);
            DrPoint mid23((x2 + x3) / 2.0, (y2 + y3) / 2.0);
            DrPoint mid13((x1 + x3) / 2.0, (y1 + y3) / 2.0);
            DrPointF centroid((x1 + x2 + x3) / 3.0, (y1 + y2 + y3) / 3.0);
            int transparent_count = 0;
            if (image.getPixel(mid12.x, mid12.y).alphaF() < alpha_tolerance) ++transparent_count;
            if (image.getPixel(mid23.x, mid23.y).alphaF() < alpha_tolerance) ++transparent_count;
            if (image.getPixel(mid13.x, mid13.y).alphaF() < alpha_tolerance) ++transparent_count;
            double avg_c = averageTransparentPixels(image, centroid, alpha_tolerance);
            if (avg_c > 0.9999) continue;                                               // #NOTE: 0.9999 is 9 out of 9 pixels are transparent
            if (avg_c > 0.6666) ++transparent_count;                                    // #NOTE: 0.6666 is 6 out of 9 pixels are transparent
            if (transparent_count > 1) continue;

            // Check average of triangle lines and centroid for transparent pixels
            double transparent_average = 0;
            transparent_average += averageTransparentPixels(image, mid12, alpha_tolerance);;
            transparent_average += averageTransparentPixels(image, mid23, alpha_tolerance);
            transparent_average += averageTransparentPixels(image, mid13, alpha_tolerance);
            transparent_average += avg_c;
            if (transparent_average > 2.49) continue;

            // Add triangle
            float ix1 = static_cast<float>(         x1 - w2d);
            float iy1 = static_cast<float>(height - y1 - h2d);
            float ix2 = static_cast<float>(         x2 - w2d);
            float iy2 = static_cast<float>(height - y2 - h2d);
            float ix3 = static_cast<float>(         x3 - w2d);
            float iy3 = static_cast<float>(height - y3 - h2d);

            float tx1 = static_cast<float>(      x1 / width);
            float ty1 = static_cast<float>(1.0 - y1 / height);
            float tx2 = static_cast<float>(      x2 / width);
            float ty2 = static_cast<float>(1.0 - y2 / height);
            float tx3 = static_cast<float>(      x3 / width);
            float ty3 = static_cast<float>(1.0 - y3 / height);

            triangle( ix1, iy1, tx1, ty1,
                      ix2, iy2, tx2, ty2,
                      ix3, iy3, tx3, ty3);
        }   // End For int i

    }   // End If

}





//####################################################################################
//##    Add Extrusion Triangles to Vertex Data
//####################################################################################
void DrEngineVertexData::extrudeFacePolygon(const std::vector<DrPointF> &outline_points, int width, int height, int steps) {
    double w2d = width  / 2.0;
    double h2d = height / 2.0;

    for (int i = 0; i < static_cast<int>(outline_points.size()); i++) {
        int point1 = i + 1;
        int point2 = i;
        if (point1 >= static_cast<int>(outline_points.size())) point1 = 0;

        float  x1 = static_cast<float>(         outline_points[point1].x);
        float  y1 = static_cast<float>(height - outline_points[point1].y);
        float tx1 = static_cast<float>(         outline_points[point1].x / width);
        float ty1 = static_cast<float>(1.0 -    outline_points[point1].y / height);

        float  x2 = static_cast<float>(         outline_points[point2].x);
        float  y2 = static_cast<float>(height - outline_points[point2].y);
        float tx2 = static_cast<float>(         outline_points[point2].x / width);
        float ty2 = static_cast<float>(1.0 -    outline_points[point2].y / height);

        x1 -= static_cast<float>(w2d);
        x2 -= static_cast<float>(w2d);
        y1 -= static_cast<float>(h2d);
        y2 -= static_cast<float>(h2d);

        float pixel_w = (1.0f / width);
        float pixel_h = (1.0f / height);
        if (tx1 > 0.5f) x1 -= pixel_w; else x1 += pixel_w;
        if (tx2 > 0.5f) x2 -= pixel_w; else x2 += pixel_w;
        if (ty1 > 0.5f) y1 -= pixel_h; else y1 += pixel_h;
        if (ty2 > 0.5f) y2 -= pixel_h; else y2 += pixel_h;

        extrude( x1, y1, tx1, ty1,
                 x2, y2, tx2, ty2, steps);
    }
}











