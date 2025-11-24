#include "Block.h"
#include "BlockRender.h"
#include "Board.h"
class BlockMove{
private:

public:
    int move_block(BlockType* shape,Rotation& rotation,Position& pos,BlockType* next_shape, Board& board);    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴
    int rotate_block(BlockType shape,Rotation& rotation,Position& pos, BlockRender& renderer, Board& board);
};