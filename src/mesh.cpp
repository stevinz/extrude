//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#include "3rd_party/handmade_math.h"
#include "compare.h"
#include "mesh.h"
#include "types/vec2.h"
#include "types/vec3.h"


//####################################################################################
//##    Builds a Vertex
//####################################################################################
Vertex Vertex::createVertex(DrVec3 pos, DrVec3 norm, DrVec3 uv, DrVec3 bary) {
    Vertex v;
    v.px = pos.x;
    v.py = pos.y;
    v.pz = pos.z;
    v.nx = norm.x;
    v.ny = norm.y;
    v.nz = norm.z;
    v.tx = uv.x;
    v.ty = uv.y;
    v.bx = bary.x;
    v.by = bary.y;
    v.bz = bary.z;
    return v;
}


//####################################################################################
//##    Mesh Constructor
//####################################################################################
DrMesh::DrMesh() { }


//####################################################################################
//##    Adds a Vertex, including:
//##        Vec3 Position
//##        Vec3 Normal
//##        Vec2 UV Texture Coordinates
//##        Vec3 Barycentric Coordinates (gives shader a number between 0.0 and 1.0 to lerp to)
//####################################################################################
void DrMesh::add(const DrVec3 &vertex, const DrVec3 &normal, const DrVec2 &text_coord, Triangle_Point point_number) {
    Vertex v;
    v.px = vertex.x;
    v.py = vertex.y;
    v.pz = vertex.z;
    v.nx = normal.x;
    v.ny = normal.y;
    v.nz = normal.z;
    v.tx = text_coord.x;
    v.ty = text_coord.y;
    switch (point_number) {
        case Triangle_Point::Point1:    v.bx = 1.0;   v.by = 0.0;   v.bz = 0.0;   break;
        case Triangle_Point::Point2:    v.bx = 0.0;   v.by = 1.0;   v.bz = 0.0;   break;
        case Triangle_Point::Point3:    v.bx = 0.0;   v.by = 0.0;   v.bz = 1.0;   break;
    }
    vertices.push_back(v);
}

void DrMesh::set(Vertex &from_vertex, Vertex &to_vertex) {
    to_vertex.px = from_vertex.px;
    to_vertex.py = from_vertex.py;
    to_vertex.pz = from_vertex.pz;

    to_vertex.nx = from_vertex.nx;
    to_vertex.ny = from_vertex.ny;
    to_vertex.nz = from_vertex.nz;

    to_vertex.tx = from_vertex.tx;
    to_vertex.ty = from_vertex.ty;

    to_vertex.bx = from_vertex.bx;
    to_vertex.by = from_vertex.by;
    to_vertex.bz = from_vertex.bz;
}


//####################################################################################
//##    Builds a Textured Quad
//####################################################################################
void DrMesh::initializeTextureQuad(float size) {
    int   width =  size;
    int   height = size;
    float w2 = width  / 2.f;
    float h2 = height / 2.f;

    // EXAMPLE: Adding Triangles
    float x1 = +w2, y1 = +h2;                   // Top Right
    float x2 = -w2, y2 = +h2;                   // Top Left
    float x3 = +w2, y3 = -h2;                   // Bottom Right
    float x4 = -w2, y4 = -h2;                   // Bottom Left

    float tx1 = 1.0, ty1 = 1.0;
    float tx2 = 0.0, ty2 = 1.0;
    float tx3 = 1.0, ty3 = 0.0;
    float tx4 = 0.0, ty4 = 0.0;

    DrVec3 n = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x3, y3, 0.f), DrVec3(x2, y2, 0.f));

    add(DrVec3(x1, y1, 0.f), n, DrVec2(tx1, ty1), Triangle_Point::Point1);
    add(DrVec3(x2, y2, 0.f), n, DrVec2(tx2, ty2), Triangle_Point::Point2);
    add(DrVec3(x3, y3, 0.f), n, DrVec2(tx3, ty3), Triangle_Point::Point3);
    add(DrVec3(x2, y2, 0.f), n, DrVec2(tx2, ty2), Triangle_Point::Point1);
    add(DrVec3(x4, y4, 0.f), n, DrVec2(tx4, ty4), Triangle_Point::Point2);
    add(DrVec3(x3, y3, 0.f), n, DrVec2(tx3, ty3), Triangle_Point::Point3);
}


//####################################################################################
//##    Builds a Textured Cube
//####################################################################################
void DrMesh::initializeTextureCube(float size) {
    int   width =  size;
    int   height = size;
    float w2 = width  / 2.f;
    float h2 = height / 2.f;
    float depth = size * c_cube_depth;

    // EXAMPLE: Adding Triangles
    float x1 = +w2, y1 = +h2;                   // Top Right
    float x2 = -w2, y2 = +h2;                   // Top Left
    float x3 = +w2, y3 = -h2;                   // Bottom Right
    float x4 = -w2, y4 = -h2;                   // Bottom Left

    float tx1 = 1.0, ty1 = 1.0;
    float tx2 = 0.0, ty2 = 1.0;
    float tx3 = 1.0, ty3 = 0.0;
    float tx4 = 0.0, ty4 = 0.0;

    cube( x1,  y1,  tx1, ty1,
          x2,  y2,  tx2, ty2,
          x3,  y3,  tx3, ty3,
          x4,  y4,  tx4, ty4, depth);
}


//####################################################################################
//##    Adds a Cube, as 3 pairs (six sides) of front and back
//####################################################################################
void DrMesh::cube(float x1, float y1, float tx1, float ty1,
                  float x2, float y2, float tx2, float ty2,
                  float x3, float y3, float tx3, float ty3,
                  float x4, float y4, float tx4, float ty4, float depth) {
    hmm_m4 rotate = Dr::IdentityMatrix();
    DrVec3 nf, nb;                                   // Normal Front, Normal Back
    DrVec3 p1f, p2f, p3f, p4f;                       // Point 1 Front, etc
    DrVec3 p1b, p2b, p3b, p4b;                       // Point 1 Back, etc

    for (int i = 0; i <= 2; ++i) {
        nf = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x3, y3, 0.f), DrVec3(x2, y2, 0.f));
        nb = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x2, y2, 0.f), DrVec3(x3, y3, 0.f));
        p1f = DrVec3(x1, y1, +depth);
        p2f = DrVec3(x2, y2, +depth);
        p3f = DrVec3(x3, y3, +depth);
        p4f = DrVec3(x4, y4, +depth);
        p1b = DrVec3(x1, y1, -depth);
        p2b = DrVec3(x2, y2, -depth);
        p3b = DrVec3(x3, y3, -depth);
        p4b = DrVec3(x4, y4, -depth);

        if (i == 1) {
            rotate = HMM_MultiplyMat4(rotate, HMM_Rotate(90.f, { 0.0, 1.0, 0.0 }));         // Angle is in degrees
        } else if (i == 2) {
            rotate = HMM_MultiplyMat4(rotate, HMM_Rotate(90.f, { 1.0, 0.0, 0.0 }));         // Angle is in degrees
        }

        nf =    rotate * nf;
        p1f =   rotate * p1f;
        p2f =   rotate * p2f;
        p3f =   rotate * p3f;
        p4f =   rotate * p4f;

        nb =    rotate * nb;
        p1b =   rotate * p1b;
        p2b =   rotate * p2b;
        p3b =   rotate * p3b;
        p4b =   rotate * p4b;

        add(p1f, nf, DrVec2(tx1, ty1), Triangle_Point::Point1);
        add(p2f, nf, DrVec2(tx2, ty2), Triangle_Point::Point2);
        add(p3f, nf, DrVec2(tx3, ty3), Triangle_Point::Point3);
        add(p2f, nf, DrVec2(tx2, ty2), Triangle_Point::Point1);
        add(p4f, nf, DrVec2(tx4, ty4), Triangle_Point::Point2);
        add(p3f, nf, DrVec2(tx3, ty3), Triangle_Point::Point3);

        add(p1b, nb, DrVec2(tx1, ty1), Triangle_Point::Point1);
        add(p3b, nb, DrVec2(tx3, ty3), Triangle_Point::Point2);
        add(p2b, nb, DrVec2(tx2, ty2), Triangle_Point::Point3);
        add(p2b, nb, DrVec2(tx2, ty2), Triangle_Point::Point1);
        add(p3b, nb, DrVec2(tx3, ty3), Triangle_Point::Point2);
        add(p4b, nb, DrVec2(tx4, ty4), Triangle_Point::Point3);
    }
}


//####################################################################################
//##    Adds a Quad, front and back
//####################################################################################
void DrMesh::quad(float x1, float y1, float tx1, float ty1,
                  float x2, float y2, float tx2, float ty2,
                  float x3, float y3, float tx3, float ty3,
                  float x4, float y4, float tx4, float ty4) {
    DrVec3 n;
    n = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x3, y3, 0.f), DrVec3(x2, y2, 0.f));

    add(DrVec3(x1, y1, +c_extrude_depth), n, DrVec2(tx1, ty1), Triangle_Point::Point1);
    add(DrVec3(x2, y2, +c_extrude_depth), n, DrVec2(tx2, ty2), Triangle_Point::Point2);
    add(DrVec3(x3, y3, +c_extrude_depth), n, DrVec2(tx3, ty3), Triangle_Point::Point3);

    add(DrVec3(x2, y2, +c_extrude_depth), n, DrVec2(tx2, ty2), Triangle_Point::Point1);
    add(DrVec3(x4, y4, +c_extrude_depth), n, DrVec2(tx4, ty4), Triangle_Point::Point2);
    add(DrVec3(x3, y3, +c_extrude_depth), n, DrVec2(tx3, ty3), Triangle_Point::Point3);

    n = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x2, y2, 0.f), DrVec3(x3, y3, 0.f));

    add(DrVec3(x1, y1, -c_extrude_depth), n, DrVec2(tx1, ty1), Triangle_Point::Point1);
    add(DrVec3(x3, y3, -c_extrude_depth), n, DrVec2(tx3, ty3), Triangle_Point::Point2);
    add(DrVec3(x2, y2, -c_extrude_depth), n, DrVec2(tx2, ty2), Triangle_Point::Point3);

    add(DrVec3(x2, y2, -c_extrude_depth), n, DrVec2(tx2, ty2), Triangle_Point::Point1);
    add(DrVec3(x3, y3, -c_extrude_depth), n, DrVec2(tx3, ty3), Triangle_Point::Point2);
    add(DrVec3(x4, y4, -c_extrude_depth), n, DrVec2(tx4, ty4), Triangle_Point::Point3);
}


//####################################################################################
//##    Adds a Triangle, front and back
//####################################################################################
void DrMesh::triangle(float x1, float y1, float tx1, float ty1,
                      float x2, float y2, float tx2, float ty2,
                      float x3, float y3, float tx3, float ty3, float depth_multiplier) {
    DrVec3 n;
    n = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x3, y3, 0.f), DrVec3(x2, y2, 0.f));

    float depth = c_extrude_depth * depth_multiplier;;

    add(DrVec3(x1, y1, +depth), n, DrVec2(tx1, ty1), Triangle_Point::Point1);
    add(DrVec3(x2, y2, +depth), n, DrVec2(tx2, ty2), Triangle_Point::Point2);
    add(DrVec3(x3, y3, +depth), n, DrVec2(tx3, ty3), Triangle_Point::Point3);

    n = DrVec3::triangleNormal(DrVec3(x1, y1, 0.f), DrVec3(x2, y2, 0.f), DrVec3(x3, y3, 0.f));

    add(DrVec3(x1, y1, -depth), n, DrVec2(tx1, ty1), Triangle_Point::Point1);
    add(DrVec3(x3, y3, -depth), n, DrVec2(tx3, ty3), Triangle_Point::Point2);
    add(DrVec3(x2, y2, -depth), n, DrVec2(tx2, ty2), Triangle_Point::Point3);
}

//####################################################################################
//##    Adds a Quad extruded from an Edge
//####################################################################################
void DrMesh::extrude(float x1, float y1, float tx1, float ty1,
                     float x2, float y2, float tx2, float ty2, int steps, float depth_multiplier) {
    float depth = c_extrude_depth * depth_multiplier;

    float step = (depth * 2.0f) / static_cast<float>(steps);
    float front = depth;
    float back =  depth - step;

    for (int i = 0; i < steps; i++) {
        DrVec3 n;
        n = DrVec3::triangleNormal(DrVec3(x1, y1, front), DrVec3(x2, y2, front), DrVec3(x1, y1, back));

        add(DrVec3(x1, y1, front), n, DrVec2(tx1, ty1), Triangle_Point::Point1);
        add(DrVec3(x1, y1, back),  n, DrVec2(tx1, ty1), Triangle_Point::Point2);
        add(DrVec3(x2, y2, front), n, DrVec2(tx2, ty2), Triangle_Point::Point3);

        n = DrVec3::triangleNormal(DrVec3(x2, y2, front), DrVec3(x2, y2, back), DrVec3(x1, y1, back));

        add(DrVec3(x2, y2, front), n, DrVec2(tx2, ty2), Triangle_Point::Point1);
        add(DrVec3(x1, y1, back),  n, DrVec2(tx1, ty1), Triangle_Point::Point2);
        add(DrVec3(x2, y2, back),  n, DrVec2(tx2, ty2), Triangle_Point::Point3);

        front -= step;
        back  -= step;
    }

}














