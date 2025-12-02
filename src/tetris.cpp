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
#define MENU 0x53   // menu
#define AI_SLEEP  150  // AI 동작 속도

#define SEC_LEFT 'a'
#define SEC_RIGHT 'd'
#define SEC_UP 'w'
#define SEC_DOWN 's'
#define SEC_DROP 'c'

#define MENU_QUIT 'q'
#define MENU_CONTINUE 'r'

//*********************************
// 함수 선언
//*********************************

// 게임 모드 입력
int input_mode();
// 시작 레벨 입력
int input_data();
// 로고 화면 + 랜덤 블록 애니메이션
void show_logo(BlockRender& renderer);
// 게임 오버 화면 표시
void show_gameover(int mode, int winner);

// 스레드 함수
void inputThread(std::atomic<int> &is_gameover, std::atomic<bool> &stopAI);
void playerThread(bool isLeft, gameState gamestate, std::atomic<int> &is_gameover, std::atomic<int> &winner, std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw);
void aiThread(gameState gamestate, std::atomic<int> &is_gameover, std::atomic<bool> &stopAI, const string& weightsFile, std::atomic<int> &winner, std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw);

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    string weightsFile = "initial_weights.txt";

    gameState gamestate;
    Position boardOffset(5, 1);
    Board board(true);
    BlockRender renderer(gamestate, boardOffset);

    ScoreManager scoreManager("scores.txt");

    while (1)
    {
        show_logo(renderer);
        std::atomic<int> is_gameover(0);
        std::atomic<bool> stopAI(false);
        std::atomic<int> winner(0);  // 0: none, 1: player, 2: AI
        std::atomic<bool> isGamePaused(false);
        std::atomic<bool> needRedraw(false);
        Utils::leftPlayerInputQueue = queue<char>();
        Utils::rightPlayerInputQueue = queue<char>();
        gamestate.resetState();

        int mode = input_mode(); // 0: single, 1: vs ai, 2: vs player
        int startLevel = input_data();
        gamestate.setLevel(startLevel);

        thread tInput = thread(inputThread, std::ref(is_gameover), std::ref(stopAI));
        thread t1 = thread(playerThread, true, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(isGamePaused), std::ref(needRedraw));
        thread t2;

        if (mode == 1) { // vs ai
            t2 = thread(aiThread, gamestate, std::ref(is_gameover), std::ref(stopAI), weightsFile, std::ref(winner), std::ref(isGamePaused), std::ref(needRedraw));
        } 
        else if (mode == 2) { // vs player
            t2 = thread(playerThread, false, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(isGamePaused), std::ref(needRedraw));
        }

        tInput.join();
        t1.join();
        if (t2.joinable()) t2.join();

        show_gameover(mode, winner);
    }

    return 0;
}
int input_mode() {
    int mode = -1;

    Utils::setColor(COLOR::GRAY);

    Utils::gotoxy(10, 3);
    printf("┏━━━━<GAME MODE>━━━━┓");
    Sleep(10);
    Utils::gotoxy(10, 4);
    printf("┃ 0: Single Player       ┃");
    Sleep(10);
    Utils::gotoxy(10, 5);
    printf("┃ 1: VS AI               ┃");
    Sleep(10);
    Utils::gotoxy(10, 6);
    printf("┃ 2: VS Player           ┃");
    Sleep(10);
    Utils::gotoxy(10, 7);
    printf("┗━━━━━━━━━━━━━━━━━━┛");
    Sleep(10);

    while (mode < 0 || mode > 2) {
        Utils::gotoxy(10, 9);
        printf("Select Mode[0-2]:       \b\b\b\b\b\b\b");
        cin >> mode;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(1024, '\n');
            continue;
        }
    }
    system("cls");

    return mode;
}

int input_data() {
    int level = 0;
    
    // 이전 입력 버퍼에 남아있는 개행 문자 제거
    cin.ignore(1024, '\n');

    Utils::setColor(COLOR::GRAY);

    Utils::gotoxy(10, 7);
    printf("┏━━━━<GAME KEY>━━━━━┓");
    Sleep(10);
    Utils::gotoxy(10, 8);
    printf("┃ UP   : Rotate Block                   ┃");
    Sleep(10);
    Utils::gotoxy(10, 9);
    printf("┃ DOWN : Move One-Step Down             ┃");
    Sleep(10);
    Utils::gotoxy(10, 10);
    printf("┃ SPACE: Move Bottom Down               ┃");
    Sleep(10);
    Utils::gotoxy(10, 11);
    printf("┃ LEFT : Move Left                      ┃");
    Sleep(10);
    Utils::gotoxy(10, 12);
    printf("┃ RIGHT: Move Right                     ┃");
    Sleep(10);
    Utils::gotoxy(10, 13);
    printf("┃ DELETE  : Go To Menu (Quit or Resume) ┃");
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

    system("cls");
    Utils::setColor(COLOR::GRAY);
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

            Block blocks[4];
            BlockGenerator::make_logo_blocks(blocks);

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

void show_gameover(int mode, int winner)
{
    if (winner == -1)
        return;

    // mode = 0: 무조건 게임오버 메시지
    if (mode == 0) {
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
    // mode = 1: 1이면 player 승, 2이면 ai 승
    else if (mode == 1) {
        if (winner == 1) {
            Utils::setColor(COLOR::GREEN);
            Utils::gotoxy(15, 8);
            printf("┏━━━━━━━━━━━━━┓");
            Utils::gotoxy(15, 9);
            printf("┃**************************┃");
            Utils::gotoxy(15, 10);
            printf("┃*      PLAYER WIN!       *┃");
            Utils::gotoxy(15, 11);
            printf("┃**************************┃");
            Utils::gotoxy(15, 12);
            printf("┗━━━━━━━━━━━━━┛");
        }
        else if (winner == 2) {
            Utils::setColor(COLOR::RED);
            Utils::gotoxy(15, 8);
            printf("┏━━━━━━━━━━━━━┓");
            Utils::gotoxy(15, 9);
            printf("┃**************************┃");
            Utils::gotoxy(15, 10);
            printf("┃*       AI WIN!          *┃");
            Utils::gotoxy(15, 11);
            printf("┃**************************┃");
            Utils::gotoxy(15, 12);
            printf("┗━━━━━━━━━━━━━┛");
        }
    }
    // mode = 2: 1이면 player1 승, 2이면 player2 승
    else if (mode == 2) {
        if (winner == 1) {
            Utils::setColor(COLOR::GREEN);
            Utils::gotoxy(15, 8);
            printf("┏━━━━━━━━━━━━━┓");
            Utils::gotoxy(15, 9);
            printf("┃**************************┃");
            Utils::gotoxy(15, 10);
            printf("┃*     PLAYER1 WIN!       *┃");
            Utils::gotoxy(15, 11);
            printf("┃**************************┃");
            Utils::gotoxy(15, 12);
            printf("┗━━━━━━━━━━━━━┛");
        }
        else if (winner == 2) {
            Utils::setColor(COLOR::RED);
            Utils::gotoxy(15, 8);
            printf("┏━━━━━━━━━━━━━┓");
            Utils::gotoxy(15, 9);
            printf("┃**************************┃");
            Utils::gotoxy(15, 10);
            printf("┃*     PLAYER2 WIN!       *┃");
            Utils::gotoxy(15, 11);
            printf("┃**************************┃");
            Utils::gotoxy(15, 12);
            printf("┗━━━━━━━━━━━━━┛");
        }
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
            
            if (keytemp == static_cast<char>(EXT_KEY)) {
                keytemp = _getch();
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::leftPlayerInputQueue.push(keytemp);
            }
            else if (keytemp == 32 || keytemp == MENU_QUIT || keytemp == MENU_CONTINUE) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::leftPlayerInputQueue.push(keytemp);
            } else if (
                keytemp == SEC_RIGHT || 
                keytemp == SEC_LEFT || 
                keytemp == SEC_DOWN || 
                keytemp == SEC_UP || 
                keytemp == SEC_DROP
            ) {
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::rightPlayerInputQueue.push(keytemp);
            }
        }
        Sleep(1);
    }
}

void playerThread(bool isLeft, gameState gamestate, std::atomic<int>& is_gameover, std::atomic<int>& winner, std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw) 
{
    srand(time(NULL) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    Board board(isLeft);
    Position boardOffset(5, 1);
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, boardOffset, isLeft);
    BlockMover mover(renderer, board, blockGenerator, gamestate);
    std::queue<char>& inputQueue = isLeft ? Utils::leftPlayerInputQueue : Utils::rightPlayerInputQueue;

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
    gamestate.show_gamestat(isLeft, true);

    for (int i = 1;; i++) {

        if (isGamePaused) {
            if (!isLeft) {
                Sleep(10);
                continue;
            }
        }

        if (!isLeft && needRedraw) {
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(isLeft, true);
            renderer.show_next_block(nextBlock);
            renderer.show_cur_block(curBlock);

            needRedraw = false;
        }

        if (is_gameover == 1) return;

        char keytemp = 0;
        bool hasInput = false;
        
        {
            std::lock_guard<std::mutex> lock(Utils::inputMutex);
            if (!inputQueue.empty()) {
                keytemp = inputQueue.front();
                inputQueue.pop();
                hasInput = true;
            }
        }

        if(hasInput){
            // MENU(Delete) 키를 눌렀을 때
            if (keytemp == MENU)
            {
                isGamePaused = true;
                std::unique_lock<std::recursive_mutex> pauseLock(Utils::gameMutex);

                system("cls");
                Utils::setColor(COLOR::GRAY);
                Utils::gotoxy(30, 10, true); printf("===== PAUSED =====");
                Utils::gotoxy(25, 12, true); printf("Press [Q] to Main Menu");
                Utils::gotoxy(25, 13, true); printf("Press [R] to Resume Game");

                while (true)
                {
                    if (is_gameover == 1) break;

                    char pauseKey = 0;
                    bool hasPauseInput = false;

                    {
                        std::lock_guard<std::mutex> inputLock(Utils::inputMutex);
                        if (!Utils::leftPlayerInputQueue.empty()) {
                            pauseKey = Utils::leftPlayerInputQueue.front();
                            Utils::leftPlayerInputQueue.pop();
                            hasPauseInput = true;
                        }
                    }

                    if (hasPauseInput)
                    {
                        if (pauseKey == MENU_QUIT)
                        {
                            winner = -1;
                            is_gameover = 1;
                            isGamePaused = false;
                            return;
                        }
                        else if (pauseKey == MENU_CONTINUE)
                        {
                            isGamePaused = false;

                            system("cls");
                            board.draw(gamestate.getLevel());
                            gamestate.show_gamestat(isLeft, true);
                            renderer.show_next_block(nextBlock);
                            renderer.show_cur_block(curBlock);
                            needRedraw = true;
                            break; 
                        }
                    }
                    Sleep(10);
                }
            }
            
            switch (keytemp)
            {
            case KEY_UP:
            case SEC_UP:
                mover.rotateBlock(curBlock);
                break;
            case KEY_LEFT:
            case SEC_LEFT:
                mover.movedLeft(curBlock);
                break;
            case KEY_RIGHT:
            case SEC_RIGHT:
                mover.movedRight(curBlock);
                break;
            case KEY_DOWN:
            case SEC_DOWN:
                is_gameover = mover.move_block(curBlock, nextBlock);
                gamestate.show_gamestat(isLeft);
                break;
            case 32:
            case SEC_DROP:
                is_gameover = hard_drop(board, curBlock, nextBlock, blockGenerator, mover, renderer, gamestate);
                gamestate.show_gamestat(isLeft);
                break;
            }
        }
        
        // 자동 낙하
        if (i % STAGE::getStage(gamestate.getLevel()).getSpeed() == 0)
        {
            is_gameover = mover.move_block(curBlock, nextBlock);
            if (is_gameover == 2) gamestate.show_gamestat(isLeft);
        }

        // 스테이지 클리어
        if (STAGE::getStage(gamestate.getLevel()).getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            if (is_gameover == 2) gamestate.show_gamestat(isLeft);
            renderer.show_next_block(nextBlock);
        }

        if (is_gameover == 1)
        {
            int expected = 0;
            winner.compare_exchange_strong(expected, 2); // 0일 때만 2로 설정 (플레이어 게임오버 → AI 승리)
            return;
        }
        
        Sleep(15);
    }
}

void aiThread(gameState gamestate, std::atomic<int>& is_gameover, std::atomic<bool>& stopAI, const string& weightsFile, std::atomic<int>& winner,
                std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw)
{
    srand(time(NULL) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    // AI Evaluator 초기화
    Evaluator evaluator;
    evaluator.loadWeights(weightsFile, true);
    
    Board board(false);  // AI 보드
    Position boardOffset(5, 1);
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, boardOffset, false);
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
    gamestate.show_gamestat(false, true);

    bool actionInProgress = false;
    Action targetAction;
    int targetRotation = 0;
    int targetColumn = 0;
    
    // 타이머를 사용한 action 사이 갭 조절
    auto lastActionTime = std::chrono::steady_clock::now();
    const auto actionGap = std::chrono::milliseconds(AI_SLEEP);

    for (int i = 1;; i++) {

        if (isGamePaused) {
            Sleep(10);
            continue;
        }

        if (needRedraw) {
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(false, true);
            renderer.show_next_block(nextBlock);
            renderer.show_cur_block(curBlock);

            needRedraw = false;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActionTime);
        // ESC로 AI 중단 시 수동 모드로 전환

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
                gamestate.show_gamestat(false);
                actionInProgress = false; // 행동 완료
                moved = true;
            }

            // 행동을 했다면 타이머 리셋
            if (moved) {
                lastActionTime = std::chrono::steady_clock::now();
            }
        }
        
        if (i % STAGE::getStage(gamestate.getLevel()).getSpeed() == 0)
        {
            is_gameover = mover.move_block(curBlock, nextBlock);
            gamestate.show_gamestat(false);
            
            // 자동 낙하로 블록이 바닥에 닿았는지 체크
            if (is_gameover == 2)  // 바닥에 닿음
            {
                actionInProgress = false;  // action 취소
                lastActionTime = std::chrono::steady_clock::now();  // 타이머 리셋
            }
        }

        // 스테이지 클리어 체크
        if (STAGE::getStage(gamestate.getLevel()).getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(false);
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

