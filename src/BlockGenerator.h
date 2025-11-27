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
    const STAGE* stage_data;
    const gameState& gs;
public:
    BlockGenerator(const STAGE stage_data[], const gameState& gs): stage_data(stage_data) ,gs(gs) {};
    ~BlockGenerator() = default;
    
    BlockType make_new_block()
    {
        int i = rand()%100;
        if(i <= stage_data[gs.getLevel()].getStickRate())		
            return BlockType::I;

        int shape = (rand()%6)+1;
        return static_cast<BlockType> (shape);
    }

    void make_logo_blocks(Block* block);
};