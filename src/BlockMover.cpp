#include "BlockMover.h"
#include "Block.h"

BlockMover::BlockMover(BlockRender& renderer, Board& board, BlockGenerator& blockGenerator) : renderer(renderer), board(board), blockGenerator(blockGenerator){}

int BlockMover::move_block(Block& block, Block& nextBlock) {
	renderer.erase_cur_block(block);
	Block testBlock = block; // 테스트 블록 생성
	testBlock.moveDown();
	if(board.isStrike(block) == 1){
		if(block.getPos().getY() <= 0);
			return 1;
		block.moveUp();
		board.mergeBlock(block);
		nextBlock.block_start();
		Block newBlock = blockGenerator.make_new_block();
		renderer.show_next_block(newBlock);
		return 2;
	}
	return 0;
}    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴

void BlockMover::rotate_block(Block& block) {
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
	if(block.getPos().getX()<13)
	{
		renderer.erase_cur_block(block);
		block.moveRight();
		if(board.isStrike(block) == 1)
			block.moveLeft();
		renderer.show_cur_block(block);
	}
}