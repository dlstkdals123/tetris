#include "BlockMover.h"
#include "Block.h"
#include "BoardConstants.h"
#include "Utils.h"

BlockMover::BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator, gameState& gamestate, int gameMode, bool isLeftPlayer) 
		: renderer(renderer), board(board), blockGenerator(blockGenerator), gamestate(gamestate), gameMode(gameMode), isLeftPlayer(isLeftPlayer){}

int BlockMover::move_block(Block& block, Block& nextBlock) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
	renderer.erase_cur_block(block);
    block.moveDown();

    if(board.isStrike(block) == 1){
        if(block.getPos().getY() <= 0)
            return 1;

        block.moveUp();
        board.mergeBlock(block);
        auto lineResult = board.deleteFullLine();
        int deletedLines = lineResult.first; // 총 삭제된 줄 수
        int attackableLines = lineResult.second; // attack 가능한 줄 수
        
        if(deletedLines > 0 ) {
            gamestate.addLines(deletedLines);
            for(int i=0; i<deletedLines; i++) {
                int score = 100 + gamestate.getLevel() * 10 + (rand() % 10);
                gamestate.addScore(score);
            }
            
            // Attack 라인 추가 (mode=1 또는 mode=2일 때만)
            // attack 가능한 줄 수만큼만 attack
            if ((gameMode == 1 || gameMode == 2) && attackableLines > 0) {
                Board* opponentBoard = isLeftPlayer ? Utils::rightPlayerBoard : Utils::leftPlayerBoard;
                
                if (opponentBoard != nullptr) {
                    opponentBoard->addAttackLines(attackableLines);
                    opponentBoard->draw(gamestate.getLevel());
                }
            }
        }
        board.draw(gamestate.getLevel());
		hasGhost = false;
        block = nextBlock;
        block.block_start();
        nextBlock = Block(blockGenerator.make_new_block());
        renderer.show_next_block(nextBlock);        
		updateGhost(block);

        renderer.show_cur_block(block); 
        return 2;
    }

    updateGhost(block);

    renderer.show_cur_block(block); 
    return 0;
}    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴

void BlockMover::rotateBlock(Block& block) {
	Rotation rotation = block.getRotation();
	Rotation next_rotation(rotation.getNextAngle());
    Block nextBlock(block.getType(), next_rotation,block.getPos()); //임시객체
    if(board.isStrike(nextBlock) == 0)
    {
		std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
        renderer.erase_cur_block(block);
		block.rotate();
		updateGhost(block);
        renderer.show_cur_block(block);
    }
}

 void BlockMover::movedLeft(Block& block) {
	if(block.getPos().getX() > BoardConstants::MIN_COLUMN)
	{
		std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
		renderer.erase_cur_block(block);
		block.moveLeft();
		if(board.isStrike(block) == 1)
			block.moveRight();

		updateGhost(block);
		renderer.show_cur_block(block);
	}
 }

void BlockMover::movedRight(Block& block) {
	if(block.getPos().getX() < BoardConstants::RIGHT_WALL)
	{
		std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
		renderer.erase_cur_block(block);
		block.moveRight();
		if(board.isStrike(block) == 1)
			block.moveLeft();

		updateGhost(block);
		renderer.show_cur_block(block);
	}
}

void BlockMover::updateGhost(const Block& current) {
	if (hasGhost) {
        renderer.erase_ghost_block(ghostBlock);
    }

    ghostBlock = current;

    while (true) {
        ghostBlock.moveDown();
        if (board.isStrike(ghostBlock) == 1) {
            ghostBlock.moveUp();
            break;
        }
    }

    renderer.show_ghost_block(ghostBlock);
    hasGhost = true;
}