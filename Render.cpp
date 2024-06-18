#pragma once


#include "Global.cpp"


// Clips and projects a triangle, then sends to to be drawn
void Clip(Triangle tri, bool transparent);

// Draws a triangle
void DrawTriangle(Triangle tri, bool transparent);

// Draws text
void DrawText(string text, int textX, int textY);

// Draws a billboarded sprite given a point in screen space, a texture, and a size
void DrawBillboard(Vector3 pos, uint8_t tex, float scale);



void Clip(Triangle tri, bool transparent)
{
    // Multiplies the coordinates to match screen space

    tri.p[0].coord.x *= screenRatio;
    tri.p[1].coord.x *= screenRatio;
    tri.p[2].coord.x *= screenRatio;

    tri.p[0].coord.x *= fov;
    tri.p[0].coord.y *= fov;
    tri.p[1].coord.x *= fov;
    tri.p[1].coord.y *= fov;
    tri.p[2].coord.x *= fov;
    tri.p[2].coord.y *= fov;


    // Clip near triangles
    vector <Triangle> newTris1; // Create and the new triangles generated from clipping
    vector <Point> newPoints1; // Clipped points from which new triangles will be constructed

    for (int p1 = 0; p1 < 3; p1++)
    {
        // If the point is inside screen, include it in newPoints.
        if (tri.p[p1].coord.z >= cameraNear)
            newPoints1.emplace_back(tri.p[p1]);

        int p2 = p1 + 1;
        if (p2 > 2)
            p2 = 0;

        Point point1 = tri.p[p1];
        Point point2 = tri.p[p2];

        if (point1.coord.z > point2.coord.z) // p1 has the smaller y.
            swap(point1, point2);

        // Z intersection
        if (point1.coord.z < cameraNear && point2.coord.z > cameraNear) // Line has points on either side of edge
        {
            float a = -(point1.coord.z - cameraNear) / ((point2.coord.z - cameraNear) - (point1.coord.z - cameraNear));

            Point newP;

            newP.coord.x = point2.coord.x * a + point1.coord.x * (1 - a);
            newP.coord.y = point2.coord.y * a + point1.coord.y * (1 - a);
            newP.coord.z = cameraNear;

            newP.uv.u = point2.uv.u * a + point1.uv.u * (1 - a);
            newP.uv.v = point2.uv.v * a + point1.uv.v * (1 - a);

            newP.vertCol = point2.vertCol * a + point1.vertCol * (1 - a);

            

            newPoints1.emplace_back(newP);
        }
    }

    for (int i = 0; i < newPoints1.size(); i++)
    {
        newPoints1[i].coord.x = newPoints1[i].coord.x / newPoints1[i].coord.z + 0.5;
        newPoints1[i].coord.y = -newPoints1[i].coord.y / newPoints1[i].coord.z + 0.5;
    }

    if (newPoints1.size() > 2)
    {
        for (int i = 0; i < newPoints1.size() - 2; i++)
        {
            if (!((newPoints1[0].coord.x < 0 && newPoints1[i + 1].coord.x < 0 && newPoints1[i + 2].coord.x < 0) ||
                (newPoints1[0].coord.x > 1 && newPoints1[i + 1].coord.x > 1 && newPoints1[i + 2].coord.x > 1) ||
                (newPoints1[0].coord.y < 0 && newPoints1[i + 1].coord.y < 0 && newPoints1[i + 2].coord.y < 0) ||
                (newPoints1[0].coord.y > 1 && newPoints1[i + 1].coord.y > 1 && newPoints1[i + 2].coord.y > 1)))
            {
                Triangle newTri = { newPoints1[0], newPoints1[i + 1], newPoints1[i + 2] };
                newTri.texture = tri.texture; // Give the new triangle the original triangle's texture

                newTris1.emplace_back(newTri);
            }
        }
    }
    else
        return; // Draw nothing


    for (int i = 0; i < newTris1.size(); i++)
    {

        DrawTriangle(newTris1[i], transparent);
    }
}







void DrawTriangle(Triangle tri, bool transparent)
{
    // The three points for the triangle
    Point p1 = tri.p[0];
    Point p2 = tri.p[1];
    Point p3 = tri.p[2];


    // Project the points into 2d space
    p1.coord.x *= screenWidth;
    p1.coord.y *= screenHeight;
    p2.coord.x *= screenWidth;
    p2.coord.y *= screenHeight;
    p3.coord.x *= screenWidth;
    p3.coord.y *= screenHeight;


    // Sort the points. The topmost point is first, then middle, then bottommost.
    if (p3.coord.y < p1.coord.y)
        swap(p3, p1);
    if (p3.coord.y < p2.coord.y)
        swap(p3, p2);
    if (p2.coord.y < p1.coord.y)
        swap(p2, p1);

    float yStart = p1.coord.y;
    float yEnd = p3.coord.y;

    if (p3.coord.x < p1.coord.x)
        swap(p3, p1);
    if (p3.coord.x < p2.coord.x)
        swap(p3, p2);
    if (p2.coord.x < p1.coord.x)
        swap(p2, p1);

    float xStart = p1.coord.x;
    float xEnd = p3.coord.x;

    if (xStart < 0)
        xStart = 0;
    else if (xStart >= screenWidth)
        return;

    if (xEnd < 0)
        return;
    else if (xEnd >= screenWidth)
        xEnd = screenWidth - 1;


    if (yStart < 0)
        yStart = 0;
    else if (yStart >= screenHeight)
        return;

    if (yEnd < 0)
        return;
    else if (yEnd >= screenHeight)
        yEnd =  screenHeight;



    // Draw the pixels along the scanline from top to middle of the triangle.
    // Change the start and end by the left and right slopes.
    for (int i = yStart; i < yEnd; i++)
    {
        for (int j = xStart; j <= xEnd; j++)
        {
            ////////////////////////////////////////////////////////////////////////////////////////////////// PERSPECTIVE CORRECTION
            // Move the points into a normalized space to make multiplying them for depth simpler.

            Point a1;
            Point a2;
            Point a3;

            a1.coord.x = (p1.coord.x - j) * p1.coord.z;
            a1.coord.y = (p1.coord.y - i) * p1.coord.z;
            a2.coord.x = (p2.coord.x - j) * p2.coord.z;
            a2.coord.y = (p2.coord.y - i) * p2.coord.z;
            a3.coord.x = (p3.coord.x - j) * p3.coord.z;
            a3.coord.y = (p3.coord.y - i) * p3.coord.z;


            // vertex weights must be more the further they are from the camera.
            // This means that they pull the colors and textures towards themselves.
            // This is done by making them closer to the center (0, 0).
            // These modified points are only used to find the interpolated texture coordinates, the screen space ones are used in deciding where to place the pixel.

            float denominator = ((a2.coord.y - a3.coord.y) * (a1.coord.x - a3.coord.x) + (a3.coord.x - a2.coord.x) * (a1.coord.y - a3.coord.y));
            denominator = 1 / denominator;

            float p1Weight = ((a2.coord.y - a3.coord.y) * -a3.coord.x + (a3.coord.x - a2.coord.x) * -a3.coord.y) * denominator;
            float p2Weight = ((a3.coord.y - a1.coord.y) * -a3.coord.x + (a1.coord.x - a3.coord.x) * -a3.coord.y) * denominator;
            float p3Weight = 1 - p1Weight - p2Weight;

            if (p1Weight >= 0 && p2Weight >= 0 && p3Weight >= 0)
            {
                float depth = (p1.coord.z * p1Weight) + (p2.coord.z * p2Weight) + (p3.coord.z * p3Weight);


                if (depth < depthBuffer[(i * screenWidth) + j])
                {
                    

                    float weightedU = ((p1.uv.u * p1Weight) + (p2.uv.u * p2Weight) + (p3.uv.u * p3Weight));
                    float weightedV = ((p1.uv.v * p1Weight) + (p2.uv.v * p2Weight) + (p3.uv.v * p3Weight));


                    // Add scrolling if using a scrolling texture
                    if (loadedTextures[tri.texture].scroll)
                        weightedV += globalAnimationClock / 6.283;

                    uint8_t colWeight = (p1.vertCol * p1Weight) + (p2.vertCol * p2Weight) + (p3.vertCol * p3Weight);



                    // Wrap the texture
                    while (weightedU >= 1)
                        weightedU -= 1;
                    while (weightedU < 0)
                        weightedU += 1;
                    while (weightedV >= 1)
                        weightedV -= 1;
                    while (weightedV < 0)
                        weightedV += 1;


                    weightedU *= loadedTextures[tri.texture].textureWidth;
                    weightedV *= loadedTextures[tri.texture].textureHeight;

                    
                    uint8_t vertexWeightedCol = loadedTextures[tri.texture].px[int(weightedU) + int(weightedV) * loadedTextures[tri.texture].textureWidth];


                    if (vertexWeightedCol != 255) // Don't draw transparent pixels
                    {
                        if (loadedTextures[tri.texture].additive)
                            vertexWeightedCol = loadedGradient.grad[paletteScreenData[i * screenWidth + j] * 64 + (vertexWeightedCol) + colWeight];
                        else
                            vertexWeightedCol = loadedGradient.grad[(vertexWeightedCol * 64) + colWeight];

                        
                        // Water reflection filter (makes it look like there is water reflecting on surfaces with a real stretch of the imagination)
                        // It may be implemented later.
                        //uint8_t waterCol = loadedTextures[1].px[int(waterU) + int(waterV) * 64];
                        //vertexWeightedCol = loadedGradient.grad[vertexWeightedCol * 64 + int(waterCol * 0.5) + 22];

                        // Draw the pixel to the screen buffer and depth buffer
                        paletteScreenData[(i * screenWidth) + j] = vertexWeightedCol;

                        if (!loadedTextures[tri.texture].additive)
                            depthBuffer[(i * screenWidth) + j] = depth;
                    }

                    
                }
            }
        }
    }

    // If the user is editing, points will appear at every vertex to show where they are.
    if (editing)
    {
        if (p1.coord.y < screenHeight && p1.coord.y >= 0 && p1.coord.x < screenWidth && p1.coord.x >= 0)
        {
            DrawBillboard(p1.coord, 10, 0.2);
        }
        if (p2.coord.y < screenHeight && p2.coord.y >= 0 && p2.coord.x < screenWidth && p2.coord.x >= 0)
        {
            DrawBillboard(p2.coord, 10, 0.2);
        }
        if (p3.coord.y < screenHeight && p3.coord.y >= 0 && p3.coord.x < screenWidth && p3.coord.x >= 0)
        {
            DrawBillboard(p3.coord, 10, 0.2);
        }
    }
}



void DrawText(string text, int textX, int textY)
{

    int i = 0; // Text being drawn

    while (i < text.length())
    {
        if (textY + 16 >= screenHeight)
            return;

        int tex = 42;
        if (text.at(i) > 47 && text.at(i) < 58)
            tex = text.at(i) - 48 + 26;

        if (text.at(i) > 64 && text.at(i) < 91)
            tex = text.at(i) - 65;


        for (int y = 0; y < 16; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                uint8_t col;

                col = loadedTextures[8].px[x + 16 * (tex % 8) + 128 * (15 - y) + 2048 * (7 - int(tex / 8))];

                paletteScreenData[textX + x + screenWidth * (y + textY)] = col;
            }
        }

        textX += 16;

        if (textX > screenWidth)
        {
            textX = 0;
            textY += 16;
        }

        i++;
    }
}



void DrawBillboard(Vector3 pos, uint8_t tex, float scale)
{
    if (pos.z > cameraNear)
    {
        scale *= (fov * screenWidth * screenRatio) / pos.z;


        for (int y = -scale; y < scale; y++)
        {
            for (int x = -scale; x < scale; x++)
            {
                int texX = (float(x) / (scale * 2) + 0.5) * loadedTextures[tex].textureWidth;
                int texY = (float(-y) / (scale * 2) + 0.5) * loadedTextures[tex].textureHeight;

                uint8_t col = loadedTextures[tex].px[texX + texY * loadedTextures[tex].textureWidth];

                if (col != 255)
                {
                    if (x + pos.x >= 0 && x + pos.x < screenWidth && y + pos.y >= 0 && y + pos.y < screenHeight)
                    {

                        if (depthBuffer[x + int(pos.x) + int(y + pos.y) * screenWidth] > pos.z - 1)
                        {
                            paletteScreenData[x + int(pos.x) + int(y + pos.y) * screenWidth] = col;
                            depthBuffer[x + int(pos.x) + int(y + pos.y) * screenWidth] = pos.z;
                        }
                    }
                }
            }
        }
    }
}