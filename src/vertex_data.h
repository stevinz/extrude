//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
#ifndef ENGINE_VERTEX_DATA_H
#define ENGINE_VERTEX_DATA_H

#include <map>

#include "mesh.h"

// Forward Declarations
class DrBitmap;
class DrImage;
class DrPointF;
class DrVec2;
class DrVec3;

// Type Definitions
typedef std::map<DrVec3, std::vector<Vertex>> NeighborMap;

// Defines
#define PAR_RGB  3
#define PAR_RGBA 4

// Constants
const int   c_vertex_length = 11;
const float c_extrude_depth = 0.5f;

// Vertex Declaration (11 total data points)
typedef struct {
    float x, y, z;          // Position
    float n1, n2, n3;       // Normal
    float u, v;             // Texture Coordinate
    float b1, b2, b3;       // Barycentric coordinate (used for wireframe rendering)
} vertex_t;

// Local Enums
enum class Trianglulation {
    Ear_Clipping,
    Monotone,
    Delaunay,
};

enum class Triangle_Point {
    Point1,
    Point2,
    Point3,
};


//####################################################################################
//##    DrEngineVertexData
//##        Stores a list of triangles for rendering
//############################
class DrEngineVertexData
{
private:
    std::vector<vertex_t>   m_vertices;

public:
    // Constructor
    DrEngineVertexData();

    // Properties
    vertex_t       *vertices() {            return m_vertices.data(); }
    int             count() const {         return m_vertices.size(); }
    int             triangleCount() const { return vertexCount() / 3; }
    int             vertexCount() const {   return count(); }

    // Creation Functions
    void    initializeExtrudedImage(DrImage *image, bool wireframe = true);

    void    initializeTextureCone();
    void    initializeTextureCube();
    void    initializeTextureQuad();


    // Helper Functions
    static  std::vector<DrPointF>   insertPoints(  const std::vector<DrPointF> &outline_points);
    static  std::vector<DrPointF>   smoothPoints(  const std::vector<DrPointF> &outline_points, int neighbors, double neighbor_distance, double weight);


    // Extrusion Functions
    void    extrudeFacePolygon(const std::vector<DrPointF> &outline_points, int width, int height, int steps);
    void    triangulateFace(const std::vector<DrPointF> &outline_points, const std::vector<std::vector<DrPointF>> &hole_list,
                            const DrBitmap &image, bool wireframe, Trianglulation type, double alpha_tolerance);

    // Building Functions
    void    add(const DrVec3 &vertex, const DrVec3 &normal, const DrVec2 &text_coord, Triangle_Point point_number);
    void    extrude(float x1, float y1, float tx1, float ty1,
                    float x2, float y2, float tx2, float ty2, int steps = 1);
    void    cube(float x1, float y1, float tx1, float ty1,
                 float x2, float y2, float tx2, float ty2,
                 float x3, float y3, float tx3, float ty3,
                 float x4, float y4, float tx4, float ty4);
    void    quad(float x1, float y1, float tx1, float ty1,
                 float x2, float y2, float tx2, float ty2,
                 float x3, float y3, float tx3, float ty3,
                 float x4, float y4, float tx4, float ty4);
    void    triangle(float x1, float y1, float tx1, float ty1,
                     float x2, float y2, float tx2, float ty2,
                     float x3, float y3, float tx3, float ty3);
};


#endif // ENGINE_VERTEX_DATA_H











