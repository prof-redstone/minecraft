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
#define CHUNKHEIGHT 120
#define RENDER_DISTANCE 10

#define air -1
#define oak_leaves -2
#define glass -3
#define stone 0
#define oak_planks 1
#define dirt 2
#define grass 3
#define diamond 4
#define oak_log 5
#define water -4
#define sand 6
#define snowy_grass 7
#define cactus 8
#define spruce_log 9
#define spruce_leaves -5

int maxChunksPerFrame = 3;
using namespace std;

int textureMapWidth = 8;
Camera camera;

bool hitBlock = false;
glm::ivec3 solidBlockPos;
glm::ivec3 airBlockPos;
signed char currentBlock = stone;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool flyMode = true;
float playerHeight = 1.8;
int waterHeight = 25;
int seed = 2;


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
    else if (block == oak_planks) { textureCo = std::vector<int>{ 1, 1, 1, 1, 1, 1 }[face]; }
    else if (block == dirt) { textureCo = std::vector<int>{ 3, 3, 3, 3, 3, 3 }[face]; }
    else if (block == grass) { textureCo = std::vector<int>{ 4, 4, 3, 5, 4, 4 }[face]; }
    else if (block == oak_leaves) { textureCo = std::vector<int>{ 7, 7, 7, 7, 7, 7 }[face]; }
    else if (block == diamond) { textureCo = std::vector<int>{ 6, 6, 6, 6, 6, 6 }[face]; }
    else if (block == glass) { textureCo = std::vector<int>{ 2, 2, 2, 2, 2, 2 }[face]; }
    else if (block == oak_log) { textureCo = std::vector<int>{ 8, 8, 9, 9, 8, 8 }[face]; }
    else if (block == sand) { textureCo = std::vector<int>{ 10, 10, 10, 10, 10, 10 }[face]; }
    else if (block == water) { textureCo = std::vector<int>{ 12, 12, 12, 12, 12, 12 }[face]; }
    else if (block == snowy_grass) { textureCo = std::vector<int>{ 13, 13, 3, 14, 13, 13 }[face]; }
    else if (block == spruce_log) { textureCo = std::vector<int>{ 18, 18, 19, 19, 18, 18 }[face]; }
    else if (block == spruce_leaves) { textureCo = std::vector<int>{ 20, 20, 20, 20, 20, 20 }[face]; }
    else if (block == cactus) { textureCo = std::vector<int>{ 16, 16, 17, 17, 16, 16 }[face]; }


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


double blockDensity(int i, int j, int k, int seed) {
    double weirdness = glm::clamp(db::perlin((double)i / 500.0, (double)k / 500.0, 100.0 * seed) * 10.0, -1.0, 1.0);
    double flatness = glm::clamp(db::perlin((double)i / 500.0, (double)k / 500.0, 200.0 * seed) * 10.0, -1.0, 1.0);
    double oceanity = glm::clamp(db::perlin((double)i / 500.0, (double)k / 500.0, 300.0 * seed) * 2.0,-1.0,1.0);

    double density = db::perlin((double)i / 86.0, (double)k / 86.0, (double)j / 86.0 + 100 * seed);
    density += db::perlin((double)i / 8.0, (double)k / 8.0) * 0.08;

    double height = (double)(j * 2) / CHUNKHEIGHT - 1.0; //hauteur j entre -1 et 1

    double fact1 = (weirdness + 1.0) / 2.0; // de 0.0 � 1.0
	double fact2 = (flatness * 1.5 + 2.5) + (oceanity + 1.0); //de 1.0 � 5.0 si oceanity = 0.0, de 3.0 a 7.0 si oceanity = 2.0

    return density + height * fact2 + (glm::sin(-height * 3.0 + 0.2) + 0.3) * fact1 + (oceanity +1.0);
}

bool asATree(int x, int y , int seed) {
    if ((x + y) % 2 == 0) return false;
    double proba = db::perlin((double)x / 200.0, (double)y / 200.0, (double)(100 * seed))+1.0;
    double density = 0.07;
    if (proba < 0.7) density = 0.003;
	else if (proba < 1.0) density = 0.01;
	else if (proba < 1.3) density = 0.08;

    return (hash2D(x, y, seed) % 1000) < (int)(density * 1000);
}

int biomeTemperature(int x, int y, int seed) {
    double temp = db::perlin( (double)x / 500.0, (double)y / 500.0 + 400*seed);
    temp += db::perlin((double)x / 5.0, (double)y / 5.0 + 500.0 * seed)*0.02;
    if (temp < -0.3) {
        return 1;
    }
    if (temp < 0.3) {
        return 0;
    }
	if (temp < 1.0) {
		return 2;
	}
}


void initChunk(chunk& chunk, int x, int y) {
    ChunkKey key;
    key.x = x;
    key.y = y;
    chunk.key = key;
    chunk.blocks.resize(CHUNKWIDTH, vector<vector<signed char>>(CHUNKHEIGHT, vector<signed char>(CHUNKWIDTH, -1)));

    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {
            for (int j = 0; j < CHUNKHEIGHT; ++j) {
                if (blockDensity(i + key.x * CHUNKWIDTH,j, k + key.y * CHUNKWIDTH, seed) < 0.0) {
                    chunk.blocks[i][j][k] = stone;
                }
                else {
                    if (j > waterHeight) {
                        chunk.blocks[i][j][k] = air;
                    }
                    else {
                        chunk.blocks[i][j][k] = water;
                    }
                }
            }
        }
    }
    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {
            int biome = biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed);
			if (biome == 0) { //plains
                for (int j = 0; j < CHUNKHEIGHT - 2; ++j) {
                    if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 1][k] == air) {
                        chunk.blocks[i][j][k] = grass;
                    }
                    else if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 2][k] == air) {
                        chunk.blocks[i][j][k] = dirt;
                    }
                }
            }
            if (biome == 1) { //plains
                for (int j = 0; j < CHUNKHEIGHT - 2; ++j) {
                    if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 1][k] == air) {
                        chunk.blocks[i][j][k] = snowy_grass;
                    }
                    else if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 2][k] == air) {
                        chunk.blocks[i][j][k] = dirt;
                    }
                }
            }
            else if (biome == 2) {
                //desert
                for (int j = 0; j < CHUNKHEIGHT - 2; ++j) {
                    if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 1][k] == air) {
                        chunk.blocks[i][j][k] = sand;
                    }
                    else if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 2][k] == air) {
                        chunk.blocks[i][j][k] = sand;
                    }
                }
			}
        }
    }
    //tree
    for (int i = -2; i < CHUNKWIDTH+2; ++i) {
        for (int k = -2; k < CHUNKWIDTH+2; ++k) {
            if (asATree(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed)) {
                int h = 0;
                for (int j = CHUNKHEIGHT - 2; j > waterHeight; --j) {
                    if (blockDensity(i + key.x * CHUNKWIDTH, j, k + key.y * CHUNKWIDTH, seed) < 0.0) {
                        h = j;
                        break;
                    }
                }
                if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 0) {
                    if (inChunk(i, h + 1, k)) chunk.blocks[i][h + 1][k] = oak_log;
                    if (inChunk(i, h + 2, k)) chunk.blocks[i][h + 2][k] = oak_log;
                    if (inChunk(i, h + 3, k)) chunk.blocks[i][h + 3][k] = oak_log;
                    if (inChunk(i, h + 4, k)) chunk.blocks[i][h + 4][k] = oak_log;

                    if (inChunk(i + 1, h + 3, k + 0) && chunk.blocks[i + 1][h + 3][k + 0] == air) chunk.blocks[i + 1][h + 3][k + 0] = oak_leaves;
                    if (inChunk(i + 2, h + 3, k + 0) && chunk.blocks[i + 2][h + 3][k + 0] == air) chunk.blocks[i + 2][h + 3][k + 0] = oak_leaves;
                    if (inChunk(i + 1, h + 3, k + 1) && chunk.blocks[i + 1][h + 3][k + 1] == air) chunk.blocks[i + 1][h + 3][k + 1] = oak_leaves;
                    if (inChunk(i + 1, h + 3, k - 1) && chunk.blocks[i + 1][h + 3][k - 1] == air) chunk.blocks[i + 1][h + 3][k - 1] = oak_leaves;
                    if (inChunk(i + 0, h + 3, k + 1) && chunk.blocks[i + 0][h + 3][k + 1] == air) chunk.blocks[i + 0][h + 3][k + 1] = oak_leaves;
                    if (inChunk(i + 0, h + 3, k - 1) && chunk.blocks[i + 0][h + 3][k - 1] == air) chunk.blocks[i + 0][h + 3][k - 1] = oak_leaves;
                    if (inChunk(i + 0, h + 3, k + 2) && chunk.blocks[i + 0][h + 3][k + 2] == air) chunk.blocks[i + 0][h + 3][k + 2] = oak_leaves;
                    if (inChunk(i + 0, h + 3, k - 2) && chunk.blocks[i + 0][h + 3][k - 2] == air) chunk.blocks[i + 0][h + 3][k - 2] = oak_leaves;
                    if (inChunk(i - 1, h + 3, k + 1) && chunk.blocks[i - 1][h + 3][k + 1] == air) chunk.blocks[i - 1][h + 3][k + 1] = oak_leaves;
                    if (inChunk(i - 1, h + 3, k + 0) && chunk.blocks[i - 1][h + 3][k + 0] == air) chunk.blocks[i - 1][h + 3][k + 0] = oak_leaves;
                    if (inChunk(i - 2, h + 3, k + 0) && chunk.blocks[i - 2][h + 3][k + 0] == air) chunk.blocks[i - 2][h + 3][k + 0] = oak_leaves;
                    if (inChunk(i - 1, h + 3, k - 1) && chunk.blocks[i - 1][h + 3][k - 1] == air) chunk.blocks[i - 1][h + 3][k - 1] = oak_leaves;
                    if (inChunk(i + 1, h + 4, k + 0) && chunk.blocks[i + 1][h + 4][k + 0] == air) chunk.blocks[i + 1][h + 4][k + 0] = oak_leaves;
                    if (inChunk(i + 2, h + 4, k + 0) && chunk.blocks[i + 2][h + 4][k + 0] == air) chunk.blocks[i + 2][h + 4][k + 0] = oak_leaves;
                    if (inChunk(i + 1, h + 4, k + 1) && chunk.blocks[i + 1][h + 4][k + 1] == air) chunk.blocks[i + 1][h + 4][k + 1] = oak_leaves;
                    if (inChunk(i + 1, h + 4, k - 1) && chunk.blocks[i + 1][h + 4][k - 1] == air) chunk.blocks[i + 1][h + 4][k - 1] = oak_leaves;
                    if (inChunk(i + 0, h + 4, k + 1) && chunk.blocks[i + 0][h + 4][k + 1] == air) chunk.blocks[i + 0][h + 4][k + 1] = oak_leaves;
                    if (inChunk(i + 0, h + 4, k - 1) && chunk.blocks[i + 0][h + 4][k - 1] == air) chunk.blocks[i + 0][h + 4][k - 1] = oak_leaves;
                    if (inChunk(i + 0, h + 4, k + 2) && chunk.blocks[i + 0][h + 4][k + 2] == air) chunk.blocks[i + 0][h + 4][k + 2] = oak_leaves;
                    if (inChunk(i + 0, h + 4, k - 2) && chunk.blocks[i + 0][h + 4][k - 2] == air) chunk.blocks[i + 0][h + 4][k - 2] = oak_leaves;
                    if (inChunk(i - 1, h + 4, k + 1) && chunk.blocks[i - 1][h + 4][k + 1] == air) chunk.blocks[i - 1][h + 4][k + 1] = oak_leaves;
                    if (inChunk(i - 1, h + 4, k + 0) && chunk.blocks[i - 1][h + 4][k + 0] == air) chunk.blocks[i - 1][h + 4][k + 0] = oak_leaves;
                    if (inChunk(i - 2, h + 4, k + 0) && chunk.blocks[i - 2][h + 4][k + 0] == air) chunk.blocks[i - 2][h + 4][k + 0] = oak_leaves;
                    if (inChunk(i - 1, h + 4, k - 1) && chunk.blocks[i - 1][h + 4][k - 1] == air) chunk.blocks[i - 1][h + 4][k - 1] = oak_leaves;
                    if (inChunk(i + 0, h + 5, k + 0) && chunk.blocks[i + 0][h + 5][k + 0] == air) chunk.blocks[i + 0][h + 5][k + 0] = oak_leaves;
                    if (inChunk(i + 1, h + 5, k + 0) && chunk.blocks[i + 1][h + 5][k + 0] == air) chunk.blocks[i + 1][h + 5][k + 0] = oak_leaves;
                    if (inChunk(i - 1, h + 5, k + 0) && chunk.blocks[i - 1][h + 5][k + 0] == air) chunk.blocks[i - 1][h + 5][k + 0] = oak_leaves;
                    if (inChunk(i + 0, h + 5, k + 1) && chunk.blocks[i + 0][h + 5][k + 1] == air) chunk.blocks[i + 0][h + 5][k + 1] = oak_leaves;
                    if (inChunk(i + 0, h + 5, k - 1) && chunk.blocks[i + 0][h + 5][k - 1] == air) chunk.blocks[i + 0][h + 5][k - 1] = oak_leaves;
                    if (inChunk(i + 0, h + 6, k + 0) && chunk.blocks[i + 0][h + 6][k + 0] == air) chunk.blocks[i + 0][h + 6][k + 0] = oak_leaves;
                }
                if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 1) {
                    if (inChunk(i, h + 1, k)) chunk.blocks[i][h + 1][k] = spruce_log;
                    if (inChunk(i, h + 2, k)) chunk.blocks[i][h + 2][k] = spruce_log;
                    if (inChunk(i, h + 3, k)) chunk.blocks[i][h + 3][k] = spruce_log;
                    if (inChunk(i, h + 4, k)) chunk.blocks[i][h + 4][k] = spruce_log;

                    if (inChunk(i + 1, h + 3, k + 0) && chunk.blocks[i + 1][h + 3][k + 0] == air) chunk.blocks[i + 1][h + 3][k + 0] = spruce_leaves;
                    if (inChunk(i + 2, h + 3, k + 0) && chunk.blocks[i + 2][h + 3][k + 0] == air) chunk.blocks[i + 2][h + 3][k + 0] = spruce_leaves;
                    if (inChunk(i + 1, h + 3, k + 1) && chunk.blocks[i + 1][h + 3][k + 1] == air) chunk.blocks[i + 1][h + 3][k + 1] = spruce_leaves;
                    if (inChunk(i + 1, h + 3, k - 1) && chunk.blocks[i + 1][h + 3][k - 1] == air) chunk.blocks[i + 1][h + 3][k - 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 3, k + 1) && chunk.blocks[i + 0][h + 3][k + 1] == air) chunk.blocks[i + 0][h + 3][k + 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 3, k - 1) && chunk.blocks[i + 0][h + 3][k - 1] == air) chunk.blocks[i + 0][h + 3][k - 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 3, k + 2) && chunk.blocks[i + 0][h + 3][k + 2] == air) chunk.blocks[i + 0][h + 3][k + 2] = spruce_leaves;
                    if (inChunk(i + 0, h + 3, k - 2) && chunk.blocks[i + 0][h + 3][k - 2] == air) chunk.blocks[i + 0][h + 3][k - 2] = spruce_leaves;
                    if (inChunk(i - 1, h + 3, k + 1) && chunk.blocks[i - 1][h + 3][k + 1] == air) chunk.blocks[i - 1][h + 3][k + 1] = spruce_leaves;
                    if (inChunk(i - 1, h + 3, k + 0) && chunk.blocks[i - 1][h + 3][k + 0] == air) chunk.blocks[i - 1][h + 3][k + 0] = spruce_leaves;
                    if (inChunk(i - 2, h + 3, k + 0) && chunk.blocks[i - 2][h + 3][k + 0] == air) chunk.blocks[i - 2][h + 3][k + 0] = spruce_leaves;
                    if (inChunk(i - 1, h + 3, k - 1) && chunk.blocks[i - 1][h + 3][k - 1] == air) chunk.blocks[i - 1][h + 3][k - 1] = spruce_leaves;
                    if (inChunk(i + 1, h + 4, k + 0) && chunk.blocks[i + 1][h + 4][k + 0] == air) chunk.blocks[i + 1][h + 4][k + 0] = spruce_leaves;
                    if (inChunk(i + 2, h + 4, k + 0) && chunk.blocks[i + 2][h + 4][k + 0] == air) chunk.blocks[i + 2][h + 4][k + 0] = spruce_leaves;
                    if (inChunk(i + 1, h + 4, k + 1) && chunk.blocks[i + 1][h + 4][k + 1] == air) chunk.blocks[i + 1][h + 4][k + 1] = spruce_leaves;
                    if (inChunk(i + 1, h + 4, k - 1) && chunk.blocks[i + 1][h + 4][k - 1] == air) chunk.blocks[i + 1][h + 4][k - 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 4, k + 1) && chunk.blocks[i + 0][h + 4][k + 1] == air) chunk.blocks[i + 0][h + 4][k + 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 4, k - 1) && chunk.blocks[i + 0][h + 4][k - 1] == air) chunk.blocks[i + 0][h + 4][k - 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 4, k + 2) && chunk.blocks[i + 0][h + 4][k + 2] == air) chunk.blocks[i + 0][h + 4][k + 2] = spruce_leaves;
                    if (inChunk(i + 0, h + 4, k - 2) && chunk.blocks[i + 0][h + 4][k - 2] == air) chunk.blocks[i + 0][h + 4][k - 2] = spruce_leaves;
                    if (inChunk(i - 1, h + 4, k + 1) && chunk.blocks[i - 1][h + 4][k + 1] == air) chunk.blocks[i - 1][h + 4][k + 1] = spruce_leaves;
                    if (inChunk(i - 1, h + 4, k + 0) && chunk.blocks[i - 1][h + 4][k + 0] == air) chunk.blocks[i - 1][h + 4][k + 0] = spruce_leaves;
                    if (inChunk(i - 2, h + 4, k + 0) && chunk.blocks[i - 2][h + 4][k + 0] == air) chunk.blocks[i - 2][h + 4][k + 0] = spruce_leaves;
                    if (inChunk(i - 1, h + 4, k - 1) && chunk.blocks[i - 1][h + 4][k - 1] == air) chunk.blocks[i - 1][h + 4][k - 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 5, k + 0) && chunk.blocks[i + 0][h + 5][k + 0] == air) chunk.blocks[i + 0][h + 5][k + 0] = spruce_leaves;
                    if (inChunk(i + 1, h + 5, k + 0) && chunk.blocks[i + 1][h + 5][k + 0] == air) chunk.blocks[i + 1][h + 5][k + 0] = spruce_leaves;
                    if (inChunk(i - 1, h + 5, k + 0) && chunk.blocks[i - 1][h + 5][k + 0] == air) chunk.blocks[i - 1][h + 5][k + 0] = spruce_leaves;
                    if (inChunk(i + 0, h + 5, k + 1) && chunk.blocks[i + 0][h + 5][k + 1] == air) chunk.blocks[i + 0][h + 5][k + 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 5, k - 1) && chunk.blocks[i + 0][h + 5][k - 1] == air) chunk.blocks[i + 0][h + 5][k - 1] = spruce_leaves;
                    if (inChunk(i + 0, h + 6, k + 0) && chunk.blocks[i + 0][h + 6][k + 0] == air) chunk.blocks[i + 0][h + 6][k + 0] = spruce_leaves;
                }
                if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 2 && (i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH)%11 == 0) {
                    if (inChunk(i, h + 1, k)) chunk.blocks[i][h + 1][k] = cactus;
                    if (inChunk(i, h + 2, k)) chunk.blocks[i][h + 2][k] = cactus;
                    if (inChunk(i, h + 3, k)) chunk.blocks[i][h + 3][k] = cactus;
                    if ((i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH) % 5 == 0) {
                        if (inChunk(i, h + 4, k)) chunk.blocks[i][h + 4][k] = cactus;
                    }
                }
            }
        }
    }
}

void updateMesh(chunk& chunk) {
    chunk.opaqueMesh.clear();
    chunk.transpMesh.clear();

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


void raycastDDA(glm::vec3 start, glm::vec3 direction_, float maxDistance = 7.0f) {
    hitBlock = false;
    glm::vec3 direction = glm::normalize(direction_);
    glm::vec3 pos = start;
    glm::ivec3 blockPos = glm::ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
    glm::ivec3 lastAirPos = blockPos;

    glm::ivec3 step;
    glm::vec3 deltaDist;
    glm::vec3 sideDist;

    float currentDistance = 0.0f;

    if (direction.x < 0) {
        step.x = -1;
        deltaDist.x = -1.0f / direction.x;
        sideDist.x = (pos.x - blockPos.x) * deltaDist.x;
    }else {
        step.x = 1;
        deltaDist.x = 1.0f / direction.x;
        sideDist.x = (blockPos.x + 1.0f - pos.x) * deltaDist.x;
    }

    if (direction.y < 0) {
        step.y = -1;
        deltaDist.y = -1.0f / direction.y;
        sideDist.y = (pos.y - blockPos.y) * deltaDist.y;
    }else {
        step.y = 1;
        deltaDist.y = 1.0f / direction.y;
        sideDist.y = (blockPos.y + 1.0f - pos.y) * deltaDist.y;
    }

    if (direction.z < 0) {
        step.z = -1;
        deltaDist.z = -1.0f / direction.z;
        sideDist.z = (pos.z - blockPos.z) * deltaDist.z;
    }else {
        step.z = 1;
        deltaDist.z = 1.0f / direction.z;
        sideDist.z = (blockPos.z + 1.0f - pos.z) * deltaDist.z;
    }

    while (currentDistance < maxDistance) {
        int chunkX = static_cast<int>(floor((float)blockPos.x / CHUNKWIDTH));
        int chunkZ = static_cast<int>(floor((float)blockPos.z / CHUNKWIDTH));
        ChunkKey chunkKey = { chunkX, chunkZ };

        auto chunkIt = chunks.find(chunkKey);
        if (chunkIt == chunks.end() || !chunkIt->second.isActive) {
            hitBlock = false;
            break;
        }

        
        int localX = blockPos.x - chunkX * CHUNKWIDTH;
        int localZ = blockPos.z - chunkZ * CHUNKWIDTH;

        
        if (localX >= 0 && localX < CHUNKWIDTH &&
            blockPos.y >= 0 && blockPos.y < CHUNKHEIGHT &&
            localZ >= 0 && localZ < CHUNKWIDTH) {

            
            signed char blockType = chunkIt->second.blocks[localX][blockPos.y][localZ];

            
            if (blockType != air) {
                hitBlock = true;
                solidBlockPos = blockPos;
                airBlockPos = lastAirPos;
                //distance = currentDistance;
                return;
            }            
            lastAirPos = blockPos;
        }

        if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
            sideDist.x += deltaDist.x;
            blockPos.x += step.x;
            currentDistance = sideDist.x - deltaDist.x;
        }
        else if (sideDist.y < sideDist.z) {
            sideDist.y += deltaDist.y;
            blockPos.y += step.y;
            currentDistance = sideDist.y - deltaDist.y;
        }
        else {
            sideDist.z += deltaDist.z;
            blockPos.z += step.z;
            currentDistance = sideDist.z - deltaDist.z;
        }
    }

}

void processClick() {
    const double CLICK_COOLDOWN = 0.3;

    static double lastClickTimeL = 0.0;
    static double lastClickTimeR = 0.0;
    double currentTime = glfwGetTime();
    if (hitBlock) {
        updateBlockTarget(glm::vec3(solidBlockPos));
        if (!camera.leftMousePressed) {
            lastClickTimeL = 0.0;
        }
        if (camera.leftMousePressed && (currentTime - lastClickTimeL) >= CLICK_COOLDOWN) {
            lastClickTimeL = currentTime;
            int chunkX = static_cast<int>(floor((float)solidBlockPos.x / CHUNKWIDTH));
            int chunkZ = static_cast<int>(floor((float)solidBlockPos.z / CHUNKWIDTH));
            ChunkKey chunkKey = { chunkX, chunkZ };
            auto chunkIt = chunks.find(chunkKey);
            if (chunkIt != chunks.end() && chunkIt->second.isActive) {
                int localX = solidBlockPos.x - chunkX * CHUNKWIDTH;
                int localZ = solidBlockPos.z - chunkZ * CHUNKWIDTH;
                chunkIt->second.blocks[localX][solidBlockPos.y][localZ] = air;

                Chunk& chunk = chunkIt->second;
                updateMesh(chunk);

                if (localX == 0) {
                    ChunkKey westKey = { chunk.key.x - 1, chunk.key.y }; 
                    auto westIt = chunks.find(westKey);
                    if (westIt != chunks.end()) {
                        if (westIt->second.isActive) {
                            updateMesh(westIt->second);
                        }
                    }
                }
                if (localX == CHUNKWIDTH-1) {
                    ChunkKey eastKey = { chunk.key.x + 1, chunk.key.y };
                    auto eastIt = chunks.find(eastKey);
                    if (eastIt != chunks.end()) {
                        if (eastIt->second.isActive) {
                            updateMesh(eastIt->second);
                        }
                    }
                }
                if (localZ == 0) {
                    ChunkKey northKey = { chunk.key.x, chunk.key.y - 1};
                    auto northIt = chunks.find(northKey);
                    if (northIt != chunks.end()) {
                        if (northIt->second.isActive) {
                            updateMesh(northIt->second);
                        }
                    }
                }
                if (localZ == CHUNKWIDTH - 1) {
                    ChunkKey southKey = { chunk.key.x, chunk.key.y + 1};
                    auto southIt = chunks.find(southKey);
                    if (southIt != chunks.end()) {
                        if (southIt->second.isActive) {
                            updateMesh(southIt->second);
                        }
                    }
                }

            }
        }

        if (!camera.rightMousePressed) {
            lastClickTimeR = 0.0;
        }
        if (camera.rightMousePressed && (currentTime - lastClickTimeR) >= CLICK_COOLDOWN) {
            lastClickTimeR = currentTime;
            int chunkX = static_cast<int>(floor((float)airBlockPos.x / CHUNKWIDTH));
            int chunkZ = static_cast<int>(floor((float)airBlockPos.z / CHUNKWIDTH));
            ChunkKey chunkKey = { chunkX, chunkZ };
            auto chunkIt = chunks.find(chunkKey);
            if (chunkIt != chunks.end() && chunkIt->second.isActive) {
                int localX = airBlockPos.x - chunkX * CHUNKWIDTH;
                int localZ = airBlockPos.z - chunkZ * CHUNKWIDTH;
                glm::ivec3 posPlayerInt = glm::ivec3(glm::floor(camera.Position.x), glm::floor(camera.Position.y), glm::floor(camera.Position.z));
                if (airBlockPos != posPlayerInt && airBlockPos != (posPlayerInt - glm::ivec3(0, 1, 0))) {
                    chunkIt->second.blocks[localX][airBlockPos.y][localZ] = currentBlock;
                }

                Chunk& chunk = chunkIt->second;
                updateMesh(chunk);
            }
        }
    }
    else {
        updateBlockTarget(glm::vec3(0.0, 0.0, 0.0));
    }

    
}

void processKey() {
    if (camera.key1Pressed) currentBlock = stone;
    if (camera.key2Pressed) currentBlock = glass;
    if (camera.key3Pressed) currentBlock = oak_planks;
    if (camera.key4Pressed) currentBlock = dirt;
    if (camera.key5Pressed) currentBlock = oak_log;
    if (camera.key6Pressed) currentBlock = grass;
    if (camera.key7Pressed) currentBlock = diamond;
    if (camera.key8Pressed) currentBlock = oak_leaves;

    if (camera.flyPressed) { 
		camera.MovementSpeed = 200.0f;
        flyMode = true; 
    }
    if (camera.walkPressed) {
        camera.MovementSpeed = 7.0f;
        flyMode = false;
    }

    if (flyMode) {
        if (camera.forwardPressed == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (camera.backwardPressed == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (camera.leftPressed == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (camera.rightPressed == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (camera.downPressed == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
        if (camera.upPressed == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
    }
    else {
        glm::vec3 dPos = glm::vec3(0.0, 0.0, 0.0);
        if (camera.forwardPressed == GLFW_PRESS) {
            dPos += glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z)) * camera.MovementSpeed * deltaTime;
        }
        if (camera.backwardPressed == GLFW_PRESS) {
            dPos += glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z)) * -camera.MovementSpeed * deltaTime;
        }
        if (camera.rightPressed == GLFW_PRESS) {
            dPos += glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z)) * camera.MovementSpeed * deltaTime;
        }
        if (camera.leftPressed == GLFW_PRESS) {
            dPos += glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z)) * -camera.MovementSpeed * deltaTime;
        }

        float largeur = 0.1;
        //glm::ivec3 blockPos = glm::ivec3(floor(camera.Position.x), floor(camera.Position.y), floor(camera.Position.z));
        bool leftBlockFree = true;
        bool rightBlockFree = true;
        bool forwardBlockFree = true;
        bool BackwardBlockFree = true;

        int blockHead = (int)(camera.Position.y);
        int blockBelly = (int)(camera.Position.y - playerHeight/2);
        int blockFoot = (int)(camera.Position.y - playerHeight);


        int chunkXLeft = static_cast<int>(floor((float)(camera.Position.x - largeur) / CHUNKWIDTH));
        int chunkZLeft = static_cast<int>(floor((float)(camera.Position.z) / CHUNKWIDTH));
        int localXLeft = (camera.Position.x - largeur) - chunkXLeft * CHUNKWIDTH;
        int localZLeft = (camera.Position.z) - chunkZLeft * CHUNKWIDTH;
        ChunkKey chunkKeyLeft = { chunkXLeft, chunkZLeft };
        auto chunkItLeft = chunks.find(chunkKeyLeft);
        if (chunkItLeft != chunks.end() && chunkItLeft->second.isActive) {
            if (chunkItLeft->second.blocks[localXLeft][blockHead][localZLeft] != air || chunkItLeft->second.blocks[localXLeft][blockBelly][localZLeft] != air || chunkItLeft->second.blocks[localXLeft][blockFoot][localZLeft] != air) {
                leftBlockFree = false;
            }
        }

        int chunkXRight = static_cast<int>(floor((float)(camera.Position.x + largeur) / CHUNKWIDTH));
        int chunkZRight = static_cast<int>(floor((float)(camera.Position.z) / CHUNKWIDTH));
        int localXRight = (camera.Position.x + largeur) - chunkXRight * CHUNKWIDTH;
        int localZRight = (camera.Position.z) - chunkZRight * CHUNKWIDTH;
        ChunkKey chunkKeyRight = { chunkXRight, chunkZRight };
        auto chunkItRight = chunks.find(chunkKeyRight);
        if (chunkItRight != chunks.end() && chunkItRight->second.isActive) {
            if (chunkItRight->second.blocks[localXRight][blockHead][localZRight] != air || chunkItRight->second.blocks[localXRight][blockBelly][localZRight] != air || chunkItRight->second.blocks[localXRight][blockFoot][localZRight] != air) {
                rightBlockFree = false;
                
            }
        }

        int chunkXForward = static_cast<int>(floor((float)(camera.Position.x) / CHUNKWIDTH));
        int chunkZForward = static_cast<int>(floor((float)(camera.Position.z + largeur) / CHUNKWIDTH));
        int localXForward = (camera.Position.x) - chunkXForward * CHUNKWIDTH;
        int localZForward = (camera.Position.z + largeur) - chunkZForward * CHUNKWIDTH;
        ChunkKey chunkKeyForward = { chunkXForward, chunkZForward };
        auto chunkItForward = chunks.find(chunkKeyForward);
        if (chunkItForward != chunks.end() && chunkItForward->second.isActive) {
            if (chunkItForward->second.blocks[localXForward][blockHead][localZForward] != air || chunkItForward->second.blocks[localXForward][blockBelly][localZForward] != air || chunkItForward->second.blocks[localXForward][blockFoot][localZForward] != air) {
                forwardBlockFree = false;
            }
        }

        int chunkXBackward = static_cast<int>(floor((float)(camera.Position.x) / CHUNKWIDTH));
        int chunkZBackward = static_cast<int>(floor((float)(camera.Position.z - largeur) / CHUNKWIDTH));
        int localXBackward = (camera.Position.x) - chunkXBackward * CHUNKWIDTH;
        int localZBackward = (camera.Position.z - largeur) - chunkZBackward * CHUNKWIDTH;
        ChunkKey chunkKeyBackward = { chunkXBackward, chunkZBackward };
        auto chunkItBackward = chunks.find(chunkKeyBackward);
        if (chunkItBackward != chunks.end() && chunkItBackward->second.isActive) {
            if (chunkItBackward->second.blocks[localXBackward][blockHead][localZBackward] != air || chunkItBackward->second.blocks[localXBackward][blockBelly][localZBackward] != air || chunkItBackward->second.blocks[localXBackward][blockFoot][localZBackward] != air) {
                BackwardBlockFree = false;
            }
        }

        if (leftBlockFree && dPos.x < 0) {
            camera.Position.x += dPos.x;
        }
        if (rightBlockFree && dPos.x > 0) {
            camera.Position.x += dPos.x;
        }
        if (forwardBlockFree && dPos.z > 0) {
            camera.Position.z += dPos.z;
        }
        if (BackwardBlockFree && dPos.z < 0) {
            camera.Position.z += dPos.z;
        }

        const float accY = -27;
        const float maxYspeed = 25;
        const float jumpHeight = 8;
        camera.Yspeed = camera.Yspeed < -maxYspeed ? -maxYspeed : camera.Yspeed + accY*deltaTime;
        float posY = camera.Position.y + camera.Yspeed * deltaTime;
        bool isOnGround = false;

        int chunkX = static_cast<int>(floor((float)(camera.Position.x) / CHUNKWIDTH));
        int chunkZ = static_cast<int>(floor((float)(camera.Position.z) / CHUNKWIDTH));
        int localX = (camera.Position.x) - chunkX * CHUNKWIDTH;
        int localZ = (camera.Position.z) - chunkZ * CHUNKWIDTH;
        ChunkKey chunkKey = { chunkX, chunkZ };
        auto chunkIt = chunks.find(chunkKey);

        if (chunkIt != chunks.end() && chunkIt->second.isActive) {
            if (camera.Yspeed <= 0.0) {
                bool blockBelow = chunkIt->second.blocks[localX][(int)(posY - playerHeight)][localZ] != air;
                
                if (blockBelow) {
                    float desiredY = (float)((int)(posY - playerHeight) + 1) + playerHeight;
                    camera.Position.y = desiredY;
                    camera.Yspeed = 0.0f;
                    isOnGround = true;
                } else {
                    camera.Position.y = posY;
                }
            }
            else if (camera.Yspeed > 0.0) {
                bool blockAbove = chunkIt->second.blocks[localX][(int)(posY)][localZ] != air;
                
                if (blockAbove) {
                    float desiredY = (float)((int)(posY));
                    camera.Position.y = desiredY;
                    camera.Yspeed = 0.0f;
                } else {
                    camera.Position.y = posY;
                }
            }
        }

        if (camera.spacePressed == GLFW_PRESS && isOnGround) {
            camera.Yspeed = jumpHeight;
        }
        

        if (camera.Position.y <= playerHeight) {
            camera.Position.y = CHUNKHEIGHT-1;
        }
        if (camera.Position.y >= CHUNKHEIGHT - 1) {
            camera.Position.y = CHUNKHEIGHT - 1;
        }
    }
}

int main() {
    SetupRender("Minecraft", &camera);
    

    camera.Position = glm::vec3(1.0, CHUNKHEIGHT-2 -20, 1.0);

    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 0.6);
    setLightDirection(sun, glm::vec3(-3.8f, 7.5f, -5.0f));

    while (shouldCloseTheApp()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //setLightDirection(sun, glm::vec3(-3.0f + glm::cos(glfwGetTime())*2.0, 3.8f + glm::sin(glfwGetTime()) * 2.0, -5.0f));
		//cout << camera.Position.y << endl;

        loadChunksAround();
        unloadDistantChunks();
        processChunkQueues();
        raycastDDA(camera.Position, camera.Front);
        processKey();
        processClick();
        

        renderScene();
    }
    terminateRender();
    return 0;
}
