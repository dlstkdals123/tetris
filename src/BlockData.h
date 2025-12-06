#pragma once

#include <cstdint>

//블록 모양 저장

enum class BlockType {
    I= 0, O, T, L, J, Z, S
};

// 블록 모양 배열 크기 상수
namespace BlockShapeConstants {
    constexpr int BLOCK_TYPE_COUNT = 7;
    constexpr int ROTATION_COUNT = 4;
    constexpr int SHAPE_SIZE = 4;
    constexpr bool CELL_FILLED = true;
    constexpr bool CELL_EMPTY = false;
}

class BlockShape {
public:
    // 각 블록 모양을 16비트 정수로 저장 (4x4 = 16비트)
    // 비트 순서: 원래 배열과 동일하게 [row][col] = bit (row * 4 + col)
    // 원래 배열: SHAPES[type][rotation][i][j] where i=row, j=col
    inline static const uint16_t SHAPES[BlockShapeConstants::BLOCK_TYPE_COUNT][BlockShapeConstants::ROTATION_COUNT] = {
        // I 블록 (막대모양)
        // 원래: 1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0
        // 비트: row0=1000, row1=1000, row2=1000, row3=1000
        {0x8888, 0xF000, 0x8888, 0xF000},
    
        // O 블록 (네모모양)
        // 원래: 1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0
        // 비트: row0=1100, row1=1100, row2=0000, row3=0000
        {0xCC00, 0xCC00, 0xCC00, 0xCC00},
    
        // T 블록 ('ㅓ' 모양)
        // 원래: 0,1,0,0,1,1,0,0,0,1,0,0,0,0,0,0
        // 비트: row0=0100, row1=1100, row2=0100, row3=0000
        {0x4C40, 0xE400, 0x8C80, 0x4E00},
    
        // L 블록 ('ㄱ'모양)
        // 원래: 1,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0
        // 비트: row0=1100, row1=0100, row2=0100, row3=0000
        {0xC440, 0xE800, 0x88C0, 0x2E00},
    
        // J 블록 ('ㄴ' 모양)
        // 원래: 1,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0
        // 비트: row0=1100, row1=1000, row2=1000, row3=0000
        {0xC880, 0x8E00, 0x44C0, 0xE200},
    
        // Z 블록 ('Z' 모양)
        // 원래: 1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0
        // 비트: row0=1100, row1=0110, row2=0000, row3=0000
        {0xC600, 0x4C80, 0xC600, 0x4C80},
    
        // S 블록 ('S' 모양)
        // 원래: 0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0
        // 비트: row0=0110, row1=1100, row2=0000, row3=0000
        {0x6C00, 0x8C40, 0x6C00, 0x8C40}
    };
    
    // 비트에서 특정 위치의 셀 값을 읽는 헬퍼 함수
    // 원래 배열: SHAPES[type][rotation][row][col]
    // getCell(type, rotation, row, col)로 호출해야 함
    static inline bool getCell(int blockType, int rotation, int row, int col) {
        if (row < 0 || row >= BlockShapeConstants::SHAPE_SIZE || 
            col < 0 || col >= BlockShapeConstants::SHAPE_SIZE) {
            return BlockShapeConstants::CELL_EMPTY;
        }
        uint16_t shape = SHAPES[blockType][rotation];
        int bitPos = row * BlockShapeConstants::SHAPE_SIZE + col;
        return (shape << bitPos) & 0x8000; // MSB 비트 추출
    }
};
