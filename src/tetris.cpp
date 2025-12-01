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
#include "Evaluator.h"
#include "ActionSimulator.h"

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
#define AI_SLEEP  15

//*********************************
// 전역 상수 : 스테이지 데이터
//*********************************

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
    STAGE(4,  11, 99999)     // Level 10
};

//*********************************
// 함수 선언
//*********************************

void show_gamestat(const gameState &gs, bool isPlayer = true, bool printed_text = false);
int input_data();
void show_logo(BlockRender &renderer);
void show_gameover(int winner);

// 스레드 함수
void inputThread(std::atomic<int> &is_gameover, std::atomic<bool> &stopAI);
void playerThread(gameState gamestate, std::atomic<int> &is_gameover, std::atomic<int> &winner);
void aiThread(gameState gamestate, std::atomic<int> &is_gameover, std::atomic<bool> &stopAI, const string& weightsFile, std::atomic<int> &winner);

int main(int argc, char* argv[])
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    string weightsFile = "initial_weights.txt";
    if (argc > 1)
    {
        weightsFile = argv[1];
    }

    gameState gamestate;
    Position boardOffset(5, 1);
    BlockRender renderer(gamestate, boardOffset);

    show_logo(renderer);

    while (1)
    {
        std::atomic<int> is_gameover(0);
        std::atomic<bool> stopAI(false);
        std::atomic<int> winner(0);  // 0: none, 1: player, 2: AI
        
        {
            std::lock_guard<std::mutex> lock(Utils::inputMutex);
            Utils::playerInputQueue = queue<char>();
            Utils::aiInputQueue = queue<char>();
        }
        gamestate.resetState();

        int startLevel = input_data();
        gamestate.setLevel(startLevel);

        // 스레드 생성: 키 입력 감지, 플레이어, AI
        thread tInput(inputThread, std::ref(is_gameover), std::ref(stopAI));
        thread t1(playerThread, gamestate, std::ref(is_gameover), std::ref(winner));
        Sleep(100);
        thread t2(aiThread, gamestate, std::ref(is_gameover), std::ref(stopAI), weightsFile, std::ref(winner));
        
        t1.join();
        t2.join();
        tInput.join();
        
        show_gameover(winner);
        Utils::setColor(COLOR::GRAY);
    }

    return 0;
}

//*********************************
// 보조 함수 구현부
//*********************************

void show_gamestat(const gameState &gs, bool isPlayer, bool printed_text)
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
    Utils::setColor(COLOR::GRAY);

    if (printed_text)
    {
        Utils::gotoxy(35, 7, isPlayer);
        printf("STAGE");

        Utils::gotoxy(35, 9, isPlayer);
        printf("SCORE");

        Utils::gotoxy(35, 12, isPlayer);
        printf("LINES");
    }

    Utils::gotoxy(41, 7, isPlayer);
    printf("%d", gs.getLevel() + 1);

    Utils::gotoxy(35, 10, isPlayer);
    printf("%10d", gs.getScore());

    Utils::gotoxy(35, 13, isPlayer);
    int remain = stage_data[gs.getLevel()].getClearLine() - gs.getLines();
    if (remain < 0)
        remain = 0;
    printf("%10d", remain);
}

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
    printf("┃ ESC  : Stop AI (Right Side)┃");
    Sleep(10);
    Utils::gotoxy(10, 14);
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

    gameState tempGs;
    BlockGenerator gen(stage_data, tempGs);

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

void show_gameover(int winner)
{
    if (winner == 1)  // Player wins (AI lost)
    {
        Utils::setColor(COLOR::GREEN);
        Utils::gotoxy(15, 8);
        printf("┏━━━━━━━━━━━━━┓");
        Utils::gotoxy(15, 9);
        printf("┃**************************┃");
        Utils::gotoxy(15, 10);
        printf("┃*     YOU WIN! (AI LOST) *┃");
        Utils::gotoxy(15, 11);
        printf("┃**************************┃");
        Utils::gotoxy(15, 12);
        printf("┗━━━━━━━━━━━━━┛");
    }
    else if (winner == 2)  // AI wins (Player lost)
    {
        Utils::setColor(COLOR::RED);
        Utils::gotoxy(15, 8);
        printf("┏━━━━━━━━━━━━━┓");
        Utils::gotoxy(15, 9);
        printf("┃**************************┃");
        Utils::gotoxy(15, 10);
        printf("┃*    YOU LOSE! (AI WIN)  *┃");
        Utils::gotoxy(15, 11);
        printf("┃**************************┃");
        Utils::gotoxy(15, 12);
        printf("┗━━━━━━━━━━━━━┛");
    }
    else  // Both lost or draw
    {
        Utils::setColor(COLOR::YELLOW);
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
    }

    fflush(stdin);
    Sleep(1000);

    _getche();
    system("cls");
}

//*********************************
// 스레드 함수 구현부
//*********************************

void inputThread(std::atomic<int>& is_gameover, std::atomic<bool>& stopAI) 
{
    while (!(is_gameover == 1)) {
        if (_kbhit())
        {
            char keytemp = _getch();
            
            // ESC 키로 AI 중단
            if (keytemp == 27) {  // ESC
                stopAI = true;
                continue;
            }
            
            if (keytemp == EXT_KEY) {
                keytemp = _getch();
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::playerInputQueue.push(keytemp);
            }
            else if (keytemp == 32) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::playerInputQueue.push(keytemp);
            }
        }
        Sleep(1);
    }
}

void playerThread(gameState gamestate, std::atomic<int>& is_gameover, std::atomic<int>& winner) 
{
    Board board(true);
    Position boardOffset(5, 1);
    BlockGenerator blockGenerator(stage_data, gamestate);
    BlockRender renderer(gamestate, boardOffset, true);
    BlockMover mover(renderer, board, blockGenerator, gamestate);

    board.init();
    board.draw(gamestate.getLevel());

    Block curBlock(blockGenerator.make_new_block());
    Block nextBlock(blockGenerator.make_new_block());

    curBlock.block_start();
    renderer.show_cur_block(curBlock);
    renderer.show_next_block(nextBlock);
    show_gamestat(gamestate, true, true);

    for (int i = 1;; i++) {
        std::queue<char>& myQueue = Utils::playerInputQueue;
        char keytemp = 0;
        
        if (!myQueue.empty())
        {
            keytemp = myQueue.front();
            myQueue.pop();
            
            switch (keytemp)
            {
            case KEY_UP:
                mover.rotateBlock(curBlock);
                break;
            case KEY_LEFT:
                mover.movedLeft(curBlock);
                break;
            case KEY_RIGHT:
                mover.movedRight(curBlock);
                break;
            case KEY_DOWN:
                is_gameover = mover.move_block(curBlock, nextBlock);
                show_gamestat(gamestate, true);
                break;
            case 32:  // SPACE
                while (is_gameover == 0)
                {
                    is_gameover = mover.move_block(curBlock, nextBlock);
                }
                show_gamestat(gamestate, true);
                break;
            }
        }
        
        // 자동 낙하
        if (i % stage_data[gamestate.getLevel()].getSpeed() == 0)
        {
            is_gameover = mover.move_block(curBlock, nextBlock);
            show_gamestat(gamestate, true);
        }

        // 스테이지 클리어
        if (stage_data[gamestate.getLevel()].getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            show_gamestat(gamestate, true);
            renderer.show_next_block(nextBlock);
        }

        if (is_gameover == 1)
        {
            winner.store(2);
            return;
        }
        
        Sleep(15);
    }
}

void aiThread(gameState gamestate, std::atomic<int>& is_gameover, std::atomic<bool>& stopAI, const string& weightsFile, std::atomic<int>& winner) 
{
    // AI Evaluator 초기화
    Evaluator evaluator;
    evaluator.loadWeights(weightsFile);
    
    Board board(false);  // AI 보드
    Position boardOffset(5, 1);
    BlockGenerator blockGenerator(stage_data, gamestate);
    BlockRender renderer(gamestate, boardOffset, false);
    BlockMover mover(renderer, board, blockGenerator, gamestate);

    board.init();
    board.draw(gamestate.getLevel());

    Block curBlock(blockGenerator.make_new_block());
    Block nextBlock(blockGenerator.make_new_block());

    curBlock.block_start();
    renderer.show_cur_block(curBlock);
    renderer.show_next_block(nextBlock);
    show_gamestat(gamestate, false, true);

    bool actionInProgress = false;
    Action targetAction;
    int targetRotation = 0;
    int targetColumn = 0;

    for (int i = 1;; i++) {
        // ESC로 AI 중단 시 수동 모드로 전환
        if (stopAI)
        {
            // 수동 입력 대기
            std::queue<char>& myQueue = Utils::aiInputQueue;
            if (!myQueue.empty())
            {
                char keytemp = myQueue.front();
                myQueue.pop();
                
                switch (keytemp)
                {
                case 'w':  // 회전
                    mover.rotateBlock(curBlock);
                    break;
                case 'a':  // 왼쪽
                    mover.movedLeft(curBlock);
                    break;
                case 'd':  // 오른쪽
                    mover.movedRight(curBlock);
                    break;
                case 's':  // 아래
                    is_gameover = mover.move_block(curBlock, nextBlock);
                    show_gamestat(gamestate, false);
                    actionInProgress = false;
                    break;
                case 'c':  // 하드 드롭
                    while (is_gameover == 0)
                    {
                        is_gameover = mover.move_block(curBlock, nextBlock);
                    }
                    show_gamestat(gamestate, false);
                    actionInProgress = false;
                    break;
                }
            }
        }
        else
        {
            // AI 자동 플레이
            if (!actionInProgress)
            {
                // 새 블록에 대한 최적 액션 계산 (다음 블록까지 고려)
                auto [bestAction, _] = evaluator.selectBestActionWithLookAhead(board, curBlock, &nextBlock, 1);
                
                targetAction = bestAction;
                targetRotation = bestAction.rotation;
                targetColumn = bestAction.column;
                actionInProgress = true;
            }
            
            // 목표 회전까지 회전
            if (curBlock.getRotation() != targetRotation)
            {
                mover.rotateBlock(curBlock);
                Sleep(AI_SLEEP);  // AI 동작 시각화
            }
            // 목표 열까지 이동
            else if (curBlock.getPos().getX() != targetColumn)
            {
                if (curBlock.getPos().getX() < targetColumn)
                {
                    mover.movedRight(curBlock);
                }
                else
                {
                    mover.movedLeft(curBlock);
                }
                Sleep(AI_SLEEP);  // AI 동작 시각화
            }
            // 하드 드롭
            else
            {
                while (is_gameover == 0)
                {
                    is_gameover = mover.move_block(curBlock, nextBlock);
                    if (is_gameover != 0) break;
                }
                show_gamestat(gamestate, false);
                actionInProgress = false;
                Sleep(AI_SLEEP);  // 블록 배치 후 잠시 대기
            }
        }
        
        // 자동 낙하 (AI는 하드 드롭을 사용하므로 영향 적음)
        if (!actionInProgress && i % stage_data[gamestate.getLevel()].getSpeed() == 0)
        {
            is_gameover = mover.move_block(curBlock, nextBlock);
            show_gamestat(gamestate, false);
        }

        // 스테이지 클리어
        if (stage_data[gamestate.getLevel()].getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            show_gamestat(gamestate, false);
            renderer.show_next_block(nextBlock);
        }

        if (is_gameover == 1)
        {
            winner.store(1);  // Player wins
            return;
        }
        
        Sleep(15);
    }
}

