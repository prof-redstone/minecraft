#include <iostream>
#include "blocks.h"


bool hasProp(int block_id, BP prop) {
    return (BLOCK_PROP[block_id] & prop) != 0;
}

bool hasAllProp(int block_id, uint32_t properties) {
    return (BLOCK_PROP[block_id] & properties) == properties;
}

bool hasAnyProp(int block_id, uint32_t properties) {
    return (BLOCK_PROP[block_id] & properties) != 0;
}