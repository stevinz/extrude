//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "compare.h"
#include "imaging.h"
#include "types/color.h"
#include "types/point.h"
#include "types/pointf.h"
#include "types/rect.h"

namespace Dr
{


//####################################################################################
//##    Loops through image and changes one pixel at a time based on a
//##    premultiplied table
//####################################################################################
DrBitmap ApplySinglePixelFilter(Image_Filter_Type filter, const DrBitmap &from_bitmap, int value) {
    DrBitmap image = from_bitmap;
    
    int table[256];
    for ( int i = 0; i < 256; ++i ) {
        switch (filter) {
            case Image_Filter_Type::Brightness:   table[i] = Dr::Clamp( i + value, 0, 255 );                                    break;
            case Image_Filter_Type::Contrast:     table[i] = Dr::Clamp( (( i - 127 ) * (value + 128) / 128 ) + 127, 0, 255 );   break;
            default: ;
        }
    }

    for (size_t y = 0; y < static_cast<size_t>(image.height); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(image.width); ++x) {

            // Grab the current pixel color
            DrColor color = image.getPixel(x, y);
            DrHsv hsv;

            switch (filter) {
                case Image_Filter_Type::Brightness:
                case Image_Filter_Type::Contrast:
                    color.setRed(   table[color.red()]   );
                    color.setGreen( table[color.green()] );
                    color.setBlue(  table[color.blue()]  );
                    break;
                case Image_Filter_Type::Saturation: {
                    // !!!!! #NOTE: QColor returns -1 if image is grayscale
                    //              If thats the case give it a default hue of 0 (red) to match shader
//                    int hue = (color.hue() == -1) ? 0 : color.hue();
//                    color.setHsv(hue, Dr::Clamp(color.saturation() + value, 0, 255), color.value(), color.alpha());
                    break;
                }
                case Image_Filter_Type::Hue:
                    hsv = color.getHsv();
                    hsv.hue = Dr::Clamp(hsv.hue + value, -360.0, 360.0);
                    color.setFromHsv(hsv);
                    break;
                case Image_Filter_Type::Grayscale: {
                    double temp = (color.redF() * 0.2126) + (color.greenF() * 0.7152) + (color.blueF() * 0.0722);
                    color.setRgbF(temp, temp, temp, color.alphaF());
                    break;
                }
                case Image_Filter_Type::Negative:
                    color.setRgbF(1.0 - color.redF(), 1.0 - color.greenF(), 1.0 - color.blueF(), color.alphaF());
                    break;
                case Image_Filter_Type::Opacity:
                    color.setAlpha( Dr::Clamp(color.alpha() + value, 0, 255) );
                    break;
            }

            // Sets the new pixel color
            image.setPixel(x, y, color);
        }
    }
    return image;
}




}   // End namespace Dr



