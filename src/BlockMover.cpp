#include "BlockMover.h"
#include "Block.h"

BlockMover::BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator, gameState& gamestate) 
		: renderer(renderer), board(board), blockGenerator(blockGenerator), gamestate(gamestate){}

int BlockMover::move_block(Block& block, Block& nextBlock) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근을 막음

	renderer.erase_cur_block(block);
	block.moveDown();

	if(board.isStrike(block) == 1){
		if(block.getPos().getY() <= 0)
			return 1;

		block.moveUp();
		board.mergeBlock(block);
		int deletedLines = board.deleteFullLine();
		if(deletedLines > 0 ) {
			gamestate.addLines(deletedLines);
			for(int i=0; i<deletedLines; i++) {
				int score = 100 + gamestate.getLevel() * 10 + (rand() % 10);
				gamestate.addScore(score);
			}
		}
		board.draw(gamestate.getLevel());
		block = nextBlock;
		block.block_start();
		nextBlock = Block(blockGenerator.make_new_block());
		renderer.show_next_block(nextBlock);
		renderer.show_cur_block(block); 
		return 2;
	}
	renderer.show_cur_block(block); 
	return 0;
}    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴

void BlockMover::rotateBlock(Block& block) {
	Rotation rotation = block.getRotation();
	Rotation next_rotation(rotation.getNextAngle());
    Block nextBlock(block.getType(), next_rotation,block.getPos()); //임시객체
    if(board.isStrike(nextBlock) == 0)
    {
        renderer.erase_cur_block(block);
		block.rotate();
        renderer.show_cur_block(block);
    }
}

 void BlockMover::movedLeft(Block& block) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);// 스레드 동시 접근을 막음

	if(block.getPos().getX()>1)
	{
		renderer.erase_cur_block(block);
		block.moveLeft();
		if(board.isStrike(block) == 1)
			block.moveRight();
		renderer.show_cur_block(block);
	}
 }

void BlockMover::movedRight(Block& block) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);// 스레드 동시 접근을 막음
	
	if(block.getPos().getX()<13)
	{
		renderer.erase_cur_block(block);
		block.moveRight();
		if(board.isStrike(block) == 1)
			block.moveLeft();
		renderer.show_cur_block(block);
	}
}