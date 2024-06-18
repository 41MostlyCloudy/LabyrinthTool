#pragma once

#include "Objects.cpp"

// Important variables
//RGBColor* screenColorData; // Screen data for player camera
uint8_t* paletteScreenData; // Screen data for player camera
RGBColor* screenColorData; // Screen data

float* depthBuffer;
float deltaT;// Multiply to get frame-independent speed.
float cameraNear = 0.5; // The near distance from the camera that triangles are clipped

// Screen configuration variables
float windowRatio = 1; // The aspect ratio of your computer screen
int screenWidth = 640;
int screenHeight = 400;
float screenRatio = 1; // The aspect ratio of the screen the game is draw to
float fov = 1; // Field of view

float globalAnimationClock = 0; // A clock that is always running and resetting to 0 at 6.283. This is an estimation of 2pi for sin waves.


double mousePosX, mousePosY = 0;
float mouseSensitivity = 0.01;

//float playerSize = 1;
//bool playerOnFloor = false;

// Resources
vector <Texture> loadedTextures;
vector <Mesh> loadedMeshes;
vector <MeshInstance> loadedMeshInstances;

GradientMap loadedGradient; // Light to dark gradient for all 256 colors in the game (using the same 256 colors)
LookupTexture lookupTexture; // 256 color lookup array for all colors on the screen

// Movement
Vector3 cameraPosition = { 0, -2, 0 };
Vector3 cameraRotation;
Vector3 cameraVelocity;

// Level properties


// Editing tool
bool editing = false;
bool editingVertex = false;
int chosenPaintColor = 0;
int editMode = 0; // 0: Vertex paint, 1: Texture paint, 2: portal placement & room creation, 3: Object placement
bool displayFPS = false;
bool displayGradient = false;




bool canMove = false;
bool spaceKeyPressed = false;