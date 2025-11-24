#pragma once
#include "gameState.h"
#include "Block.h"
class BlockRender {
private:
    const gameState& gs;
    const Position& boardOffset{5,1};
public:
    BlockRender(const gameState& gs, const Position& boardOffset);
    void show_cur_block(Block& block);
    void erase_cur_block(Block& block);
    void show_next_block(Block& block);
};