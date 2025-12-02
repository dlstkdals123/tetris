#include "Block.h"
#include "BlockRender.h"
#include "Board.h"
#include "BlockGenerator.h"
#include "gameState.h"
class BlockMover{
private:
    BlockRender& renderer;
    Board& board;
    BlockGenerator& blockGenerator;
    gameState& gamestate;

    Block ghostBlock;
    bool hasGhost = false;

    void updateGhost(const Block& current);
public:
    BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator, gameState& gamestate);
    ~BlockMover() = default;
    int move_block(Block& block, Block& nextBlock);
    void rotateBlock(Block& block);

    void movedLeft(Block& block);

    void movedRight(Block& block);
};