//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
//  File:
//      A container class for one image to be loaded
//
#ifndef DRIMAGE_H
#define DRIMAGE_H

#include <vector>

#include "bitmap.h"
#include "pointf.h"


// Local Constants
#define         vtr                     std::vector
const double    c_alpha_tolerance =     0.875;


//####################################################################################
//##    DrImage
//##        Class to hold an Image
//############################
class DrImage
{
private:
    // Local Variables
    std::string                 m_simple_name;                                              // Simple name, i.e. "pretty tree 1"
    DrBitmap                    m_bitmap;                                                   // Stored image as DrBitmap

public:
    vtr<vtr<DrPointF>>          m_poly_list;                                                // Stores list of image outline points
    vtr<vtr<vtr<DrPointF>>>     m_hole_list;                                                // Stores list of hole  outline points
    bool                        m_outline_canceled      { false };                          // True when Image Outline has been canceled, when true extrudes in 3D as simple square
    bool                        m_outline_processed     { false };                          // Turns true when autoOutlinePoints() has completed successfully

private:
    // Internal Variables
    std::string                 m_folder_name           { "" };                             // Used for External Images to belong to a category


public:
    // Constructors
    DrImage(std::string image_name, DrBitmap &bitmap, bool outline = true);

    // Settings
    std::string         getName()   { return m_simple_name; }

    // Image Helper Functions
    void                autoOutlinePoints();
    bool                outlineCanceled()                   { return m_outline_canceled; }
    bool                outlineProcessed()                  { return m_outline_processed; }
    void                setSimpleBox();

    // Getters / Setters
    std::string         getSimplifiedName()                 { return m_simple_name; }
    const DrBitmap&     getBitmap() const                   { return m_bitmap; }


    // Internal Variable Functions
    std::string         getFolderName()                     { return m_folder_name; }
    void                setFolderName(std::string folder)   { m_folder_name = folder; }

};

#endif // DRIMAGE_H











