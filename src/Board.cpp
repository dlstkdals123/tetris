#include "Board.h"
#include "Block.h"
#include "BoardConstants.h"
#include <Windows.h>
#include <iostream>
#include <random>

using namespace std;

Board::Board(bool isPlayer) : isPlayer(isPlayer)
{
    int i, j;
    for (i = 0; i < BoardConstants::PLAY_HEIGHT; i++)
    {
        for (j = 0; j < BoardConstants::BOARD_WIDTH; j++)
        {
            if ((j == BoardConstants::LEFT_WALL) || (j == BoardConstants::RIGHT_WALL))
            {
                total_block[i][j] = 1; // 좌우 벽
            }
            else
            {
                total_block[i][j] = 0; // 빈 공간
            }
        }
    }

    for (j = 0; j < BoardConstants::BOARD_WIDTH; j++)
    {                           // 화면의 제일 밑의 줄은 1로 채운다.
        total_block[BoardConstants::FLOOR_ROW][j] = 1; // 바닥
    }
}

Board::Board(const Board& other) : isPlayer(other.isPlayer)
{
    for (int i = 0; i < BoardConstants::BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < BoardConstants::BOARD_WIDTH; j++)
        {
            total_block[i][j] = other.total_block[i][j];
        }
    }
}

Board::~Board()
{
}

int Board::init()
{
    int i, j;
    for (i = 0; i < BoardConstants::PLAY_HEIGHT; i++)
    {
        for (j = 0; j < BoardConstants::BOARD_WIDTH; j++)
        {
            if ((j == BoardConstants::LEFT_WALL) || (j == BoardConstants::RIGHT_WALL))
            {
                total_block[i][j] = 1; // 좌우 벽
            }
            else
            {
                total_block[i][j] = 0; // 빈 공간
            }
        }
    }

    for (j = 0; j < BoardConstants::BOARD_WIDTH; j++)
    {                           // 화면의 제일 밑의 줄은 1로 채운다.
        total_block[BoardConstants::FLOOR_ROW][j] = 1; // 바닥
    }

    return 0;
}

void Board::initWithState(int stateType, std::mt19937& rng)
{
    // 먼저 기본 초기화
    init();
    
    if (stateType == 0)
    {
        // 0줄: 완전히 비어있음 (이미 init()에서 처리됨)
        return;
    }
    
    if (stateType == 5)
    {
        // 5: 랜덤 상태 - 0~10줄 사이 랜덤하게 채움
        std::uniform_int_distribution<int> rowDist(BoardConstants::MIN_ROW, BoardConstants::MAX_ROW); // 행
        std::uniform_int_distribution<int> colDist(BoardConstants::MIN_COLUMN, BoardConstants::MAX_COLUMN); // 열 (벽 제외)
        std::uniform_int_distribution<int> fillRowsDist(0, 10); // 채울 줄 수 (0~10)
        
        int numFilledRows = fillRowsDist(rng);
        int filledCount = 0;
        
        // 랜덤하게 블록 배치
        while (filledCount < numFilledRows * BoardConstants::PLAY_WIDTH && filledCount < 120) // 최대 120개 블록
        {
            int row = rowDist(rng);
            int col = colDist(rng);
            
            if (total_block[row][col] == 0)
            {
                total_block[row][col] = 1;
                filledCount++;
            }
        }
        return;
    }
    
    // 1~4줄: 각각 1줄, 2줄, 3줄, 4줄을 꽉 채우고 1자로 비어있게 함
    std::uniform_int_distribution<int> colDist(BoardConstants::MIN_COLUMN, BoardConstants::MAX_COLUMN); // 플레이 영역 열 (벽 제외)
    
    // 아래에서부터 stateType개의 줄을 채움
    for (int row = BoardConstants::MAX_ROW; row >= BoardConstants::PLAY_HEIGHT - stateType; row--)
    {
        // 한 줄을 모두 채움
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
        {
            total_block[row][col] = 1;
        }
    }
    
    // 각 줄에서 랜덤하게 1자씩 비움
    for (int row = BoardConstants::MAX_ROW; row >= BoardConstants::PLAY_HEIGHT - stateType; row--)
    {
        int emptyCol = colDist(rng);
        total_block[row][emptyCol] = 0;
    }
}

void Board::draw(const int &level) const
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

    int i, j;
    Utils::setColor(COLOR::DARK_GRAY);

    for (i = 0; i < BoardConstants::BOARD_HEIGHT; i++)
    {
        for (j = 0; j < BoardConstants::BOARD_WIDTH; j++)
        {
            if (j == BoardConstants::LEFT_WALL || j == BoardConstants::RIGHT_WALL || i == BoardConstants::FLOOR_ROW)
            { // 레벨에 따라 외벽 색이 변함
                Utils::setColor((level % 6) + 1);
            }
            else
            {
                Utils::setColor(COLOR::DARK_GRAY);
            }
            Utils::gotoxy((j * 2) + Utils::ab_x, i + Utils::ab_y, isPlayer);
            if (total_block[i][j] == 1)
            {
                cout << "■";
            }
            else if (total_block[i][j] == 2)
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
    Utils::gotoxy(77, 23);
}

int Board::isStrike(const Block &block) const
{
    int i, j;
    int x = block.getPos().getX();
    int y = block.getPos().getY();

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (BlockShape::SHAPES[(int)block.getType()][block.getRotation()][i][j] == 0)
            { // 블럭의 해당 위치가 채워져있는지 검사.
                continue;
            }

            if (((x + j) == BoardConstants::LEFT_WALL) || ((x + j) == BoardConstants::RIGHT_WALL))
            { // 벽 충돌시
                return 1;
            }

            if (y + i >= 0)
            {
                if (total_block[y + i][x + j] == 1 || total_block[y + i][x + j] == 2)
                { // 바닥 or 다른 블록(일반 블록 또는 attack 라인)에 닿았는지 검사
                    return 1;
                }
            }
        }
    }
    return 0;
}

void Board::mergeBlock(const Block &block)
{
    Position pos = block.getPos();
    int i, j;
    int x = pos.getX();
    int y = pos.getY();
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (BlockShape::SHAPES[(int)block.getType()][block.getRotation()][i][j] == 1)
            {
                if (y + i >= 0)
                {
                    total_block[y + i][x + j] = 1;
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
        // 줄이 꽉 찼는지 확인
        bool isFull = true;
        bool hasAttackLine = false; // 2가 포함되어 있는지
        bool hasOnlyOnes = true; // 1로만 이루어져 있는지
        
        for (j = BoardConstants::MIN_COLUMN; j <= BoardConstants::MAX_COLUMN; j++)
        {
            if (total_block[i][j] == 0)
            {
                isFull = false;
                break;
            }
            if (total_block[i][j] == 2)
            {
                hasAttackLine = true;
                hasOnlyOnes = false;
            }
            else if (total_block[i][j] != 1)
            {
                hasOnlyOnes = false;
            }
        }
        
        if (!isFull) {
            continue; // 줄이 꽉 차지 않았으면 건너뜀
        }
        
        // 줄이 꽉 찼으면 삭제
        deletedLines++;
        
        // 1로만 이루어진 줄(2가 없음)이면 attack 가능한 줄로 카운트
        if (hasOnlyOnes && !hasAttackLine) {
            attackableLines++;
        }
        
        // 2가 포함된 줄도 삭제는 가능 (attack 불가능하지만 삭제는 가능)
        
        // 줄 삭제: 위의 줄들을 아래로 이동
        for (k = i; k > 0; k--)
        {
            for (j = BoardConstants::MIN_COLUMN; j <= BoardConstants::MAX_COLUMN; j++)
            {
                total_block[k][j] = total_block[k - 1][j];
            }
        }
        for (j = BoardConstants::MIN_COLUMN; j <= BoardConstants::MAX_COLUMN; j++)
        {
            total_block[0][j] = 0;
        }
        
        // 삭제 후 인덱스 조정 (한 줄이 삭제되었으므로 i를 다시 확인해야 함)
        i--; // 다음 반복에서 같은 인덱스를 다시 확인
    }
    
    return std::make_pair(deletedLines, attackableLines);
}

// Attack 라인 추가 (상대방에게 공격)
int Board::addAttackLines(int numLines)
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
    
    if (numLines <= 0) return 0;
    
    // 기존 블록을 위로 밀어냄
    // 맨 위 블록이 화면 밖으로 나가면 게임오버
    for (int i = 0; i < numLines; i++) {
        // 맨 위 줄(row 0)에 블록이 있는지 확인
        bool hasBlockAtTop = false;
        for (int j = BoardConstants::MIN_COLUMN; j <= BoardConstants::MAX_COLUMN; j++) {
            if (total_block[0][j] != 0) {
                hasBlockAtTop = true;
                break;
            }
        }
        
        if (hasBlockAtTop) {
            // 게임오버: 맨 위 블록이 화면 밖으로 나감
            return -1;
        }
        
        // 모든 줄을 위로 한 칸씩 이동
        for (int row = 0; row < BoardConstants::MAX_ROW; row++) {
            for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++) {
                total_block[row][col] = total_block[row + 1][col];
            }
        }
        
        // 맨 아래에 attack 라인 추가 (랜덤하게 1칸씩 비워서)
        // 랜덤하게 한 칸을 선택해서 비움
        int emptyCol = BoardConstants::MIN_COLUMN + (rand() % BoardConstants::PLAY_WIDTH);
        
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++) {
            if (col == emptyCol) {
                total_block[BoardConstants::MAX_ROW][col] = 0; // 빈 칸
            } else {
                total_block[BoardConstants::MAX_ROW][col] = 2; // Attack 라인 (값 2)
            }
        }
        
        // 벽은 그대로 유지
        total_block[BoardConstants::MAX_ROW][BoardConstants::LEFT_WALL] = 1;
        total_block[BoardConstants::MAX_ROW][BoardConstants::RIGHT_WALL] = 1;
    }
    
    return numLines;
}

// Feature 추출을 위한 접근 함수들
char Board::getCell(int row, int col) const
{
    if (row < 0 || row >= BoardConstants::BOARD_HEIGHT || col < 0 || col >= BoardConstants::BOARD_WIDTH)
    {
        return 0;
    }
    return total_block[row][col];
}

const char* Board::getRow(int row) const
{
    if (row < 0 || row >= BoardConstants::BOARD_HEIGHT)
    {
        return nullptr;
    }
    return total_block[row];
}