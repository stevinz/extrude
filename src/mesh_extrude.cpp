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
#include <limits>
#include <list>
#include <vector>

#include "3rd_party/mesh_optimizer/meshoptimizer.h"
#include "3rd_party/poly_partition.h"
#include "3rd_party/polyline_simplification.h"
#include "compare.h"
#include "imaging.h"
#include "mesh.h"
#include "types/color.h"
#include "types/image.h"
#include "types/point.h"
#include "types/pointf.h"
#include "types/polygonf.h"

//####################################################################################
//##    Builds an Extruded DrImage Model
//####################################################################################
void DrMesh::initializeExtrudedImage(DrImage *image, int quality, float depth_multiplier) {

    int w = image->getBitmap().width;
    int h = image->getBitmap().height;

    for (int poly_number = 0; poly_number < static_cast<int>(image->m_poly_list.size()); poly_number++) {
        if (w < 1 || h < 1) continue;

        // ***** Triangulate Concave Hull
        std::vector<DrPointF>              &points =    image->m_poly_list[poly_number];
        std::vector<std::vector<DrPointF>> &hole_list = image->m_hole_list[poly_number];

        // ***** Pick ONE of the following three
        double alpha_tolerance = (image->m_outline_processed) ? c_alpha_tolerance : 0.0;
        triangulateFace(points, hole_list, image->getBitmap(), wireframe, Trianglulation::Triangulate_Opt, alpha_tolerance, depth_multiplier);
        //triangulateFace(points, hole_list, image->getBitmap(), wireframe, Trianglulation::Ear_Clipping, alpha_tolerance, depth_multiplier)
        //triangulateFace(points, hole_list, image->getBitmap(), wireframe, Trianglulation::Monotone, alpha_tolerance, depth_multiplier);
        
        // !!!!! #TODO: For greatly improved Trianglulation::Delaunay, break polygon into convex polygons before running algorithm

        // ***** Add extruded triangles from Hull and Holes
        //int slices = wireframe ? 3 : 1;
        int slices = (quality / 3) + 1;
        extrudeFacePolygon(points, w, h, slices, false, depth_multiplier);
        for (auto &hole : hole_list) {
            extrudeFacePolygon(hole, w, h, slices, false, depth_multiplier);
        }
    }

    // Optimize and smooth mesh
    optimizeMesh();

    // ----- Experimental, doesnt work great yet -----
    //smoothMesh();                               
}


//####################################################################################
//##    Optimize Mesh
//####################################################################################
void DrMesh::optimizeMesh() {
    // Remap Table
    DrMesh result;
    size_t total_indices = vertexCount();
    std::vector<unsigned int> remap(total_indices);        
    size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));
	    
    // 1. Indexing
    result.indices.resize(total_indices);
	meshopt_remapIndexBuffer(&result.indices[0], NULL, total_indices, &remap[0]);
	result.vertices.resize(total_vertices);
	meshopt_remapVertexBuffer(&result.vertices[0], &vertices[0], total_indices, sizeof(Vertex), &remap[0]);
    // 2. Vertex cache optimization
    meshopt_optimizeVertexCache(&result.indices[0], &result.indices[0], result.indices.size(), result.vertices.size());
    // 3. Overdraw optimization
    meshopt_optimizeOverdraw(&result.indices[0], &result.indices[0], result.indices.size(), &result.vertices[0].px, result.vertices.size(), sizeof(Vertex), 1.05f);
    // 4. Vertex fetch optimization
    meshopt_optimizeVertexFetch(&result.vertices[0], &result.indices[0], result.indices.size(), &result.vertices[0], result.vertices.size(), sizeof(Vertex));

    // Set indices and vertices from optimized mesh
    indices.resize(result.indices.size());
    vertices.resize(result.vertices.size());
    for (size_t i = 0; i < result.indices.size(); i++) indices[i] = result.indices[i];
    for (size_t i = 0; i < result.vertices.size(); i++) vertices[i] = result.vertices[i];
}


//####################################################################################
//##    Smooth Mesh
//####################################################################################
void DrMesh::smoothMesh() {
    std::vector<int> counts;
    counts.resize(11);

    // Keep list of vertices processed
    std::vector<bool> processed;
    for(size_t i = 0; i < vertexCount(); i++) {
        processed.push_back(false);
    }

    // Keep list of vertices smoothed
    std::vector<Vertex> smoothed;
    smoothed.resize(vertexCount());

    // Loop through all points, find neighbors, then average the points
    for(size_t one_vertex = 0; one_vertex < vertexCount(); one_vertex++) {
        if (processed[one_vertex]) continue;

        // Find list of points that are identical
        std::list<unsigned int> same_points;
        same_points.push_back(one_vertex);
        processed[one_vertex] = true;
        for(size_t j = 0; j < vertexCount(); j++) {
            if (one_vertex == j) continue;
            if (Dr::IsCloseTo(vertices[one_vertex].px, vertices[j].px, 0.5f) &&
                Dr::IsCloseTo(vertices[one_vertex].py, vertices[j].py, 0.5f) &&
                Dr::IsCloseTo(vertices[one_vertex].pz, vertices[j].pz, 0.5f)) {
                    same_points.push_back(j);
                    processed[j] = true;
            }
        }
        same_points.unique();

        // ***** Get list of neighbors
        std::list<unsigned int> neighbors;
        for(auto point : same_points) {
            for(size_t j = 0; j < indexCount(); j += 3) {
                if        (indices[j+0] == point) {
                    neighbors.push_back(indices[j+1]);   
                    neighbors.push_back(indices[j+2]);
                } else if (indices[j+1] == point) {
                    neighbors.push_back(indices[j+0]);   
                    neighbors.push_back(indices[j+2]);
                } else if (indices[j+2] == point) {
                    neighbors.push_back(indices[j+0]);   
                    neighbors.push_back(indices[j+1]);
                }
            }
        }
        neighbors.unique();

        // ***** Average with neighbors
        float weight = 1.0;
        Vertex v = vertices[one_vertex];
        Vertex o = vertices[one_vertex];
        v.px *= weight;
        v.py *= weight;           
        v.pz *= weight;
        v.nx *= weight;
        v.ny *= weight;
        v.nz *= weight;
        v.tx *= weight;
        v.ty *= weight;
        for(auto n : neighbors) {
            // Add neighbor diminished by distance
            float edge_length = DrVec3(o.px, o.py, o.pz).distance({vertices[n].px, vertices[n].py, vertices[n].pz});
            if (edge_length ==0) edge_length = std::numeric_limits<float>::epsilon();
            float d = 1.f / edge_length;
            v.px += (vertices[n].px * d);
            v.py += (vertices[n].py * d);
            v.pz += (vertices[n].pz * d);
            v.nx += (vertices[n].nx * d);
            v.ny += (vertices[n].ny * d);
            v.nz += (vertices[n].nz * d);
            v.tx += (vertices[n].tx * d);
            v.ty += (vertices[n].ty * d);
            weight += d;
        }
        v.px /= weight;
        v.py /= weight;
        v.pz /= weight;
        v.nx /= weight;
        v.ny /= weight;
        v.nz /= weight;
        v.tx = Dr::Clamp(v.tx/weight, 0.f, 1.f);
        v.ty = Dr::Clamp(v.ty/weight, 0.f, 1.f);

        DrVec3 normal = DrVec3(v.nx, v.ny, v.nz).normalized();
        v.nx = normal.x;
        v.ny = normal.y;
        v.nz = normal.z;
                
        // Set all same points to new averaged point
        for(auto point : same_points) {
            DrMesh::set(v, smoothed[point]);            
        }
    }

    // ***** Set smoothed vertices
    for (size_t i = 0; i < vertices.size(); i++) {
        DrMesh::set(smoothed[i], vertices[i]);
    }

    // ***** Reset barycentric coordinates
    for (size_t i = 0; i < indices.size(); i += 3) {
        vertices[indices[i+0]].bx = 1;
        vertices[indices[i+0]].by = 0;
        vertices[indices[i+0]].bz = 0;
        vertices[indices[i+1]].bx = 0;
        vertices[indices[i+1]].by = 1;
        vertices[indices[i+1]].bz = 0;
        vertices[indices[i+2]].bx = 0;
        vertices[indices[i+2]].by = 0;
        vertices[indices[i+2]].bz = 1;
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
std::vector<DrPointF> DrMesh::smoothPoints(const std::vector<DrPointF> &outline_points, int neighbors, double neighbor_distance, double weight) {
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
//##        Useful for adding points before a smoothing function to add weight to the
//##        middle of ong straight sections
//####################################################################################
std::vector<DrPointF> DrMesh::insertPoints(const std::vector<DrPointF> &outline_points) {
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

void DrMesh::triangulateFace(const std::vector<DrPointF> &outline_points, const std::vector<std::vector<DrPointF>> &hole_list,
                             const DrBitmap &image, bool wireframe, Trianglulation type, double alpha_tolerance, float depth_multiplier) {
    int width =  image.width;
    int height = image.height;
    double w2d = width  / 2.0;
    double h2d = height / 2.0;

    // ***** Copy DrPointFs into TPPLPoly
    if (outline_points.size() < 3) return;
    std::list<TPPLPoly> testpolys, result;
    TPPLPoly poly; 
    poly.Init(outline_points.size());
    for (int i = 0; i < static_cast<int>(outline_points.size()); i++) {
        poly[i].x = outline_points[i].x;
        poly[i].y = outline_points[i].y;
    }
    testpolys.push_back( poly );

    // ***** Remove holes
    int hole_count = 0;
    for (auto hole : hole_list) {
        int point_count = 0;
        Winding_Orientation winding = DrPolygonF::findWindingOrientation(hole);
        if (winding == Winding_Orientation::CounterClockwise) {
            std::reverse(hole.begin(), hole.end());
        }
        TPPLPoly poly; 
        poly.Init(hole.size());
        poly.SetHole(true);
        for (int i = 0; i < static_cast<int>(hole.size()); i++) {
            poly[i].x = hole[i].x;
            poly[i].y = hole[i].y;
            point_count++;
        }
        if (point_count >= 3) {
            testpolys.push_back(poly);
            hole_count++;
        }
    }
    
    TPPLPartition pp;
    std::list<TPPLPoly> outpolys;

    if (hole_count > 0) {
        pp.RemoveHoles(&testpolys, &outpolys);
    } else {
        outpolys = testpolys;
    }

    // ***** Run triangulation
    switch (type) {
        case Trianglulation::Ear_Clipping:      pp.Triangulate_EC(&outpolys, &result);                  break;
        case Trianglulation::Triangulate_Opt:   pp.Triangulate_OPT(&(*outpolys.begin()), &result);      break;
        case Trianglulation::Monotone:          pp.Triangulate_MONO(&outpolys, &result);                break; 
    }

    // ***** Add triangulated convex hull to vertex data
    for (auto poly : result) {
        float x1 = static_cast<float>(         poly[0].x - w2d);
        float y1 = static_cast<float>(height - poly[0].y - h2d);
        float x2 = static_cast<float>(         poly[1].x - w2d);
        float y2 = static_cast<float>(height - poly[1].y - h2d);
        float x3 = static_cast<float>(         poly[2].x - w2d);
        float y3 = static_cast<float>(height - poly[2].y - h2d);

        float tx1 = static_cast<float>(poly[0].x / static_cast<double>(width));
        float ty1 = static_cast<float>(poly[0].y / static_cast<double>(height));
        float tx2 = static_cast<float>(poly[1].x / static_cast<double>(width));
        float ty2 = static_cast<float>(poly[1].y / static_cast<double>(height));
        float tx3 = static_cast<float>(poly[2].x / static_cast<double>(width));
        float ty3 = static_cast<float>(poly[2].y / static_cast<double>(height));

        triangle(x1, y1, tx1, ty1,
                 x3, y3, tx3, ty3,
                 x2, y2, tx2, ty2, depth_multiplier);
    }
    
}

//####################################################################################
//##    Add Extrusion Triangles to Vertex Data
//####################################################################################
void DrMesh::extrudeFacePolygon(const std::vector<DrPointF> &outline_points, int width, int height, int steps, bool reverse, float depth_multiplier) {
    double w2d = width  / 2.0;
    double h2d = height / 2.0;

    for (int i = 0; i < static_cast<int>(outline_points.size()); i++) {
        int point1 = i + 1;
        int point2 = i;
        if (point1 >= static_cast<int>(outline_points.size())) point1 = 0;

        float  x1 = static_cast<float>(         outline_points[point1].x);
        float  y1 = static_cast<float>(height - outline_points[point1].y);
        float tx1 = static_cast<float>(         outline_points[point1].x / width);
        float ty1 = static_cast<float>(         outline_points[point1].y / height);

        float  x2 = static_cast<float>(         outline_points[point2].x);
        float  y2 = static_cast<float>(height - outline_points[point2].y);
        float tx2 = static_cast<float>(         outline_points[point2].x / width);
        float ty2 = static_cast<float>(         outline_points[point2].y / height);

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

        if (reverse == false) {
            extrude(x1, y1, tx1, ty1,
                    x2, y2, tx2, ty2, steps, depth_multiplier);
        } else {
            extrude(x2, y2, tx2, ty2,
                    x1, y1, tx1, ty1, steps, depth_multiplier);
        }
    }
}



