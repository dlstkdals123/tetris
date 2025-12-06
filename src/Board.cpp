#include "Board.h"
#include "Block.h"
#include "BoardConstants.h"
#include "GameConstants.h"
#include "BlockData.h"
#include <Windows.h>
#include <iostream>
#include <cstdlib>

using namespace std;

char Board::getCell(int row, int col) const
{
    if (row < 0 || row >= BoardConstants::BOARD_HEIGHT || col < 0 || col >= BoardConstants::BOARD_WIDTH)
    {
        return BoardConstants::CELL_EMPTY;
    }
    
    // 각 컬럼은 2비트로 저장됨 (col * 2 위치에 저장)
    uint32_t bitPos = col * 2;
    return (char)((rows[row] >> bitPos) & BoardConstants::CELL_MASK);
}

void Board::setCell(int row, int col, char value)
{
    if (row < 0 || row >= BoardConstants::BOARD_HEIGHT || col < 0 || col >= BoardConstants::BOARD_WIDTH)
    {
        return;
    }
    
    // 각 컬럼은 2비트로 저장됨 (col * 2 위치에 저장)
    uint32_t bitPos = col * 2;
    
    // 기존 값 제거 후 새 값 설정
    rows[row] &= ~(BoardConstants::CELL_MASK << bitPos);
    rows[row] |= ((value & BoardConstants::CELL_MASK) << bitPos);
}

Board::Board(bool isPlayer) : isPlayer(isPlayer)
{
    for (int i = 0; i < BoardConstants::PLAY_HEIGHT; i++)
        rows[i] = 0 | BoardConstants::WALL_BIT;

    rows[BoardConstants::BOARD_HEIGHT - 1] = BoardConstants::FLOOR_BIT;
}

Board::Board(const Board& other) : isPlayer(other.isPlayer)
{
    // 비트 패킹된 행들을 직접 복사
    for (int i = 0; i < BoardConstants::BOARD_HEIGHT; i++)
        rows[i] = other.rows[i];
}

Board::~Board()
{
}


void Board::draw(const int &level) const
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

    string line;
    line.reserve(BoardConstants::BOARD_WIDTH * 3);

    int i, j;
    Utils::setColor(COLOR::DARK_GRAY);

    for (i = 0; i < BoardConstants::BOARD_HEIGHT; i++)
    {
        Utils::gotoxy(Utils::ab_x, i + Utils::ab_y, isPlayer);
        line.clear();

        
        for (j = 0; j < BoardConstants::BOARD_WIDTH; j++)
        {
            if (j == BoardConstants::LEFT_WALL || j == BoardConstants::RIGHT_WALL || i == BoardConstants::FLOOR_ROW)
            { // 레벨에 따라 외벽 색이 변함
                Utils::setColor((level % GameConstants::LevelColor::COLOR_COUNT) + GameConstants::LevelColor::COLOR_OFFSET);
            }
            else
            {
                Utils::setColor(COLOR::DARK_GRAY);
            }
            Utils::gotoxy((j * GameConstants::BlockRender::X_COORD_MULTIPLIER) + Utils::ab_x, i + Utils::ab_y, isPlayer);
            char cellValue = getCell(i, j);
            if (cellValue == BoardConstants::CELL_FILLED)
            {
                cout << "■";
            }
            else if (cellValue == BoardConstants::CELL_ATTACK)
            {
                // Attack 라인은 다른 색상으로 표시 (RED)
                Utils::setColor(COLOR::RED);
                cout << "■";
                Utils::setColor(COLOR::DARK_GRAY);
            }
            else
            {
                cout << "  ";
            }
        }
    }

    cout.flush();

    Utils::setColor(COLOR::BLACK);
    Utils::gotoxy(GameConstants::BlockRender::CURSOR_X, GameConstants::BlockRender::CURSOR_Y);
}

int Board::isStrike(const Block &block) const
{
    int i, j;
    int x = block.getPos().getX();
    int y = block.getPos().getY();

    for (i = 0; i < GameConstants::BlockRender::SHAPE_SIZE; i++)
    {
        for (j = 0; j < GameConstants::BlockRender::SHAPE_SIZE; j++)
        {
            if (BlockShape::getCell((int)block.getType(), block.getRotation(), i, j) == BlockShapeConstants::CELL_EMPTY)
            { // 블럭의 해당 위치가 채워져있는지 검사.
                continue;
            }

            if (((x + j) == BoardConstants::LEFT_WALL) || ((x + j) == BoardConstants::RIGHT_WALL))
            { // 벽 충돌시
                return BoardConstants::STRIKE_TRUE;
            }

            if (y + i >= GameConstants::Simulation::GAME_OVER_Y_THRESHOLD)
            {
                char cellValue = getCell(y + i, x + j);
                if (cellValue == BoardConstants::CELL_FILLED || cellValue == BoardConstants::CELL_ATTACK)
                { // 바닥 or 다른 블록(일반 블록 또는 attack 라인)에 닿았는지 검사
                    return BoardConstants::STRIKE_TRUE;
                }
            }
        }
    }
    return BoardConstants::STRIKE_FALSE;
}

void Board::mergeBlock(const Block &block)
{
    Position pos = block.getPos();
    int i, j;
    int x = pos.getX();
    int y = pos.getY();
    for (i = 0; i < GameConstants::BlockRender::SHAPE_SIZE; i++)
    {
        for (j = 0; j < GameConstants::BlockRender::SHAPE_SIZE; j++)
        {
            if (BlockShape::getCell((int)block.getType(), block.getRotation(), i, j) == BlockShapeConstants::CELL_FILLED)
            {
                if (y + i >= GameConstants::Simulation::GAME_OVER_Y_THRESHOLD)
                {
                    setCell(y + i, x + j, BoardConstants::CELL_FILLED);
                }
            }
        }
    }
}
// check_full_line
std::pair<int, int> Board::deleteFullLine()
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

    int i, j, k;
    int deletedLines = 0;
    int attackableLines = 0; // attack 가능한 줄 수 (1로만 이루어진 줄)

    for (i = 0; i < BoardConstants::PLAY_HEIGHT; i++)
    {
        bool isFull = true;
        bool containsAttackLine = false;
        
        // 플레이 영역의 각 셀(2비트)을 확인 - 비트 연산으로 최적화
        // 12개 컬럼을 확인 (비트 2-25, 각 2비트씩)
        for (j = BoardConstants::MIN_COLUMN; j <= BoardConstants::MAX_COLUMN; j++)
        {
            char cellValue = getCell(i, j);
            if (cellValue == BoardConstants::CELL_EMPTY)
            {
                isFull = false;
                break;
            }
            if (cellValue == BoardConstants::CELL_ATTACK)
            {
                containsAttackLine = true;
            }
        }
        
        if (!isFull) {
            continue; // 줄이 꽉 차지 않았으면 건너뜀
        }
        
        // 줄이 꽉 찼으면 삭제
        deletedLines++;
        
        // 1로만 이루어진 줄(2가 없음)이면 attack 가능한 줄로 카운트
        if (!containsAttackLine) {
            attackableLines++;
        }
        
        // 줄 삭제: 위의 줄들을 아래로 이동
        for (k = i; k > 0; k--)
        {
            // 벽을 제외한 플레이 영역만 복사
            uint32_t srcRow = rows[k - 1];
            uint32_t dstRow = rows[k];
            
            // 벽 위치(0, 13)는 유지하고, 플레이 영역(1-12)만 복사
            uint32_t leftWall = dstRow & BoardConstants::LEFT_WALL_MASK;
            uint32_t playArea = srcRow & BoardConstants::PLAY_AREA_MASK;
            uint32_t rightWall = dstRow & BoardConstants::RIGHT_WALL_MASK;
            
            rows[k] = leftWall | playArea | rightWall;
        }
        // 맨 위 줄을 빈 공간으로 초기화 (벽 제외) - 비트 연산으로 최적화
        uint32_t& topRow = rows[GameConstants::Simulation::GAME_OVER_Y_THRESHOLD];
        uint32_t leftWall = topRow & BoardConstants::LEFT_WALL_MASK;
        uint32_t rightWall = topRow & BoardConstants::RIGHT_WALL_MASK;
        topRow = leftWall | rightWall; // 플레이 영역은 0으로 초기화됨
        
        // 삭제 후 인덱스 조정 (한 줄이 삭제되었으므로 i를 다시 확인해야 함)
        i--; // 다음 반복에서 같은 인덱스를 다시 확인
    }
    
    return std::make_pair(deletedLines, attackableLines);
}

// Attack 라인 추가 (상대방에게 공격)
int Board::addAttackLines(int numLines)
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
    
    if (numLines <= GameConstants::Simulation::GAME_OVER_Y_THRESHOLD) return GameConstants::BoardReturn::SUCCESS;
    
    // 기존 블록을 위로 밀어냄
    // 맨 위 블록이 화면 밖으로 나가면 게임오버
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < numLines; i++) {
        // 맨 위 줄(row 0)에 블록이 있는지 비트 연산으로 확인
        uint32_t playArea = rows[GameConstants::Simulation::GAME_OVER_Y_THRESHOLD] & BoardConstants::PLAY_AREA_MASK;
        
        // 플레이 영역이 비어있지 않으면 게임오버
        if (playArea != 0) {
            // 게임오버: 맨 위 블록이 화면 밖으로 나감
            return GameConstants::BoardReturn::GAME_OVER;
        }
        
        // 모든 줄을 위로 한 칸씩 이동
        for (int row = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; row < BoardConstants::MAX_ROW; row++) {
            // 벽을 제외한 플레이 영역만 복사
            uint32_t srcRow = rows[row + GameConstants::BlockRotation::ROTATION_INCREMENT];
            uint32_t dstRow = rows[row];
            
            // 벽 위치(0, 13)는 유지하고, 플레이 영역(1-12)만 복사
            uint32_t leftWall = dstRow & BoardConstants::LEFT_WALL_MASK;
            uint32_t playArea = srcRow & BoardConstants::PLAY_AREA_MASK;
            uint32_t rightWall = dstRow & BoardConstants::RIGHT_WALL_MASK;
            
            rows[row] = leftWall | playArea | rightWall;
        }
        
        // 맨 아래에 attack 라인 추가 (랜덤하게 1칸씩 비워서)
        // 랜덤하게 한 칸을 선택해서 비움
        int emptyCol = BoardConstants::MIN_COLUMN + (rand() % BoardConstants::PLAY_WIDTH);
        
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++) {
            if (col == emptyCol) {
                setCell(BoardConstants::MAX_ROW, col, BoardConstants::CELL_EMPTY); // 빈 칸
            } else {
                setCell(BoardConstants::MAX_ROW, col, BoardConstants::CELL_ATTACK); // Attack 라인
            }
        }
        
        // 벽은 그대로 유지
        setCell(BoardConstants::MAX_ROW, BoardConstants::LEFT_WALL, BoardConstants::CELL_FILLED);
        setCell(BoardConstants::MAX_ROW, BoardConstants::RIGHT_WALL, BoardConstants::CELL_FILLED);
    }
    
    return numLines;
}