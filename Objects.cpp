#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono> // Deals with time
#include <stdlib.h> // For rand()

using namespace std;



// Color structure
struct RGBColor { uint8_t r, g, b; };


// RGB Lookup texture for 256 color palette
struct LookupTexture { RGBColor px[255]; };


// texture of any size (defaulting to 64x) using colors from the lookup table
struct Texture
{
    vector <uint8_t> px; // Array of texture pixels
    int textureWidth = 64;
    int textureHeight = 64;
    bool scroll = false;
    bool additive = false; // Additive textures will change the brightness of textures behind themselves
    bool noCull = false; // Triangles with this texture will have no back face culling (they will really just make a duplicate triangle for the back)
};


// 2D structure
struct UV { float u = 0, v = 0; };


// 3D structure
struct Vector3 { float x = 0, y = 0, z = 0; };


// A point on a mesh with usual point data
struct Point
{
    Vector3 coord;
    float vertCol = 0;
    UV uv;
};


// a triangle structure
struct Triangle
{
    Point p[3]; // The position of each vertex
    uint8_t texture = 0; // The texture to be drawn on the triangle
    float dist = 0; // The triangle's distance from the camera, using its nearest points
};


// A 3d object structure
struct Mesh { vector<Triangle> tris; };


// A 3d object instance
struct MeshInstance
{
    Mesh instanceMesh;
    int meshNumber = 0; // The position of the instanced mesh in the mesh vector. This is only used when a reference to the original reference is needed.
    Vector3 position;
    Vector3 rotation;
};


// A map of the values from light to dark for all 256 colors in the color palette for quick lighting.
struct GradientMap
{
    uint8_t grad[16384];
};