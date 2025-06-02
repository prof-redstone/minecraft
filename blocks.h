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
	TRANSPARENT = 1 << 1, //glass, leaves, water, short grass
	RENDER_INSIDE = 1 << 2, //water
	PLANT = 1 << 3, //move with wind
	DONT_CAST_SHADOW = 1 << 4, //glass, water
	SEMI_TRANSPARENT = 1 << 5 //semi-transparent blocks like water
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
    TRANSPARENT|RENDER_INSIDE,        // oka_leaves
    SOLID,                            // cactus 
    SOLID,                            // spruce_log
    TRANSPARENT|RENDER_INSIDE,        // spruce_leaves
    TRANSPARENT|DONT_CAST_SHADOW|SEMI_TRANSPARENT,// water
    PLANT|TRANSPARENT|DONT_CAST_SHADOW,// short grass
};

bool hasProp(int block_id, BP prop);

bool hasAllProp(int block_id, uint32_t properties);

bool hasAnyProp(int block_id, uint32_t properties);
