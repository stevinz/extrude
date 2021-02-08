//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#ifndef DR_RANDOM_H
#define DR_RANDOM_H

#include <random>

// Forward Declarations
class DrColor;


namespace Dr {


    //####################################################################################
    //##    Random Functions
    //############################
    /// @brief: Returns a number between lower (inclusive) and upper (exclusive)
    int         RandomInt(int lower, int upper);
    /// @brief: Returns a number between lower (inclusive) and upper (inclusive)
    double      RandomDouble(double lower, double upper);
    /// @brief: Returns a random boolean
    bool        RandomBool();
    /// @brief: Returns a random DrColor, fully opaque
    DrColor     RandomColor();


}   // End namespace Dr

#endif // DR_RANDOM_H



