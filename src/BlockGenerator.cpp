#include "BlockGenerator.h"
#include "Block.h"
#include <cstdlib>

void BlockGenerator::make_logo_blocks(Block* block)
{
	int posX[4] = { 6,12,19,24 };
	int posY = 14;

	for (int i = 0; i < 4; i++) {
		Rotation rot(rand() % 4);
		BlockType type = static_cast<BlockType>(rand() % 7);
		Position pos(posX[i], posY);

		block[i] = Block(type, rot, pos);
	}
}
