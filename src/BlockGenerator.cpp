#include "BlockGenerator.h"
#include "Block.h"
#include "GameConstants.h"
#include <cstdlib>

void BlockGenerator::make_logo_blocks(Block* block)
{
	int posX[GameConstants::LogoAnimation::BLOCK_COUNT] = { 
		GameConstants::LogoBlock::POS_X_1,
		GameConstants::LogoBlock::POS_X_2,
		GameConstants::LogoBlock::POS_X_3,
		GameConstants::LogoBlock::POS_X_4
	};
	int posY = GameConstants::LogoBlock::POS_Y;

	for (int i = 0; i < GameConstants::LogoAnimation::BLOCK_COUNT; i++) {
		Rotation rot(rand() % GameConstants::BlockRotation::MAX_ROTATIONS);
		BlockType type = static_cast<BlockType>(rand() % GameConstants::BlockType::COUNT);
		Position pos(posX[i], posY);

		block[i] = Block(type, rot, pos);
	}
}
