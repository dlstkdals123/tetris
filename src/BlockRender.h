#pragma once
#include "gameState.h"
#include "Block.h"
#include "Board.h"

class BlockRender {
private:
    const gameState& gs;
    const Position& boardOffset{5,1};
    bool isLeft;
    Board board;

public:
    BlockRender(const gameState& gs, const Position& boardOffset, bool isLeft = true);
    void show_cur_block(Block& block);
    void erase_cur_block(Block& block);
    void show_next_block(Block& block);
    void show_ghost_block(const Block& block);
    void erase_ghost_block(const Block& block);
    void draw_next_block_frame(bool isPlay);
};