
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "render.hpp"

#define sizeChunk 5
using namespace std;

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

vector<float> getTexture(unsigned int x) {
    if (x == 0) {
        return { 0.0,0.0 };
    }if (x == 1) {
        return { 0.25,0.0 };
    }if (x == 2) {
        return { 0.5,0.0 };
    }if (x == 3) {
        return { 0.75,0.0 };
    }if (x == 4) {
        return { 0.0,0.25 };
    }if (x == 6) {
        return { 0.25,0.25 };
    }
}

vector<bool> placeFacesQ(vector<vector<vector<int>>>& tab, int x, int y, int z) {
    vector<bool> rez = { false, false, false, false, false, false };
    if (tab[x][y][z] == -1) return rez;
    if (x == 0 || tab[x - 1][y][z] == -1) rez[0] = true;
    if (x == sizeChunk - 1 || tab[x + 1][y][z] == -1) rez[1] = true;

    if (y == 0 || tab[x][y - 1][z] == -1) rez[2] = true;
    if (y == sizeChunk - 1 || tab[x][y + 1][z] == -1) rez[3] = true;

    if (z == 0 || tab[x][y][z - 1] == -1) rez[4] = true;
    if (z == sizeChunk - 1 || tab[x][y][z + 1] == -1) rez[5] = true;

    return rez;
}

int main() {
    std::cout << "Minecraft\n";
    SetupRender("Minecraft");

    std::vector<float> worldMesh;

    vector<vector<vector<int>>> chunk(sizeChunk, vector<vector<int>>(sizeChunk, vector<int>(sizeChunk, 0)));
    for (int i = 0; i < sizeChunk; ++i) {
        for (int j = 0; j < sizeChunk; ++j) {
            for (int k = 0; k < sizeChunk; ++k) {
                chunk[i][j].push_back(static_cast<int>(k));
            }
        }
    }
    chunk[4][4][4] = -1;
    for (int i = 0; i < sizeChunk; ++i) {
        for (int j = 0; j < sizeChunk; ++j) {
            for (int k = 0; k < sizeChunk; ++k) {
                vector<float> uv = getTexture(k);
                vector<bool> rez = placeFacesQ(chunk, i,j,k);
                std::vector<float> face;
                if (rez[2]) {
                    face = addBottomFace(i, j, k, uv[0], uv[1], 0.25);
                    worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                }
                if (rez[3]) {
                    face = addTopFace(i, j+1, k, uv[0], uv[1], 0.25);
                    worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                }
                if (rez[1]) {
                    face = addXPosFace(i+1, j, k, uv[0], uv[1], 0.25);
                    worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                }
                if (rez[0]) {
                    face = addXNegFace(i, j, k, uv[0], uv[1], 0.25);
                    worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                }
                if (rez[5]) {
                    face = addZPosFace(i, j, k+1, uv[0], uv[1], 0.25);
                    worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                }
                if (rez[4]) {
                    face = addZNegFace(i, j, k, uv[0], uv[1], 0.25);
                    worldMesh.insert(worldMesh.end(), face.begin(), face.end());
                }
            }
        }
    }

    Mesh* mesh = setupMeshTexture(worldMesh);
    setMeshTextureFile(mesh, "sources/textures/all.png");






    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 1.0);


    while (shouldCloseTheApp()) {
        renderScene();
    }
    terminateRender();
    return 0;
}
