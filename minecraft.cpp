#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "camera.h"
#include "render.hpp"

#define CHUNKWIDTH 10
#define CHUNKHEIGHT 30
#define RENDER_DISTANCE 11
int maxChunksPerFrame = 1;
using namespace std;

int textureMapWidth = 4;
Camera camera;

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
    vector<vector<vector<int>>> blocks;
    std::vector<float> mesh;
    Mesh* meshObj = nullptr;
    bool isActive = false;
} Chunk;

std::unordered_map<ChunkKey, Chunk> chunks;
std::vector<ChunkKey> chunksToLoadQueue;
std::vector<ChunkKey> chunksToUnloadQueue;

void initChunk(chunk& chunk, int x, int y) {
    ChunkKey key;
    key.x = x;
    key.y = y;
    chunk.key = key;
    chunk.blocks.resize(CHUNKWIDTH, vector<vector<int>>(CHUNKHEIGHT, vector<int>(CHUNKWIDTH, -1)));

    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {
            for (int j = 0; j < CHUNKHEIGHT; ++j) {
                /*chunk.blocks[i][j][k] = k % 5;
                if ((i + j + k) % 2 == 0) {
                    chunk.blocks[i][j][k] = -1;
                }*/
                chunk.blocks[i][j][k] = -1;
                if (j < glm::cos((i + x * CHUNKWIDTH) * 0.1)* glm::cos((k + y * CHUNKWIDTH) * 0.1) * 5 + 7) {
                    chunk.blocks[i][j][k] = 4;
                }
            }
        }
    }
}

void updateMesh(chunk& chunk) {
    chunk.mesh.clear();
    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int j = 0; j < CHUNKHEIGHT; ++j) {
            for (int k = 0; k < CHUNKWIDTH; ++k) {

                if (chunk.blocks[i][j][k] == -1) continue;

                if (i == 0 || chunk.blocks[i - 1][j][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
                    if (chunk.blocks[i][j][k] != 2) {
                        addXNegFace(chunk.mesh, i+chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addXNegFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (i == CHUNKWIDTH - 1 || chunk.blocks[i + 1][j][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 1);
                    if (chunk.blocks[i][j][k] != 2) {
                        addXPosFace(chunk.mesh, i + 1 + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addXPosFace(chunk.mesh, i + 1 + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (j == 0 || chunk.blocks[i][j - 1][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 2);
                    if (chunk.blocks[i][j][k] != 2) {
                        addBottomFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addBottomFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (j == CHUNKHEIGHT - 1 || chunk.blocks[i][j + 1][k] == -1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 3);
                    if (chunk.blocks[i][j][k] != 2) {
                        addTopFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j + 1, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addTopFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j + 1, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (k == 0 || chunk.blocks[i][j][k - 1] == -1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 4);
                    if (chunk.blocks[i][j][k] != 2) {
                        addZNegFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addZNegFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
                if (k == CHUNKWIDTH - 1 || chunk.blocks[i][j][k + 1] == -1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 5);
                    if (chunk.blocks[i][j][k] != 2) {
                        addZPosFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + 1 + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        addZPosFace(chunk.mesh, i + chunk.key.x*CHUNKWIDTH, j, k + 1 + chunk.key.y*CHUNKWIDTH, faceUV[0], faceUV[1], faceUV[2]);
                    }
                }
            }
        }
    }
    if (chunk.meshObj == NULL) {
        chunk.meshObj = setupMeshTexture(chunk.mesh);
        setMeshTextureFile(chunk.meshObj, "sources/textures/all.png");
    }
    else {
        updateMesh(chunk.meshObj, chunk.mesh);
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

void unloadChunk(const ChunkKey& key) {
    auto it = chunks.find(key);
    if (it != chunks.end()) {
        Chunk& chunk = it->second;

        if (chunk.meshObj != nullptr) {
            deleteMesh(chunk.meshObj);
            chunk.meshObj = nullptr;
        }

        chunk.mesh.clear();
        chunk.mesh.shrink_to_fit(); 

        chunk.isActive = false;

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
            int distanceX = abs(chunk.key.x - pChunkX);
            int distanceY = abs(chunk.key.y - pChunkY);
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

        unloadChunk(key);
        std::cout << "unload : " << key.x << " , " << key.y << std::endl;
        unloadOps++;
    }

    
    int loadOps = 0;
    while (!chunksToLoadQueue.empty() && loadOps < maxChunksPerFrame) {
        ChunkKey key = chunksToLoadQueue.front();
        chunksToLoadQueue.erase(chunksToLoadQueue.begin());

        auto it = chunks.find(key);
        if (it == chunks.end()) {
            Chunk chunk;
            initChunk(chunk, key.x, key.y);
            chunk.key = key;
            chunks[key] = chunk;
            it = chunks.find(key);
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

    camera.Position = glm::vec3(0.0, 20.0, 0.0);

    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 0.8);

    while (shouldCloseTheApp()) {
        loadChunksAround();
        unloadDistantChunks();
        processChunkQueues();
        renderScene();
    }
    terminateRender();
    return 0;
}
