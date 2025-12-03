#pragma once
#include "gameState.h"
#include "Block.h"
#include "Board.h"
#include "GameConstants.h"
#include "BoardConstants.h"

class BlockRender {
private:
    const gameState& gs;
    const Position& boardOffset{GameConstants::BoardOffset::X, GameConstants::BoardOffset::Y};
    Board& board;
    bool isLeft;

public:
    BlockRender(const gameState& gs, const Position& boardOffset, Board& board, bool isLeft = true);
    void show_cur_block(Block& block);
    void erase_cur_block(Block& block);
    void show_next_block(Block& block);
    void show_ghost_block(const Block& block);
    void erase_ghost_block(const Block& block);
};