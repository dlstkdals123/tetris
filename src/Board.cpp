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
                if (total_block[y + i][x + j] == 1)
                { // 바닥 or 다른 블록에 닿았는지 검사
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
int Board::deleteFullLine()
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지

    int i, j, k;
    int deletedLines = 0;

    for (i = 0; i < BoardConstants::PLAY_HEIGHT; i++)
    {
        for (j = BoardConstants::MIN_COLUMN; j <= BoardConstants::MAX_COLUMN; j++)
        {
            if (total_block[i][j] == 0)
            {
                break;
            }
        }
        if (j == BoardConstants::RIGHT_WALL)
        { // 한줄이 다 채워졌을 떄
            deletedLines++;
            // show_total_block();

            // Utils::setColor(COLOR::BLUE);
            // Utils::gotoxy(1 * 2 + Utils::ab_x, i + Utils::ab_y, isPlayer);

            // for (j = 1; j < 13; j++)
            // {
            //     cout << "□";
            //     cout.flush();
            //     Sleep(10);
            // }
            // Utils::gotoxy(1 * 2 + Utils::ab_x, i + Utils::ab_y, isPlayer);
            // for (j = 1; j < 13; j++)
            // {
            //     cout << "  ";
            //     cout.flush();
            //     Sleep(10);
            // }

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
        }
    }
    return deletedLines;
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