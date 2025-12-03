#include "BlockRender.h"
#include "Utils.h"
#include "Block.h"
#include "Board.h"
#include <iostream>
using namespace std;

BlockRender::BlockRender(const gameState& gs, const Position& boardOffset, Board& board, bool isLeft): gs(gs), boardOffset(boardOffset), board(board), isLeft(isLeft) {};

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
				Utils::gotoxy((i+x)*2+boardOffset.getX(),j+y+boardOffset.getY(), isLeft);
				printf("■");
			}
		}
	}
	Utils::setColor(COLOR::BLACK);
	Utils::gotoxy(77,23, isLeft);
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
				Utils::gotoxy((i+x)*2+boardOffset.getX(),j+y+boardOffset.getY(), isLeft);
				printf("  ");
				//break;
				
			}
		}
	}
}

void BlockRender::show_next_block(Block& block) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
	Utils::setColor(COLOR::GRAY);
	
    int i,j;
    
	for(i=1;i<7;i++)
	{
		Utils::gotoxy(33, i, isLeft);
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

void BlockRender::show_ghost_block(const Block& block) {
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);

    Position pos = block.getPos();
    Rotation rotation = block.getRotation();
    int x = pos.getX();
    int y = pos.getY();
    int angle = rotation.getAngle();
    BlockType shape = block.getType();

    Utils::setColor(COLOR::GRAY);   // 고스트는 회색

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (j + y < 0) continue;

            if (BlockShape::SHAPES[static_cast<int>(shape)][angle][j][i] == 1) {
                Utils::gotoxy((i + x) * 2 + boardOffset.getX(),
                              j + y + boardOffset.getY(),
                              isLeft);
                printf("□");       // 빈 사각형으로
            }
        }
    }
    Utils::setColor(COLOR::BLACK);
}

void BlockRender::erase_ghost_block(const Block& block) {
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);

    Position pos = block.getPos();
    int x = pos.getX();
    int y = pos.getY();
    Rotation rotation = block.getRotation();
    int angle = rotation.getAngle();
    BlockType shape = block.getType();

    Utils::setColor(COLOR::BLACK);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (j + y < 0) continue;

            if (BlockShape::SHAPES[static_cast<int>(shape)][angle][j][i] == 1 && board.getCell(j + y, i + x) == 0) {
                Utils::gotoxy((i + x) * 2 + boardOffset.getX(),
                              j + y + boardOffset.getY(),
                              isLeft);
                printf("  ");
            }
        }
    }
}