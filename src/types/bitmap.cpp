//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "../3rd_party/stb/stb_image.h"
#include "../3rd_party/stb/stb_image_resize.h"
#include "../3rd_party/stb/stb_image_write.h"
#include "bitmap.h"
#include "color.h"
#include "pointf.h"
#include "polygonf.h"
#include "rect.h"


//####################################################################################
//##    Constructors
//####################################################################################
DrBitmap::DrBitmap(Bitmap_Format desired_format) {  
    data.clear(); 
    format = desired_format;
    switch (format) {
        case Bitmap_Format::Grayscale:   channels = 1;   break;
        case Bitmap_Format::ARGB:        channels = 4;   break;
    }
}
DrBitmap::~DrBitmap() { 
    data.clear(); 
}

DrBitmap::DrBitmap(const DrBitmap &bitmap, Bitmap_Format desired_format) : DrBitmap(bitmap.width, bitmap.height, desired_format) {
    if (bitmap.format == format && data.size()) {
        channels =  bitmap.channels;
        width =     bitmap.width;
        height =    bitmap.height;
        data.resize(width * height * bitmap.channels);                                              // Resize data vector
        memcpy(&data[0], &bitmap.data[0], data.size());                                             // Copy data
    } else {
        for (int x = 0; x < bitmap.width; ++x) {
            for (int y = 0; y < bitmap.height; ++y) {
                this->setPixel(x, y, bitmap.getPixel(x, y));
            }
        }
    }
}

// Create empty bitmap
DrBitmap::DrBitmap(int width_, int height_, Bitmap_Format desired_format) : DrBitmap(desired_format) {
    width =     width_;
    height =    height_;
    data.resize(width * height * channels);
    std::fill(data.begin(), data.end(), 0);
}

DrBitmap::DrBitmap(std::string filename, Bitmap_Format desired_format) {
    loadFromFile(filename, desired_format);
}

DrBitmap::DrBitmap(const unsigned char *from_data, const int &number_of_bytes, bool compressed, int width_, int height_) {
    loadFromMemory(from_data, number_of_bytes, compressed, width_, height_);
}


//####################################################################################
//##    Manipulation
//####################################################################################
DrBitmap DrBitmap::copy() { return (*this); }

DrBitmap DrBitmap::copy(DrRect &copy_rect) {
    // Bounds checking
    int check_left = copy_rect.left();
    int check_top  = copy_rect.top();
    if (check_left < 0) {
        copy_rect.width -= abs(check_left);
        copy_rect.x     += abs(check_left);
    }
    if (check_top < 0) {
        copy_rect.height -= abs(check_top);
        copy_rect.y      += abs(check_top);
    }
    if (copy_rect.width <= 0 || copy_rect.height <= 0) return DrBitmap(0, 0);
    if (copy_rect.right() > this->width - 1)   copy_rect.width =  width - copy_rect.left();
    if (copy_rect.bottom() > this->height - 1) copy_rect.height = height - copy_rect.top();
    if (copy_rect.width <= 0 || copy_rect.height <= 0) return DrBitmap(0, 0);

    // Create empty DrBitmap
    DrBitmap copy(copy_rect.width, copy_rect.height, format);

    // Copy source
    int source_x = copy_rect.left();
    for (int x = 0; x < copy.width; ++x) {
        int source_y = copy_rect.top();
        for (int y = 0; y < copy.height; ++y) {
            copy.setPixel(x, y, this->getPixel(source_x, source_y));
            ++source_y;
        }
        ++source_x;
    }
    return copy;
}

// Returns a clockwise polygon representing a box around this image
DrPolygonF DrBitmap::polygon() const {
    DrPolygonF box;
    box.addPoint( DrPointF(0,               0) );                           // Top Left
    box.addPoint( DrPointF(this->width - 1, 0) );                           // Top Right
    box.addPoint( DrPointF(this->width - 1, this->height - 1) );            // Bottom Right
    box.addPoint( DrPointF(0,               this->height - 1) );            // Bottom Left
    return box;
}

DrRect DrBitmap::rect() const {
    return DrRect(0, 0, width, height);
}

// !!!!! #WARNING: No out of bounds checks are done here for speed!!
DrColor DrBitmap::getPixel(int x, int y) const {
    size_t index = (y * this->width * channels) + (x * channels);
    switch (format) {
        case Bitmap_Format::Grayscale:  return DrColor(data[index+0], data[index+0], data[index+0], data[index+0]);
        case Bitmap_Format::ARGB:       return DrColor(data[index+2], data[index+1], data[index+0], data[index+3]);
    }
}

// DrBitmap data is in the format (Format_ARGB32)
void DrBitmap::setPixel(int x, int y, DrColor color) {
    size_t index = (y * this->width * channels) + (x * channels);
    switch (format) {
        case Bitmap_Format::Grayscale:   
            data[index]   = (color.redF() * 0.2126) + (color.greenF() * 0.7152) + (color.blueF() * 0.0722);
            break;
        case Bitmap_Format::ARGB:        
            data[index]   = color.blue();
            data[index+1] = color.green();
            data[index+2] = color.red();
            data[index+3] = color.alpha();       
            break;
    }
}


//####################################################################################
//##    Testing Alpha
//####################################################################################
void DrBitmap::fuzzyAlpha() {
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            DrColor color = getPixel(x, y);
            if ((color.red() <  10 && color.green() <  10 && color.blue() <  10) ||
                (color.red() > 245 && color.green() > 245 && color.blue() > 245)) {
                switch (format) {
                    case Bitmap_Format::Grayscale:  color.setRgbF(0, 0, 0, 0);
                    case Bitmap_Format::ARGB:       color.setAlpha(0);
                }
                setPixel(x, y, color);
            }
        }
    }
}


//####################################################################################
//##    Loading Images
//####################################################################################
void DrBitmap::loadFromFile(std::string filename, Bitmap_Format desired_format) {
    // Load Image
    unsigned char* ptr = stbi_load(filename.data(), &width, &height, &channels, desired_format);

    // Error Check
    if (ptr == nullptr || width == 0 || height == 0) {
        width = 0; height = 0; 
        return;
    }

    // Copy Image
    data.resize(width * height * channels);                                                     // Resize data vector
    memcpy(&data[0], ptr, data.size());                                                         // Copy data
    stbi_image_free(ptr);                                                                       // Free the loaded pixels
}

void DrBitmap::loadFromMemory(const unsigned char *from_data, const int &number_of_bytes, bool compressed, int width_, int height_) {
    // Load Raw Data
    if (compressed == false) {
        width = width_;
        height = height_;
        data.resize(number_of_bytes);                                                           // Resize data vector
        memcpy(&data[0], from_data, number_of_bytes);                                           // Copy data

    // Load Image
    } else {
        const stbi_uc *compressed_data = reinterpret_cast<const stbi_uc*>(from_data);
        unsigned char* ptr = stbi_load_from_memory(compressed_data, number_of_bytes, &width, &height, &channels, channels);

        // Error Check
        if (ptr == nullptr || width == 0 || height == 0) {
            width = 0; height = 0; 
            return;
        }

        // Copy Image
        data.resize(width * height * channels);                                                 // Resize data vector
        memcpy(&data[0], ptr, data.size());                                                     // Copy data
        stbi_image_free(ptr);                                                                   // Free the loaded pixels
    }
}

// Aligns pixel format (stb ABGR vs QImage ARGB) for stbi_write
void DrBitmap::saveFormat(std::vector<unsigned char> &formatted) {
    formatted.resize(width * height * channels);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            DrColor pixel = getPixel(x, y);

            size_t index = (y * width * channels) + (x * channels);
            formatted[index] =   pixel.red();
            formatted[index+1] = pixel.green();
            formatted[index+2] = pixel.blue();
            formatted[index+3] = pixel.alpha();
        }
    }
}

int DrBitmap::saveAsBmp(std::string filename) {
    std::vector<unsigned char> formatted;
    saveFormat(formatted);
    int result = stbi_write_bmp(filename.c_str(), width, height, channels, formatted.data());
    return result;
}

int DrBitmap::saveAsJpg(std::string filename, int quality) {
    std::vector<unsigned char> formatted;
    saveFormat(formatted);
    int result = stbi_write_jpg(filename.c_str(), width, height, channels, formatted.data(), quality);
    return result;
}

int DrBitmap::saveAsPng(std::string filename) {
    std::vector<unsigned char> formatted;
    saveFormat(formatted);
    int result = stbi_write_png(filename.c_str(), width, height, channels, formatted.data(), width * channels);
    return result;
}







