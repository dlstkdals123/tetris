#include "BlockRender.h"
#include "Utils.h"
#include "Block.h"
#include "Board.h"
#include <iostream>
using namespace std;

BlockRender::BlockRender(const gameState& gs, const Position& boardOffset, bool isPlayer): gs(gs), boardOffset(boardOffset), isPlayer(isPlayer) {};

void BlockRender::show_cur_block(Block& block) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

  int i,j;
	Position pos = block.getPos();
	Rotation rotation = block.getRotation();
	int x = pos.getX();
	int y = pos.getY();
	int angle = rotation.getAngle();
	BlockType shape = block.getType();
	switch(shape)
	{
	case BlockType::I:
		Utils::setColor(COLOR::RED);
		break;
	case BlockType::O:
		Utils::setColor(COLOR::BLUE);
		break;
	case BlockType::T:
		Utils::setColor(COLOR::SKY_BLUE);
		break;
	case BlockType::L:
		Utils::setColor(COLOR::WHITE);
		break;
	case BlockType::J:
		Utils::setColor(COLOR::YELLOW);
		break;
	case BlockType::Z:
		Utils::setColor(COLOR::VOILET);
		break;
	case BlockType::S:
		Utils::setColor(COLOR::GREEN);
		break;
	}

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if( (j+y) <0)
				continue;

			if(BlockShape::SHAPES[static_cast<int>(shape)][angle][j][i] == 1)
			{
				Utils::gotoxy((i+x)*2+boardOffset.getX(),j+y+boardOffset.getY(), isPlayer);
				printf("■");
			}
		}
	}
	Utils::setColor(COLOR::BLACK);
	Utils::gotoxy(77,23, isPlayer);
}

void BlockRender::erase_cur_block(Block& block) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

	Position pos = block.getPos();
    int i,j;
	int x = pos.getX();
	int y = pos.getY();
	int angle = block.getRotation();

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(BlockShape::SHAPES[static_cast<int>(block.getType())][angle][j][i] == 1)
			{
				Utils::gotoxy((i+x)*2+boardOffset.getX(),j+y+boardOffset.getY(), isPlayer);
				printf("  ");
				//break;
				
			}
		}
	}
}

void BlockRender::show_next_block(Block& block) {
		std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
		
    int i,j;
    Utils::setColor(static_cast<COLOR>((gs.getLevel() + 1) % 6 + 1));
	for(i=1;i<7;i++)
	{
		Utils::gotoxy(33, i, isPlayer);
		for(j=0;j<6;j++)
		{
			if(i==1 || i==6 || j==0 || j==5)
			{
				printf("■");				
			}else{
				printf("  ");
			}

		}
	}
	Position next_pos(15, 1);
	Rotation next_rotation(0);
	Block nextBlock(block.getType(), next_rotation, next_pos);
	show_cur_block(nextBlock);
}