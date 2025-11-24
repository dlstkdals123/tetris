#include "BlockMover.h"
#include "Block.h"

BlockMover::BlockMover(BlockRender& renderer, Board& board) : renderer(renderer), board(board){}

int BlockMover::move_block(BlockType* shape,Rotation& rotation,Position& pos,BlockType* next_shape) {

	pos.moveDown();	//블럭을 한칸 아래로 내림
    Block thisBlock(*shape, rotation, pos);
	if(board.isStrike(thisBlock) == 1)
	{
		if(pos.getY()<=0)	//게임오버
		{
			pos.moveUp();
			return 1;
		}
		pos.moveUp();
		board.mergeBlock(thisBlock);
		Block nextBlock(*next_shape);
		renderer.show_next_block(*next_shape);
		return 2;
	}
	return 0;
}    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴

void BlockMover::rotate_block(BlockType* shape,Rotation& rotation,Position& pos) {
    Rotation next_rotation(rotation.getNextAngle());
    Block nextBlock(*shape, next_rotation, pos);
    if(board.isStrike(nextBlock) == 0)
    {
        renderer.erase_cur_block(*shape,rotation,pos);
        rotation.rotateClockwise();
        renderer.show_cur_block(*shape,next_rotation,pos);
    }
}

 void BlockMover::movedLeft(BlockType* shape,Rotation& rotation,Position& pos) {
	Block thisBlock(*shape, rotation, pos);
	if(pos.getX()>1)
	{
		renderer.erase_cur_block(*shape, rotation, pos);
		pos.moveLeft();
		if(board.isStrike(thisBlock) == 1)
			pos.moveRight();
		renderer.show_cur_block(*shape,rotation,pos);
	}
 }

void BlockMover::movedRight(BlockType* shape,Rotation& rotation,Position& pos) {
	Block thisBlock(*shape, rotation, pos);
	if(pos.getX()<13)
	{
		renderer.erase_cur_block(*shape,rotation,pos);
		pos.moveRight();
		if(board.isStrike(thisBlock) == 1)
			pos.moveLeft();
		renderer.show_cur_block(*shape,rotation,pos);
	}
}