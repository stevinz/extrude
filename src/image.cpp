//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "../libs/hull_finder.h"
#include "../libs/polyline_simplification.h"
#include "image.h"
#include "image_filters.h"
#include "mesh.h"
#include "vertex_data.h"

// Local Constants
const int c_neighbors =             5;                  // Number of neighbors to smooth points with


//####################################################################################
//##    Constructors
//####################################################################################
DrImage::DrImage(std::string image_name, DrBitmap &bitmap, bool outline) {
    this->m_simple_name = image_name;
    this->m_bitmap = bitmap;

    if (outline) {
        autoOutlinePoints();
    } else {
        m_poly_list.push_back( bitmap.polygon().points() );
        m_hole_list.push_back( { } );
    }
}



//####################################################################################
//##    Sets Image Shape as simple box
//####################################################################################
void DrImage::setSimpleBox() {
    std::vector<DrPointF> one_poly = m_bitmap.polygon().points();
    std::vector<std::vector<DrPointF>> hole_list {{ }};
    HullFinder::EnsureWindingOrientation(one_poly, Winding_Orientation::CounterClockwise);
    m_poly_list.clear();
    m_hole_list.clear();
    m_poly_list.push_back(one_poly);
    m_hole_list.push_back(hole_list);
    m_outline_canceled =  true;
    m_outline_processed = false;
}


//####################################################################################
//##    Loads list of points for Image and Image Holes
//####################################################################################        
void DrImage::autoOutlinePoints() {
    m_poly_list.clear();
    m_hole_list.clear();

    // ***** Break pixmap into seperate images for each object in image
    std::vector<DrBitmap>   bitmaps;
    std::vector<DrRect>     rects;
    bool    cancel = Dr::FindObjectsInBitmap(m_bitmap, bitmaps, rects, c_alpha_tolerance, true);
    int     number_of_objects = static_cast<int>(bitmaps.size());

    // ***** If Find Objects In Bitmap never finished, just add simple box shape
    if (cancel) { setSimpleBox(); return; }

    // ******************** Go through each image (object) and Polygon for it
    for (int image_number = 0; image_number < number_of_objects; image_number++) {
        // Grab next image, check if its valid
        DrBitmap &image = bitmaps[image_number];
        if (image.width < 1 || image.height < 1) continue;

        // Trace edge of image
        std::vector<DrPointF> one_poly = Dr::TraceImageOutline(image);

        // Add rect offset, and add 1.00 pixels buffer around image
        double plus_one_pixel_percent_x = 1.0 + (1.00 / m_bitmap.width);
        double plus_one_pixel_percent_y = 1.0 + (1.00 / m_bitmap.height);
        for (auto &point : one_poly) {
            point.x += rects[image_number].left();
            point.y += rects[image_number].top();
            point.x = point.x * plus_one_pixel_percent_x;
            point.y = point.y * plus_one_pixel_percent_y;
        }

        // Old Way of Simplifying Points on Similar Slopes
        ///int split = wireframe ? int((((image.width() + image.height()) / 2) * 0.2) / 5) : 1000;      // Splits longest lines of outline into 5 triangles
        ///one_poly = simplifyPoints(one_poly, 0.030,     5, true );        // First run with averaging points to reduce triangles among similar slopes
        ///one_poly = simplifyPoints(one_poly, 0.001, split, false);        // Run again with smaller tolerance to reduce triangles along straight lines

        // Remove duplicate first point
        if (one_poly.size() > 3) one_poly.pop_back();

        // Optimize point list
        if (one_poly.size() > (c_neighbors * 2)) {
            one_poly = DrEngineVertexData::smoothPoints(one_poly, c_neighbors, 20.0, 1.0);
            one_poly = PolylineSimplification::RamerDouglasPeucker(one_poly, 0.075);
            one_poly = DrEngineVertexData::insertPoints(one_poly);
        }

        // If we only have a couple points left, add shape as a box of the original image, otherwise use PolylineSimplification points
        if (one_poly.size() < 4) {
            ///points = HullFinder::FindConcaveHull(points, 5.0);
            one_poly.clear();
            one_poly.push_back( DrPointF(rects[image_number].topLeft().x,        rects[image_number].topLeft().y) );
            one_poly.push_back( DrPointF(rects[image_number].topRight().x,       rects[image_number].topRight().y) );
            one_poly.push_back( DrPointF(rects[image_number].bottomRight().x,    rects[image_number].bottomRight().y) );
            one_poly.push_back( DrPointF(rects[image_number].bottomLeft().x,     rects[image_number].bottomLeft().y) );
        }

        // Check winding
        HullFinder::EnsureWindingOrientation(one_poly, Winding_Orientation::CounterClockwise);

        // Add polygon to list of polygons in shape
        m_poly_list.push_back(one_poly);


        // ******************** Copy image and finds holes as seperate outlines
        DrBitmap holes = image.copy();
        Dr::FillBorder(holes, Dr::white, holes.rect());                     // Ensures only holes are left as black spots

        // Breaks holes into seperate images for each Hole
        std::vector<DrBitmap> hole_images;
        std::vector<DrRect>   hole_rects;        
        Dr::FindObjectsInBitmap(holes, hole_images, hole_rects, c_alpha_tolerance, false);

        // Go through each image (Hole) create list for it
        std::vector<std::vector<DrPointF>> hole_list;
        for (int hole_number = 0; hole_number < static_cast<int>(hole_images.size()); hole_number++) {
            DrBitmap &hole = hole_images[hole_number];
            if (hole.width < 1 || hole.height < 1) continue;

            // Trace edge of hole
            std::vector<DrPointF> one_hole = Dr::TraceImageOutline(hole);

            // Add in sub image offset to points and hole rects
            for (auto &point : one_hole) {
                point.x += rects[image_number].left() + hole_rects[hole_number].left();
                point.y += rects[image_number].top()  + hole_rects[hole_number].top();
            }

            // Remove duplicate first point
            if (one_hole.size() > 3) one_hole.pop_back();

            // Optimize point list
            if (one_hole.size() > (c_neighbors * 2)) {
                one_hole = DrEngineVertexData::smoothPoints(one_hole, c_neighbors, 50.0, 1.0);
                one_hole = PolylineSimplification::RamerDouglasPeucker(one_hole, 0.1);
                one_hole = DrEngineVertexData::insertPoints(one_hole);
            }

            if (one_hole.size() > 3) {
                HullFinder::EnsureWindingOrientation(one_hole, Winding_Orientation::Clockwise);
                hole_list.push_back(one_hole);
            }
        }
        m_hole_list.push_back(hole_list);

    }   // End for each bitmap


    // ***** Mark this DrImage as having traced the image outline
    m_outline_canceled = false;
    m_outline_processed = true;


}   // End autoOutlinePoints()









