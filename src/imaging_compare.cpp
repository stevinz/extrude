//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "3rd_party/stb/stb_image_write.h"
#include "compare.h"
#include "imaging.h"
#include "types/color.h"
#include "types/point.h"
#include "types/pointf.h"
#include "types/rect.h"


namespace Dr
{


//####################################################################################
//##    Returns True if bitmaps are identical, false if not
//####################################################################################
bool CompareBitmaps(const DrBitmap &bitmap1, const DrBitmap &bitmap2) {
    if (bitmap1.width  != bitmap2.width ) return false;
    if (bitmap1.height != bitmap2.height) return false;
    for (int x = 0; x < bitmap1.width; ++x) {
        for (int y = 0; y < bitmap1.height; ++y) {
            if (bitmap1.getPixel(x, y) != bitmap2.getPixel(x, y)) return false;
        }
    }
    return true;
}


//####################################################################################
//##    Returns black / white image (binary)
//##        alpha_tolerance is from 0.0 to 1.0
//##        NORMAL  (inverse == false): transparent areas are black, objects are white
//##        INVERSE (inverse == true) : transparent areas are white, objects are black
//####################################################################################
DrBitmap BlackAndWhiteFromAlpha(const DrBitmap &bitmap, double alpha_tolerance, bool inverse) {
    DrColor color1 = Dr::transparent;
    DrColor color2 = Dr::white;
    if (inverse) Dr::Swap(color1, color2);

    DrBitmap black_white(bitmap);
    int alpha_i = static_cast<int>(alpha_tolerance * 255.0);

    for (int x = 0; x < bitmap.width; ++x) {
        for (int y = 0; y < bitmap.height; ++y) {
            black_white.setPixel(x, y, ((bitmap.getPixel(x, y).alpha() < alpha_i) ? color1 : color2));
        }
    }
    return black_white;
}


//####################################################################################
//##    Flood Fill
/// @brief      Fills in an area of similar colored pixels starting at (at_x, at_y) with (fill_color)
/// @returns    DrBitmap as new image of flood
/// @ref    (bitmap):               Image passed in to be flooded, is altered during function
/// @value  (tolerance):            Percentage of how similar color should be to continue to fill, 0.0 to 1.0
/// @value  (type):                 Specifies algorithm used to compare neighbors during fill routine
/// @ref    (flood_pixel_count):    Number of total pixels in flood
/// @ref    (flood_rect):           Bounding box of fill area
//####################################################################################
#define FLOOD_NOT_PROCESSED         0
#define FLOOD_WAS_PROCESSED         1
#define FLOOD_MARKED_FOR_PROCESS    2

DrBitmap FloodFill(DrBitmap &bitmap, int at_x, int at_y, DrColor fill_color, double tolerance, Flood_Fill_Type type,
                   int &flood_pixel_count, DrRect &flood_rect) {
    flood_pixel_count = 0;

    // Get scan lines
    DrBitmap flood(bitmap);
    DrBitmap processed(bitmap);

    // Check if start point is in range
    flood_rect = DrRect(0, 0, 0, 0);
    if (at_x < 0 || at_y < 0 || at_x > bitmap.width - 1 || at_y > bitmap.height - 1) {
        return DrBitmap();
    } else if (bitmap.width < 1 || bitmap.height < 1) {
        return DrBitmap();
    } else if ((bitmap.width == 1) && (bitmap.height == 1)) {
        bitmap.setPixel(0, 0, fill_color);
        flood.setPixel(0, 0, fill_color);
        flood_rect = DrRect(at_x, at_y, 1, 1);
        return flood;
    }

    // Get starting color, set processed image to all zeros
    DrColor start_color = bitmap.getPixel(at_x, at_y);

    for (int x = 0; x < bitmap.width; ++x) {
        for (int y = 0; y < bitmap.height; ++y) {
            flood.setPixel(x, y, 0);
            processed.setPixel(x, y, FLOOD_NOT_PROCESSED);
        }
    }

    // Push starting point onto vector
    std::vector<DrPoint> points;
    points.clear();
    points.push_back(DrPoint(at_x, at_y));
    bool processed_some;

    int min_x = at_x, max_x = at_x;
    int min_y = at_y, max_y = at_y;
    do {
        // Go through each point and find new points to fill
        processed_some = false;
        for (auto point : points) {
            if (processed.getPixel(point.x, point.y) == FLOOD_WAS_PROCESSED) continue;
            bitmap.setPixel(point.x, point.y, fill_color);
            flood.setPixel(point.x, point.y, fill_color);
            processed.setPixel(point.x, point.y, FLOOD_WAS_PROCESSED);
            if (point.x < min_x) min_x = point.x;
            if (point.x > max_x) max_x = point.x;
            if (point.y < min_y) min_y = point.y;
            if (point.y > max_y) max_y = point.y;
            ++flood_pixel_count;

            int x_start = (point.x > 0) ?                 point.x - 1 : 0;
            int x_end =   (point.x < bitmap.width - 1)  ? point.x + 1 : bitmap.width  - 1;
            int y_start = (point.y > 0) ?                 point.y - 1 : 0;
            int y_end =   (point.y < bitmap.height - 1) ? point.y + 1 : bitmap.height - 1;

            for (int x = x_start; x <= x_end; ++x) {
                for (int y = y_start; y <= y_end; ++y) {
                    if (x == point.x && y == point.y) continue;

                    if (type == Flood_Fill_Type::Compare_4) {
                        if ( (x == point.x - 1) && (y == point.y - 1) ) continue;
                        if ( (x == point.x + 1) && (y == point.y - 1) ) continue;
                        if ( (x == point.x - 1) && (y == point.y + 1) ) continue;
                        if ( (x == point.x + 1) && (y == point.y + 1) ) continue;
                    }

                    if (processed.getPixel(x, y) == FLOOD_NOT_PROCESSED) {
                        if (Dr::IsSameColor(start_color, bitmap.getPixel(x, y), tolerance)) {
                            points.push_back(DrPoint(x, y));
                            processed.setPixel(x, y, FLOOD_MARKED_FOR_PROCESS);
                            processed_some = true;
                        }
                    }
                }
            }
        }

        // Remove any points that have been processed
        auto it = points.begin();
        while (it != points.end()) {
            if (processed.getPixel(it->x, it->y) == FLOOD_WAS_PROCESSED)
                it = points.erase(it);
            else
                ++it;
        }
    } while ((points.size() > 0) && processed_some);

    flood_rect = DrRect(min_x, min_y, (max_x - min_x) + 1, (max_y - min_y) + 1);
    return flood;
}






//####################################################################################
//##    Fill border
//##        Traces Border of 'rect' and makes sure to fill in any Dr::transparent areas with fill_color
//####################################################################################
void FillBorder(DrBitmap &bitmap, DrColor fill_color, DrRect rect) {
    DrRect fill_rect;
    int    fill_qty;

    int y1 = rect.top();
    int y2 = rect.bottom();
    for (int x = rect.left(); x < rect.left() + rect.width; x++) {
        if (bitmap.getPixel(x, y1) == Dr::transparent) {
            Dr::FloodFill(bitmap, x, y1, fill_color, 0.001, Flood_Fill_Type::Compare_4, fill_qty, fill_rect);
        }
        if (bitmap.getPixel(x, y2) == Dr::transparent) {
            Dr::FloodFill(bitmap, x, y2, fill_color, 0.001, Flood_Fill_Type::Compare_4, fill_qty, fill_rect);
        }
    }

    int x1 = rect.left();
    int x2 = rect.right();
    for (int y = rect.top(); y < rect.top() + rect.height; y++) {
        if (bitmap.getPixel(x1, y) == Dr::transparent) {
            Dr::FloodFill(bitmap, x1, y, fill_color, 0.001, Flood_Fill_Type::Compare_4, fill_qty, fill_rect);
        }
        if (bitmap.getPixel(x2, y) == Dr::transparent) {
            Dr::FloodFill(bitmap, x2, y, fill_color, 0.001, Flood_Fill_Type::Compare_4, fill_qty, fill_rect);
        }
    }
}




//####################################################################################
//##    Find Objects
//##        Seperates parts of an image divided by alpha space into seperate images, returns image count.
//##        The images are stored into the reference array passed in 'images', the images are black and white.
//##            Black where around the ouside of of the object, and the object itself is white.
//##        Rects of images are returned in 'rects'
//####################################################################################
#define INVERTED_COLORS     true

bool FindObjectsInBitmap(const DrBitmap &bitmap, std::vector<DrBitmap> &bitmaps, std::vector<DrRect> &rects,
                        double alpha_tolerance, bool convert) {
    DrBitmap     black_white;
    if (convert) black_white = BlackAndWhiteFromAlpha(bitmap, alpha_tolerance, INVERTED_COLORS);
    else         black_white = bitmap;

    DrColor compare(Dr::transparent);

    // If convert is true, all object pixels will be transparent pixels. If all pixels are transparent we don't need to fill, we can
    // just return a solid square later on and not have to run expensive flood fill routine
    bool pixels = false;
    if (convert == true) {
        for (int x = 0; x < black_white.width; ++x) {
            for (int y = 0; y < black_white.height; ++y) {
                if (black_white.getPixel(x, y) != compare) {
                    pixels = true;
                    break;
                }
            }
            if (pixels) break;
        }
    }

    // There are transparent pixels, we need to run flood fill routines to isolate objects
    if (pixels || convert == false) {
        // Loop through every pixel in image, if we find a spot that has an object,
        // flood fill that spot and add the resulting image shape to the array of object images
        for (int x = 0; x < black_white.width; ++x) {
            // Process Pixel
            for (int y = 0; y < black_white.height; ++y) {
                if (black_white.getPixel(x, y) == compare) {
                    DrRect      rect;
                    int         flood_pixel_count;
                    DrBitmap    flood_fill = FloodFill(black_white, x, y, Dr::red, 0.001, Flood_Fill_Type::Compare_4, flood_pixel_count, rect);

                    // Add buffer around rect, create image of rect only
                    rect.adjust(-1, -1, 1, 1);
                    DrBitmap    fill_only = flood_fill.copy(rect);

                    // If adequate image, add to list of floods
                    if (fill_only.width >= 1 && fill_only.height >= 1 && flood_pixel_count > 1) {
                        rects.push_back( rect );
                        bitmaps.push_back( fill_only );
                    }
                }
            }
        }

    // No non-object pixels, fill with Dr::red and return
    } else {
        if (black_white.width > 0 && black_white.height > 0) {
            for (int x = 0; x < black_white.width; ++x) {
                for (int y = 0; y < black_white.height; ++y) {
                    black_white.setPixel(x, y, Dr::red);
                }
            }
            rects.push_back( black_white.rect() );
            bitmaps.push_back( black_white );
        }
    }

    return false;
}



//####################################################################################
//##    Returns a clockwise list of points representing an alpha outline of an image.
//##    This algorithm works by moving around the image in a clockwise manner trying to stay
//##    on the largest angle between two points. Idea and code written by Scidian Software.
//##        !!!!! #NOTE: Image passed in should be black and white,
//##                     probably from DrImageing::BlackAndWhiteFromAlpha()
//####################################################################################
#define TRACE_NOT_BORDER            0           // Pixels that are not near the edge
#define TRACE_START_PIXEL           1           // Starting pixel
#define TRACE_NOT_PROCESSED         2           // Pixels that can be part of the border
#define TRACE_PROCESSED_ONCE        3           // Pixels that added to the border once
#define TRACE_PROCESSED_TWICE       4           // Pixels that added to the border twice        (after a there and back again trace)

std::vector<DrPointF> TraceImageOutline(const DrBitmap &bitmap) {
    // Initialize images
    DrBitmap processed = bitmap;
    int border_pixel_count = 0;

    // Initialize point array, verify image size
    std::vector<DrPoint> points { };
    if (bitmap.width < 1 || bitmap.height < 1) return std::vector<DrPointF> { };

    // ***** Find starting point, and also set processed image bits
    //       !!!!! #NOTE: Important that y loop is on top, we need to come at pixel from the left
    DrPoint last_point;
    bool has_start_point = false;

    for (int x = 0; x < bitmap.width; ++x) {
        for (int y = 0; y < bitmap.height; ++y) {
            // If pixel is part of the exterior, it cannot be part of the border
            if (bitmap.getPixel(x, y) == Dr::transparent) {
                processed.setPixel(x, y, TRACE_NOT_BORDER);
                continue;
            }

            // Run through all pixels this pixel is touching to see if any are transparent (i.e. Dr::transparent (0))
            bool can_be_border = false;
            if (x == 0 || y == 0 || (x == bitmap.width - 1) || (y == bitmap.height - 1)) {
                can_be_border = true;
            } else {
                for (int i = x - 1; i <= x + 1; ++i) {
                    for (int j = y - 1; j <= y + 1; ++j) {
                        if (bitmap.getPixel(i, j) == Dr::transparent) can_be_border = true;
                    }
                }
            }

            // If not touching any transparent pixels, and not on edge, cannot be part of border
            if (!can_be_border) {
                processed.setPixel(x, y, TRACE_NOT_BORDER);

            // Otherwise mark it as not processed and check if we have a start point
            } else {
                processed.setPixel(x, y, TRACE_NOT_PROCESSED);
                ++border_pixel_count;
                if (!has_start_point) {
                    points.push_back(DrPoint(x, y));
                    last_point = DrPoint(x - 1, y);
                    processed.setPixel(x, y, TRACE_START_PIXEL);
                    has_start_point = true;
                }
            }
        }
    }
    if (border_pixel_count < 3) return std::vector<DrPointF> { };


    // ***** Find outline points
    std::vector<DrPoint> surround;
    bool back_at_start;
    long trace_count = 0;
    long total_pixels = bitmap.width * bitmap.height;
    do {
        // Collect list of points around current point
        surround.clear();
        DrPoint current_point = points.back();
        int x_start = (current_point.x > 0) ?                 current_point.x - 1 : 0;
        int x_end =   (current_point.x < bitmap.width - 1)  ? current_point.x + 1 : bitmap.width  - 1;
        int y_start = (current_point.y > 0) ?                 current_point.y - 1 : 0;
        int y_end =   (current_point.y < bitmap.height - 1) ? current_point.y + 1 : bitmap.height - 1;
        for (int x = x_start; x <= x_end; ++x) {
            for (int y = y_start; y <= y_end; ++y) {
                if (x == current_point.x && y == current_point.y) continue;
                ///if ( (x == current_point.x - 1) && (y == current_point.y - 1) ) continue;
                ///if ( (x == current_point.x + 1) && (y == current_point.y - 1) ) continue;
                ///if ( (x == current_point.x - 1) && (y == current_point.y + 1) ) continue;
                ///if ( (x == current_point.x + 1) && (y == current_point.y + 1) ) continue;
                if (processed.getPixel(x, y) != TRACE_PROCESSED_TWICE && processed.getPixel(x, y) != TRACE_NOT_BORDER) {
                    surround.push_back(DrPoint(x, y));
                }
            }
        }

        // Compare surrounding points to see which one has the greatest angle measured clockwise from the last set of points
        double   last_point_angle = Dr::CalcRotationAngleInDegrees(current_point.toPointF(), last_point.toPointF());
        double   angle_diff = 0;
        bool     first = true;
        DrPoint  next_point;
        for (auto point : surround) {
            // Find angle of point
            double check_angle = Dr::CalcRotationAngleInDegrees(current_point.toPointF(), point.toPointF());
            while (check_angle > 0) {                 check_angle -= 360.0; }
            while (check_angle <= last_point_angle) { check_angle += 360.0; }

            // Add penalty for each time pixel has been checked
            ///if (processed_lines[point.y][point.x] != TRACE_START_PIXEL)
            ///    check_angle += (processed_lines[point.y][point.x] - TRACE_NOT_PROCESSED) * 360.0;

            // See if next closest point
            double check_point_diff = check_angle - last_point_angle;
            if (first || check_point_diff < angle_diff) {
                angle_diff = check_point_diff;
                next_point = point;
                first = false;
            }
        }

        // If we found an angle, we found our next point, add it to the list
        if (surround.size() > 0) {
            if (processed.getPixel(current_point.x, current_point.y) == TRACE_NOT_PROCESSED)
                processed.setPixel(current_point.x, current_point.y, TRACE_PROCESSED_ONCE);
            else if (processed.getPixel(current_point.x, current_point.y) == TRACE_PROCESSED_ONCE)
                processed.setPixel(current_point.x, current_point.y, TRACE_PROCESSED_TWICE);
            last_point = current_point;
            points.push_back(next_point);
        }

        // Check if we're back at start and no more options
        back_at_start = (processed.getPixel(points.back().x, points.back().y) == TRACE_START_PIXEL);
        ++trace_count;

    } while ((surround.size() > 0) && !back_at_start);

    // ***** Convert to DrPointF array and return
    std::vector<DrPointF> hull_points;
    for (size_t i = 0; i < points.size(); ++i) {
        hull_points.push_back(DrPointF(points[i].x, points[i].y));
    }
    return hull_points;
}



//####################################################################################
//##    Returns a mostly random list of points of possible edges of an image
//##        !!!!! #NOTE: Image passed in should be black and white
//##                     (i.e. from DrImageing::BlackAndWhiteFromAlpha())
//####################################################################################
std::vector<DrPointF> OutlinePointList(const DrBitmap &bitmap) {
    std::vector<DrPointF> points;
    points.clear();

    // Loop through every pixel to see if is possibly on border
    for (int y = 0; y < bitmap.height; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            if (bitmap.getPixel(x, y) == Dr::transparent) continue;

            // Run through all pixels this pixel is touching to see if they are transparent (i.e. black)
            int x_start, x_end, y_start, y_end;
            x_start = (x > 0) ? x - 1 : x;
            y_start = (y > 0) ? y - 1 : y;
            x_end =   (x < (bitmap.width - 1))  ? x + 1 : x;
            y_end =   (y < (bitmap.height - 1)) ? y + 1 : y;
            bool touching_transparent = false;
            for (int i = x_start; i <= x_end; ++i) {
                for (int j = y_start; j <= y_end; ++j) {
                    if (bitmap.getPixel(i, j) == Dr::transparent) touching_transparent = true;
                    if (touching_transparent) break;
                }
                if (touching_transparent) break;
            }

            if (touching_transparent) {
                points.push_back(DrPointF(x, y));
            } else {
                if ((x == 0 && y == 0) ||
                    (x == 0 && y == (bitmap.height - 1)) ||
                    (x == (bitmap.width - 1) && y == 0) ||
                    (x == (bitmap.width - 1) && y == (bitmap.height - 1))) {
                    points.push_back(DrPointF(x, y));
                }
            }
        }
    }
    return points;
}



}   // End namespace Dr









