#pragma once

#include "Global.cpp"
#include "UsefulFunctions.cpp"
#include <sstream>
#include <iomanip>


// Loads objects and textures
void LoadAssets();

// Saves a model in the project as a .obj file
void SaveModel(int model);




void SaveModel(int model)
{
    string fileName = "TLModel" + to_string(model) + ".obj";

    ofstream objectData;
    objectData.open(fileName);

    if (objectData.is_open())
    {
        // Write the file header
        objectData << "# LabyrinthTool\n";
        objectData << "# \n";
        objectData << "o TLModel" << to_string(model) << "\n";

        // Create lists of vertices and uv coordinates used in triangles
        vector <int> triVerts;
        vector <int> triUVs;


        // Creat a list of all vertices
        vector <Point> meshPoints;

        for (int i = 0; i < loadedMeshes[model].tris.size(); i++)
        {
            for (int k = 0; k < 3; k++)
            {
                bool addPoint = true;

                Point newPoint = loadedMeshes[model].tris[i].p[k];

                for (int j = 0; j < meshPoints.size(); j++)
                {
                    if (meshPoints[j].coord.x == newPoint.coord.x
                        && meshPoints[j].coord.y == newPoint.coord.y
                        && meshPoints[j].coord.z == newPoint.coord.z)
                    {
                        addPoint = false;
                        triVerts.emplace_back(j + 1);
                        j = 10000000;
                    }
                }

                if (addPoint)
                {
                    meshPoints.emplace_back(newPoint);
                    triVerts.emplace_back(meshPoints.size());
                }
            }
        }

        // Write the list to the file
        for (int i = 0; i < meshPoints.size(); i++)
        {
            objectData << "v " << to_string(meshPoints[i].coord.x);
            objectData << " " << to_string(meshPoints[i].coord.y);
            objectData << " " << to_string(meshPoints[i].coord.z);

            stringstream stream;
            stream << fixed << setprecision(4) << float(64 - meshPoints[i].vertCol) /64;
            string vCol = stream.str();
            
            objectData << " " << vCol << " 0.0000 0.0000\n";
        }

        // Create a list of all uvs
        vector <UV> meshUV;

        for (int i = 0; i < loadedMeshes[model].tris.size(); i++)
        {
            for (int k = 0; k < 3; k++)
            {
                bool addUV = true;

                UV newUV = loadedMeshes[model].tris[i].p[k].uv;

                for (int j = 0; j < meshUV.size(); j++)
                {
                    if (meshUV[j].u == newUV.u && meshUV[j].v == newUV.v)
                    {
                        addUV = false;
                        triUVs.emplace_back(j + 1);
                        j = 10000000;
                    }
                }

                if (addUV)
                {
                    meshUV.emplace_back(newUV);
                    triUVs.emplace_back(meshUV.size());
                }
            }
        }

        // Write the list to the file
        for (int i = 0; i < meshUV.size(); i++)
        {
            objectData << "vt " << to_string(meshUV[i].u);
            objectData << " " << to_string(meshUV[i].v) << "\n";
        }

        objectData << "s 0\n";

        // Write the triangle data to the file
        for (int i = 0; i < loadedMeshes[model].tris.size(); i++)
        {
            objectData << "f " << triVerts[i * 3] << "/" << triUVs[i * 3] << " "
                << triVerts[(i * 3) + 1] << "/" << triUVs[(i * 3) + 1] << " "
                << triVerts[(i * 3) + 2] << "/" << triUVs[(i * 3) + 2] << "\n";
        }


        objectData.close();
    }
}

                                                                       



void LoadAssets()
{
    // .lttex file format
    // 
    // COLOR PALETTE
    //      -256 colors, 3 bytes per color
    // 
    // COLOR GRADIENT
    //      -16384 colors, 1 byte per color
    //
    // TEXTURE DATA (for each texture
    //      -Boolean flags:
    //          -1 byte
    //              -Scrolling, additive, no cull
    //      -Texture width:
    //          -2 bytes
    //      -Texture height:
    //          -2 bytes
    //      -Pixel data:
    //          -1 byte per pixel (corresponding with a color in the color palette)


    ifstream texFile("CombinedTextures.lttex", ios::binary | ios::in);

    if (texFile.is_open()) {
        
        texFile.seekg(0, ios::end);
        int fileLength = texFile.tellg();
        texFile.seekg(0, ios::beg);

        texFile.read((char*)&lookupTexture, 768); // Read in the lookup texture
        
        texFile.read((char*)&loadedGradient, 16384); // Read in the gradient

        
        while (texFile.tellg() < fileLength)
        {
            
            Texture addTex;

            uint8_t flags = 0;

            texFile.read((char*)&flags, 1);

            // Boolean flags: Scrolling, additive, no cull


            bool noCull = flags >> 2;
            if (flags > 2)
                flags -= 4;
            bool add = flags >> 1;
            if (flags > 1)
                flags -= 2;
            bool scroll = flags;

            addTex.scroll = scroll;
            addTex.additive = add;
            addTex.noCull = noCull;

            uint8_t texWidth1;
            uint8_t texWidth2;

            uint8_t texHeight1;
            uint8_t texHeight2;

            texFile.read((char*)&texWidth1, 1);
            texFile.read((char*)&texWidth2, 1);

            texFile.read((char*)&texHeight1, 1);
            texFile.read((char*)&texHeight2, 1);

            addTex.textureWidth = int(texWidth2) + (int(texWidth1) * 256);
            addTex.textureHeight = int(texHeight2) + (int(texHeight1) * 256);
            

            for (int i = 0; i < addTex.textureWidth * addTex.textureHeight; i++)
            {
                uint8_t col = 0;

                texFile.read((char*)&col, 1);
                
                addTex.px.emplace_back(col);
            }

            loadedTextures.emplace_back(addTex);
        }
    }


    texFile.close();


    bool fileExists = true;
    string fileName;

    int m = 0;


    // Read .obj files

    fileExists = true;
    fileName;

    m = 0;

    while (fileExists)
    {
        fileName = "TLModel" + to_string(m) + ".obj";

        ifstream objectData;
        objectData.open(fileName);



        if (objectData.is_open())
        {
            Mesh newMesh;


            string line;

            vector <Vector3> vertexData;
            vector <uint8_t> colorData;
            vector <UV> uvData;

            for (int i = 0; i < 4; i++) getline(objectData, line); // The first three lines are not used

            while (line[0] == 'v' && line[1] != 't') // Read in the vertex data
            {
                Vector3 pos;
                int index = 2;
                string num = "";

                while (line[index] != ' ') { num += line[index]; index++; }
                pos.x = stof(num);
                num = "";
                index++;
                while (line[index] != ' ') { num += line[index]; index++; }
                pos.y = stof(num);
                num = "";
                index++;
                while (line[index] != ' ') { num += line[index]; index++; }
                pos.z = stof(num);
                num = "";
                index++;

                vertexData.emplace_back(pos);

                while (line[index] != ' ') { num += line[index]; index++; }

                colorData.emplace_back(64 - stof(num) * 64);

                getline(objectData, line);
            }
            while (line[0] == 'v') // Read in the uv data
            {
                UV uv;
                int index = 3;
                string num = "";

                while (line[index] != ' ') { num += line[index]; index++; }
                uv.u = stof(num);
                num = "";
                index++;
                while (index < line.length()) { num += line[index]; index++; }
                uv.v = stof(num);

                uvData.emplace_back(uv);

                getline(objectData, line);
            }


            while (getline(objectData, line) && (line[0] == 's' || line[0] == 'f')) // Read in the triangles
            {
                if (line[0] != 's')
                {
                    Triangle newTri;
                    int index = 2;
                    string num = "";
                    int pos;

                    while (line[index] != '/') { num += line[index]; index++; }
                    pos = stoi(num) - 1;
                    num = "";
                    index++;
                    newTri.p[0].coord = vertexData[pos];
                    newTri.p[0].vertCol = colorData[pos];
                    while (line[index] != ' ') { num += line[index]; index++; }
                    pos = stoi(num) - 1;
                    num = "";
                    index++;
                    newTri.p[0].uv = uvData[pos];

                    while (line[index] != '/') { num += line[index]; index++; }
                    pos = stoi(num) - 1;
                    num = "";
                    index++;
                    newTri.p[1].coord = vertexData[pos];
                    newTri.p[1].vertCol = colorData[pos];
                    while (line[index] != ' ') { num += line[index]; index++; }
                    pos = stoi(num) - 1;
                    num = "";
                    index++;
                    newTri.p[1].uv = uvData[pos];

                    while (line[index] != '/') { num += line[index]; index++; }
                    pos = stoi(num) - 1;
                    num = "";
                    index++;
                    newTri.p[2].coord = vertexData[pos];
                    newTri.p[2].vertCol = colorData[pos];
                    while (index < line.length()) { num += line[index]; index++; }
                    pos = stoi(num) - 1;
                    num = "";
                    index++;
                    newTri.p[2].uv = uvData[pos];


                    if (newTri.p[0].vertCol == 64)
                        newTri.p[0].vertCol = 63;
                    if (newTri.p[1].vertCol == 64)
                        newTri.p[1].vertCol = 63;
                    if (newTri.p[2].vertCol == 64)
                        newTri.p[2].vertCol = 63;


                    newTri.texture = AssignTriTexture(newTri);

                    newMesh.tris.emplace_back(newTri);
                }
            }

            loadedMeshes.emplace_back(newMesh);


            objectData.close();
            m++;
        }
        else fileExists = false;
    }
    


    // Load mesh instances
    // Currently, there is no system for saving and loading instances, so they are hard-coded here for the time being.
    
    // Test area model
    MeshInstance newInstance0;
    newInstance0.instanceMesh = loadedMeshes[0];
    newInstance0.meshNumber = 0;

    newInstance0.position = { 0, 0, 0 };


    loadedMeshInstances.emplace_back(newInstance0);  
}