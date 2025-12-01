#pragma once
#include "BlockData.h"
#include "rotation.h"
#include "position.h"
#include "BoardConstants.h"

// 블록 객체 = shape, rotation, position 보관, 이동+회전,
class Block{
private:
    BlockType type;
    Rotation rotation;
    Position pos;
public:
    Block(BlockType type = BlockType::I) : type(type), rotation(0), pos(BoardConstants::BLOCK_START_X, BoardConstants::BLOCK_START_Y) {}
    Block(BlockType type, Rotation rotation, Position pos) : type(type), rotation(rotation), pos(pos) {}
    ~Block() = default;
    BlockType getType() const {
        return type;
    }
    int getRotation() const {
        return rotation.getAngle();
    }
    const Position& getPos() const {
        return pos;
    }
    void setPos(const Position& newPos) {
        this->pos = newPos;
    }
    void moveDown() {
        this->pos.moveDown();
    }
    void moveUp() {
        this->pos.moveUp();
    }
    void moveLeft() {
        this->pos.moveLeft();
    }
    void moveRight() {
        this->pos.moveRight();
    }
    void rotate() {
        this->rotation.setAngle(rotation.getNextAngle());
    }
    void block_start() {
        this->pos.set(BoardConstants::BLOCK_START_X, BoardConstants::BLOCK_START_Y);
        this->rotation.reset();
    }
};