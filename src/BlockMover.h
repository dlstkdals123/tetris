#include "Block.h"
#include "BlockRender.h"
#include "Board.h"
class BlockMover{
private:
    BlockRender renderer;
    Board board;
    BlockGenerator blockGenerator;
public:
    BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator);
    ~BlockMover() = default;
    int move_block(Block& block, Block& nextBlock);
    void rotate_block(Block& block);

    void movedLeft(Block& block);

    void movedRight(Block& block);
};