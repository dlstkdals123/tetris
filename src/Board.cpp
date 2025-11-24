#include "Board.h"
#include "Block.h"
#include <Windows.h>
#include <iostream>

using namespace std;

Board::Board()
{
    int i, j;
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 14; j++)
        {
            if ((j == 0) || (j == 13))
            {
                total_block[i][j] = 1; // 좌우 벽
            }
            else
            {
                total_block[i][j] = 0; // 빈 공간
            }
        }
    }

    for (j = 0; j < 14; j++)
    {                           // 화면의 제일 밑의 줄은 1로 채운다.
        total_block[20][j] = 1; // 바닥
    }
}
Board::~Board()
{
}

int Board::init()
{
    int i, j;
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 14; j++)
        {
            if ((j == 0) || (j == 13))
            {
                total_block[i][j] = 1; // 좌우 벽
            }
            else
            {
                total_block[i][j] = 0; // 빈 공간
            }
        }
    }

    for (j = 0; j < 14; j++)
    {                           // 화면의 제일 밑의 줄은 1로 채운다.
        total_block[20][j] = 1; // 바닥
    }

    return 0;
}

void Board::draw(const int &level) const
{
    int i, j;
    Utils::setColor(COLOR::DARK_GRAY);

    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 14; j++)
        {
            if (j == 0 || j == 13 || i == 20)
            { // 레벨에 따라 외벽 색이 변함
                Utils::setColor((level % 6) + 1);
            }
            else
            {
                Utils::setColor(COLOR::DARK_GRAY);
            }
            Utils::gotoxy((j * 2) + Utils::ab_x, i + Utils::ab_y);
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

int Board::isStrike(const Block &block)
{
    int i, j;
    int x = block.getPos().getX();
    int y = block.getPos().getY();

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (block.getShape(i, j) == 0)
            { // 블럭의 해당 위치가 채워져있는지 검사.
                continue;
            }

            if (((x + j) == 0) || ((x + j) == 13))
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
    int i, j;
    int x = block.getx();
    int y = block.gety();
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (block.getShape(i, j) == 1)
            {
                if (y + i >= 0)
                {
                    total_block[y + i][x + j] = 1;
                }
            }
        }
    }
    // check_full_line();
    // show_total_block();
}
// check_full_line
int Board::deleteFullLine()
{
    int i, j, k;
    int deletedLines = 0;

    for (i = 0; i < 20; i++)
    {
        for (j = 1; j < 13; j++)
        {
            if (total_block[i][j] == 0)
            {
                break;
            }
        }
        if (j == 13)
        { // 한줄이 다 채워졌을 떄
            deletedLines++;
            // show_total_block();

            Utils::setColor(COLOR::BLUE);
            Utils::gotoxy(1 * 2 + Utils::ab_x, i + Utils::ab_y);

            for (j = 1; j < 13; j++)
            {
                cout << "□";
                cout.flush();
                Sleep(10);
            }
            Utils::gotoxy(1 * 2 + Utils::ab_x, i + Utils::ab_y);
            for (j = 1; j < 13; j++)
            {
                cout << "  ";
                cout.flush();
                Sleep(10);
            }

            for (k = i; k > 0; k--)
            {
                for (j = 1; j < 13; j++)
                {
                    total_block[k][j] = total_block[k - 1][j];
                }
            }
            for (j = 1; j < 13; j++)
            {
                total_block[0][j] = 0;
            }
        }
    }
    return deletedLines;
}

void Board::show_next_block()
{
    int i, j;
    Utils::setColor(COLOR::DARK_GRAY);
    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 14; j++)
        {
            if (j == 0 || j == 13 || i == 20) // 레벨에 따라 외벽 색이 변함
            {
                Utils::setColor(static_cast<COLOR>((gs.getLevel() % 6) + 1));
            }
            else
            {
                Utils::setColor(COLOR::DARK_GRAY);
            }
            Utils::gotoxy((j * 2) + boardOffset.getX(), i + boardOffset.getY());
            int total_block = board.data if (total_block[i][j] == 1)
            {
                printf("■");
            }
            else
            {
                printf("  ");
            }
        }
    }
    Utils::setColor(COLOR::BLACK);
    Utils::gotoxy(77, 23);
}