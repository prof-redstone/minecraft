#pragma once
#include <iostream>

#define air 0
#define stone 1
#define dirt 2
#define grass 3
#define sand 4
#define snowy_grass 5
#define glass 6
#define diamond_ore 7
#define oak_planks 8
#define oak_log 9
#define oak_leaves 10
#define cactus 11
#define spruce_log 12
#define spruce_leaves 13
#define water 14
#define short_grass 15
#define poppy 16
#define dandelion 17
#define dead_bush 18
#define sweet_berry 19
#define kelp 20
#define gravel 21

#define coal_ore 22
#define ice 23
#define bedrock 24
#define brain_coral 25
#define tube_coral 26
#define fire_coral_block 27
#define tube_coral_block 28

enum BP {//block properties
	SOLID = 1 << 0,
	TRANSPARENT = 1 << 1, //glass, leaves, water, short grass
	RENDER_INSIDE = 1 << 2, //water
	PLANT = 1 << 3, //move with wind
	DONT_CAST_SHADOW = 1 << 4, //glass, water
	SEMI_TRANSPARENT = 1 << 5, //semi-transparent blocks like water
	CROSS_SHAPE = 1 << 6, //blocks that have a cross shape like dandelion, poppy, short grass

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
    TRANSPARENT|RENDER_INSIDE|PLANT,  // oka_leaves
    SOLID,                            // cactus 
    SOLID,                            // spruce_log
    TRANSPARENT|RENDER_INSIDE | PLANT,// spruce_leaves
    TRANSPARENT|DONT_CAST_SHADOW|SEMI_TRANSPARENT,// water
    PLANT|TRANSPARENT | CROSS_SHAPE,// short grass
    PLANT|TRANSPARENT | CROSS_SHAPE,//poppy
    PLANT | TRANSPARENT | CROSS_SHAPE,//dandelion
    PLANT | TRANSPARENT  | CROSS_SHAPE,//dead bush
    PLANT | TRANSPARENT | CROSS_SHAPE,//sweet berry
    PLANT | TRANSPARENT | CROSS_SHAPE,//kelp
    SOLID,                            // gravel 
	SOLID,                            // coal_ore
    SOLID | TRANSPARENT ,   // ice
    SOLID,                            // bedrock
    PLANT | TRANSPARENT | CROSS_SHAPE, // brain_coral
    PLANT | TRANSPARENT | CROSS_SHAPE, // tube_coral
    SOLID, // fire_coral_block
	SOLID, // tube_coral_block

};

bool hasProp(int block_id, BP prop);

bool hasAllProp(int block_id, uint32_t properties);

bool hasAnyProp(int block_id, uint32_t properties);
