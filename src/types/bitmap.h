//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#ifndef DR_BITMAP_H
#define DR_BITMAP_H

#include <string>
#include <vector>

// Forward Declarations
class DrColor;
class DrPolygonF;
class DrRect;

// Local constants
const int   c_number_channels = 4;


//####################################################################################
//##    DrBitmap
//##        Holds an image, compatible / loads with stb_image
//############################
class DrBitmap
{
public:
    int     channels =  c_number_channels;          // Number of 8-bit components per pixel: R, G, B, A
    int     width =     0;                          // Image width
    int     height =    0;                          // Image height

    std::vector<unsigned char> data;                // Pixel data


public:
    // Constructors
    DrBitmap();
    ~DrBitmap();
    DrBitmap(const DrBitmap &bitmap);
    DrBitmap(int width_, int height_);
    DrBitmap(std::string filename);
    DrBitmap(const unsigned char *from_data, const int &number_of_bytes,
             bool compressed = true, int width_ = 0, int height_ = 0);

    // Manipulation
    DrBitmap    copy();
    DrBitmap    copy(DrRect &copy_rect);
    DrPolygonF  polygon() const;
    DrRect      rect() const;
    DrColor     getPixel(int x, int y) const;
    void        setPixel(int x, int y, DrColor color);

    // Image Loaders
    void    loadFromFile(std::string filename);
    void    loadFromMemory(const unsigned char *compressed_data, const int &number_of_bytes,
                           bool compressed = true, int width_ = 0, int height_ = 0);


    void    saveFormat(std::vector<unsigned char> &formatted);          // Realigns pixels with stb image format
    int     saveAsBmp(std::string filename);                            // Returns 0 on failure, non-zero on success
    int     saveAsJpg(std::string filename, int quality = 100);         // Returns 0 on failure, non-zero on success, Quality 0-100
    int     saveAsPng(std::string filename);                            // Returns 0 on failure, non-zero on success


    // Functions
    bool    isValid() const     { return (width > 0 && height > 0); }

};


#endif // DR_BITMAP_H









