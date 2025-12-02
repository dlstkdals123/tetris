#pragma once
#include <iostream>
#include <cstdlib>
#include "rotation.h"
#include "position.h"
#include "BlockData.h"
#include "STAGE.h"
#include "gameState.h"
#include "Block.h"
class BlockGenerator{
private:
    const gameState& gs;
public:
    BlockGenerator(const gameState& gs): gs(gs) {};
    ~BlockGenerator() = default;
    
    BlockType make_new_block()
    {
        int i = rand()%100;
        STAGE stage_data = STAGE::getStage(gs.getLevel());
        if(i <= stage_data.getStickRate())		
            return BlockType::I;

        int shape = (rand()%6)+1;
        return static_cast<BlockType> (shape);
    }

    static void make_logo_blocks(Block* block);
};