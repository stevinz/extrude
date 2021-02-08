//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
//  For info and code framework for Lapacian Smooting see:
//      http://rodolphe-vaillant.fr/?c=code
//      https://github.com/silenthell/harmonic_weights_triangle_mesh
//
#include <limits>

#include "image_filters.h"
#include "vertex_data.h"
#include "types/vec3.h"


//####################################################################################
//##    Rounds to two decimal places, this keeps neighbors
//####################################################################################
float roundToDecimalPlace(float number_to_round, int decimal_places) {
    // If rounding to decimal_places == 2:
    //      37.66666 * 100 = 3766.66
    //      3766.66 + .5 =   3767.16    for rounding off value
    //      then cast to int so value is 3767
    //      then divide by 100 so the value converted into 37.67
    float power = powf(10.f, decimal_places);
    float value = static_cast<int>(number_to_round * power + 0.5f);
    return (static_cast<float>(value) / power);
}

//####################################################################################
//##    Loads singular vertex from Data Array
//####################################################################################
Vertex DrEngineVertexData::getVertex(int vertex_number) {
    GLfloat *p = m_data.data() + (vertex_number * c_vertex_length);
    Vertex v;
    v.position.x =          *p++;
    v.position.y =          *p++;
    v.position.z =          *p++;
    v.normal.x =            *p++;
    v.normal.y =            *p++;
    v.normal.z =            *p++;
    v.texture_coords.x =    *p++;
    v.texture_coords.y =    *p++;
    v.barycentric.x =       *p++;
    v.barycentric.y =       *p++;
    v.barycentric.z =       *p++;

    v.position.z *= 10.f;
    v.position.x = roundToDecimalPlace(v.position.x, 1);
    v.position.y = roundToDecimalPlace(v.position.y, 1);
    v.position.z = roundToDecimalPlace(v.position.z, 1);
    return v;
}

//####################################################################################
//##    Sets a singular Vertex into Data Array
//####################################################################################
void DrEngineVertexData::setVertex(int vertex_number, Vertex v) {
    GLfloat *p = m_data.data() + (vertex_number * c_vertex_length);
    *p++ = v.position.x;
    *p++ = v.position.y;
    *p++ = v.position.z / 10.f;
    *p++ = v.normal.x;
    *p++ = v.normal.y;
    *p++ = v.normal.z;
    *p++ = v.texture_coords.x;
    *p++ = v.texture_coords.y;
    *p++ = v.barycentric.x;
    *p++ = v.barycentric.y;
    *p++ = v.barycentric.z;
}


//####################################################################################
//##    Adds point_to_add to neighbor_list if point is not already included
//####################################################################################
void addNeighbor(std::vector<Vertex> &neighbor_list, Vertex point_to_add) {
    for (auto &point : neighbor_list) {
        if (point.position == point_to_add.position)
            return;
    }
    neighbor_list.push_back(point_to_add);
}

//####################################################################################
//##    Loads Data Array into Mesh (list of triangles)
//####################################################################################
Mesh DrEngineVertexData::getMesh(NeighborMap &neighbors) {
    Mesh mesh;
    for (int i = 0; i < vertexCount(); i += 3) {
        Vertex point0 = getVertex(i);
        Vertex point1 = getVertex(i+1);
        Vertex point2 = getVertex(i+2);

        addNeighbor(neighbors[point0.position], point1);
        addNeighbor(neighbors[point0.position], point2);
        addNeighbor(neighbors[point1.position], point0);
        addNeighbor(neighbors[point1.position], point2);
        addNeighbor(neighbors[point2.position], point0);
        addNeighbor(neighbors[point2.position], point1);

        Triangle tri;
        tri.points.push_back( point0 );
        tri.points.push_back( point1 );
        tri.points.push_back( point2 );
        mesh.m_triangles.push_back(tri);
    }
    return mesh;
}



//####################################################################################
//##    Smooths Vertices based on 'weight' of neighbors, recalculates normals
//####################################################################################
void DrEngineVertexData::smoothVertices(float weight) {

    // ***** Get Data Array into Mesh, find neighbors
    NeighborMap neighbors;
    Mesh mesh = getMesh(neighbors);

    // ***** Smooth points
    for (auto &triangle : mesh.m_triangles) {
        for (auto &point : triangle.points) {
            DrVec3  position(0.f);
            DrVec3  normals(0.f);
            DrVec3  texture(0.f);
            float   total_weight = 0.f;

            std::size_t prev = neighbors[point.position].size() - 1;
            std::size_t next = 1;
            for (std::size_t i = 0; i < neighbors[point.position].size(); i++) {
                Vertex neighbor = neighbors[point.position][i];
                ///Vertex neighbor_prev = neighbors[point.position][prev];
                ///Vertex neighbor_next = neighbors[point.position][next];

                ///
                /// #NOTE: Not fully implemented, Laplacian smoothing would go here
                ///
                ///
                float neighbor_weight = weight;
                ///
                ///
                ///

                position += (neighbor_weight * neighbor.position);
                normals  += (neighbor_weight * neighbor.normal);
                texture  += (neighbor_weight * neighbor.texture_coords);
                total_weight += neighbor_weight;

                // Increment neighbors
                prev = i;
                next++;
                if (next >= neighbors[point.position].size()) next = 0;
            }

            // When using cotan weights smoothing may be unstable, in this case we need to set t < 1
            // sometimes you even need to get as low as t < 0.5
            float t = 0.9f;
            point.position =       ((position / total_weight) * t + point.position * (1.f - t)).toGlmVec3();
            point.texture_coords = ((texture /  total_weight) * t + point.texture_coords * (1.f - t)).toGlmVec3();
            point.normal =         ((normals /  total_weight) * t + point.normal * (1.f - t)).toGlmVec3();
        }
    }

    // ***** Set Mesh Data back into Data Array
    for (int i = 0; i < vertexCount(); i += 3) {
        Triangle tri = mesh.m_triangles[static_cast<std::size_t>(i / 3)];

        // Recalculate normals
        DrVec3 v1 = tri.points[2].position - tri.points[0].position;
        DrVec3 v2 = tri.points[1].position - tri.points[0].position;
        DrVec3 n = v1.cross( v2 );
        n.normalize();
        for (auto &point : tri.points) { point.normal = n; }

        setVertex(i,   tri.points[0]);
        setVertex(i+1, tri.points[1]);
        setVertex(i+2, tri.points[2]);
    }
}























