#pragma once

#include "Global.cpp"


// Move a point in 3d space
Vector3 Translate(Vector3 vect, Vector3 vect2);

// Find the normal of a triangle
float CalculateNormal(Triangle tri);

// Assigns a texture to a triangle based on the position of its uv map
// In the future, I hope to remove this method, probably to include textures per triangle saved in the .obj files. Unfortunately Blender would not be able to use that, and sometimes objects need to be reedited in Blender without the hassle of converting them.
// I do not plan on saving textures per model instead of per triangle.
int AssignTriTexture(Triangle tri);

// Tests collision with triangles and then return the player's new velocity
Vector3 TestCollision(Vector3 vel);

// Changes the size of the screen
void ResizeScreen(int newW, int newH, GLFWwindow* window);



// Functions for rotating vectors around axes
Vector3 RotVecX(Vector3 vec, float angle);
Vector3 RotVecY(Vector3 vec, float angle);
Vector3 RotVecZ(Vector3 vec, float angle);



Vector3 Translate(Vector3 vect, Vector3 vect2)
{
    return { vect.x + vect2.x, vect.y + vect2.y, vect.z + vect2.z };
}



float CalculateNormal(Triangle tri)
{
    // Use Cross Product formula to find the normal of the triangle. Only draw triangles with a normal facing the camera
    Vector3 normal;
    normal.x = ((tri.p[1].coord.y - tri.p[0].coord.y) * (tri.p[2].coord.z - tri.p[0].coord.z)) - ((tri.p[1].coord.z - tri.p[0].coord.z) * (tri.p[2].coord.y - tri.p[0].coord.y));
    normal.y = ((tri.p[1].coord.z - tri.p[0].coord.z) * (tri.p[2].coord.x - tri.p[0].coord.x)) - ((tri.p[1].coord.x - tri.p[0].coord.x) * (tri.p[2].coord.z - tri.p[0].coord.z));
    normal.z = ((tri.p[1].coord.x - tri.p[0].coord.x) * (tri.p[2].coord.y - tri.p[0].coord.y)) - ((tri.p[1].coord.y - tri.p[0].coord.y) * (tri.p[2].coord.x - tri.p[0].coord.x));

    // Normalize the face's normal vector
    float vecLength = 1 / sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    normal.x *= vecLength;
    normal.y *= vecLength;
    normal.z *= vecLength;

    Vector3 vec3;
    float vecLength2 = 1 / sqrt(tri.p[0].coord.x * tri.p[0].coord.x + tri.p[0].coord.y * tri.p[0].coord.y + tri.p[0].coord.z * tri.p[0].coord.z);
    vec3.x = tri.p[0].coord.x * vecLength2;
    vec3.y = tri.p[0].coord.y * vecLength2;
    vec3.z = tri.p[0].coord.z * vecLength2;

    // Return the dot product
    return (normal.x * vec3.x) + (normal.y * vec3.y) + (normal.z * vec3.z);
}



int AssignTriTexture(Triangle tri)
{
    int nearestU = tri.p[0].uv.u;
    if (tri.p[1].uv.u < nearestU)
        nearestU = tri.p[1].uv.u;
    if (tri.p[2].uv.u < nearestU)
        nearestU = tri.p[2].uv.u;

    int nearestV = tri.p[0].uv.v;
    if (tri.p[1].uv.v < nearestV)
        nearestV = tri.p[1].uv.v;
    if (tri.p[2].uv.v < nearestV)
        nearestV = tri.p[2].uv.v;

    if (nearestU < 0)
        nearestU = 0;
    if (nearestV < 0)
        nearestV = 0;

    tri.p[0].uv.u -= nearestU;
    tri.p[1].uv.u -= nearestU;
    tri.p[2].uv.u -= nearestU;
    tri.p[0].uv.v -= nearestV;
    tri.p[1].uv.v -= nearestV;
    tri.p[2].uv.v -= nearestV;

    return nearestU + nearestV * 8;
}



// Functions for transforming Vectors
Vector3 RotVecX(Vector3 vec, float angle)
{
    Vector3 retVec;

    retVec.x = vec.x;
    retVec.y = vec.y * cos(angle) + vec.z * sin(angle);
    retVec.z = vec.y * -sin(angle) + vec.z * cos(angle);

    return retVec;
}


Vector3 RotVecY(Vector3 vec, float angle)
{
    Vector3 retVec;

    retVec.x = vec.x * cos(angle) + vec.z * -sin(angle);
    retVec.y = vec.y;
    retVec.z = vec.x * sin(angle) + vec.z * cos(angle);

    return retVec;
}


Vector3 RotVecZ(Vector3 vec, float angle)
{
    Vector3 retVec;

    retVec.x = vec.x * cos(angle) + vec.y * sin(angle);
    retVec.y = vec.x * sin(angle) + vec.y * cos(angle);
    retVec.z = vec.z;

    return retVec;
}



Vector3 TestCollision(Vector3 vel)
{
    // Test each tri with the player's coordinates pre-movement
    // Test each tri with the player's coordinates post-movement
    // If player goes from positive to negative side of plane, player has collided with plane

    // Find player intersection with wall

    // move the player to the wall if colliding

    // The vector by which the player will be translated
    Vector3 playerMove = vel;
    // The player's distance to travel
    float moveDist = sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
    // The velocity the player slides along the wall
    Vector3 slideVect;

    bool collided = false;

    float colliderSize = 1.2;

    
    if (!editing)
    {
        // Test collision on every triangle
        for (int i = 0; i < loadedMeshInstances.size(); i++) // Exclude testing box for now
        {
            for (int j = 0; j < loadedMeshInstances.at(i).instanceMesh.tris.size(); j++)
            {
                Triangle tri = loadedMeshInstances.at(i).instanceMesh.tris[j];

                // Use Cross Product formula to find the normal of the triangle.
                Vector3 a;
                a.x = tri.p[1].coord.x - tri.p[0].coord.x;
                a.y = tri.p[1].coord.y - tri.p[0].coord.y;
                a.z = tri.p[1].coord.z - tri.p[0].coord.z;
                Vector3 b;
                b.x = tri.p[2].coord.x - tri.p[0].coord.x;
                b.y = tri.p[2].coord.y - tri.p[0].coord.y;
                b.z = tri.p[2].coord.z - tri.p[0].coord.z;

                Vector3 normal;
                normal.x = (a.y * b.z) - (a.z * b.y);
                normal.y = (a.z * b.x) - (a.x * b.z);
                normal.z = (a.x * b.y) - (a.y * b.x);

                // normalize the normal
                float vecLength = 1 / sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                normal.x *= vecLength;
                normal.y *= vecLength;
                normal.z *= vecLength;



                // The start and end points of the player's movement
                Vector3 start = cameraPosition;
                start.x *= -1;
                start.y *= -1;
                start.z *= -1;
                Vector3 end = cameraPosition;
                end = Translate(end, playerMove);
                end.x *= -1;
                end.y *= -1;
                end.z *= -1;

                // Add extra space for collision sphere
                start.x -= normal.x * colliderSize;
                start.y -= normal.y * colliderSize;
                start.z -= normal.z * colliderSize;

                end.x -= normal.x * colliderSize;
                end.y -= normal.y * colliderSize;
                end.z -= normal.z * colliderSize;



                // Find distance of start and end point to plane
                // Do this with the dot product formula
                float startDist = normal.x * (start.x - tri.p[0].coord.x) + normal.y * (start.y - tri.p[0].coord.y) + normal.z * (start.z - tri.p[0].coord.z);
                float endDist = normal.x * (end.x - tri.p[0].coord.x) + normal.y * (end.y - tri.p[0].coord.y) + normal.z * (end.z - tri.p[0].coord.z);

                


                if (startDist >= -0.5 && endDist < 0) // If there is a collision (collision is detected on to slightly inside of the wall)
                {

                    // find the point of collision
                    Vector3 colPoint;
                    float denom = startDist - endDist;


                    colPoint.x = (startDist * end.x - endDist * start.x) / denom;
                    colPoint.y = (startDist * end.y - endDist * start.y) / denom;
                    colPoint.z = (startDist * end.z - endDist * start.z) / denom;


                    // Test whether plane collision is inside the triangle1
                    Vector3 u;
                    u.x = tri.p[1].coord.x - tri.p[0].coord.x;
                    u.y = tri.p[1].coord.y - tri.p[0].coord.y;
                    u.z = tri.p[1].coord.z - tri.p[0].coord.z;
                    Vector3 v;
                    v.x = tri.p[2].coord.x - tri.p[0].coord.x;
                    v.y = tri.p[2].coord.y - tri.p[0].coord.y;
                    v.z = tri.p[2].coord.z - tri.p[0].coord.z;
                    Vector3 w;
                    w.x = colPoint.x - tri.p[0].coord.x;
                    w.y = colPoint.y - tri.p[0].coord.y;
                    w.z = colPoint.z - tri.p[0].coord.z;

                    float dotUV = u.x * v.x + u.y * v.y + u.z * v.z;
                    float dotWV = w.x * v.x + w.y * v.y + w.z * v.z;
                    float dotVV = v.x * v.x + v.y * v.y + v.z * v.z;
                    float dotWU = w.x * u.x + w.y * u.y + w.z * u.z;
                    float dotUU = u.x * u.x + u.y * u.y + u.z * u.z;

                    float a = (dotUV * dotWV - dotVV * dotWU) / (dotUV * dotUV - dotUU * dotVV);
                    float b = (dotUV * dotWU - dotUU * dotWV) / (dotUV * dotUV - dotUU * dotVV);


                    if (a >= 0 && b >= 0 && a + b <= 1) // Point is inside triangle
                    {

                        // Find the player's vector into the wall
                        Vector3 vectPastWall;
                        vectPastWall.x = end.x - colPoint.x;
                        vectPastWall.y = end.y - colPoint.y;
                        vectPastWall.z = end.z - colPoint.z;

                        // Find the player's distance into the wall
                        float perpendicularDist = vectPastWall.x * -normal.x + vectPastWall.y * -normal.y + vectPastWall.z * -normal.z;

                        // Find the player's vector perpendicular to the wall
                        Vector3 perpendicularVect;

                        perpendicularVect.x = -normal.x * perpendicularDist;
                        perpendicularVect.y = -normal.y * perpendicularDist;
                        perpendicularVect.z = -normal.z * perpendicularDist;


                        // Subtract it from the full velocity to find the velocity against the wall
                        Vector3 tanVect;
                        tanVect.x = -(vectPastWall.x - perpendicularVect.x);
                        tanVect.y = -(vectPastWall.y - perpendicularVect.y);
                        tanVect.z = -(vectPastWall.z - perpendicularVect.z);



                        // Calculate the distance of the new vector
                        Vector3 colPoint2 = Translate(cameraPosition, colPoint);

                        // Offset the distance by the collider size
                        colPoint2.x += normal.x * colliderSize;
                        colPoint2.y += normal.y * colliderSize;
                        colPoint2.z += normal.z * colliderSize;

                        float dist = sqrt(colPoint2.x * colPoint2.x + colPoint2.y * colPoint2.y + colPoint2.z * colPoint2.z);

                        

                        if (dist < moveDist)
                        {
                            playerMove = colPoint;

                            // Move the player to the edge of the collision box
                            playerMove.x += normal.x * colliderSize;
                            playerMove.y += normal.y * colliderSize;
                            playerMove.z += normal.z * colliderSize;

                            slideVect = tanVect;
                            moveDist = dist;
                            collided = true;
                        }
                    }
                }
            }
        }
    }


    if (collided)
    {
        cameraPosition.x = -playerMove.x;
        cameraPosition.y = -playerMove.y;
        cameraPosition.z = -playerMove.z;
    }
    else
        cameraPosition = Translate(cameraPosition, playerMove);

    return (slideVect);
}




void ResizeScreen(int newW, int newH, GLFWwindow* window)
{
    // Swap front and back buffers and clear them all
    // This results in a small blink when changing screen dimensions; to be fixed soon.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    screenWidth = newW;
    screenHeight = newH;

    // Update the screen data to the screen size
    delete[] paletteScreenData;
    delete[] depthBuffer;
    delete[] screenColorData;

    paletteScreenData = new uint8_t[screenWidth * screenHeight];
    depthBuffer = new float[screenWidth * screenHeight];
    screenColorData = new RGBColor[screenWidth * screenHeight];

    



    float width = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
    float height = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

    windowRatio = height / width;

    screenRatio = float(screenWidth) / float(screenHeight);

    windowRatio *= screenRatio;

    screenRatio = 1 / screenRatio;
}