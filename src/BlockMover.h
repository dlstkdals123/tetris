#include "Block.h"
#include "BlockRender.h"
#include "Board.h"
class BlockMover{
private:
    BlockRender renderer;
    Board board;
public:
    BlockMover(BlockRender& renderer, Board& board);
    ~BlockMover() = default;
    int move_block(BlockType* shape,Rotation& rotation,Position& pos,BlockType* next_shape);    //????????? 1?????? ???? ?????? ?????? 2?? ????
    void rotate_block(BlockType* shape,Rotation& rotation,Position& pos);

    void movedLeft(BlockType* shape,Rotation& rotation,Position& pos);

    void movedRight(BlockType* shape,Rotation& rotation,Position& pos);
};