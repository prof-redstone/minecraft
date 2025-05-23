#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"
#include "camera.h"
#include "render.hpp"

#define CHUNKWIDTH 20
#define CHUNKHEIGHT 100
#define RENDER_DISTANCE 10
int maxChunksPerFrame = 20;
using namespace std;

int textureMapWidth = 4;
Camera camera;

#define air -1
#define leaves -2
#define glass -3
#define stone 0
#define planks 1
#define dirt 2
#define grass 3
#define diamond 4



const vector<vector<int>> faceCorrespondence = {
    {0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3},

    {4, 4, 4, 5, 4, 4},
    {6, 6, 6, 6, 6, 6},
    {7, 7, 7, 7, 7, 7}
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

vector<float> getFaceUV(int block, int face) {
    int textureCo = 0;
    if(block == stone){ textureCo = std::vector<int>{ 0, 0, 0, 0, 0, 0 }[face];}
    else if (block == planks) { textureCo = std::vector<int>{ 1, 1, 1, 1, 1, 1 }[face]; }
    else if (block == dirt) { textureCo = std::vector<int>{ 3, 3, 3, 3, 3, 3 }[face]; }
    else if (block == grass) { textureCo = std::vector<int>{ 4, 4, 4, 5, 4, 4 }[face]; }
    else if (block == leaves) { textureCo = std::vector<int>{ 7, 7, 7, 7, 7, 7 }[face]; }

    vector<float> rez = { (float)(textureCo % textureMapWidth)/textureMapWidth,(float)(textureCo / textureMapWidth) / textureMapWidth,1.0f / textureMapWidth };
    return rez;
}

typedef struct ChunkKey {
    int x;
    int y;

    bool operator==(const ChunkKey& other) const {
        return x == other.x && y == other.y;
    }
} ChunkKey;

namespace std {
    template<>
    struct hash<ChunkKey> {
        std::size_t operator()(const ChunkKey& key) const {
            return hash<int>()(key.x) ^ (hash<int>()(key.y) << 1);
        }
    };
}

typedef struct chunk {
    ChunkKey key;
    vector<vector<vector<signed char>>> blocks;
    std::vector<float> opaqueMesh;
    std::vector<float> transpMesh;
    Mesh* opaqueMeshObj = nullptr;
    Mesh* transpMeshObj = nullptr;
    bool isActive = false;
    bool decorated = false;
} Chunk;

std::unordered_map<ChunkKey, Chunk> chunks;
std::vector<ChunkKey> chunksToLoadQueue;
std::vector<ChunkKey> chunksToUnloadQueue;

uint32_t hash2D(int x, int y, int seed = 0) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u
        + static_cast<uint32_t>(y) * 668265263u
        + static_cast<uint32_t>(seed) * 982451653u; // grands nombres premiers
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= (h >> 16);
    return h;
}

bool inChunk(int i, int j, int k) {
    return 0 <= i && i < CHUNKWIDTH && 0 <= j && j < CHUNKHEIGHT && 0 <= k && k < CHUNKWIDTH;
}


void initChunk(chunk& chunk, int x, int y) {
    ChunkKey key;
    key.x = x;
    key.y = y;
    chunk.key = key;
    chunk.decorated = false;
    chunk.blocks.resize(CHUNKWIDTH, vector<vector<signed char>>(CHUNKHEIGHT, vector<signed char>(CHUNKWIDTH, -1)));

    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {
            for (int j = 0; j < CHUNKHEIGHT; ++j) {
                double h = db::perlin((double)(i + key.x * CHUNKWIDTH) / 86.0, (double)(k + key.y * CHUNKWIDTH) / 86.0, (double)(j) / 86.0);
                h += db::perlin((double)(i + key.x * CHUNKWIDTH) / 8.0, (double)(k + key.y * CHUNKWIDTH) / 8.0) * 0.08;
                if ((h + 1.0)*0.5 > (double)j / CHUNKHEIGHT) {
                    chunk.blocks[i][j][k] = stone;
                }
                else {
                    chunk.blocks[i][j][k] = air;
                }
            }
        }
    }
    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {

            for (int j = 0; j < CHUNKHEIGHT-2; ++j) {
                if (chunk.blocks[i][j][k] != air && chunk.blocks[i][j + 1][k] == air) {
                    chunk.blocks[i][j][k] = grass;
                }else if(chunk.blocks[i][j][k] != air && chunk.blocks[i][j + 2][k] == air) {
                    chunk.blocks[i][j][k] = dirt;
                }
            }

        }
    }

    //tree
    for (int i = -3; i < CHUNKWIDTH+3; ++i) {
        for (int k = -3; k < CHUNKWIDTH+3; ++k) {
            if (hash2D(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, 0) % 100 < 1) {
                int h = 0;
                for (int j = CHUNKHEIGHT - 2; j > 0; --j) {
                    double t = db::perlin((double)(i + key.x * CHUNKWIDTH) / 86.0, (double)(k + key.y * CHUNKWIDTH) / 86.0, (double)(j) / 86.0);
                    t += db::perlin((double)(i + key.x * CHUNKWIDTH) / 8.0, (double)(k + key.y * CHUNKWIDTH) / 8.0) * 0.08;
                    if ((t + 1.0) * 0.5 > (double)j / CHUNKHEIGHT) {
                        h = j;
                        break;
                    }
                }
                if (inChunk(i, h + 1, k)) chunk.blocks[i][h + 1][k] = planks;
                if (inChunk(i, h + 2, k)) chunk.blocks[i][h + 2][k] = planks;
                if (inChunk(i, h + 3, k)) chunk.blocks[i][h + 3][k] = planks;
                if (inChunk(i, h + 4, k)) chunk.blocks[i][h + 4][k] = planks;

                if (inChunk(i + 1, h + 3, k + 0)) chunk.blocks[i + 1][h + 3][k + 0] = leaves;
                if (inChunk(i + 1, h + 3, k + 1)) chunk.blocks[i + 1][h + 3][k + 1] = leaves;
                if (inChunk(i + 1, h + 3, k - 1)) chunk.blocks[i + 1][h + 3][k - 1] = leaves;
                if (inChunk(i + 0, h + 3, k + 1)) chunk.blocks[i + 0][h + 3][k + 1] = leaves;
                if (inChunk(i + 0, h + 3, k - 1)) chunk.blocks[i + 0][h + 3][k - 1] = leaves;
                if (inChunk(i - 1, h + 3, k + 1)) chunk.blocks[i - 1][h + 3][k + 1] = leaves;
                if (inChunk(i - 1, h + 3, k + 0)) chunk.blocks[i - 1][h + 3][k + 0] = leaves;
                if (inChunk(i - 1, h + 3, k - 1)) chunk.blocks[i - 1][h + 3][k - 1] = leaves;

                if (inChunk(i + 1, h + 4, k + 0)) chunk.blocks[i + 1][h + 4][k + 0] = leaves;
                if (inChunk(i + 1, h + 4, k + 1)) chunk.blocks[i + 1][h + 4][k + 1] = leaves;
                if (inChunk(i + 1, h + 4, k - 1)) chunk.blocks[i + 1][h + 4][k - 1] = leaves;
                if (inChunk(i + 0, h + 4, k + 1)) chunk.blocks[i + 0][h + 4][k + 1] = leaves;
                if (inChunk(i + 0, h + 4, k - 1)) chunk.blocks[i + 0][h + 4][k - 1] = leaves;
                if (inChunk(i - 1, h + 4, k + 1)) chunk.blocks[i - 1][h + 4][k + 1] = leaves;
                if (inChunk(i - 1, h + 4, k + 0)) chunk.blocks[i - 1][h + 4][k + 0] = leaves;
                if (inChunk(i - 1, h + 4, k - 1)) chunk.blocks[i - 1][h + 4][k - 1] = leaves;
                if (inChunk(i + 0, h + 5, k + 0)) chunk.blocks[i + 0][h + 5][k + 0] = leaves;
            }
        
        }
    }
}

void updateMesh(chunk& chunk) {
    chunk.opaqueMesh.clear();

    ChunkKey westKey = { chunk.key.x - 1, chunk.key.y };
    ChunkKey eastKey = { chunk.key.x + 1, chunk.key.y };
    ChunkKey northKey = { chunk.key.x, chunk.key.y - 1 };
    ChunkKey southKey = { chunk.key.x, chunk.key.y + 1 };

    bool hasWestChunk = false;
    bool hasEastChunk = false;
    bool hasNorthChunk = false;
    bool hasSouthChunk = false;

    Chunk* westChunk = nullptr;
    Chunk* eastChunk = nullptr;
    Chunk* northChunk = nullptr;
    Chunk* southChunk = nullptr;

    auto westIt = chunks.find(westKey);
    if (westIt != chunks.end()) {
        hasWestChunk = true;
        westChunk = &(westIt->second);
    }

    auto eastIt = chunks.find(eastKey);
    if (eastIt != chunks.end()) {
        hasEastChunk = true;
        eastChunk = &(eastIt->second);
    }

    auto northIt = chunks.find(northKey);
    if (northIt != chunks.end()) {
        hasNorthChunk = true;
        northChunk = &(northIt->second);
    }

    auto southIt = chunks.find(southKey);
    if (southIt != chunks.end()) {
        hasSouthChunk = true;
        southChunk = &(southIt->second);
    }

    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int j = 0; j < CHUNKHEIGHT; ++j) {
            for (int k = 0; k < CHUNKWIDTH; ++k) {
                if (chunk.blocks[i][j][k] == air) continue;

                if (i == 0) {
                    if (hasWestChunk && westChunk->blocks[CHUNKWIDTH - 1][j][k] <0) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
                        if (chunk.blocks[i][j][k] >= 0) {
                            addXNegFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else if (!hasWestChunk || westChunk->blocks[CHUNKWIDTH - 1][j][k] == air) {
                            addXNegFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
                else if (chunk.blocks[i - 1][j][k] <0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addXNegFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else if(chunk.blocks[i - 1][j][k] == air) {
                        addXNegFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }

                if (i == CHUNKWIDTH - 1 ) {
                    if (hasEastChunk && eastChunk->blocks[0][j][k] <0) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 1);
                        if (chunk.blocks[i][j][k] >= 0) {
                            addXPosFace(chunk.opaqueMesh, i + 1 + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else if (!hasEastChunk || eastChunk->blocks[0][j][k] == air) {
                            addXPosFace(chunk.transpMesh, i + 1 + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
                else if (chunk.blocks[i + 1][j][k] <0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 1);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addXPosFace(chunk.opaqueMesh, i + 1 + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else if (chunk.blocks[i + 1][j][k] == air) {
                        addXPosFace(chunk.transpMesh, i + 1 + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }

                if (j == 0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 2);
                    if (chunk.blocks[i][j][k] >= 0) {
                        //addBottomFace(chunk.mesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        //addBottomFace(chunk.mesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                else if (chunk.blocks[i][j - 1][k] <0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 2);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addBottomFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else if (chunk.blocks[i][j-1][k] == air) {
                        addBottomFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }

                if (j == CHUNKHEIGHT - 1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 3);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addTopFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j + 1, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addTopFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j + 1, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                else if (chunk.blocks[i][j + 1][k] <0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 3);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addTopFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j + 1, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else if (chunk.blocks[i][j+1][k] == air) {
                        addTopFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j + 1, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }

                if (k == 0) {
                    if (hasNorthChunk && northChunk->blocks[i][j][CHUNKWIDTH - 1] <0) {

                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 4);
                        if (chunk.blocks[i][j][k] >= 0) {
                            addZNegFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else if (!hasNorthChunk || northChunk->blocks[i][j][CHUNKWIDTH - 1] == air) {
                            addZNegFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
                else if (chunk.blocks[i][j][k - 1] <0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 4);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addZNegFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else if (chunk.blocks[i][j][k-1] == air) {
                        addZNegFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }

                if (k == CHUNKWIDTH - 1) {
                    if (hasSouthChunk && southChunk->blocks[i][j][0] <0) {

                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 5);
                        if (chunk.blocks[i][j][k] >= 0) {
                            addZPosFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + 1 + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addZPosFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + 1 + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
                else if (chunk.blocks[i][j][k + 1] <0) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 5);
                    if (chunk.blocks[i][j][k] >= 0) {
                        addZPosFace(chunk.opaqueMesh, i + chunk.key.x * CHUNKWIDTH, j, k + 1 + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else if (chunk.blocks[i][j][k+1] == air) {
                        addZPosFace(chunk.transpMesh, i + chunk.key.x * CHUNKWIDTH, j, k + 1 + chunk.key.y * CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
            }
        }
    }

    if (chunk.opaqueMeshObj == NULL) {
        chunk.opaqueMeshObj = setupOpaqueMeshTexture(chunk.opaqueMesh);
        setMeshTextureFile(chunk.opaqueMeshObj, "sources/textures/all.png");
    }
    else {
        updateMeshTexture(chunk.opaqueMeshObj, chunk.opaqueMesh);
    }

    if (chunk.transpMeshObj == NULL) {
        chunk.transpMeshObj = setupTranspMeshTexture(chunk.transpMesh);
        setMeshTextureFile(chunk.transpMeshObj, "sources/textures/all.png");
    }
    else {
        updateMeshTexture(chunk.transpMeshObj, chunk.transpMesh);
    }
}



void loadChunksAround() {
    const int renderDistance = RENDER_DISTANCE;

    int pChunkX = static_cast<int>(floor(camera.Position.x / CHUNKWIDTH));
    int pChunkY = static_cast<int>(floor(camera.Position.z / CHUNKWIDTH));

    for (int xOffset = -renderDistance; xOffset <= renderDistance; xOffset++) {
        for (int yOffset = -renderDistance; yOffset <= renderDistance; yOffset++) {
            int chunkX = pChunkX + xOffset;
            int chunkY = pChunkY + yOffset;

            int distanceSquared = xOffset * xOffset + yOffset * yOffset;
            if (distanceSquared <= renderDistance * renderDistance) {
                ChunkKey key = { chunkX, chunkY };
                auto it = chunks.find(key);
                if (it == chunks.end() || !it->second.isActive) {
                    if (std::find(chunksToLoadQueue.begin(), chunksToLoadQueue.end(), key) == chunksToLoadQueue.end()) {//pour pas doublons
                        chunksToLoadQueue.push_back(key);
                    }
                }
            }
        }
    }
}



void unloadDistantChunks() {
    const int unloadDistance = RENDER_DISTANCE + 1;

    int pChunkX = static_cast<int>(floor(camera.Position.x / CHUNKWIDTH));
    int pChunkY = static_cast<int>(floor(camera.Position.z / CHUNKWIDTH));

    std::vector<ChunkKey> chunksToUnload;

    for (auto& pair : chunks) {
        Chunk& chunk = pair.second;

        if (chunk.isActive) {
            int distanceX = chunk.key.x - pChunkX;
            int distanceY = chunk.key.y - pChunkY;
            int distanceSquared = distanceX * distanceX + distanceY * distanceY;

            if (distanceSquared > unloadDistance * unloadDistance) {
                if (std::find(chunksToUnloadQueue.begin(), chunksToUnloadQueue.end(), chunk.key) == chunksToUnloadQueue.end()) {
                    chunksToUnloadQueue.push_back(chunk.key);
                }
            }
        }
    }
}

void processChunkQueues() {    
    int unloadOps = 0;
    while (!chunksToUnloadQueue.empty() && unloadOps < maxChunksPerFrame) {
        ChunkKey key = chunksToUnloadQueue.front();
        chunksToUnloadQueue.erase(chunksToUnloadQueue.begin());

        auto it = chunks.find(key);
        if (it != chunks.end()) {
            Chunk& chunk = it->second;

            if (chunk.opaqueMeshObj != nullptr) {
                deleteMesh(chunk.opaqueMeshObj);
                chunk.opaqueMeshObj = nullptr;
            }
            chunk.opaqueMesh.clear();
            chunk.opaqueMesh.shrink_to_fit();

            if (chunk.transpMeshObj != nullptr) {
                deleteMesh(chunk.transpMeshObj);
                chunk.transpMeshObj = nullptr;
            }
            chunk.transpMesh.clear();
            chunk.transpMesh.shrink_to_fit();

            chunk.isActive = false;

        }
        unloadOps++;
    }

    
    int loadOps = 0;
    while (!chunksToLoadQueue.empty() && loadOps < maxChunksPerFrame) {
        ChunkKey key = chunksToLoadQueue.front();
        chunksToLoadQueue.erase(chunksToLoadQueue.begin());

        auto it = chunks.find(key);
        if (it == chunks.end()) {
            Chunk chunk;
            chunk.key = key;
            initChunk(chunk, key.x, key.y);
            chunks[key] = chunk;
            it = chunks.find(key);


            ChunkKey westKey = { chunk.key.x - 1, chunk.key.y };
            ChunkKey eastKey = { chunk.key.x + 1, chunk.key.y };
            ChunkKey northKey = { chunk.key.x, chunk.key.y - 1 };
            ChunkKey southKey = { chunk.key.x, chunk.key.y + 1 };

            auto westIt = chunks.find(westKey);
            if (westIt != chunks.end()) {
                if (westIt->second.isActive) {
                    updateMesh(westIt->second);
                }
            }

            auto eastIt = chunks.find(eastKey);
            if (eastIt != chunks.end()) {
                if (eastIt->second.isActive) {
                    updateMesh(eastIt->second);
                }
            }

            auto northIt = chunks.find(northKey);
            if (northIt != chunks.end()) {
                if (northIt->second.isActive) {
                    updateMesh(northIt->second);
                }
            }

            auto southIt = chunks.find(southKey);
            if (southIt != chunks.end()) {
                if (southIt->second.isActive) {
                    updateMesh(southIt->second);
                }
            }
        }

        Chunk& chunk = it->second;
        if (!chunk.isActive) {
            chunk.isActive = true;
            updateMesh(chunk);
            loadOps++;
        }
    }
}


int main() {
    SetupRender("Minecraft", &camera);
    

    loadChunksAround();

    camera.Position = glm::vec3(0.0, 70.0, 0.0);

    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 0.5);
    setLightDirection(sun, glm::vec3(-3.0f, 3.8f, -5.0f));

    while (shouldCloseTheApp()) {
        loadChunksAround();
        unloadDistantChunks();
        processChunkQueues();

        renderScene();
    }
    terminateRender();
    return 0;
}
