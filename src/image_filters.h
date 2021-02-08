//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#ifndef IMAGE_FILTER_H
#define IMAGE_FILTER_H

#include "types/bitmap.h"
#include "types/pointf.h"

// Imaging Enumerations
enum class Flood_Fill_Type {
    Compare_4,
    Compare_8,
};


//####################################################################################
//##    Image editing / object finding
//############################
namespace Dr {

    // ***** Comparison
    bool        CompareBitmaps(const DrBitmap &bitmap1, const DrBitmap &bitmap2);

    // ***** Object Counting / Fill (a la Ravens Project)
    DrBitmap    BlackAndWhiteFromAlpha(const DrBitmap &bitmap, double alpha_tolerance, bool inverse);
    void        FillBorder(DrBitmap &bitmap, DrColor fill_color, DrRect rect);
    bool        FindObjectsInBitmap(const DrBitmap &bitmap, std::vector<DrBitmap> &bitmaps, std::vector<DrRect> &rects, 
                                    double alpha_tolerance, bool convert = true);
    DrBitmap    FloodFill(DrBitmap &bitmap, int at_x, int at_y, DrColor fill_color, double tolerance, Flood_Fill_Type type,
                          int &flood_pixel_count, DrRect &flood_rect);

    // ***** Outlining
    std::vector<DrPointF>       OutlinePointList(const DrBitmap &bitmap);
    std::vector<DrPointF>       TraceImageOutline(const DrBitmap &bitmap);

}

#endif // IMAGE_FILTER_H







