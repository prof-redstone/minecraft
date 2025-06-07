#pragma once
#include <vector>
#include "blocks.h"


//3 co x y z, block_type, should replace existing block
inline const std::vector<std::tuple<int, int, int, int, int>> oak_tree_1 = {

    {0, 1, 0, oak_log, 1},
    {0, 2, 0, oak_log, 1},
    {0, 3, 0, oak_log, 1},
    {0, 4, 0, oak_log, 1},

    {1, 3, 0, oak_leaves, 0},
    {2, 3, 0, oak_leaves, 0},
    {1, 3, 1, oak_leaves, 0},
    {1, 3, -1, oak_leaves, 0},
    {0, 3, 1, oak_leaves, 0},
    {0, 3, -1, oak_leaves, 0},
    {0, 3, 2, oak_leaves, 0},
    {0, 3, -2, oak_leaves, 0},
    {-1, 3, 1, oak_leaves, 0},
    {-1, 3, 0, oak_leaves, 0},
    {-2, 3, 0, oak_leaves, 0},
    {-1, 3, -1, oak_leaves, 0},
    {1, 4, 0, oak_leaves, 0},
    {2, 4, 0, oak_leaves, 0},
    {1, 4, 1, oak_leaves, 0},
    {1, 4, -1, oak_leaves, 0},
    {0, 4, 1, oak_leaves, 0},
    {0, 4, -1, oak_leaves, 0},
    {0, 4, 2, oak_leaves, 0},
    {0, 4, -2, oak_leaves, 0},
    {-1, 4, 1, oak_leaves, 0},
    {-1, 4, 0, oak_leaves, 0},
    {-2, 4, 0, oak_leaves, 0},
    {-1, 4, -1, oak_leaves, 0},
    {0, 5, 0, oak_leaves, 0},
    {1, 5, 0, oak_leaves, 0},
    {-1, 5, 0, oak_leaves, 0},
    {0, 5, 1, oak_leaves, 0},
    {0, 5, -1, oak_leaves, 0},
    {0, 6, 0, oak_leaves, 0}
};

inline const std::vector<std::tuple<int, int, int, int, int>> oak_tree_2 = {
    {0, 1, 0, oak_log, 1},
    {0, 2, 0, oak_log, 1},
    {0, 3, 0, oak_log, 1},
    {1, 2, 0, oak_leaves, 0},
    {-1, 2, 0, oak_leaves, 0},
    {0, 2, 1, oak_leaves, 0},
    {0, 2, -1, oak_leaves, 0},
    {1, 3, 0, oak_leaves, 0},
    {1, 3, 1, oak_leaves, 0},
    {1, 3, -1, oak_leaves, 0},
    {0, 3, 1, oak_leaves, 0},
    {0, 3, -1, oak_leaves, 0},
    {-1, 3, 1, oak_leaves, 0},
    {-1, 3, 0, oak_leaves, 0},
    {-1, 3, -1, oak_leaves, 0},
    {0, 4, 0, oak_leaves, 0},
    {1, 4, 0, oak_leaves, 0},
    {-1, 4, 0, oak_leaves, 0},
    {0, 4, 1, oak_leaves, 0},
    {0, 4, -1, oak_leaves, 0}
};

inline const std::vector<std::tuple<int, int, int, int, int>> oak_tree_3 = {

    {0, 1, 0, oak_log, 1},
    {0, 2, 0, oak_log, 1},
    {0, 3, 0, oak_log, 1},
    {0, 4, 0, oak_log, 1},

    {1, 3, 0, oak_leaves, 0},
    {2, 3, 0, oak_leaves, 0},
    {2, 3, 1, oak_leaves, 0},
    {2, 3, -1, oak_leaves, 0},
    {1, 3, 1, oak_leaves, 0},
    {1, 3, -1, oak_leaves, 0},
    {0, 3, 1, oak_leaves, 0},
    {0, 3, -1, oak_leaves, 0},
    {0, 3, 2, oak_leaves, 0},
    {1, 3, 2, oak_leaves, 0},
    {-1, 3, 2, oak_leaves, 0},
    {0, 3, -2, oak_leaves, 0},
    {1, 3, -2, oak_leaves, 0},
    {-1, 3, -2, oak_leaves, 0},
    {-1, 3, 1, oak_leaves, 0},
    {-1, 3, 0, oak_leaves, 0},
    {-2, 3, 0, oak_leaves, 0},
    {-2, 3, 1, oak_leaves, 0},
    {-2, 3, -1, oak_leaves, 0},
    {-1, 3, -1, oak_leaves, 0},

    {1, 4, 0, oak_leaves, 0},
    {2, 4, 0, oak_leaves, 0},
    {1, 4, 1, oak_leaves, 0},
    {1, 4, -1, oak_leaves, 0},
    {0, 4, 1, oak_leaves, 0},
    {0, 4, -1, oak_leaves, 0},
    {0, 4, 2, oak_leaves, 0},
    {0, 4, -2, oak_leaves, 0},
    {-1, 4, 1, oak_leaves, 0},
    {-1, 4, 0, oak_leaves, 0},
    {-2, 4, 0, oak_leaves, 0},
    {-1, 4, -1, oak_leaves, 0},

    {0, 5, 0, oak_leaves, 0},
    {1, 5, 0, oak_leaves, 0},
    {-1, 5, 0, oak_leaves, 0},
    {0, 5, 1, oak_leaves, 0},
    {0, 5, -1, oak_leaves, 0},
    {0, 6, 0, oak_leaves, 0}
};

inline const std::vector<std::tuple<int, int, int, int, int>> oak_tree_4 = {
    {1, 1, 0, oak_leaves, 0},
    {0, 1, 1, oak_leaves, 0},
    {0, 1, -1, oak_leaves, 0},
    {-1, 1, 0, oak_leaves, 0},
    {0, 2, 0, oak_leaves, 0},
};


inline const std::vector<std::tuple<int, int, int, int, int>> spruce_tree_1 = {

    {0, 1, 0, spruce_log, 1},
    {0, 2, 0, spruce_log, 1},
    {0, 3, 0, spruce_log, 1},
    {0, 4, 0, spruce_log, 1},
    {0, 5, 0, spruce_log, 1},
    {0, 6, 0, spruce_log, 1},
    {0, 7, 0, spruce_log, 1},
    {0, 8, 0, spruce_log, 1},

    {1, 3, 0, spruce_leaves, 0},
    {2, 3, 0, spruce_leaves, 0},
    {2, 3, 1, spruce_leaves, 0},
    {2, 3, -1, spruce_leaves, 0},
    {1, 3, 1, spruce_leaves, 0},
    {1, 3, -1, spruce_leaves, 0},
    {0, 3, 1, spruce_leaves, 0},
    {0, 3, -1, spruce_leaves, 0},
    {0, 3, 2, spruce_leaves, 0},
    {1, 3, 2, spruce_leaves, 0},
    {-1, 3, 2, spruce_leaves, 0},
    {0, 3, -2, spruce_leaves, 0},
    {1, 3, -2, spruce_leaves, 0},
    {-1, 3, -2, spruce_leaves, 0},
    {-1, 3, 1, spruce_leaves, 0},
    {-1, 3, 0, spruce_leaves, 0},
    {-2, 3, 0, spruce_leaves, 0},
    {-2, 3, 1, spruce_leaves, 0},
    {-2, 3, -1, spruce_leaves, 0},
    {-1, 3, -1, spruce_leaves, 0},

    {1, 4, 0, spruce_leaves, 0},
    {-1, 4, 0, spruce_leaves, 0},
    {0, 4, 1, spruce_leaves, 0},
    {0, 4, -1, spruce_leaves, 0},


    {1, 5, 0, spruce_leaves, 0},
    {2, 5, 0, spruce_leaves, 0},
    {1, 5, 1, spruce_leaves, 0},
    {1, 5, -1, spruce_leaves, 0},
    {0, 5, 1, spruce_leaves, 0},
    {0, 5, -1, spruce_leaves, 0},
    {0, 5, 2, spruce_leaves, 0},
    {0, 5, -2, spruce_leaves, 0},
    {-1, 5, 1, spruce_leaves, 0},
    {-1, 5, 0, spruce_leaves, 0},
    {-2, 5, 0, spruce_leaves, 0},
    {-1, 5, -1, spruce_leaves, 0},

    {1, 6, 0, spruce_leaves, 0},
    {-1, 6, 0, spruce_leaves, 0},
    {0, 6, 1, spruce_leaves, 0},
    {0, 6, -1, spruce_leaves, 0},

    {1, 7, 0, spruce_leaves, 0},
    {1, 7, 1, spruce_leaves, 0},
    {1, 7, -1, spruce_leaves, 0},
    {0, 7, 1, spruce_leaves, 0},
    {0, 7, -1, spruce_leaves, 0},
    {-1, 7, 1, spruce_leaves, 0},
    {-1, 7, 0, spruce_leaves, 0},
    {-1, 7, -1, spruce_leaves, 0},

    {1, 8, 0, spruce_leaves, 0},
    {-1, 8, 0, spruce_leaves, 0},
    {0, 8, 1, spruce_leaves, 0},
    {0, 8, -1, spruce_leaves, 0},

    {0, 9, 0, spruce_leaves, 0}
};

inline const std::vector<std::tuple<int, int, int, int, int>> spruce_tree_2 = {
    {0, 1, 0, spruce_log, 1},
    {0, 2, 0, spruce_log, 1},
    {0, 3, 0, spruce_log, 1},
    {0, 4, 0, spruce_log, 1},
    {0, 5, 0, spruce_log, 1},
    
    {1, 2, 0, spruce_leaves, 0},
    {2, 2, 0, spruce_leaves, 0},
    {1, 2, 1, spruce_leaves, 0},
    {1, 2, -1, spruce_leaves, 0},
    {0, 2, 1, spruce_leaves, 0},
    {0, 2, -1, spruce_leaves, 0},
    {0, 2, 2, spruce_leaves, 0},
    {0, 2, -2, spruce_leaves, 0},
    {-1, 2, 1, spruce_leaves, 0},
    {-1, 2, 0, spruce_leaves, 0},
    {-2, 2, 0, spruce_leaves, 0},
    {-1, 2, -1, spruce_leaves, 0},
    
    {1, 3, 0, spruce_leaves, 0},
    {1, 3, 1, spruce_leaves, 0},
    {1, 3, -1, spruce_leaves, 0},
    {0, 3, 1, spruce_leaves, 0},
    {0, 3, -1, spruce_leaves, 0},
    {-1, 3, 1, spruce_leaves, 0},
    {-1, 3, 0, spruce_leaves, 0},
    {-1, 3, -1, spruce_leaves, 0},
    
    {1, 4, 0, spruce_leaves, 0},
    {-1, 4, 0, spruce_leaves, 0},
    {0, 4, 1, spruce_leaves, 0},
    {0, 4, -1, spruce_leaves, 0},
    
    {0, 5, 1, spruce_leaves, 0},
    {0, 5, -1, spruce_leaves, 0},
    {1, 5, 0, spruce_leaves, 0},
    {-1, 5, 0, spruce_leaves, 0},

    {0, 6, 0, spruce_leaves, 0}
};

inline const std::vector<std::tuple<int, int, int, int, int>> spruce_tree_3 = {
    
    {0, 1, 0, spruce_log, 1},
    {0, 2, 0, spruce_log, 1},
    {0, 3, 0, spruce_log, 1},
    {0, 4, 0, spruce_log, 1},

    {1, 2, 0, spruce_leaves, 0}, 
    {-1, 2, 0, spruce_leaves, 0},
    {2, 2, 0, spruce_leaves, 0}, 
    {-2, 2, 0, spruce_leaves, 0},
    {0, 2, 1, spruce_leaves, 0}, 
    {0, 2, -1, spruce_leaves, 0},
    {0, 2, 2, spruce_leaves, 0}, 
    {0, 2, -2, spruce_leaves, 0},
    {1, 2, 1, spruce_leaves, 0}, 
    {1, 2, -1, spruce_leaves, 0},
    {-1, 2, 1, spruce_leaves, 0}, 
    {-1, 2, -1, spruce_leaves, 0},

    {1, 3, 0, spruce_leaves, 0},
    {1, 3, 1, spruce_leaves, 0},
    {1, 3, -1, spruce_leaves, 0},
    {0, 3, 1, spruce_leaves, 0},
    {0, 3, -1, spruce_leaves, 0},
    {-1, 3, 0, spruce_leaves, 0},
    {-1, 3, -1, spruce_leaves, 0},
    {-1, 3, 1, spruce_leaves, 0},

    {1, 4, 0, spruce_leaves, 0},
    {-1, 4, 0, spruce_leaves, 0},
    {0, 4, 1, spruce_leaves, 0},
    {0, 4, -1, spruce_leaves, 0},

    {0, 5, 0, spruce_leaves, 0}
};

inline const std::vector<std::tuple<int, int, int, int, int>> spruce_tree_4 = {
    {1, 1, 0, spruce_leaves, 0},
    {0, 1, 1, spruce_leaves, 0},
    {0, 1, -1, spruce_leaves, 0},
    {-1, 1, 0, spruce_leaves, 0},
    {0, 2, 0, spruce_leaves, 0},
};



inline const std::vector<std::tuple<int, int, int, int, int>> cactus_1 = {
    {0, 1, 0, cactus, 1},
    {0, 2, 0, cactus, 1},
    {0, 3, 0, cactus, 1}
};


inline const std::vector<std::tuple<int, int, int, int, int>> cactus_2 = {
    {0, 1, 0, cactus, 1},
    {0, 2, 0, cactus, 1},
    {0, 3, 0, cactus, 1},
    {0, 4, 0, cactus, 1}
};

inline const std::vector<std::tuple<int, int, int, int, int>> cactus_3 = {
    {0, 1, 0, cactus, 1},
    {0, 2, 0, cactus, 1}
};