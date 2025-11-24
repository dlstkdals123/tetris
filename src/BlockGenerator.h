#pragma once
#include <iostream>
#include <cstdlib>
#include "rotation.h"
#include "position.h"
#include "BlockData.h"
#include "STAGE.h"
#include "gameState.h"
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
        if(i <= stage_data[gs.getLevel()].getStickRate())		//막대기 나올확률 계산
            return BlockType::I;							//막대기 모양으로 리턴

        int shape = (rand()%6)+1;		//shape에는 1~6의 값이 들어감
        return static_cast<BlockType> (shape);
    }
};