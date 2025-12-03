#include "BlockMover.h"
#include "Block.h"
#include "BoardConstants.h"
#include "Utils.h"
#include "GameConstants.h"

BlockMover::BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator, gameState& gamestate, int gameMode, bool isLeftPlayer) 
		: renderer(renderer), board(board), blockGenerator(blockGenerator), gamestate(gamestate), gameMode(gameMode), isLeftPlayer(isLeftPlayer){}

int BlockMover::move_block(Block& block, Block& nextBlock) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
	renderer.erase_cur_block(block);
    
    // 현재 위치에서 이미 충돌하는지 확인 (공격 라인으로 인한 충돌)
    if (board.isStrike(block) == BoardConstants::STRIKE_TRUE) {
        // 공격 라인이 올라와서 블록과 겹친 상황
        // 충돌이 해소될 때까지 한 칸씩 위로 이동
        while (board.isStrike(block) == BoardConstants::STRIKE_TRUE) {
            block.moveUp();
            
            // 게임오버 체크 (블록이 화면 밖으로 나갔는지)
            if (block.getPos().getY() <= GameConstants::Simulation::GAME_OVER_Y_THRESHOLD) {
                return GameConstants::GameState::GAME_OVER;
            }
        }
        
        // 충돌이 해소되었으므로, 블록이 바닥에 부딪힌 것처럼 착지 처리
        board.mergeBlock(block);
        auto lineResult = board.deleteFullLine();
        int deletedLines = lineResult.first; // 총 삭제된 줄 수
        int attackableLines = lineResult.second; // attack 가능한 줄 수
        
        if(deletedLines > 0 ) {
            gamestate.addLines(deletedLines);
            for(int i=0; i<deletedLines; i++) {
                int score = GameConstants::Score::BASE_SCORE + gamestate.getLevel() * GameConstants::Score::LEVEL_MULTIPLIER + (rand() % GameConstants::Score::RANDOM_BONUS_MAX);
                gamestate.addScore(score);
            }
            
            // Attack 라인 추가 (VS AI 또는 VS Player 모드일 때만)
            // attack 가능한 줄 수만큼만 attack
            if ((gameMode == GameConstants::GameMode::VS_AI || gameMode == GameConstants::GameMode::VS_PLAYER) && attackableLines > 0) {
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
        return GameConstants::GameState::BLOCK_LANDED;
    }
    
    // 기존 로직: 아래로 이동
    block.moveDown();

    if(board.isStrike(block) == BoardConstants::STRIKE_TRUE){
        if(block.getPos().getY() <= GameConstants::Simulation::GAME_OVER_Y_THRESHOLD)
            return GameConstants::GameState::GAME_OVER;

        block.moveUp();
        board.mergeBlock(block);
        auto lineResult = board.deleteFullLine();
        int deletedLines = lineResult.first; // 총 삭제된 줄 수
        int attackableLines = lineResult.second; // attack 가능한 줄 수
        
        if(deletedLines > 0 ) {
            gamestate.addLines(deletedLines);
            for(int i=0; i<deletedLines; i++) {
                int score = GameConstants::Score::BASE_SCORE + gamestate.getLevel() * GameConstants::Score::LEVEL_MULTIPLIER + (rand() % GameConstants::Score::RANDOM_BONUS_MAX);
                gamestate.addScore(score);
            }
            
            // Attack 라인 추가 (VS AI 또는 VS Player 모드일 때만)
            // attack 가능한 줄 수만큼만 attack
            if ((gameMode == GameConstants::GameMode::VS_AI || gameMode == GameConstants::GameMode::VS_PLAYER) && attackableLines > 0) {
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
        return GameConstants::GameState::BLOCK_LANDED;
    }

    updateGhost(block);

    renderer.show_cur_block(block); 
    return GameConstants::GameState::CONTINUE;
}    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴

void BlockMover::rotateBlock(Block& block) {
	Rotation rotation = block.getRotation();
	Rotation next_rotation(rotation.getNextAngle());
    Block nextBlock(block.getType(), next_rotation,block.getPos()); //임시객체
    if(board.isStrike(nextBlock) == BoardConstants::STRIKE_FALSE)
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
		if(board.isStrike(block) == BoardConstants::STRIKE_TRUE)
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
		if(board.isStrike(block) == BoardConstants::STRIKE_TRUE)
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
        if (board.isStrike(ghostBlock) == BoardConstants::STRIKE_TRUE) {
            ghostBlock.moveUp();
            break;
        }
    }

    renderer.show_ghost_block(ghostBlock);
    hasGhost = true;
}