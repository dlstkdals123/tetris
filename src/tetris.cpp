#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <iostream>

#include "Board.h"
#include "position.h"
#include "STAGE.h"
#include "COLOR.h"
#include "gameState.h"
#include "Block.h"
#include "BlockGenerator.h"
#include "BlockRender.h"
#include "BlockMover.h"
#include "Utils.h"
#include "Evaluator.h"
#include "ActionSimulator.h"
#include "ScoreManager.h"

#include <thread>
#include <atomic>
#include <chrono>

using namespace std;

//*********************************
// 상수 선언 (키 값)
//*********************************

#define EXT_KEY   0xffffffe0  // 확장키 인식값 
#define KEY_LEFT  0x4b
#define KEY_RIGHT 0x4d
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define AI_SLEEP  0  // AI 동작 속도

#define SEC_LEFT 'a'
#define SEC_RIGHT 'd'
#define SEC_UP 'w'
#define SEC_DOWN 's'
#define SEC_DROP 'c'

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

// 플레이 타입 선택
int select_playtype();
// 시작 레벨 입력
int input_data();
// 로고 화면 + 랜덤 블록 애니메이션
void show_logo(BlockRender& renderer);
// 게임 오버 화면 표시
void show_gameover(int winner);
// 하드드랍
int hard_drop(Board &board, Block &block, Block &nextBlock, BlockGenerator &blockGenerator, BlockMover &mover, BlockRender &renderer, gameState &gamestate);

// 스레드 함수
void inputThread(std::atomic<int> &is_gameover, std::atomic<bool> &stopAI);
void playerThread(gameState gamestate, std::atomic<int> &is_gameover, std::atomic<int> &winner, ScoreManager& scoreManager, bool isPlayer1 = true);
void aiThread(gameState gamestate, std::atomic<int> &is_gameover, std::atomic<bool> &stopAI, const string& weightsFile, std::atomic<int> &winner);

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    string weightsFile = "initial_weights.txt";

    gameState gamestate;
    Position boardOffset(5, 1);
    Board board(true);
    BlockRender renderer(gamestate, board, boardOffset);

    show_logo(renderer);
    ScoreManager scoreManager("scores.txt");

    while (1)
    {
        scoreManager.printTopN(3, 70, 5, true);
        std::atomic<int> is_gameover(0);
        std::atomic<bool> stopAI(false);
        std::atomic<int> winner(0);  // 0: none, 1: player, 2: AI
        
        {
            std::lock_guard<std::mutex> lock(Utils::inputMutex);
            Utils::playerInputQueue = queue<char>();
            Utils::aiInputQueue = queue<char>();
        }
        gamestate.resetState();

        int playType = select_playtype();
        int startLevel = input_data();
        gamestate.setLevel(startLevel);

        if(playType == 1) {
            scoreManager.printTopN(3, 70, 5, true);
        } else {
            scoreManager.printTopN(3, 40, 22, true);
        }

        if (playType == 1) // single play 
        {
            thread tInput(inputThread, std::ref(is_gameover), std::ref(stopAI));
            thread t1(playerThread, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(scoreManager), true);

            t1.join();
            tInput.join();
            show_gameover(0); 
        }
        else if (playType == 2) { // multiplay
            thread tInput(inputThread, std::ref(is_gameover), std::ref(stopAI));
            thread t1(playerThread, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(scoreManager), true);
            thread t2(playerThread, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(scoreManager), false);

            t1.join();
            t2.join();
            tInput.join();

            show_gameover(winner);
        } 
        else { // ai play
            // 스레드 생성: 키 입력 감지, 플레이어, AI
            thread tInput(inputThread, std::ref(is_gameover), std::ref(stopAI));
            thread t1(playerThread, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(scoreManager), true);
            Sleep(100);
            thread t2(aiThread, gamestate, std::ref(is_gameover), std::ref(stopAI), weightsFile, std::ref(winner));

            t1.join();
            t2.join();
            tInput.join();

            show_gameover(winner);
        }
        Utils::setColor(COLOR::GRAY);
    }

    return 0;
}

//*********************************
// 보조 함수 구현부
//*********************************
int select_playtype() {
    int playType = 0;

    Utils::setColor(COLOR::GRAY);

    Utils::gotoxy(10, 7);
    printf("┏━━━━<GAME KEY>━━━━━┓");
    Sleep(10);
    Utils::gotoxy(10, 8);
    printf("┃ press 1  :  Single Play     ┃");
    Sleep(10);
    Utils::gotoxy(10, 9);
    printf("┃ press 2  :  Multy Play      ┃");
    Sleep(10);
    Utils::gotoxy(10, 10);
    printf("┃ press 3  : Play with Alphago┃");
    Sleep(10);
    Utils::gotoxy(10, 11);
    printf("┗━━━━━━━━━━━━━━┛");

    while (playType < 1 || playType > 3)
    {
        Utils::gotoxy(10, 3);
        printf("Select Play Type[1-3]:       \b\b\b\b\b\b\b");

        cin >> playType;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(1024, '\n');
            continue;
        }
    }
    system("cls");

    return playType;
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

            {
                for (int i = 0; i < 4; i++) {
                    renderer.show_cur_block(blocks[i]);
                }
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
        printf("┃*      PLAYER1 WIN!      *┃");
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
        printf("┃*      PLAYER2 WIN!      *┃");
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

int hard_drop(Board &board, Block &block, Block &nextBlock, BlockGenerator &blockGenerator, BlockMover &mover, BlockRender &renderer, gameState &gamestate) {
    renderer.erase_cur_block(block);

    while (true) {
        block.moveDown();  
        if (board.isStrike(block)) {
            block.moveUp();
            break;
        }
    }
    
    board.mergeBlock(block);
    if (block.getPos().getY() <= 0) { 
        board.draw(gamestate.getLevel());
        return 1;
    }
    
    int deletedLines = board.deleteFullLine();
    if (deletedLines > 0) {
        gamestate.addLines(deletedLines);
        for (int i = 0; i < deletedLines; i++) {
            int score = 100 + gamestate.getLevel() * 10 + (rand() % 10);
            gamestate.addScore(score);
        }
    }
    block = nextBlock;
    block.block_start();
    nextBlock = Block(blockGenerator.make_new_block());
    renderer.show_next_block(nextBlock);
    mover.updateGhost(block);
    renderer.show_cur_block(block);   
    board.draw(gamestate.getLevel()); 
    
    return 2;
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
            
            if (keytemp == EXT_KEY) {
                keytemp = _getch();
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::playerInputQueue.push(keytemp);
            }
            else if (keytemp == 32) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::playerInputQueue.push(keytemp);
            } else if (
                keytemp == SEC_RIGHT || 
                keytemp == SEC_LEFT || 
                keytemp == SEC_DOWN || 
                keytemp == SEC_UP || 
                keytemp == SEC_DROP
            ) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::secPlayerInputQueue.push(keytemp);
            }
        }
        Sleep(1);
    }
}

void playerThread(gameState gamestate, std::atomic<int>& is_gameover, std::atomic<int>& winner, ScoreManager& scoreManager, bool isPlayer1) 
{
    srand(time(NULL) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    bool isPlayer = isPlayer1;
    Board board(isPlayer);
    Position boardOffset(5, 1);
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, board, boardOffset, isPlayer);
    BlockMover mover(renderer, board, blockGenerator, gamestate);

    board.init();
    board.draw(gamestate.getLevel());
    renderer.draw_next_block_frame();
    Block curBlock(blockGenerator.make_new_block());
    Block nextBlock(blockGenerator.make_new_block());

    curBlock.block_start();
    mover.updateGhost(curBlock);
    {
        std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
        renderer.show_cur_block(curBlock);
        renderer.show_next_block(nextBlock);
    }
    gamestate.show_gamestat(isPlayer, true);

    for (int i = 1;; i++) {
        std::queue<char>& myQueue = isPlayer ? Utils::playerInputQueue : Utils::secPlayerInputQueue;
        char keytemp = 0;
        
        if (!myQueue.empty())
        {
            keytemp = myQueue.front();
            myQueue.pop();
            if (isPlayer) {
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
                    gamestate.show_gamestat(isPlayer);
                    break;
                case 32:
                    is_gameover = hard_drop(board, curBlock, nextBlock, blockGenerator, mover, renderer, gamestate);
                    gamestate.show_gamestat(isPlayer);
                    break;
                }
            }
            else {
                switch (keytemp)
                {
                case SEC_UP:
                    mover.rotateBlock(curBlock);
                    break;
                case SEC_LEFT:
                    mover.movedLeft(curBlock);
                    break;
                case SEC_RIGHT:
                    mover.movedRight(curBlock);
                    break;
                case SEC_DOWN:
                    is_gameover = mover.move_block(curBlock, nextBlock);
                    gamestate.show_gamestat(isPlayer);
                    break;
                case SEC_DROP:
                    is_gameover = hard_drop(board, curBlock, nextBlock, blockGenerator, mover, renderer, gamestate);
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
            scoreManager.addScore(gamestate.getScore());
            int expected = 0;
            if (winner.compare_exchange_strong(expected, 2))  // 0일 때만 2로 설정 (플레이어 게임오버 → AI 승리)
            {
                return;
            }
            return;  // 이미 설정되어 있으면 그냥 리턴
        }
        
        Sleep(15);
    }
}

void aiThread(gameState gamestate, std::atomic<int>& is_gameover, std::atomic<bool>& stopAI, const string& weightsFile, std::atomic<int>& winner) 
{
    srand(time(NULL) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    bool isPlayer = false;
    // AI Evaluator 초기화
    Evaluator evaluator;
    evaluator.loadWeights(weightsFile);
    
    Board board(false);  // AI 보드
    Position boardOffset(5, 1);
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, board, boardOffset, false);
    BlockMover mover(renderer, board, blockGenerator, gamestate);

    board.init();
    board.draw(gamestate.getLevel());

    Block curBlock(blockGenerator.make_new_block());
    Block nextBlock(blockGenerator.make_new_block());

    curBlock.block_start();
    mover.updateGhost(curBlock);
    {
        std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
        renderer.show_cur_block(curBlock);
        renderer.show_next_block(nextBlock);
    }
    gamestate.show_gamestat(isPlayer, true);

    bool actionInProgress = false;
    Action targetAction;
    int targetRotation = 0;
    int targetColumn = 0;
    
    // 타이머를 사용한 action 사이 갭 조절
    auto lastActionTime = std::chrono::steady_clock::now();
    const auto actionGap = std::chrono::milliseconds(AI_SLEEP);

    for (int i = 1;; i++) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActionTime);
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
                    gamestate.show_gamestat(isPlayer);
                    actionInProgress = false;
                    break;
                case 'c':  // 하드 드롭
                    while (is_gameover == 0)
                    is_gameover = hard_drop(board, curBlock, nextBlock, blockGenerator, mover, renderer, gamestate);
                    gamestate.show_gamestat(isPlayer);
                    actionInProgress = false;
                    break;
                }
            }
        }
        else
        {
            // 시간이 아직 안 됐으면 이 블록을 건너뛰고 아래의 자동 낙하 로직으로 넘어갑니다.
            if (elapsed >= actionGap) 
            {
                // 1. 새로운 행동 결정 (아직 진행 중인 행동이 없다면)
                if (!actionInProgress)
                {
                    auto [bestAction, _] = evaluator.selectBestActionWithLookAhead(board, curBlock, &nextBlock, 1);
                    targetAction = bestAction;
                    targetRotation = bestAction.rotation;
                    targetColumn = bestAction.column;
                    actionInProgress = true;
                }
                
                // 2. 행동 실행 (회전 -> 이동 -> 드롭 순서)
                bool moved = false; // 이번 턴에 움직였는지 체크

                if (curBlock.getRotation() != targetRotation)
                {
                    mover.rotateBlock(curBlock);
                    moved = true;
                }
                else if (curBlock.getPos().getX() != targetColumn)
                {
                    if (curBlock.getPos().getX() < targetColumn) mover.movedRight(curBlock);
                    else mover.movedLeft(curBlock);
                    moved = true;
                }
                else
                {
                    // 위치와 회전이 맞으면 하드 드롭
                    while (is_gameover == 0)
                    {
                        is_gameover = mover.move_block(curBlock, nextBlock);
                        if (is_gameover != 0) break;
                    }
                    gamestate.show_gamestat(isPlayer);
                    actionInProgress = false; // 행동 완료
                    moved = true;
                }

                // 행동을 했다면 타이머 리셋
                if (moved) {
                    lastActionTime = std::chrono::steady_clock::now();
                }
            }
        }
        
        if (i % stage_data[gamestate.getLevel()].getSpeed() == 0)
        {
            int moveResult = mover.move_block(curBlock, nextBlock);
            gamestate.show_gamestat(isPlayer);
            
            // 자동 낙하로 블록이 바닥에 닿았는지 체크
            if (moveResult == 2)  // 바닥에 닿음
            {
                actionInProgress = false;  // action 취소
                lastActionTime = std::chrono::steady_clock::now();  // 타이머 리셋
            }
        }

        // 스테이지 클리어 체크
        if (stage_data[gamestate.getLevel()].getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(isPlayer);
            renderer.show_next_block(nextBlock);
        }

        if (is_gameover == 1)
        {
            int expected = 0;
            if (winner.compare_exchange_strong(expected, 1))  // 0일 때만 1로 설정 (AI 게임오버 → 플레이어 승리)
            {
                return;
            }
            return;  // 이미 설정되어 있으면 그냥 리턴
        }

        {
            std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
            Utils::gotoxy(77, 23, true);
        }
        
        Sleep(15);
    }
}

