

#include "FileHandling.cpp"
#include "Render.cpp"


// Next order of business:

// - Fix collision
// - Add jumping, running velocity, and player mode / camera mode for editing
// - Add sound
// - Create rooms and plane culling. Culling planes will be on axes, so that checking them is very fast (playerX > portal1). Crossing a plane will bring its associated rooms in or out of the level
// - Add object system
// - Add level system
// - Decide object light level by vertex light on floor at collision point (even if in air)
// - Add simple animation system





    

// Runs everything
void RunEngine();

// Takes user input
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Updates physics, called every frame
void UpdatePhysics(float delta);



// The actions performed when starting and running the engine
void RunEngine()
{
    
    // Load the meshes
    LoadAssets();


    glDisable(GL_DEPTH_TEST);


    // Initialize the library
    glfwInit();

    // Create a windowed mode window and its OpenGL context
    float width = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
    float height = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

    
    GLFWwindow* window = glfwCreateWindow(width, height, "", glfwGetPrimaryMonitor(), nullptr);

    // Update the screen data to the screen size
    ResizeScreen(screenWidth, screenHeight, window);

    glfwSetCursorPos(window, 0, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Create Screen Texture
    unsigned int screenTex;
    glGenTextures(1, &screenTex);
    glBindTexture(GL_TEXTURE_2D, screenTex);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


    // Keeps track of whether the scrolling textures should move each frame
    bool readyToScrollTexture = false;
    
    //

    
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Start the delta timer
        std::chrono::high_resolution_clock time;
        auto start = time.now();

        // Update the global animation clock
        globalAnimationClock += deltaT * 0.005;

        if (globalAnimationClock > 6.283) // Wraps between 0 and approximately 2pi to make animating with sin waves easy (mostly for water effect)
            globalAnimationClock -= 6.283;

        //globalPulse = sin(globalAnimationClock - 3.14159);

        // Update game physics
        UpdatePhysics(deltaT);

        for (int y = 0; y < (screenWidth * screenHeight); y++)
        {
            depthBuffer[y] = 400; // Here, 400 is the farthest away from the camera an object can render
            paletteScreenData[y] = 0;
        }

        /////////////////////////////////////////////////////////////////////////// Drawing
 
        
        // Drawing a billboarded texture of the earth.
        // Later, change this to a more robust object system for billboards.
        Vector3 worldPoint = { -39.5, -17, -67.5 };

        worldPoint = Translate(worldPoint, cameraPosition);
        worldPoint = RotVecY(worldPoint, cameraRotation.y);
        worldPoint = RotVecX(worldPoint, cameraRotation.x);

        worldPoint.x *= screenRatio;
        worldPoint.x *= fov;
        worldPoint.y *= fov;

        worldPoint.x = worldPoint.x / (worldPoint.z) + 0.5;
        worldPoint.y = -worldPoint.y / (worldPoint.z) + 0.5;

        worldPoint.x *= screenWidth;
        worldPoint.y *= screenHeight;

        DrawBillboard(worldPoint, 11, 2);
        
        
        // A list of all the triangles that will be drawn
        vector <Triangle> sortedTriangles;

        sortedTriangles.clear();

        // Draw the triangles for each loaded mesh
        for (int i = 0; i < loadedMeshInstances.size(); i++)
        {
            for (int j = 0; j < loadedMeshInstances.at(i).instanceMesh.tris.size(); j++)
            {
                Triangle worldPoint = loadedMeshInstances.at(i).instanceMesh.tris[j];
                
                // Apply object rotation
                worldPoint.p[0].coord = RotVecY(worldPoint.p[0].coord, loadedMeshInstances.at(i).rotation.y);
                worldPoint.p[1].coord = RotVecY(worldPoint.p[1].coord, loadedMeshInstances.at(i).rotation.y);
                worldPoint.p[2].coord = RotVecY(worldPoint.p[2].coord, loadedMeshInstances.at(i).rotation.y);


                // Apply object transformations and rotations
                worldPoint.p[0].coord = Translate(worldPoint.p[0].coord, loadedMeshInstances.at(i).position);
                worldPoint.p[1].coord = Translate(worldPoint.p[1].coord, loadedMeshInstances.at(i).position);
                worldPoint.p[2].coord = Translate(worldPoint.p[2].coord, loadedMeshInstances.at(i).position);

                
                worldPoint.p[0].coord = Translate(worldPoint.p[0].coord, cameraPosition);
                worldPoint.p[1].coord = Translate(worldPoint.p[1].coord, cameraPosition);
                worldPoint.p[2].coord = Translate(worldPoint.p[2].coord, cameraPosition);

                
                // Find the normal of the triangle
                float normal = CalculateNormal(worldPoint);


                // Draw the projected triangle.
                if (normal < 0 || loadedTextures[worldPoint.texture].noCull) // Back face culling
                {     
                    // Rotate each triangle to match camera space, and then project it.

                    worldPoint.p[0].coord = RotVecY(worldPoint.p[0].coord, cameraRotation.y);
                    worldPoint.p[1].coord = RotVecY(worldPoint.p[1].coord, cameraRotation.y);
                    worldPoint.p[2].coord = RotVecY(worldPoint.p[2].coord, cameraRotation.y);

                    worldPoint.p[0].coord = RotVecX(worldPoint.p[0].coord, cameraRotation.x);
                    worldPoint.p[1].coord = RotVecX(worldPoint.p[1].coord, cameraRotation.x);
                    worldPoint.p[2].coord = RotVecX(worldPoint.p[2].coord, cameraRotation.x);


                    // vertex editing
                    if (editing)
                    {
                        if (worldPoint.p[0].coord.z > 0 && worldPoint.p[0].coord.z < 4 && abs(worldPoint.p[0].coord.x) < 1 && abs(worldPoint.p[0].coord.y) < 1)
                        {
                            if (editingVertex)
                            {
                                loadedMeshInstances.at(i).instanceMesh.tris[j].p[0].vertCol = chosenPaintColor;
                                loadedMeshes[loadedMeshInstances.at(i).meshNumber].tris[j].p[0].vertCol = chosenPaintColor;
                            }
                            else
                                worldPoint.p[0].vertCol = chosenPaintColor;
                        }
                        else if (worldPoint.p[1].coord.z > 0 && worldPoint.p[1].coord.z < 4 && abs(worldPoint.p[1].coord.x) < 1 && abs(worldPoint.p[1].coord.y) < 1)
                        {
                            if (editingVertex)
                            {
                                loadedMeshInstances.at(i).instanceMesh.tris[j].p[1].vertCol = chosenPaintColor;
                                loadedMeshes[loadedMeshInstances.at(i).meshNumber].tris[j].p[1].vertCol = chosenPaintColor;
                            }
                            else
                                worldPoint.p[1].vertCol = chosenPaintColor;
                        }
                        else if (worldPoint.p[2].coord.z > 0 && worldPoint.p[2].coord.z < 4 && abs(worldPoint.p[2].coord.x) < 1 && abs(worldPoint.p[2].coord.y) < 1)
                        {
                            if (editingVertex)
                            {
                                loadedMeshInstances.at(i).instanceMesh.tris[j].p[2].vertCol = chosenPaintColor;
                                loadedMeshes[loadedMeshInstances.at(i).meshNumber].tris[j].p[2].vertCol = chosenPaintColor;
                            }
                            else
                                worldPoint.p[2].vertCol = chosenPaintColor;
                        }

                    }

                    // Sort the triangle
                    if (loadedTextures[worldPoint.texture].additive) // Additive textures are rendered last
                    {
                        worldPoint.dist = 1000;
                        sortedTriangles.emplace_back(worldPoint);
                    }
                    else
                    {
                        float dist = 0.1;

                        dist = worldPoint.p[0].coord.z;
                        if (worldPoint.p[1].coord.z < dist)
                            dist = worldPoint.p[1].coord.z;
                        if (worldPoint.p[2].coord.z < dist)
                            dist = worldPoint.p[2].coord.z;

                        worldPoint.dist = dist;

                        int index = 0;

                        while (index < sortedTriangles.size() && dist < sortedTriangles[index].dist)
                        {
                            index++;
                        }

                        sortedTriangles.emplace(sortedTriangles.begin() + index, worldPoint);
                    }
                }
            }
        }

        
        // Clip, project and draw the triangles
        for (int i = 0; i < sortedTriangles.size(); i++)
        {
            Clip(sortedTriangles[i], false);
        }



        
        // warp the water texture

        for (int y = 0; y < 64; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                float warpedU = float(x) / 64;
                float warpedV = float(y) / 64;

                warpedU += sin(globalAnimationClock + warpedU * 2 * 3.14159) * 0.07;
                warpedV += sin(globalAnimationClock + warpedV * 2 * 3.14159) * 0.07;

                // Wrap the texture
                while (warpedU > 1)
                    warpedU -= 1;
                while (warpedU < 0)
                    warpedU += 1;
                while (warpedV > 1)
                    warpedV -= 1;
                while (warpedV < 0)
                    warpedV += 1;

                warpedU *= 64;
                warpedV *= 64;

                loadedTextures[0].px[x + y * 64] = loadedTextures[1].px[int(warpedU) + int(warpedV) * 64];
            }
        }

        // Set the scrolling water texture to the warped water texture
        loadedTextures[2].px = loadedTextures[0].px;

        
        
        // Shows the gradients on the screen
        if (displayGradient)
        {
            for (int y = 0; y < 256; y++)
            {
                for (int x = 0; x < 64; x++)
                {
                    if (x + screenWidth * (y + 64) < screenWidth * screenHeight)
                        paletteScreenData[x + screenWidth * (y + 64)] = loadedGradient.grad[x + y * 64];
                }
            }
        }


        // Draw framerate counter
        if (displayFPS)
            DrawText("FPS " + to_string(1000 / deltaT), 0, 0); // Draw FPS counter
        
        if (editing)
        {
            if (editMode == 0)
            {
                DrawText("EDIT VERTEX PAINT", 0, 16);
                DrawText("VERTEX COLOR " + to_string(chosenPaintColor), 0, 32);
            }
            else if(editMode == 1)
            {
                DrawText("EDIT TEXTURE", 0, 16);
                DrawText("TEXTURE " + to_string(chosenPaintColor), 0, 32);
            }
        }



        // Draw to the screen
        for (int i = 0; i < screenWidth * screenHeight; i++)
            screenColorData[i] = lookupTexture.px[paletteScreenData[i]];

        
        ///////////////////////////////////////////////////////////////////////////

        // Create window quad

        if (windowRatio > 1)
        {
            glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0); glVertex3f(-1.0f, -1/windowRatio, 0.0f);
            glTexCoord2f(0.0, 0.0); glVertex3f(-1.0f, 1/windowRatio, 0.0f);
            glTexCoord2f(1.0, 0.0); glVertex3f(1.0f, 1/windowRatio, 0.0f);
            glTexCoord2f(1.0, 1.0); glVertex3f(1.0f, -1/windowRatio, 0.0f);
            glEnd();
        }
        else
        {
            glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0); glVertex3f(-windowRatio, -1.0f, 0.0f);
            glTexCoord2f(0.0, 0.0); glVertex3f(-windowRatio, 1.0f, 0.0f);
            glTexCoord2f(1.0, 0.0); glVertex3f(windowRatio, 1.0f, 0.0f);
            glTexCoord2f(1.0, 1.0); glVertex3f(windowRatio, -1.0f, 0.0f);
            glEnd();
        }
        
        
        // Possible code for displaying 4 windows.
        /*
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0); glVertex3f(-windowRatio, -1.0f, 0.0f);
        glTexCoord2f(0.0, 0.0); glVertex3f(-windowRatio, 0.0f, 0.0f);
        glTexCoord2f(1.0, 0.0); glVertex3f(0, 0.0f, 0.0f);
        glTexCoord2f(1.0, 1.0); glVertex3f(0, -1.0f, 0.0f);
        glEnd();
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0); glVertex3f(-windowRatio, 0.0f, 0.0f);
        glTexCoord2f(0.0, 0.0); glVertex3f(-windowRatio, 1.0f, 0.0f);
        glTexCoord2f(1.0, 0.0); glVertex3f(0, 1.0f, 0.0f);
        glTexCoord2f(1.0, 1.0); glVertex3f(0, 0.0f, 0.0f);
        glEnd();
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0); glVertex3f(0, 0.0f, 0.0f);
        glTexCoord2f(0.0, 0.0); glVertex3f(0, 1.0f, 0.0f);
        glTexCoord2f(1.0, 0.0); glVertex3f(windowRatio, 1.0f, 0.0f);
        glTexCoord2f(1.0, 1.0); glVertex3f(windowRatio, 0.0f, 0.0f);
        glEnd();
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0); glVertex3f(0, -1.0f, 0.0f);
        glTexCoord2f(0.0, 0.0); glVertex3f(0, 0.0f, 0.0f);
        glTexCoord2f(1.0, 0.0); glVertex3f(windowRatio, 0.0f, 0.0f);
        glTexCoord2f(1.0, 1.0); glVertex3f(windowRatio, -1.0f, 0.0f);
        glEnd();
        */
        

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, screenColorData);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();


        // Process player input
        processInput(window);

        

        

        // Find the frame time
        auto end = time.now();
        using ms = std::chrono::duration<float, std::milli>;
        deltaT = std::chrono::duration_cast<ms>(end - start).count();
    }

    glfwTerminate();
}



void UpdatePhysics(float delta)
{
    
    /*
    if (!editing)
    {
        if (cameraVelocity.y < 3)
            cameraVelocity.y += 0.002 * delta;
        else
            cameraVelocity.y = 3;

        if (spaceKeyPressed)
        {
            cameraVelocity.y = -3;
            cout << "a";
        }
    }*/
    /*
    if (!playerOnFloor)
    {
        if (cameraVelocity.y < 20)
            cameraVelocity.y += 0.2 * delta;
        else
            cameraVelocity.y = 20;
    }
    else
        cameraVelocity.y = 0;
        */

    //if (spaceKeyPressed)
        //cameraVelocity.y = -300;

    //playerOnFloor = false;
    


    Vector3 rotatedVelocity = RotVecY(cameraVelocity, -cameraRotation.y);

    rotatedVelocity.x *= 0.03 * delta;
    rotatedVelocity.y *= 0.03 * delta;
    rotatedVelocity.z *= 0.03 * delta;


    // Move player, then check collision.

    // Move the player to the point of nearest collision.

    // Rotate the movementVelocity to face along the wall normal.

    // Slide player, then check collision.

    // Move the player to the nearest collision.


    //if (canMove)
    Vector3 slideVelocity = TestCollision(rotatedVelocity);
    TestCollision(slideVelocity);

    //if (!(slideVelocity.x == 0 && slideVelocity.y == 0 && slideVelocity.z == 0))
        
    

    //for (int i = 0; i < 1; i++)
    //{
        //loadedMeshInstances[i].rotation.y += 0.001 * delta;
        //loadedMeshInstances[i].rotation.x += 0.001 * delta;
        //loadedMeshInstances[i].rotation.z += 0.001 * delta;
    //}
}



void processInput(GLFWwindow* window)
{
    //if (editing)
    //{
    cameraVelocity.x = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
    cameraVelocity.y = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    cameraVelocity.z = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    //}
    //else
    //{
    //    cameraVelocity.x = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
    //    cameraVelocity.z = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    //}
    


    spaceKeyPressed = false;

    glfwSetKeyCallback(window, key_callback);
    

    glfwGetCursorPos(window, &mousePosX, &mousePosY);


    // Set rotation to wrap

    float mouseS = 1 / (mouseSensitivity);

    if (mousePosY > 1.5 * mouseS)
    {
        mousePosY = 1.5 * mouseS;
    }
    else if (mousePosY < -1.5 * mouseS)
    {
        mousePosY = -1.5 * mouseS;
    }
    if (mousePosX > 6.4 * mouseS)
    {
        mousePosX -= 6.4 * mouseS;
    }
    else if (mousePosX < -6.4 * mouseS)
    {
        mousePosX += 6.4 * mouseS;
    }
    glfwSetCursorPos(window, mousePosX, mousePosY);


    cameraRotation.y = mousePosX * mouseSensitivity;
    cameraRotation.x = mousePosY * mouseSensitivity;


    editingVertex = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        chosenPaintColor = ((float(mousePosY) + 150) / 300) * 64;
        if (chosenPaintColor > 63)
            chosenPaintColor = 63;
    }
}


// Checks for input the moment a key is pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
           glfwTerminate();
        }

        if (key == GLFW_KEY_SPACE)
        {
            spaceKeyPressed = true;
        }

        // This was part of an older idea to have multiple gradients for different lighting. It was implemented earlier. With the 256 color palette, however, it would be easier to make 1 gradient more fleshed out.
        // The first three values set an initial RGB value to brighten every color to make the brightest color in the gradient.
        // The last three set an RGB step size for darkening each color in the gradient.
        // Depending on what colors you choose for the palette and gradient, you could get different colors with smoother or more banded gradients. (instead of the default white to black, yellow to blue creates more outdoor-like lighting)
        //GenerateGradient(128, 128, 128, 6, 6, 6); // Cloudy
        //GenerateGradient(128, 100, 64, 6, 6, 8); // Gold
        //GenerateGradient(128, 120, 80, 7, 6, 5); // Day
        //GenerateGradient(150, 130, 100, 7, 7, 6); // Red
        //GenerateGradient(128, 128, 128, 7, 7, 5); // Blue
        //GenerateGradient(128, 128, 128, 8, 8, 8); // Darker
        //GenerateGradient(100, 120, 128, 6, 6, 6); // Darker

        // In the future, I could make a program that allows you to edit the color palette, see where and how often colors appear in the corresponding gradients, and possibly generate palettes based on colors.
        // This would make creating palettes much faster, and make the resulting gradients could be much smoother.
        // This could also include the feature to generate gradients with different coloring through the method shown above.
        
        if (key == GLFW_KEY_1)
        {
            editing = !editing;
        }

        if (key == GLFW_KEY_2)
        {
            displayFPS = !displayFPS;
        }

        if (key == GLFW_KEY_3)
        {
            displayGradient = !displayGradient;
        }
        /*
        if (key == GLFW_KEY_3)
        {
            editMode++;

            if (editMode > 1)
                editMode = 0;
        }*/

        if (key == GLFW_KEY_KP_9)
            ResizeScreen(960, 600, window);

        if (key == GLFW_KEY_KP_8)
            ResizeScreen(640, 400, window);

        if (key == GLFW_KEY_KP_7)
            ResizeScreen(320, 200, window);

        if (key == GLFW_KEY_KP_6)
            ResizeScreen(240, 160, window);

        if (key == GLFW_KEY_KP_5)
            ResizeScreen(160, 100, window);

        if (key == GLFW_KEY_KP_4)
            ResizeScreen(80, 50, window);

        if (key == GLFW_KEY_KP_3) // Widescreen
            ResizeScreen(800, 400, window);

        if (key == GLFW_KEY_KP_2) // extra cinematic
            ResizeScreen(1000, 400, window);

        if (key == GLFW_KEY_KP_1) // phone
            ResizeScreen(400, 640, window);

        if (key == GLFW_KEY_KP_0) // 4/3 high res
            ResizeScreen(800, 600, window);

        if (key == GLFW_KEY_EQUAL)
        {
            fov -= 0.125;
        }

        if (key == GLFW_KEY_MINUS)
        {
            fov += 0.125;
        }

        if (key == GLFW_KEY_RIGHT_SHIFT)
        {
            for (int i = 0; i < loadedMeshes.size(); i++)
                SaveModel(i);
        }
    }
}


int main(void)
{
    RunEngine();

    return 0;
}