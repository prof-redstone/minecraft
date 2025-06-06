#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"
#include "blocks.h"
#include "camera.h"
#include "render.hpp"
#include "structure.h"

#define CHUNKWIDTH 30
#define CHUNKHEIGHT 120
#define RENDER_DISTANCE 10


int maxChunksPerFrame = 2;
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

void addPlantFace(vector<float>& mesh, float x, float y, float z, float ou, float ov, float s) {
    mesh.insert(mesh.end(), {
        x-1, y, z, ou, ov + s,
        x-1, y + 1, z, ou, ov,
        x, y + 1, z + 1, ou + s, ov,
        x-1, y, z, ou, ov + s,
        x, y + 1, z + 1, ou + s, ov,
        x, y, z + 1, ou + s, ov + s,

        x, y, z, ou, ov + s,
        x, y + 1, z, ou, ov,
        x-1, y + 1, z + 1, ou + s, ov,
        x, y, z, ou, ov + s,
        x-1, y + 1, z + 1, ou + s, ov,
        x-1, y, z + 1, ou + s, ov + s,

        x - 1, y + 1, z, ou, ov,
        x - 1, y, z, ou, ov + s,
        x, y + 1, z + 1, ou + s, ov,
        x, y + 1, z + 1, ou + s, ov,
        x - 1, y, z, ou, ov + s,
        x, y, z + 1, ou + s, ov + s,

        x, y + 1, z, ou, ov,
        x, y, z, ou, ov + s,
        x - 1, y + 1, z + 1, ou + s, ov,
        x - 1, y + 1, z + 1, ou + s, ov,
        x, y, z, ou, ov + s,
        x - 1, y, z + 1, ou + s, ov + s
        
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
    else if (block == short_grass) { textureCo = std::vector<int>{ 21, 21, 21, 21, 21, 21 }[face]; }
    else if (block == poppy) { textureCo = std::vector<int>{ 22, 22, 22, 22, 22, 22 }[face]; }
    else if (block == dandelion) { textureCo = std::vector<int>{ 23, 23, 23, 23, 23, 23 }[face]; }
    else if (block == dead_bush) { textureCo = std::vector<int>{ 24, 24, 24, 24, 24, 24 }[face]; }
    else if (block == sweet_berry) { textureCo = std::vector<int>{ 25, 25, 25, 25, 25, 25 }[face]; }
    else if (block == kelp) { textureCo = std::vector<int>{ 27, 27, 27, 27, 27, 27 }[face]; }
    else if (block == gravel) { textureCo = std::vector<int>{ 11, 11, 11, 11, 11, 11 }[face]; }


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
    std::vector<float> opaqueNoShadowMesh;
    std::vector<float> transpMesh;
    std::vector<float> transpNoShadowMesh;
    std::vector<float> plantMesh;
    Mesh* opaqueMeshObj = nullptr;
    Mesh* transpMeshObj = nullptr;
    Mesh* opaqueNoShadowMeshObj = nullptr;
    Mesh* transpNoShadowMeshObj = nullptr;
    Mesh* plantMeshObj = nullptr;
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
    double weirdness = glm::clamp(db::perlin((double)i / 500.0, (double)k / 500.0, 100.0 * seed) * 5.0, -1.0, 1.0);
    double flatness = glm::clamp(db::perlin((double)i / 500.0, (double)k / 500.0, 200.0 * seed) * 5.0, -1.0, 1.0);
    double oceanity = glm::clamp(db::perlin((double)i / 500.0, (double)k / 500.0, 300.0 * seed) * 2.0,-1.0,1.0);

    double density = db::perlin((double)i / 86.0, (double)k / 86.0, (double)j / 86.0 + 100 * seed);
    density += db::perlin((double)i / 8.0, (double)k / 8.0) * 0.08;

    double height = (double)(j * 2) / CHUNKHEIGHT - 1.0; //hauteur j entre -1 et 1

    double fact1 = (weirdness + 1.0) / 2.0; // de 0.0 � 1.0
	double fact2 = (flatness * 1.5 + 2.5) + (oceanity + 1.0); //de 1.0 � 5.0 si oceanity = 0.0, de 3.0 a 7.0 si oceanity = 2.0

    return density + height * fact2 + (glm::sin(-height * 3.0 + 0.2) + 0.3) * fact1 + (oceanity +1.0);
}

bool asATree(int x, int y , int seed) {
    if ((x + y) % 4 != 0) return false;
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

void placeBlockStructure(vector<vector<vector<signed char>>>& chunk, std::vector<std::tuple<int, int, int, int, int>> structure, int lx, int ly, int lz) {
    for (const auto& block : structure) {
        int x = std::get<0>(block);
        int y = std::get<1>(block);
        int z = std::get<2>(block);
        signed char blockType = std::get<3>(block);
        bool replaceExisting = std::get<4>(block);
        if (inChunk(x +lx, y+ly, z+lz)) {
            if (replaceExisting || !hasProp(chunk[x + lx][y + ly][z + lz], BP::SOLID)) {
                chunk[x+lx][y+ly][z+lz] = blockType;
            }
        }
    }
}

void initChunk(chunk& chunk, int x, int y) {
    ChunkKey key;
    key.x = x;
    key.y = y;
    chunk.key = key;
    chunk.blocks.resize(CHUNKWIDTH, vector<vector<signed char>>(CHUNKHEIGHT, vector<signed char>(CHUNKWIDTH, air)));

    //vector de taille CHUNKWIDTH par CHUNKWIDTH de int :
	vector<vector<int>> surfaceLevel(CHUNKWIDTH, vector<int>(CHUNKHEIGHT, 0));


    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {
            //density gen
            for (int j = 0; j < CHUNKHEIGHT; ++j) {
                if (blockDensity(i + key.x * CHUNKWIDTH,j, k + key.y * CHUNKWIDTH, seed) < 0.0) {
                    if(j > surfaceLevel[i][k]) {
                        surfaceLevel[i][k] = j;
					}
                    chunk.blocks[i][j][k] = stone;
                }else {
                    if (j > waterHeight) {
                        chunk.blocks[i][j][k] = air;
                    }
                    else {
                        chunk.blocks[i][j][k] = water;
                    }
                }                
            }
            //differentiation
            int biome = biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed);
            if (biome == 0) { //plains
                for (int j = 0; j < CHUNKHEIGHT - 2; ++j) {
                    if (j >= waterHeight) {
                        if (chunk.blocks[i][j][k] == stone && !hasProp(chunk.blocks[i][j + 1][k], SOLID)) {
                            chunk.blocks[i][j][k] = grass;
                        }
                        else if (chunk.blocks[i][j][k] == stone && !hasProp(chunk.blocks[i][j + 2][k], SOLID)) {
                            chunk.blocks[i][j][k] = dirt;
                        }
                    }else {
                        if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 1][k] == water) {
                            chunk.blocks[i][j][k] = sand;
                        }
                    }

                    if ((j == waterHeight) && (chunk.blocks[i][j][k] == grass) 
                        || (j == waterHeight+1) && (chunk.blocks[i][j][k] == grass) 
                        || (j == waterHeight) && (chunk.blocks[i][j][k] == dirt)) {
                        chunk.blocks[i][j][k] = sand;
                    }
                }
            }
            if (biome == 1) { //plains
                for (int j = 0; j < CHUNKHEIGHT - 2; ++j) {
                    if (j >= waterHeight) {
                        if (chunk.blocks[i][j][k] == stone && !hasProp(chunk.blocks[i][j + 1][k], SOLID)) {
                            chunk.blocks[i][j][k] = snowy_grass;
                        }
                        else if (chunk.blocks[i][j][k] == stone && !hasProp(chunk.blocks[i][j + 2][k], SOLID)) {
                            chunk.blocks[i][j][k] = dirt;
                        }
                    }
                    else {
                        if (chunk.blocks[i][j][k] == stone && chunk.blocks[i][j + 1][k] == water) {
                            chunk.blocks[i][j][k] = gravel;
                        }
                    }

                    if ((j == waterHeight) && (chunk.blocks[i][j][k] == snowy_grass)
                        || (j == waterHeight + 1) && (chunk.blocks[i][j][k] == snowy_grass)
                        || (j == waterHeight) && (chunk.blocks[i][j][k] == dirt)) {
                        chunk.blocks[i][j][k] = gravel;
                    }
                }
            }
            else if (biome == 2) {
                //desert
                for (int j = 0; j < CHUNKHEIGHT - 2; ++j) {
                    if (chunk.blocks[i][j][k] == stone && !hasProp(chunk.blocks[i][j + 1][k], SOLID)) {
                        chunk.blocks[i][j][k] = sand;
                    }
                    else if (chunk.blocks[i][j][k] == stone && !hasProp(chunk.blocks[i][j + 2][k], SOLID)) {
                        chunk.blocks[i][j][k] = sand;
                    }
                }
            }


        }
    }
    //tree
    for (int i = -2; i < CHUNKWIDTH + 2; ++i) {
        for (int k = -2; k < CHUNKWIDTH+2; ++k) {
            if (asATree(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed)) {
                int h = 0;
                if (i < 0 || k < 0 || i >= CHUNKWIDTH || k >= CHUNKWIDTH) {
                    for (int j = CHUNKHEIGHT - 2; j >= waterHeight; --j) {
                        if (blockDensity(i + key.x * CHUNKWIDTH, j, k + key.y * CHUNKWIDTH, seed) < 0.0) {
                            h = j;
                            break;
                        }
                    }
                }else {
					h = surfaceLevel[i][k];                    
                }
                if (h < waterHeight+2) {
                    break;
                }

                if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 0) {
					placeBlockStructure(chunk.blocks, oak_tree_1, i,h,k);
                }else
                if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 1) {
                    placeBlockStructure(chunk.blocks, spruce_tree_1, i, h, k);
                }else
                if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 2 && (i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH)%11 == 0) {
                    if ((i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH) % 5 == 0) {
                        placeBlockStructure(chunk.blocks, cactus_2, i, h, k);
                    }
                    else if ((i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH) % 5 == 1) {
                        placeBlockStructure(chunk.blocks, cactus_3, i, h, k);
                    }
                    else {
						placeBlockStructure(chunk.blocks, cactus_1, i, h, k);
                    }
                }
            }
        }
    }

    //decoration
    for (int i = 0; i < CHUNKWIDTH; ++i) {
        for (int k = 0; k < CHUNKWIDTH; ++k) {
            int h = surfaceLevel[i][k];
            if (h < waterHeight || chunk.blocks[i][h + 1][k] != air) {
                break;
            }
            if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 0) {
                if ((hash2D(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) % 100) < 4) {
				    chunk.blocks[i][h + 1][k] = short_grass;
                    continue;
                }
                if ((hash2D(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed + 1) % 100) < 1) {
                    chunk.blocks[i][h + 1][k] = poppy;
                    continue;
                }
                if ((hash2D(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed + 2) % 100) < 1) {
                    chunk.blocks[i][h + 1][k] = dandelion;
                    continue;
                }
            }
            if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 1) {
                if ((hash2D(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) % 100) < 1) {
                    chunk.blocks[i][h + 1][k] = sweet_berry;
                    continue;
                }
            }
            if (h != 0 && biomeTemperature(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) == 2) {
                if ((hash2D(i + key.x * CHUNKWIDTH, k + key.y * CHUNKWIDTH, seed) % 200) < 1) {
                    chunk.blocks[i][h + 1][k] = dead_bush;
                    continue;
                }
            }
        }
	}
}

void updateMesh(chunk& chunk) {
    chunk.opaqueMesh.clear();
    chunk.opaqueNoShadowMesh.clear();
	chunk.transpMesh.clear();
    chunk.transpNoShadowMesh.clear();
	chunk.plantMesh.clear();


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
                if (hasProp(chunk.blocks[i][j][k], PLANT)) {
                    if (hasProp(chunk.blocks[i][j][k], CROSS_SHAPE)) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
                        addPlantFace(chunk.plantMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        continue;
                    }
                    else {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
                        addXNegFace(chunk.plantMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        faceUV = getFaceUV(chunk.blocks[i][j][k], 1);
                        addXPosFace(chunk.plantMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        faceUV = getFaceUV(chunk.blocks[i][j][k], 2);
                        addBottomFace(chunk.plantMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        faceUV = getFaceUV(chunk.blocks[i][j][k], 3);
                        addTopFace(chunk.plantMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        faceUV = getFaceUV(chunk.blocks[i][j][k], 4);
                        addZNegFace(chunk.plantMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        faceUV = getFaceUV(chunk.blocks[i][j][k], 5);
                        addZPosFace(chunk.plantMesh, i, j, k+1, faceUV[0], faceUV[1], faceUV[2]);
						continue;
                    }
                }

				//X POSITIVE FACE
                if (i == 0) {
                    if (hasWestChunk && hasProp(westChunk->blocks[CHUNKWIDTH - 1][j][k], TRANSPARENT) && (westChunk->blocks[CHUNKWIDTH - 1][j][k] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE)) ) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
						if ( ! hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addXNegFace(chunk.opaqueMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addXNegFace(chunk.transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                        else {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addXNegFace(chunk.opaqueNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addXNegFace(chunk.transpNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                    }
                }
                else if ( hasProp(chunk.blocks[i - 1][j][k], TRANSPARENT) && (chunk.blocks[i - 1][j][k] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE)) ) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 0);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addXNegFace(chunk.opaqueMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addXNegFace(chunk.transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addXNegFace(chunk.opaqueNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addXNegFace(chunk.transpNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
				//X NEGATIVE FACE
                if (i == CHUNKWIDTH - 1 ) {
                    if (hasEastChunk && hasProp(eastChunk->blocks[0][j][k], TRANSPARENT) && (eastChunk->blocks[0][j][k] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 1);
                        if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addXPosFace(chunk.opaqueMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addXPosFace(chunk.transpMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                        else {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addXPosFace(chunk.opaqueNoShadowMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addXPosFace(chunk.transpNoShadowMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                    }
                }
                else if (hasProp(chunk.blocks[i + 1][j][k], TRANSPARENT) && (chunk.blocks[i + 1][j][k] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 1);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addXPosFace(chunk.opaqueMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addXPosFace(chunk.transpMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addXPosFace(chunk.opaqueNoShadowMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addXPosFace(chunk.transpNoShadowMesh, i + 1, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }

                //BOTTOM FACE
                if (j == 0) {//face vers le bas, inutile
                    /*vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 2);
                    if (chunk.blocks[i][j][k] >= 0) {
                        //addBottomFace(chunk.mesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }
                    else {
                        //addBottomFace(chunk.mesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                    }*/
                }
                else if (hasProp(chunk.blocks[i][j-1][k], TRANSPARENT) && (chunk.blocks[i][j-1][k] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 2);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addBottomFace(chunk.opaqueMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addBottomFace(chunk.transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addBottomFace(chunk.opaqueNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addBottomFace(chunk.transpNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
				//TOP FACE
                if (j == CHUNKHEIGHT - 1) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 3);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addTopFace(chunk.opaqueMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addTopFace(chunk.transpMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addTopFace(chunk.opaqueNoShadowMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addTopFace(chunk.transpNoShadowMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
                else if (hasProp(chunk.blocks[i][j + 1][k], TRANSPARENT) && (chunk.blocks[i][j + 1][k] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 3);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addTopFace(chunk.opaqueMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addTopFace(chunk.transpMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addTopFace(chunk.opaqueNoShadowMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addTopFace(chunk.transpNoShadowMesh, i, j + 1, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }


				//Z NEGATIVE FACE
                if (k == 0) {
                    if (hasNorthChunk && hasProp(northChunk->blocks[i][j][CHUNKWIDTH - 1], TRANSPARENT) && (northChunk->blocks[i][j][CHUNKWIDTH - 1] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 4);
                        if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addZNegFace(chunk.opaqueMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addZNegFace(chunk.transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                        else {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addZNegFace(chunk.opaqueNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addZNegFace(chunk.transpNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                    }
                }
                else if (hasProp(chunk.blocks[i][j][k-1], TRANSPARENT) && (chunk.blocks[i][j][k-1] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 4);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addZNegFace(chunk.opaqueMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addZNegFace(chunk.transpMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addZNegFace(chunk.opaqueNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addZNegFace(chunk.transpNoShadowMesh, i, j, k, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
				//Z POSITIVE FACE
                if (k == CHUNKWIDTH - 1) {
                    if (hasSouthChunk && hasProp(southChunk->blocks[i][j][0], TRANSPARENT) && (southChunk->blocks[i][j][0] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                        vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 5);
                        if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addZPosFace(chunk.opaqueMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addZPosFace(chunk.transpMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                        else {
                            if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                                addZPosFace(chunk.opaqueNoShadowMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                            }
                            else {
                                addZPosFace(chunk.transpNoShadowMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                            }
                        }
                    }
                }
                else if (hasProp(chunk.blocks[i][j][k+1], TRANSPARENT) && (chunk.blocks[i][j][k+1] != chunk.blocks[i][j][k] || hasProp(chunk.blocks[i][j][k], RENDER_INSIDE))) {
                    vector<float> faceUV = getFaceUV(chunk.blocks[i][j][k], 5);
                    if (!hasProp(chunk.blocks[i][j][k], DONT_CAST_SHADOW)) {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addZPosFace(chunk.opaqueMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addZPosFace(chunk.transpMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                    else {
                        if (!hasProp(chunk.blocks[i][j][k], SEMI_TRANSPARENT)) {
                            addZPosFace(chunk.opaqueNoShadowMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                        }
                        else {
                            addZPosFace(chunk.transpNoShadowMesh, i, j, k + 1, faceUV[0], faceUV[1], faceUV[2]);
                        }
                    }
                }
            }
        }
    }

    if(!chunk.opaqueMesh.empty()) {
        if (chunk.opaqueMeshObj == NULL) {
            chunk.opaqueMeshObj = setupMeshTexture(chunk.opaqueMesh, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH), false, true, false);
            setMeshTextureFile(chunk.opaqueMeshObj, "sources/textures/all.png");
        }else {
            updateMeshTexture(chunk.opaqueMeshObj, chunk.opaqueMesh);
        }
        setMeshPosition(chunk.opaqueMeshObj, glm::vec3(chunk.key.x* CHUNKWIDTH, 0, chunk.key.y* CHUNKWIDTH));
    }
    if (!chunk.opaqueNoShadowMesh.empty()) {
        if (chunk.opaqueNoShadowMeshObj == NULL) {
            chunk.opaqueNoShadowMeshObj = setupMeshTexture(chunk.opaqueNoShadowMesh, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH), false, false, false);
            setMeshTextureFile(chunk.opaqueNoShadowMeshObj, "sources/textures/all.png");
        }
        else {
            updateMeshTexture(chunk.opaqueNoShadowMeshObj, chunk.opaqueNoShadowMesh);
        }
        setMeshPosition(chunk.opaqueNoShadowMeshObj, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH));
    }

    if (!chunk.transpMesh.empty()) {
        if (chunk.transpMeshObj == NULL) {
            chunk.transpMeshObj = setupMeshTexture(chunk.transpMesh, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH), true, true, false);
            setMeshTextureFile(chunk.transpMeshObj, "sources/textures/all.png");
        }
        else {
            updateMeshTexture(chunk.transpMeshObj, chunk.transpMesh);
        }
        setMeshPosition(chunk.transpMeshObj, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH));
    }
    if (!chunk.transpNoShadowMesh.empty()) {
        if (chunk.transpNoShadowMeshObj == NULL) {
            chunk.transpNoShadowMeshObj = setupMeshTexture(chunk.transpNoShadowMesh, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH), true, false, false);
            setMeshTextureFile(chunk.transpNoShadowMeshObj, "sources/textures/all.png");
        }
        else {
            updateMeshTexture(chunk.transpNoShadowMeshObj, chunk.transpNoShadowMesh);
        }
        setMeshPosition(chunk.transpNoShadowMeshObj, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH));
    }

    if (!chunk.plantMesh.empty()) {
        if (chunk.plantMeshObj == NULL) {
            chunk.plantMeshObj = setupMeshTexture(chunk.plantMesh, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH), false, true, true);
            setMeshTextureFile(chunk.plantMeshObj, "sources/textures/all.png");
        }
        else {
            updateMeshTexture(chunk.plantMeshObj, chunk.plantMesh);
        }
        setMeshPosition(chunk.plantMeshObj, glm::vec3(chunk.key.x * CHUNKWIDTH, 0, chunk.key.y * CHUNKWIDTH));
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

            if (chunk.opaqueNoShadowMeshObj != nullptr) {
                deleteMesh(chunk.opaqueNoShadowMeshObj);
                chunk.opaqueNoShadowMeshObj = nullptr;
            }
            chunk.opaqueNoShadowMesh.clear();
            chunk.opaqueNoShadowMesh.shrink_to_fit();

            if (chunk.transpNoShadowMeshObj != nullptr) {
                deleteMesh(chunk.transpNoShadowMeshObj);
                chunk.transpNoShadowMeshObj = nullptr;
            }
            chunk.transpNoShadowMesh.clear();
            chunk.transpNoShadowMesh.shrink_to_fit();

            if (chunk.plantMeshObj != nullptr) {
                deleteMesh(chunk.plantMeshObj);
                chunk.plantMeshObj = nullptr;
            }
            chunk.plantMesh.clear();
            chunk.plantMesh.shrink_to_fit();

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

            
            if (blockType != air && blockType != water) {
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
		camera.MovementSpeed = 100.0f;
        flyMode = true; 
    }
    if (camera.walkPressed) {
        camera.MovementSpeed = 6.0f;
        flyMode = false;
    }

    if (flyMode) {
        if (camera.forwardPressed)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (camera.backwardPressed)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (camera.leftPressed)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (camera.rightPressed)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (camera.downPressed)
            camera.ProcessKeyboard(DOWN, deltaTime);
        if (camera.upPressed)
            camera.ProcessKeyboard(UP, deltaTime);
    }
    else {
        glm::vec3 dPos = glm::vec3(0.0, 0.0, 0.0);
        if (camera.forwardPressed) {
            dPos += glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z)) * camera.MovementSpeed * deltaTime;
        }
        if (camera.backwardPressed) {
            dPos += glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z)) * -camera.MovementSpeed * deltaTime;
        }
        if (camera.rightPressed) {
            dPos += glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z)) * camera.MovementSpeed * deltaTime;
        }
        if (camera.leftPressed) {
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
            if (hasProp(chunkItLeft->second.blocks[localXLeft][blockHead][localZLeft], SOLID) || hasProp(chunkItLeft->second.blocks[localXLeft][blockBelly][localZLeft], SOLID) || hasProp(chunkItLeft->second.blocks[localXLeft][blockFoot][localZLeft], SOLID) ) {
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
            if (hasProp(chunkItRight->second.blocks[localXRight][blockHead][localZRight], SOLID) || hasProp(chunkItRight->second.blocks[localXRight][blockBelly][localZRight], SOLID) || hasProp(chunkItRight->second.blocks[localXRight][blockFoot][localZRight], SOLID)) {
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
            if (hasProp(chunkItForward->second.blocks[localXForward][blockHead][localZForward], SOLID) || hasProp(chunkItForward->second.blocks[localXForward][blockBelly][localZForward], SOLID) || hasProp(chunkItForward->second.blocks[localXForward][blockFoot][localZForward], SOLID) ) {
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
            if (hasProp(chunkItBackward->second.blocks[localXBackward][blockHead][localZBackward], SOLID) || hasProp(chunkItBackward->second.blocks[localXBackward][blockBelly][localZBackward], SOLID) || hasProp(chunkItBackward->second.blocks[localXBackward][blockFoot][localZBackward], SOLID)) {
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
                bool blockBelow = hasProp(chunkIt->second.blocks[localX][(int)(posY - playerHeight)][localZ], SOLID);
                
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
                bool blockAbove = hasProp(chunkIt->second.blocks[localX][(int)(posY)][localZ], SOLID);
                
                if (blockAbove) {
                    float desiredY = (float)((int)(posY));
                    camera.Position.y = desiredY;
                    camera.Yspeed = 0.0f;
                } else {
                    camera.Position.y = posY;
                }
            }
        }

        if (camera.spacePressed && isOnGround) {
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

void setAtmosphere() {
    int chunkX = static_cast<int>(floor((float)camera.Position.x / CHUNKWIDTH));
    int chunkZ = static_cast<int>(floor((float)camera.Position.z / CHUNKWIDTH));
    ChunkKey chunkKey = { chunkX, chunkZ };

    auto chunkIt = chunks.find(chunkKey);
    if (chunkIt != chunks.end() && chunkIt->second.isActive) {
        int localX = camera.Position.x - chunkX * CHUNKWIDTH;
        int localZ = camera.Position.z - chunkZ * CHUNKWIDTH;
		int blockY = static_cast<int>(floor(camera.Position.y));
        if (blockY < 0) return;
        if (blockY >= CHUNKHEIGHT-1) return;
        signed char blockType = chunkIt->second.blocks[localX][blockY][localZ];
        if (blockType == water) {
			camera.colorOverlay = glm::vec4(0.0f, 0.0f, 1.0f, 0.2f);
        }
        else {
            camera.colorOverlay = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}

int main() {
    SetupRender("Minecraft", &camera);
    

    camera.Position = glm::vec3(1.0, CHUNKHEIGHT-2 -20, 1.0);

    Light* sun = createLight(DIRECTIONAL, true);
    setLightColor(sun, glm::vec3(1.0, 1.0, 1.0));
    setLightIntensity(sun, 0.60);
    setLightDirection(sun, glm::vec3(-3.8f, 7.5f, -5.0f));

    Light* skyLight = createLight(DIRECTIONAL, false);
    setLightColor(skyLight, glm::vec3(0.5, 0.8, 0.9));
    setLightIntensity(skyLight, 0.15);
    setLightDirection(skyLight, glm::vec3(0.0f, 1.0f, 0.0f));

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
		setAtmosphere();
        

        renderScene();
    }
    terminateRender();
    return 0;
}
