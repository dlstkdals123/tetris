#include "BlockRender.h"
#include "Utils.h"
#include "Block.h"
#include "Board.h"
#include "GameConstants.h"
#include "BlockData.h"

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

    for(i=0;i<GameConstants::BlockRender::SHAPE_SIZE;i++)
    {
        for(j=0;j<GameConstants::BlockRender::SHAPE_SIZE;j++)
        {
            if( (j+y) < GameConstants::Simulation::GAME_OVER_Y_THRESHOLD)
                continue;

			if(BlockShape::getCell(static_cast<int>(shape), angle, j, i) == BlockShapeConstants::CELL_FILLED)
			{
				Utils::gotoxy((i+x)*GameConstants::BlockRender::X_COORD_MULTIPLIER+boardOffset.getX(),j+y+boardOffset.getY(), isLeft);
				printf("■");
			}
		}
	}
	Utils::setColor(COLOR::BLACK);
	Utils::gotoxy(GameConstants::BlockRender::CURSOR_X,GameConstants::BlockRender::CURSOR_Y, isLeft);
}

void BlockRender::erase_cur_block(Block& block) {
	std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

    Position pos = block.getPos();
    int i,j;
    int x = pos.getX();
    int y = pos.getY();
    int angle = block.getRotation();

	for(i=0;i<GameConstants::BlockRender::SHAPE_SIZE;i++)
	{
		for(j=0;j<GameConstants::BlockRender::SHAPE_SIZE;j++)
		{
			if(BlockShape::getCell(static_cast<int>(block.getType()), angle, j, i) == BlockShapeConstants::CELL_FILLED)
			{
				Utils::gotoxy((i+x)*GameConstants::BlockRender::X_COORD_MULTIPLIER+boardOffset.getX(),j+y+boardOffset.getY(), isLeft);
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
    
	for(i=GameConstants::NextBlockBox::START_Y;i<GameConstants::NextBlockBox::END_Y + 1;i++)
	{
		Utils::gotoxy(GameConstants::NextBlockBox::X_POS, i, isLeft);
		for(j=0;j<GameConstants::NextBlockBox::WIDTH;j++)
		{
			if(i==GameConstants::NextBlockBox::BORDER_TOP_Y || i==GameConstants::NextBlockBox::BORDER_BOTTOM_Y || j==GameConstants::NextBlockBox::BORDER_LEFT_X || j==GameConstants::NextBlockBox::BORDER_RIGHT_X)
			{
				printf("■ ");				
			}else{
				printf("  ");
			}

		}
	}
	Position next_pos(GameConstants::NextBlock::POS_X, GameConstants::NextBlock::POS_Y);
	Rotation next_rotation(GameConstants::BlockRotation::INITIAL_ROTATION);
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

    for (int i = 0; i < GameConstants::BlockRender::SHAPE_SIZE; ++i) {
        for (int j = 0; j < GameConstants::BlockRender::SHAPE_SIZE; ++j) {
            if (j + y < GameConstants::Simulation::GAME_OVER_Y_THRESHOLD) continue;

            if (BlockShape::getCell(static_cast<int>(shape), angle, j, i) == BlockShapeConstants::CELL_FILLED) {
                Utils::gotoxy((i + x) * GameConstants::BlockRender::X_COORD_MULTIPLIER + boardOffset.getX(),
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
    for (int i = 0; i < GameConstants::BlockRender::SHAPE_SIZE; ++i) {
        for (int j = 0; j < GameConstants::BlockRender::SHAPE_SIZE; ++j) {
            if (j + y < GameConstants::Simulation::GAME_OVER_Y_THRESHOLD) continue;

            if (BlockShape::getCell(static_cast<int>(shape), angle, j, i) == BlockShapeConstants::CELL_FILLED && board.getCell(j + y, i + x) == GameConstants::Simulation::NO_COLLISION) {
                Utils::gotoxy((i + x) * GameConstants::BlockRender::X_COORD_MULTIPLIER + boardOffset.getX(),
                              j + y + boardOffset.getY(),
                              isLeft);
                printf("  ");
            }
        }
    }
}
