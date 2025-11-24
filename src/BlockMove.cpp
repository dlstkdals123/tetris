#include "BlockMove.h"
#include "Block.h"

int BlockMove::move_block(BlockType* shape,Rotation& rotation,Position& pos,BlockType* next_shape, Board& board) {
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
		// merge_block(*shape,rotation,pos);
		// *shape = *next_shape;
		// *next_shape = make_new_block();
		
		// block_start(*shape,rotation,pos);	//rotation,pos는 참조임
		// show_next_block(*next_shape);
		return 2;
	}
	return 0;
}    //게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴

int BlockMove::rotate_block(BlockType shape,Rotation& rotation,Position& pos, BlockRender& renderer, Board& board) {
    Rotation next_rotation(rotation.getNextAngle());
    Block nextBlock(shape, next_rotation, pos);
    if(board.isStrike(nextBlock) == 0)
    {
        renderer.erase_cur_block(shape,rotation,pos);
        rotation.rotateClockwise();
        renderer.show_cur_block(shape,rotation,pos);
    }
}