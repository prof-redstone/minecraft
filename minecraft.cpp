
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "render.hpp"

#define sizeChunk 10
using namespace std;

int textureMapWidth = 4;

vector<vector<int>> faceCorrespondence = {
    {0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3},

    {4, 4, 4, 5, 4, 4},
    {6, 6, 6, 6, 6, 6}
};

std::vector<float> addTopFace(float x, float y, float z, float ou, float ov, float s) {
    return {
        x, y, z, ou, ov+s,//facing top
        x, y, z+1, ou + s, ov + s,
        x+1, y, z+1, ou + s, ov,

        x, y, z, ou, ov + s,
        x + 1, y, z+1, ou + s, ov,
        x + 1, y, z, ou, ov
    };
}

std::vector<float> addBottomFace(float x, float y, float z, float ou, float ov, float s) {
    return {
        x + 1, y, z,     ou,     ov + s,
        x + 1, y, z + 1,   ou + s, ov + s,
        x, y, z + 1,     ou + s, ov,

        x + 1, y, z,     ou,     ov + s,
        x, y, z + 1,     ou + s, ov,
        x, y, z,       ou,     ov
    };
}

std::vector<float> addXPosFace(float x, float y, float z, float ou, float ov, float s) {
    return {
        x, y, z, ou, ov + s,
        x, y + 1, z, ou, ov,    
        x, y + 1, z + 1, ou + s, ov,      

        x, y, z,ou,ov + s,
        x, y + 1, z + 1, ou + s, ov, 
        x, y, z + 1, ou + s, ov + s
    };
}

std::vector<float> addXNegFace(float x, float y, float z, float ou, float ov, float s) {
    return {
        x, y,   z + 1,     ou,     ov + s,
        x, y + 1, z + 1,     ou,     ov,
        x, y + 1, z,       ou + s, ov,

        x, y,   z + 1,     ou,     ov + s,
        x, y + 1, z,       ou + s, ov,
        x, y,   z,       ou + s, ov + s
    };
}

std::vector<float> addZPosFace(float x, float y, float z, float ou, float ov, float s) {
    return {
        x,   y,   z,       ou,     ov + s,
        x + 1, y,   z,       ou + s, ov + s,
        x + 1, y + 1, z,       ou + s, ov,

        x,   y,   z,       ou,     ov + s,
        x + 1, y + 1, z,       ou + s, ov,
        x,   y + 1, z,       ou,     ov
    };
}

std::vector<float> addZNegFace(float x, float y, float z, float ou, float ov, float s) {
    return {
        x + 1, y,   z,       ou,     ov + s,
        x,   y,   z,       ou + s, ov + s,
        x,   y + 1, z,       ou + s, ov,

        x + 1, y,   z,       ou,     ov + s,
        x,   y + 1, z,       ou + s, ov,
        x + 1, y + 1, z,       ou,     ov
    };
}



vector<float> getFaceUV(int bloc, int face) {
    if (bloc >= faceCorrespondence.size()) bloc = 0;
    int textureCo = faceCorrespondence[bloc][face];
    vector<float> rez = { (float)(textureCo % textureMapWidth)/textureMapWidth,(float)(textureCo / textureMapWidth) / textureMapWidth,1.0f / textureMapWidth };
    return rez;
}

int main() {
    std::cout << "Minecraft\n";
    SetupRender("Minecraft");

    std::vector<float> worldMesh;
    std::vector<float> transpMesh;

    vector<vector<vector<int>>> chunk(sizeChunk, vector<vector<int>>(sizeChunk, vector<int>(sizeChunk, 0)));
    for (int i = 0; i < sizeChunk; ++i) {
        for (int j = 0; j < sizeChunk; ++j) {
            for (int k = 0; k < sizeChunk; ++k) {
                chunk[i][j][k] = k%5;
                if ((i + j + k) % 2 == 0) {
                    chunk[i][j][k] = -1;
                }
            }
        }
    }
    //chunk[4][4][4] = -1;

    for (int i = 0; i < sizeChunk; ++i) {
        for (int j = 0; j < sizeChunk; ++j) {
            for (int k = 0; k < sizeChunk; ++k) {

                if (chunk[i][j][k] == -1) continue;

                if (i == 0 || chunk[i - 1][j][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 0);
                    vector<float> face = addXNegFace(i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    if (chunk[i][j][k] != 2) {
                        worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                    }else {
                        transpMesh.insert(transpMesh.end(), face.begin(), face.end());
                    }
                }
                if (i == sizeChunk - 1 || chunk[i + 1][j][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 1);
                    vector<float> face = addXPosFace(i+1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    if (chunk[i][j][k] != 2) {
                        worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                    }
                    else {
                        transpMesh.insert(transpMesh.end(), face.begin(), face.end());
                    }
                }
                if (j == 0 || chunk[i][j - 1][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 2);
                    vector<float> face = addBottomFace(i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    if (chunk[i][j][k] != 2) {
                        worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                    }
                    else {
                        transpMesh.insert(transpMesh.end(), face.begin(), face.end());
                    }
                }
                if (j == sizeChunk - 1 || chunk[i][j + 1][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 3);
                    vector<float> face = addTopFace(i, j+1, k, faceUV[0], faceUV[1], faceUV[2]);
                    if (chunk[i][j][k] != 2) {
                        worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                    }
                    else {
                        transpMesh.insert(transpMesh.end(), face.begin(), face.end());
                    }
                }
                if (k == 0 || chunk[i][j][k - 1] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 4);
                    vector<float> face = addZNegFace(i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    if (chunk[i][j][k] != 2) {
                        worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                    }
                    else {
                        transpMesh.insert(transpMesh.end(), face.begin(), face.end());
                    }
                }
                if (k == sizeChunk - 1 || chunk[i][j][k + 1] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 5);
                    vector<float> face = addZPosFace(i, j, k+1, faceUV[0], faceUV[1], faceUV[2]);
                    if (chunk[i][j][k] != 2) {
                        worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                    }
                    else {
                        transpMesh.insert(transpMesh.end(), face.begin(), face.end());
                    }
                }
            }
        }
    }

    Mesh* mesh = setupMeshTexture(worldMesh);
    setMeshTextureFile(mesh, "sources/textures/all.png");
    Mesh* mesh2 = setupMeshTexture(transpMesh);
    setMeshTextureFile(mesh2, "sources/textures/all.png");






    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 1.0);



    while (shouldCloseTheApp()) {
        renderScene();
    }
    terminateRender();
    return 0;
}
