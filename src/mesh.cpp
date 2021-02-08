//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "mesh.h"


//####################################################################################
//##    Builds a Vertex
//####################################################################################
Vertex Vertex::createVertex(DrVec3 pos, DrVec3 norm, DrVec3 uv, DrVec3 bary) {
    Vertex v;
    v.position =        pos;
    v.normal =          norm;
    v.texture_coords =  uv;
    v.barycentric =     bary;
    return v;
}


