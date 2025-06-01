#pragma once
#include <iostream>

#define air 0
#define stone 1
#define dirt 2
#define grass 3
#define sand 4
#define snowy_grass 5
#define glass 6
#define diamond 7
#define oak_planks 8
#define oak_log 9
#define oak_leaves 10
#define cactus 11
#define spruce_log 12
#define spruce_leaves 13
#define water 14
#define short_grass 15


enum BP {//block properties
	SOLID = 1 << 0,
	TRANSPARENT = 1 << 1, //glass, leaves
	RENDER_INSIDE = 1 << 2, //water
	PLANT = 1 << 3, //move with wind
	DONT_CAST_SHADOW = 1 << 4 //glass, water
};

const std::uint32_t BLOCK_PROP[] = {
    TRANSPARENT,              // air 
    SOLID,                            // stone 
    SOLID,                            // dirt 
    SOLID,                            // grass 
    SOLID,                            // sand 
    SOLID,                            // snowy_gras 
    SOLID|TRANSPARENT|DONT_CAST_SHADOW,// glass 
    SOLID,                            // diamond 
    SOLID,                            // oka_plank
    SOLID,                            // oka_log 
    TRANSPARENT|RENDER_INSIDE,  // oka_leaves
    SOLID,                            // cactus 
    SOLID,                            // spruce_log
    TRANSPARENT|RENDER_INSIDE,  // spruce_leaves
    TRANSPARENT|DONT_CAST_SHADOW,     // water
    PLANT|TRANSPARENT|DONT_CAST_SHADOW,// short grass
};

bool hasProp(int block_id, BP prop);

bool hasAllProp(int block_id, uint32_t properties);

bool hasAnyProp(int block_id, uint32_t properties);




/*
#define air -1
#define glass -3
#define water -4
#define spruce_leaves -5
#define short_grass -6
#define oak_leaves -20
#define stone 0
#define oak_planks 1
#define dirt 2
#define grass 3
#define diamond 4
#define oak_log 5
#define sand 6
#define snowy_grass 7
#define cactus 8
#define spruce_log 9
*/