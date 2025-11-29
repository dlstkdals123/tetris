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

#include <thread>
#include <atomic>

using namespace std;

//*********************************
// 상수 선언 (키 값)
//*********************************

#define EXT_KEY   0xffffffe0  // 확장키 인식값 
#define KEY_LEFT  0x4b
#define KEY_RIGHT 0x4d
#define KEY_UP    0x48
#define KEY_DOWN  0x50

#define AI_LEFT 'a'
#define AI_RIGHT 'd'
#define AI_UP 'w'
#define AI_DOWN 's'
#define AI_SPACE 'c'

//*********************************
// 함수 선언
//*********************************

// 시작 레벨 입력
int input_data();
// 로고 화면 + 랜덤 블록 애니메이션
void show_logo(BlockRender& renderer);
// 게임 오버 화면 표시
void show_gameover();

//*********************************
// 스레드 관련 함수 추가
//*********************************

// 키 입력 스레드
void inputThread(std::atomic<int>& is_gameover);
// 게임 스레드
void gameThread(gameState gamestate, std::atomic<int>& is_gameover, bool isPlayer = true);

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    gameState gamestate; // 점수, 레벨, 라인 상태
    Position boardOffset(5, 1); // 블록 생성 좌표
    BlockRender renderer(gamestate, boardOffset);

    show_logo(renderer);

    while (1)
    {
        // 새 게임 시작 시 상태 초기화
        std::atomic<int> is_gameover(0);
        {
            // 큐 비우기
            std::lock_guard<std::mutex> lock(Utils::inputMutex); // 스레드 동시 접근 방지
            Utils::playerInputQueue = queue<char>();
            Utils::aiInputQueue = queue<char>();
        }
        gamestate.resetState();

        // 시작 레벨 입력
        int startLevel = input_data();
        gamestate.setLevel(startLevel);


        // 스레드 생성
        thread tInput(inputThread, std::ref(is_gameover));
        thread t1(gameThread, gamestate, std::ref(is_gameover), true);
        Sleep(100);
        thread t2(gameThread, gamestate, std::ref(is_gameover), false);
        t1.join();
        t2.join();
        tInput.join();
        show_gameover();
        Utils::setColor(COLOR::GRAY);
    }

    return 0;
}

//*********************************
// 보조 함수 구현부
//*********************************

int input_data() {
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
    system("cls");

    return level - 1;
}

void show_logo(BlockRender& renderer)
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

    gameState tempGs;   // 임시 생성 (사용x)
    BlockGenerator gen(tempGs);

    for (i = 0;; ++i)
    {
        if (i % 40 == 0)
        {
            for (j = 0; j < 5; ++j)
            {
                Utils::gotoxy(17, 14 + j);
                printf("                                                          ");
            }

            Block blocks[4];
            gen.make_logo_blocks(blocks);

            for (int i = 0; i < 4; i++) {
                renderer.show_cur_block(blocks[i]);
            }
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

//*********************************
// 스레드 함수 구현부
//*********************************

void inputThread(std::atomic<int>& is_gameover)
{
    while (!(is_gameover == 1)) {
        if (_kbhit())
        {
            char keytemp = _getch();
            if (keytemp == EXT_KEY) {
                keytemp = _getch();
                std::lock_guard<std::mutex> lock(Utils::inputMutex); // 스레드 동시 접근 방지
                Utils::playerInputQueue.push(keytemp);
            }
            else if (keytemp == 32) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex); // 스레드 동시 접근 방지
                Utils::playerInputQueue.push(keytemp);
            }
            else if (keytemp == AI_LEFT || keytemp == AI_RIGHT || keytemp == AI_UP || keytemp == AI_DOWN || keytemp == AI_SPACE) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex); // 스레드 동시 접근 방지
                Utils::aiInputQueue.push(keytemp);
            }
        }
        Sleep(1);
    }
}

void gameThread(gameState gamestate, std::atomic<int>& is_gameover, bool isPlayer)
{
    Board board(isPlayer); // 쌓인 블록 / 벽 / 바닥 관리
    Position boardOffset(5, 1); // 블록 생성 좌표
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, boardOffset, isPlayer);
    BlockMover mover(renderer, board, blockGenerator, gamestate);

    board.init();

    board.draw(gamestate.getLevel());

    Block curBlock(blockGenerator.make_new_block());
    Block nextBlock(blockGenerator.make_new_block());

    curBlock.block_start();
    renderer.show_cur_block(curBlock);
    renderer.show_next_block(nextBlock);
    gamestate.show_gamestat(isPlayer, true);

    for (int i = 1;; i++) {
        std::queue<char>& myQueue = isPlayer ? Utils::playerInputQueue : Utils::aiInputQueue;
        char keytemp = 0;
        if (!myQueue.empty())
        {
            {
                keytemp = myQueue.front();
                myQueue.pop();
            }
            if (isPlayer)
            {
                switch (keytemp)
                {
                case KEY_UP:
                    mover.rotateBlock(curBlock);
                    break;
                case KEY_LEFT: // 왼쪽 이동
                    mover.movedLeft(curBlock);
                    break;
                case KEY_RIGHT: // 오른쪽 이동
                    mover.movedRight(curBlock);
                    break;
                case KEY_DOWN: // 한 칸 아래로
                    is_gameover = mover.move_block(curBlock, nextBlock);
                    gamestate.show_gamestat(isPlayer);
                    break;
                case 32:
                    while (is_gameover == 0)
                    {
                        is_gameover = mover.move_block(curBlock, nextBlock);
                    }
                    gamestate.show_gamestat(isPlayer);
                    break;
                }
            }
            else {
                switch (keytemp)
                {
                case AI_UP: // 회전
                    mover.rotateBlock(curBlock);
                    break;
                case AI_LEFT: // 왼쪽 이동
                    mover.movedLeft(curBlock);
                    break;
                case AI_RIGHT: // 오른쪽 이동
                    mover.movedRight(curBlock);
                    break;
                case AI_DOWN: // 한 칸 아래로
                    is_gameover = mover.move_block(curBlock, nextBlock);
                    gamestate.show_gamestat(isPlayer);
                    break;
                case AI_SPACE:
                    while (is_gameover == 0)
                    {
                        is_gameover = mover.move_block(curBlock, nextBlock);
                    }
                    gamestate.show_gamestat(isPlayer);
                    break;
                }
            }
        }
        // 자동 낙하
        if (i % STAGE::getStage(gamestate.getLevel()).getSpeed() == 0)
        {
            is_gameover = mover.move_block(curBlock, nextBlock);
            gamestate.show_gamestat(isPlayer);
        }

        // 스테이지 클리어
        if (STAGE::getStage(gamestate.getLevel()).getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(isPlayer);
            renderer.show_next_block(nextBlock);
        }

        if (is_gameover == 1)
        {
            return; // 새 게임 시작
        }

        {
            std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
            Utils::gotoxy(77, 23, true);
        }
        Sleep(15);
    }
}

