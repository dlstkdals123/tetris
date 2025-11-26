#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <Windows.h>
#include <time.h>
#include <iostream>

#include "Board.h"
#include "position.h"
#include "rotation.h"
#include "STAGE.h"
#include "COLOR.h"
#include "gameState.h"
#include "Block.h"
#include "BlockData.h"
#include "BlockGenerator.h"
#include "BlockRender.h"
#include "BlockMover.h"
#include "Utils.h"

using namespace std;

//*********************************
// 상수 선언 (키 값)
//*********************************

#define EXT_KEY   0xffffffe0  // 확장키 인식값 
#define KEY_LEFT  0x4b
#define KEY_RIGHT 0x4d
#define KEY_UP    0x48
#define KEY_DOWN  0x50

//*********************************
// 전역 상수 : 스테이지 데이터
//*********************************

// 순서대로 speed, stick_rate, clear_line
const STAGE stage_data[10] = {
    STAGE(40, 20, 20),       // Level 1
    STAGE(38, 18, 20),       // Level 2
    STAGE(35, 18, 20),       // Level 3
    STAGE(30, 17, 20),       // Level 4
    STAGE(25, 16, 20),       // Level 5
    STAGE(20, 14, 20),       // Level 6
    STAGE(15, 14, 20),       // Level 7
    STAGE(10, 13, 20),       // Level 8
    STAGE(6,  12, 20),       // Level 9
    STAGE(4,  11, 99999)     // Level 10 (사실상 마지막)
};

//*********************************
// 함수 선언
//*********************************

// 현재 스코어 / 스테이지 / 남은 라인 표시
void show_gamestat(const gameState &gs, bool printed_text = false);
// 시작 레벨 입력
void input_data(gameState &gs);
// 로고 화면 + 랜덤 블록 애니메이션
void show_logo(BlockRender &renderer);
// 게임 오버 화면 표시
void show_gameover();

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    gameState gamestate; // 점수, 레벨, 라인 상태
    Board board; // 쌓인 블록 / 벽 / 바닥 관리
    Position boardOffset(5, 1); // 블록 생성 좌표
    BlockGenerator blockGenerator(stage_data, gamestate);
    BlockRender renderer(gamestate, boardOffset);
    BlockMover mover(renderer, board, blockGenerator, gamestate);

    show_logo(renderer);

    while (1)
    {
        // 새 게임 시작 시 상태 초기화
        gamestate.resetState();
        board.init();

        // 시작 레벨 입력
        input_data(gamestate);

        // 보드 초기 출력
        board.draw(gamestate.getLevel());

        // 현재 조작 블록 / 다음 블록 생성
        Block curBlock(blockGenerator.make_new_block());
        Block nextBlock(blockGenerator.make_new_block());

        curBlock.block_start();
        renderer.show_cur_block(curBlock);
        renderer.show_next_block(nextBlock);
        show_gamestat(gamestate, true);
        
        int is_gameover = 0;

        for (int i = 1;; i++)
        {
            // 키보드 입력 처리
            if (_kbhit())
            {
                char keytemp = _getche();

                if (keytemp == EXT_KEY)
                {
                    keytemp = _getche();
                    switch (keytemp)
                    {
                    case KEY_UP: // 회전
                    {
                        mover.rotateBlock(curBlock);
                        break;
                    }
                    case KEY_LEFT: // 왼쪽 이동
                        mover.movedLeft(curBlock);
                        break;

                    case KEY_RIGHT: // 오른쪽 이동
                        mover.movedRight(curBlock);
                        break;

                    case KEY_DOWN: // 한 칸 아래로
                        is_gameover = mover.move_block(curBlock, nextBlock);
                        show_gamestat(gamestate);
                        break;
                    }
                }
                else if (keytemp == 32) // 스페이스바: 바닥까지 즉시 드랍
                {
                    while (is_gameover == 0)
                    {
                        is_gameover = mover.move_block(curBlock, nextBlock);
                    }
                    show_gamestat(gamestate);
                }
            }

            // 자동 낙하
            if (i % stage_data[gamestate.getLevel()].getSpeed() == 0)
            {
                is_gameover = mover.move_block(curBlock, nextBlock);
                show_gamestat(gamestate);
            }

            // 스테이지 클리어
            if (stage_data[gamestate.getLevel()].getClearLine() <= gamestate.getLines())
            {
                gamestate.levelUp();
                board.draw(gamestate.getLevel());
                show_gamestat(gamestate);
                renderer.show_next_block(nextBlock);
            }

            // 게임 오버
            if (is_gameover == 1)
            {
                show_gameover();
                Utils::setColor(COLOR::GRAY);
                break; // 새 게임 시작
            }

            Utils::gotoxy(77, 23);
            Sleep(15);
            Utils::gotoxy(77, 23);
        }
    }

    return 0;
}

//*********************************
// 보조 함수 구현부
//*********************************

void show_gamestat(const gameState &gs, bool printed_text)
{
    Utils::setColor(COLOR::GRAY);

    if (printed_text)
    {
        Utils::gotoxy(35, 7);
        printf("STAGE");

        Utils::gotoxy(35, 9);
        printf("SCORE");

        Utils::gotoxy(35, 12);
        printf("LINES");
    }

    Utils::gotoxy(41, 7);
    printf("%d", gs.getLevel() + 1);

    Utils::gotoxy(35, 10);
    printf("%10d", gs.getScore());

    Utils::gotoxy(35, 13);
    int remain = stage_data[gs.getLevel()].getClearLine() - gs.getLines();
    if (remain < 0)
        remain = 0;
    printf("%10d", remain);
}

void input_data(gameState &gs)
{
    int level = 0;

    Utils::setColor(COLOR::GRAY);

    Utils::gotoxy(10, 7);
    printf("┏━━━━<GAME KEY>━━━━━┓");
    Sleep(10);
    Utils::gotoxy(10, 8);
    printf("┃ UP   : Rotate Block        ┃");
    Sleep(10);
    Utils::gotoxy(10, 9);
    printf("┃ DOWN : Move One-Step Down  ┃");
    Sleep(10);
    Utils::gotoxy(10, 10);
    printf("┃ SPACE: Move Bottom Down    ┃");
    Sleep(10);
    Utils::gotoxy(10, 11);
    printf("┃ LEFT : Move Left           ┃");
    Sleep(10);
    Utils::gotoxy(10, 12);
    printf("┃ RIGHT: Move Right          ┃");
    Sleep(10);
    Utils::gotoxy(10, 13);
    printf("┗━━━━━━━━━━━━━━┛");

    while (level < 1 || level > 8)
    {
        Utils::gotoxy(10, 3);
        printf("Select Start level[1-8]:       \b\b\b\b\b\b\b");

        cin >> level;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(1024, '\n');
            continue;
        }
    }

    gs.setLevel(level - 1);
    system("cls");
}

void show_logo(BlockRender &renderer)
{
    int i, j;

    Utils::gotoxy(13, 3);
    printf("┏━━━━━━━━━━━━━━━━━━━━━━━┓");
    Sleep(100);
    Utils::gotoxy(13, 4);
    printf("┃◆◆◆  ◆◆◆  ◆◆◆   ◆◆     ◆   ◆◆◆ ┃");
    Sleep(100);
    Utils::gotoxy(13, 5);
    printf("┃  ◆    ◆        ◆     ◆ ◆    ◆   ◆     ┃");
    Sleep(100);
    Utils::gotoxy(13, 6);
    printf("┃  ◆    ◆◆◆    ◆     ◆◆     ◆     ◆   ┃");
    Sleep(100);
    Utils::gotoxy(13, 7);
    printf("┃  ◆    ◆        ◆     ◆ ◆    ◆       ◆ ┃");
    Sleep(100);
    Utils::gotoxy(13, 8);
    printf("┃  ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆ ┃");
    Sleep(100);
    Utils::gotoxy(13, 9);
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━┛");

    Utils::gotoxy(28, 20);
    printf("Please Press Any Key~!");

    for (i = 0;; ++i)
    {
        if (i % 40 == 0)
        {
            for (j = 0; j < 5; ++j)
            {
                Utils::gotoxy(17, 14 + j);
                printf("                                                          ");
            }

            Position logo_pos1(6, 14);
            Position logo_pos2(12, 14);
            Position logo_pos3(19, 14);
            Position logo_pos4(24, 14);

            Rotation logo_rot1(rand() % 4);
            Rotation logo_rot2(rand() % 4);
            Rotation logo_rot3(rand() % 4);
            Rotation logo_rot4(rand() % 4);

            Block b1(static_cast<BlockType>(rand() % 7), logo_rot1, logo_pos1);
            Block b2(static_cast<BlockType>(rand() % 7), logo_rot2, logo_pos2);
            Block b3(static_cast<BlockType>(rand() % 7), logo_rot3, logo_pos3);
            Block b4(static_cast<BlockType>(rand() % 7), logo_rot4, logo_pos4);

            renderer.show_cur_block(b1);
            renderer.show_cur_block(b2);
            renderer.show_cur_block(b3);
            renderer.show_cur_block(b4);
        }

        if (_kbhit())
            break;

        Sleep(30);
    }

    _getche();
    system("cls");
}

void show_gameover()
{
    Utils::setColor(COLOR::RED);
    Utils::gotoxy(15, 8);
    printf("┏━━━━━━━━━━━━━┓");
    Utils::gotoxy(15, 9);
    printf("┃**************************┃");
    Utils::gotoxy(15, 10);
    printf("┃*        GAME OVER       *┃");
    Utils::gotoxy(15, 11);
    printf("┃**************************┃");
    Utils::gotoxy(15, 12);
    printf("┗━━━━━━━━━━━━━┛");

    fflush(stdin);
    Sleep(1000);

    _getche();
    system("cls");
}
