#include "Block.h"
#include "BlockRender.h"
#include "Board.h"
#include "BlockGenerator.h"
#include "gameState.h"
#include "GameConstants.h"
class BlockMover{
private:
    BlockRender& renderer;
    Board& board;
    BlockGenerator& blockGenerator;
    gameState& gamestate;
    int gameMode; // 0: single, 1: vs ai, 2: vs player
    bool isLeftPlayer; // true: left player, false: right player/AI

    Block ghostBlock;
    bool hasGhost = false;

    
public:
    BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator, gameState& gamestate, int gameMode = GameConstants::GameMode::SINGLE_PLAYER, bool isLeftPlayer = true);
    ~BlockMover() = default;
    int move_block(Block& block, Block& nextBlock);
    void rotateBlock(Block& block);

    void movedLeft(Block& block);

    void movedRight(Block& block);
    
    void updateGhost(const Block& current);
    
    // 공격 처리를 위한 getter
    int getGameMode() const { return gameMode; }
    bool getIsLeftPlayer() const { return isLeftPlayer; }
};