
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "render.hpp"

#define sizeChunk 100
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

void addTopFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x, y, z, ou, ov + s,
        x, y, z + 1, ou + s, ov + s,
        x + 1, y, z + 1, ou + s, ov,
        x, y, z, ou, ov + s,
        x + 1, y, z + 1, ou + s, ov,
        x + 1, y, z, ou, ov
        });
}

void addBottomFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x + 1, y, z, ou, ov + s,
        x + 1, y, z + 1, ou + s, ov + s,
        x, y, z + 1, ou + s, ov,
        x + 1, y, z, ou, ov + s,
        x, y, z + 1, ou + s, ov,
        x, y, z, ou, ov
        });
}

void addXPosFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x, y, z, ou, ov + s,
        x, y + 1, z, ou, ov,
        x, y + 1, z + 1, ou + s, ov,
        x, y, z, ou, ov + s,
        x, y + 1, z + 1, ou + s, ov,
        x, y, z + 1, ou + s, ov + s
        });
}

void addXNegFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x, y, z + 1, ou, ov + s,
        x, y + 1, z + 1, ou, ov,
        x, y + 1, z, ou + s, ov,
        x, y, z + 1, ou, ov + s,
        x, y + 1, z, ou + s, ov,
        x, y, z, ou + s, ov + s
        });
}

void addZPosFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x, y, z, ou, ov + s,
        x + 1, y, z, ou + s, ov + s,
        x + 1, y + 1, z, ou + s, ov,
        x, y, z, ou, ov + s,
        x + 1, y + 1, z, ou + s, ov,
        x, y + 1, z, ou, ov
        });
}

void addZNegFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x + 1, y, z, ou, ov + s,
        x, y, z, ou + s, ov + s,
        x, y + 1, z, ou + s, ov,
        x + 1, y, z, ou, ov + s,
        x, y + 1, z, ou + s, ov,
        x + 1, y + 1, z, ou, ov
        });
}

vector<float> getFaceUV(int bloc, int face) {
    if (bloc >= faceCorrespondence.size()) bloc = 0;
    int textureCo = faceCorrespondence[bloc][face];
    vector<float> rez = { (float)(textureCo % textureMapWidth)/textureMapWidth,(float)(textureCo / textureMapWidth) / textureMapWidth,1.0f / textureMapWidth };
    return rez;
}

int main() {
    std::cout << "1\n";
    SetupRender("Minecraft");

    std::vector<float> worldMesh;
    std::vector<float> transpMesh;
    worldMesh.reserve(sizeChunk * sizeChunk * sizeChunk * 6 * 30); // estimation
    transpMesh.reserve(sizeChunk * sizeChunk * sizeChunk * 30);

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
                    if (chunk[i][j][k] != 2) {
                        addXNegFace(worldMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }else {
                        addXNegFace(transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (i == sizeChunk - 1 || chunk[i + 1][j][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 1);
                    if (chunk[i][j][k] != 2) {
                        addXPosFace(worldMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addXPosFace(transpMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (j == 0 || chunk[i][j - 1][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 2);
                    if (chunk[i][j][k] != 2) {
                        addBottomFace(worldMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addBottomFace(transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (j == sizeChunk - 1 || chunk[i][j + 1][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 3);
                    if (chunk[i][j][k] != 2) {
                        addTopFace(worldMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addTopFace(transpMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (k == 0 || chunk[i][j][k - 1] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 4);
                    if (chunk[i][j][k] != 2) {
                        addZNegFace(worldMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addZNegFace(transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (k == sizeChunk - 1 || chunk[i][j][k + 1] == -1) {
                    vector<float> faceUV = getFaceUV(chunk[i][j][k], 5);
                    if (chunk[i][j][k] != 2) {
                        addZPosFace(worldMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addZPosFace(transpMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
            }
        }
    }
    cout << 2 << endl;
    Mesh* mesh = setupMeshTexture(worldMesh);
    setMeshTextureFile(mesh, "sources/textures/all.png");
    Mesh* mesh2 = setupMeshTexture(transpMesh);
    setMeshTextureFile(mesh2, "sources/textures/all.png");
    cout << 3 << endl;






    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 1.0);



    while (shouldCloseTheApp()) {
        renderScene();
    }
    terminateRender();
    return 0;
}
