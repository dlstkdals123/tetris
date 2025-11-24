#pragma once
#include "BlockData.h"
#include "rotation.h"
#include "position.h"

// 블록 객체 = shape, rotation, position 보관, 이동+회전,
class Block{
private:
    BlockType type;
    Rotation rotation;
    Position pos;
public:
    Block(BlockType type = BlockType::I) : type(type), rotation(0), pos(5, -4) {}
    Block(BlockType type, Rotation rotation, Position pos) : type(type), rotation(rotation), pos(pos) {}

    BlockType getType() const {
        return type;
    }
    int getRotation() const {
        return rotation.getAngle();
    }
    const Position& getPos() const {
        return pos;
    }
    Block moveDown() {
        Block b = *this;
        b.pos.moveDown();
        return b;
    }
    void block_start(Rotation& rotation,Position& pos) {
	    pos.set(5, -4);
	    rotation.reset();
    }
};