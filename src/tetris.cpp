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
#include "GameConstants.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <cstdlib>

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
// AI 난이도 입력
int input_ai_difficulty();
// 시작 레벨 입력
int input_data(int mode = GameConstants::GameMode::SINGLE_PLAYER);
// 로고 화면 + 랜덤 블록 애니메이션
void show_logo(BlockRender& renderer, ScoreManager& scoreManager);
// 게임 오버 화면 표시
void show_gameover(int mode, int winner);
// 하드 드롭
int hard_drop(Board &board, Block &block, Block &nextBlock, BlockGenerator &blockGenerator, BlockMover &mover, BlockRender &renderer, gameState &gamestate);

// 스레드 함수
void inputThread(std::atomic<int> &is_gameover, std::atomic<bool> &stopAI);
void playerThread(bool isLeft ,gameState gamestate, std::atomic<int> &is_gameover, std::atomic<int> &winner, std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw, ScoreManager& scoreManager, int mode);
void aiThread(gameState gamestate, std::atomic<int> &is_gameover, std::atomic<bool> &stopAI, const string& weightsFile, std::atomic<int> &winner, std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw, int mode, int aiDifficulty);

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    string weightsFile = "initial_weights.txt";

    gameState gamestate;
    Position boardOffset(GameConstants::BoardOffset::X, GameConstants::BoardOffset::Y);
    Board board(true);
    BlockRender renderer(gamestate, boardOffset, board);

    ScoreManager scoreManager(GameConstants::ScoreManager::SCORES_FILE_NAME);

    while (1)
    {
        show_logo(renderer, scoreManager);
        std::atomic<int> is_gameover(GameConstants::GameState::CONTINUE);
        std::atomic<bool> stopAI(false);
        std::atomic<int> winner(GameConstants::Winner::NONE);
        std::atomic<bool> isGamePaused(false);
        std::atomic<bool> needRedraw(false);
        Utils::leftPlayerInputQueue = queue<char>();
        Utils::rightPlayerInputQueue = queue<char>();
        gamestate.resetState();

        int mode = input_mode();
        int aiDifficulty = GameConstants::AIDifficulty::DEFAULT;
        if (mode == GameConstants::GameMode::VS_AI) {
            aiDifficulty = input_ai_difficulty();
        }
        int startLevel = input_data(mode);
        gamestate.setLevel(startLevel);

        scoreManager.printTopN(GameConstants::ScoreManager::TOP_N_COUNT, GameConstants::ScoreManager::SINGLE_PLAYER_TOP_SCORE_X, GameConstants::ScoreManager::SINGLE_PLAYER_TOP_SCORE_Y);

        thread tInput = thread(inputThread, std::ref(is_gameover), std::ref(stopAI));
        thread t1 = thread(playerThread, true, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(isGamePaused), std::ref(needRedraw), std::ref(scoreManager), mode);
        thread t2;

        if (mode == GameConstants::GameMode::VS_AI) {
            t2 = thread(aiThread, gamestate, std::ref(is_gameover), std::ref(stopAI), weightsFile, std::ref(winner), std::ref(isGamePaused), std::ref(needRedraw), mode, aiDifficulty);
        } 
        else if (mode == GameConstants::GameMode::VS_PLAYER) {
            t2 = thread(playerThread, false, gamestate, std::ref(is_gameover), std::ref(winner), std::ref(isGamePaused), std::ref(needRedraw), std::ref(scoreManager), mode);
        }

        tInput.join();
        t1.join();
        if (t2.joinable()) t2.join();

        show_gameover(mode, winner);
    }

    return 0;
}
int input_mode() {
    int mode = GameConstants::InputInitial::MODE_DEFAULT;

    Utils::setColor(COLOR::GRAY);

    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y);
    printf("┏━━━━━━<GAME MODE>━━━━━━━┓");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET);
    printf("┃ 1: Single Player       ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 2);
    printf("┃ 2: VS AI               ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 3);
    printf("┃ 3: VS Player           ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 4);
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━┛");
    Sleep(GameConstants::Delay::MENU_SLEEP);

    while (mode < GameConstants::GameMode::MIN || mode > GameConstants::GameMode::MAX) {
        Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 5);
        printf("Select Mode[1-3]:       \b\b\b\b\b\b\b");
        cin >> mode;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(GameConstants::Input::BUFFER_SIZE, '\n');
            continue;
        }
    }
    system("cls");

    return mode;
}

int input_ai_difficulty() {
    int difficulty = GameConstants::InputInitial::DIFFICULTY_DEFAULT;
    
    // 이전 입력 버퍼에 남아있는 개행 문자 제거
    cin.ignore(GameConstants::Input::BUFFER_SIZE, '\n');
    
    Utils::setColor(COLOR::GRAY);
    
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y);
    printf("┏━━━━━<AI DIFFICULTY>━━━━┓");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET);
    printf("┃ 1: Easy                ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 2);
    printf("┃ 2: Normal              ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 3);
    printf("┃ 3: Hard                ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 4);
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━┛");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    
    while (difficulty < GameConstants::AIDifficulty::MIN || difficulty > GameConstants::AIDifficulty::MAX) {
        Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::MENU_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 5);
        printf("Select AI Difficulty[1-3]:       \b\b\b\b\b\b\b");
        cin >> difficulty;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(GameConstants::Input::BUFFER_SIZE, '\n');
            continue;
        }
    }
    system("cls");
    
    return difficulty;
}

int input_data(int mode) {
    int level = GameConstants::InputInitial::LEVEL_DEFAULT;
    
    // 이전 입력 버퍼에 남아있는 개행 문자 제거
    cin.ignore(GameConstants::Input::BUFFER_SIZE, '\n');

    Utils::setColor(COLOR::GRAY);

    // 항상 왼쪽에 기본(UP/DOWN/LEFT/RIGHT) 키 안내를 출력
    int leftX = GameConstants::UI::MENU_LEFT_X;
    int startY = GameConstants::UI::KEY_INFO_Y;

    Utils::gotoxy(leftX, startY);
    printf("┏━━━━━━━━━━━━━━<GAME KEY>━━━━━━━━━━━━━━━┓");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET);
    if (mode == GameConstants::GameMode::VS_PLAYER)
        printf("┃ LEFT PLAYER                           ┃");
    else
        printf("┃ PLAYER                                ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 2);
    printf("┃ UP    : Rotate Block                  ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 3);
    printf("┃ DOWN  : Move One-Step Down            ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 4);
    printf("┃ SPACE : Move Bottom Down              ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 5);
    printf("┃ LEFT  : Move Left                     ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 6);
    printf("┃ RIGHT : Move Right                    ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 7);
    printf("┃ DELETE: Go To Menu (Quit or Resume)   ┃");
    Sleep(GameConstants::Delay::MENU_SLEEP);
    Utils::gotoxy(leftX, startY + GameConstants::LogoUI::LINE_OFFSET * 8);
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");

    // VS Player일 때만 오른쪽에 RIGHT PLAYER 키(W A S D C) 안내 추가
    if (mode == GameConstants::GameMode::VS_PLAYER) {
        int rightX = GameConstants::UI::MENU_LEFT_X + GameConstants::UI::VS_PLAYER_KEY_OFFSET;

        Utils::gotoxy(rightX, startY);
        printf("┏━━━━━━━━━━━━━━<GAME KEY>━━━━━━━━━━━━━━━┓");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET);
        printf("┃ RIGHT PLAYER                          ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 2);
        printf("┃ W      : Rotate Block                 ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 3);
        printf("┃ S      : Move One-Step Down           ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 4);
        printf("┃ C      : Move Bottom Down             ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 5);
        printf("┃ A      : Move Left                    ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 6);
        printf("┃ D      : Move Right                   ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 7);
        printf("┃ DELETE: Go To Menu (Quit or Resume)   ┃");
        Sleep(GameConstants::Delay::MENU_SLEEP);
        Utils::gotoxy(rightX, startY + GameConstants::LogoUI::LINE_OFFSET * 8);
        printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
    }

    while (level < GameConstants::Level::MIN || level > GameConstants::Level::MAX)
    {
        Utils::gotoxy(GameConstants::UI::MENU_LEFT_X, GameConstants::UI::LEVEL_INPUT_Y);
        printf("Select Start level[1-8]:       \b\b\b\b\b\b\b");

        cin >> level;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(GameConstants::Input::BUFFER_SIZE, '\n');
            continue;
        }
    }
    system("cls");

    return level - GameConstants::Level::MIN;
}
void show_logo(BlockRender& renderer, ScoreManager& scoreManager)
{
    int i, j;

    system("cls");
    Utils::setColor(COLOR::GRAY);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y);
    printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
    Sleep(GameConstants::Delay::LOGO_SLEEP);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y + GameConstants::LogoUI::LINE_OFFSET);
    printf("┃◆◆◆◆◆  ◆◆◆  ◆◆◆◆◆   ◆      ◆   ◆◆◆ ┃");
    Sleep(GameConstants::Delay::LOGO_SLEEP);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 2);
    printf("┃  ◆    ◆      ◆     ◆ ◆    ◆   ◆   ┃");
    Sleep(GameConstants::Delay::LOGO_SLEEP);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 3);
    printf("┃  ◆    ◆◆◆    ◆     ◆◆     ◆    ◆  ┃");
    Sleep(GameConstants::Delay::LOGO_SLEEP);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 4);
    printf("┃  ◆    ◆      ◆     ◆ ◆    ◆     ◆ ┃");
    Sleep(GameConstants::Delay::LOGO_SLEEP);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 5);
    printf("┃  ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆ ┃");
    Sleep(GameConstants::Delay::LOGO_SLEEP);
    Utils::gotoxy(GameConstants::UI::LOGO_LEFT_X, GameConstants::UI::LOGO_TOP_Y + GameConstants::LogoUI::LINE_OFFSET * 6);
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");

    Utils::gotoxy(GameConstants::UI::LOGO_TEXT_X, GameConstants::UI::LOGO_TEXT_Y);
    printf("Please Press Any Key~!");

    scoreManager.printTopN(GameConstants::ScoreManager::TOP_N_COUNT, GameConstants::ScoreManager::LOGO_TOP_SCORE_X + 30, GameConstants::UI::LOGO_TOP_Y);

    while (_kbhit()) {
        _getch();  // 버퍼의 모든 키 제거
    }

    for (i = 0;; ++i)
    {
        if (i % GameConstants::LogoAnimation::ANIMATION_INTERVAL == 0)
        {
            for (j = 0; j < GameConstants::LogoAnimation::CLEAR_LINES; ++j)
            {
                Utils::gotoxy(GameConstants::UI::LOGO_BLOCK_X, GameConstants::LogoAnimation::START_ROW + j);
                printf("                                                          ");
            }

            Block blocks[GameConstants::LogoAnimation::BLOCK_COUNT];
            BlockGenerator::make_logo_blocks(blocks);

            {
                for (int i = 0; i < GameConstants::LogoAnimation::BLOCK_COUNT; i++) {
                    renderer.show_cur_block(blocks[i]);
                }
            }
        }

        if (_kbhit())
            break;

        Sleep(GameConstants::Delay::LOGO_ANIMATION);
    }

    _getche();
    system("cls");
}

void show_gameover(int mode, int winner)
{
    if (winner == GameConstants::Winner::CANCELLED)
        return;

    // Single Player 모드: 무조건 게임오버 메시지
    if (mode == GameConstants::GameMode::SINGLE_PLAYER) {
        Utils::setColor(COLOR::YELLOW);
        Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_Y);
        printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
        Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_1_Y);
        printf("┃**************************┃");
        Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::TEXT_Y);
        printf("┃*        GAME OVER       *┃");
        Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_2_Y);
        printf("┃**************************┃");
        Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_BOTTOM_Y);
        printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
    }
    // VS AI 모드: Player 또는 AI 승리
    else if (mode == GameConstants::GameMode::VS_AI) {
        if (winner == GameConstants::Winner::PLAYER) {
            Utils::setColor(COLOR::GREEN);
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_Y);
            printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_1_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::TEXT_Y);
            printf("┃*       PLAYER WIN!      *┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_2_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_BOTTOM_Y);
            printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
        }
        else if (winner == GameConstants::Winner::AI) {
            Utils::setColor(COLOR::RED);
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_Y);
            printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_1_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::TEXT_Y);
            printf("┃*        AI WIN!         *┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_2_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_BOTTOM_Y);
            printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
        }
    }
    // VS Player 모드: Player1 또는 Player2 승리
    else if (mode == GameConstants::GameMode::VS_PLAYER) {
        if (winner == GameConstants::Winner::PLAYER1) {
            Utils::setColor(COLOR::GREEN);
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_Y);
            printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_1_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::TEXT_Y);
            printf("┃*     PLAYER1 WIN!       *┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_2_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_BOTTOM_Y);
            printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
        }
        else if (winner == GameConstants::Winner::PLAYER2) {
            Utils::setColor(COLOR::RED);
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_Y);
            printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_1_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::TEXT_Y);
            printf("┃*      PLAYER2 WIN!      *┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_TOP_2_Y);
            printf("┃**************************┃");
            Utils::gotoxy(GameConstants::UI::GAME_OVER_X, GameConstants::GameOverUI::BOX_BOTTOM_Y);
            printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
        }
    }

    Sleep(GameConstants::Delay::GAME_OVER_WAIT);
    system("cls");
}

int hard_drop(Board &board, Block &block, Block &nextBlock, BlockGenerator &blockGenerator, BlockMover &mover, BlockRender &renderer, gameState &gamestate) {
    renderer.erase_cur_block(block);

    // 현재 위치에서 이미 충돌하는지 확인 (공격 라인으로 인한 충돌)
    if (board.isStrike(block) == BoardConstants::STRIKE_TRUE) {
        // 공격 라인이 올라와서 블록과 겹친 상황
        // 충돌이 해소될 때까지 한 칸씩 위로 이동
        while (board.isStrike(block) == BoardConstants::STRIKE_TRUE) {
            block.moveUp();
            
            // 게임오버 체크 (블록이 화면 밖으로 나갔는지)
            if (block.getPos().getY() < GameConstants::Simulation::GAME_OVER_Y_THRESHOLD) {
                board.draw(gamestate.getLevel());
                return GameConstants::GameState::GAME_OVER;
            }
        }
        // 충돌이 해소되었으므로, 블록이 바닥에 부딪힌 것처럼 착지 처리
        board.mergeBlock(block);
    } else {
        // 기존 로직: 바닥까지 떨어뜨림
        while (true) {
            block.moveDown();  
            if (board.isStrike(block)) {
                block.moveUp();
                break;
            }
        }
        
        board.mergeBlock(block);
    }
    if (block.getPos().getY() <= GameConstants::Simulation::GAME_OVER_Y_THRESHOLD) { 
        board.draw(gamestate.getLevel());
        return GameConstants::GameState::GAME_OVER;
    }
    
    auto lineResult = board.deleteFullLine();
    int deletedLines = lineResult.first; // 총 삭제된 줄 수
    int attackableLines = lineResult.second; // attack 가능한 줄 수
    
    if (deletedLines > 0) {
        gamestate.addLines(deletedLines);
        for (int i = 0; i < deletedLines; i++) {
            int score = GameConstants::Score::BASE_SCORE + gamestate.getLevel() * GameConstants::Score::LEVEL_MULTIPLIER + (rand() % GameConstants::Score::RANDOM_BONUS_MAX);
            gamestate.addScore(score);
        }
        
        // Attack 라인 추가 (VS AI 또는 VS Player 모드일 때만)
        // attack 가능한 줄 수만큼만 attack
        int gameMode = mover.getGameMode();
        bool isLeftPlayer = mover.getIsLeftPlayer();
        if ((gameMode == GameConstants::GameMode::VS_AI || gameMode == GameConstants::GameMode::VS_PLAYER) && attackableLines > 0) {
            Board* opponentBoard = isLeftPlayer ? Utils::rightPlayerBoard : Utils::leftPlayerBoard;

            if (opponentBoard != nullptr) {
                opponentBoard->addAttackLines(attackableLines);
                opponentBoard->draw(gamestate.getLevel());
            }
        }
    }
    
    block = nextBlock;
    block.block_start();
    nextBlock = Block(blockGenerator.make_new_block());
    renderer.show_next_block(nextBlock);
    mover.updateGhost(block);
    board.draw(gamestate.getLevel());
    renderer.show_cur_block(block);   
    
    return GameConstants::GameState::BLOCK_LANDED;
}

//*********************************
// 스레드 함수 구현부
//*********************************

void inputThread(std::atomic<int>& is_gameover, std::atomic<bool>& stopAI) 
{
    while (!(is_gameover == GameConstants::GameState::GAME_OVER)) {
        if (_kbhit())
        {
            char keytemp = _getch();
            
            if (keytemp == static_cast<char>(EXT_KEY)) {
                keytemp = _getch();
                std::lock_guard<std::mutex> lock(Utils::inputMutex);
                Utils::leftPlayerInputQueue.push(keytemp);
            }
            else if (keytemp == GameConstants::Input::SPACE || keytemp == MENU_QUIT || keytemp == MENU_CONTINUE) {
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
        Sleep(GameConstants::Delay::INPUT_THREAD);
    }
}

void playerThread(bool isLeft, gameState gamestate, std::atomic<int>& is_gameover, std::atomic<int>& winner, 
                    std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw, ScoreManager& scoreManager, int mode)
{
    srand(time(NULL) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    Board board(isLeft);
    int localGameOver = GameConstants::GameState::CONTINUE;
    
    // Board 포인터 등록
    if (isLeft) {
        Utils::leftPlayerBoard = &board;
    } else {
        Utils::rightPlayerBoard = &board;
    }
    
    Position boardOffset(GameConstants::BoardOffset::X, GameConstants::BoardOffset::Y);
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, boardOffset, board, isLeft);
    BlockMover mover(renderer, board, blockGenerator, gamestate, mode, isLeft);
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

    for (int i = GameConstants::Loop::GAME_LOOP_START;; i++) {

        if (isGamePaused) {
            if (!isLeft) {
                Sleep(GameConstants::Delay::PAUSE_SLEEP);
                continue;
            }
        }

        if (!isLeft && needRedraw) {
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(isLeft, true);
            renderer.show_next_block(nextBlock);
            renderer.show_cur_block(curBlock);
            mover.updateGhost(curBlock);

            needRedraw = false;
        }

        if (is_gameover == GameConstants::GameState::GAME_OVER) return;

        char keytemp = GameConstants::InputInitial::KEY_DEFAULT;
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
                Utils::gotoxy(GameConstants::UI::PAUSE_MENU_X, GameConstants::UI::PAUSE_MENU_Y, true); printf("===== PAUSED =====");
                Utils::gotoxy(GameConstants::UI::PAUSE_TEXT_X, GameConstants::UI::PAUSE_TEXT_START_Y, true); printf("Press [q] to Main Menu");
                Utils::gotoxy(GameConstants::UI::PAUSE_TEXT_X, GameConstants::UI::PAUSE_TEXT_START_Y + GameConstants::UI::PAUSE_TEXT_OFFSET, true); printf("Press [r] to Resume Game");

                while (true)
                {
                    if (is_gameover == GameConstants::GameState::GAME_OVER) break;

                    char pauseKey = GameConstants::InputInitial::PAUSE_KEY_DEFAULT;
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
                            winner = GameConstants::Winner::CANCELLED;
                            is_gameover = GameConstants::GameState::GAME_OVER;
                            isGamePaused = false;
                            return;
                        }
                        else if (pauseKey == MENU_CONTINUE)
                        {
                            isGamePaused = false;

                            std::lock_guard<std::mutex> inputLock(Utils::inputMutex);
                            system("cls");
                            board.draw(gamestate.getLevel());
                            gamestate.show_gamestat(isLeft, true);
                            renderer.show_next_block(nextBlock);
                            renderer.show_cur_block(curBlock);
                            mover.updateGhost(curBlock);
                            needRedraw = true;
                            break; 
                        }
                    }
                    Sleep(GameConstants::Delay::PAUSE_SLEEP);
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
                localGameOver = mover.move_block(curBlock, nextBlock);
                gamestate.show_gamestat(isLeft);
                break;
            case GameConstants::Input::SPACE:
            case SEC_DROP:
                localGameOver = hard_drop(board, curBlock, nextBlock, blockGenerator, mover, renderer, gamestate);
                gamestate.show_gamestat(isLeft);
                break;
            }
        }
        
        // 자동 낙하
        if (i % STAGE::getStage(gamestate.getLevel()).getSpeed() == 0)
        {
            localGameOver = mover.move_block(curBlock, nextBlock);
            if (localGameOver == GameConstants::GameState::BLOCK_LANDED) gamestate.show_gamestat(isLeft);
        }

        // 스테이지 클리어
        if (STAGE::getStage(gamestate.getLevel()).getClearLine() <= gamestate.getLines())
        {
            gamestate.levelUp();
            board.draw(gamestate.getLevel());
            if (localGameOver == GameConstants::GameState::BLOCK_LANDED) gamestate.show_gamestat(isLeft);
            renderer.show_next_block(nextBlock);
        }

        if (localGameOver == GameConstants::GameState::GAME_OVER)
        {
            scoreManager.addScore(gamestate.getScore());
            is_gameover = GameConstants::GameState::GAME_OVER;
            if (isLeft) {
                Utils::rightPlayerBoard = nullptr;
                winner = GameConstants::Winner::AI;
            } else {
                Utils::leftPlayerBoard = nullptr;
                winner = GameConstants::Winner::PLAYER;
            }
            return;
        }

        if (is_gameover == GameConstants::GameState::GAME_OVER) {
            return;
        }
        
        Sleep(GameConstants::Delay::GAME_LOOP);
    }
}

void aiThread(gameState gamestate, std::atomic<int>& is_gameover, std::atomic<bool>& stopAI, const string& weightsFile, std::atomic<int>& winner,
                std::atomic<bool>& isGamePaused, std::atomic<bool>& needRedraw, int mode, int aiDifficulty)
{
    srand(time(NULL) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    // AI Evaluator 초기화
    Evaluator evaluator;
    evaluator.loadWeights(weightsFile);
    
    // 난이도별 설정
    double epsilon;
    int delayMs;
    switch(aiDifficulty) {
        case GameConstants::AIDifficulty::EASY: epsilon = GameConstants::AIDifficulty::EASY_EPSILON; delayMs = GameConstants::AIDifficulty::EASY_DELAY; break;
        case GameConstants::AIDifficulty::NORMAL: epsilon = GameConstants::AIDifficulty::NORMAL_EPSILON; delayMs = GameConstants::AIDifficulty::NORMAL_DELAY; break;
        case GameConstants::AIDifficulty::HARD: epsilon = GameConstants::AIDifficulty::HARD_EPSILON; delayMs = GameConstants::AIDifficulty::HARD_DELAY; break;
        default: epsilon = GameConstants::AIDifficulty::DEFAULT_EPSILON; delayMs = GameConstants::AIDifficulty::DEFAULT_DELAY; break; // 기본값: Normal
    }
    
    Board board(false);  // AI 보드
    int localGameOver = GameConstants::GameState::CONTINUE;
    
    // Board 포인터 등록
    Utils::rightPlayerBoard = &board;
    
    Position boardOffset(GameConstants::BoardOffset::X, GameConstants::BoardOffset::Y);
    BlockGenerator blockGenerator(gamestate);
    BlockRender renderer(gamestate, boardOffset, board, false);
    BlockMover mover(renderer, board, blockGenerator, gamestate, mode, false);

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
    int targetRotation = GameConstants::BlockRotation::INITIAL_ROTATION;
    int targetColumn = GameConstants::DefaultAction::COLUMN;
    
    // 타이머를 사용한 action 사이 갭 조절 (난이도별 동적 delay)
    auto lastActionTime = std::chrono::steady_clock::now();
    const auto actionGap = std::chrono::milliseconds(delayMs);

    for (int i = GameConstants::Loop::GAME_LOOP_START;; i++) {

        if (isGamePaused) {
            Sleep(GameConstants::Delay::PAUSE_LOOP);
            continue;
        }

        if (needRedraw) {
            board.draw(gamestate.getLevel());
            gamestate.show_gamestat(false, true);
            renderer.show_next_block(nextBlock);
            renderer.show_cur_block(curBlock);
            mover.updateGhost(curBlock);

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
                // Epsilon-greedy 정책: epsilon 확률로 랜덤 액션 선택
                if ((double)rand() / RAND_MAX < epsilon)
                {
                    // Exploration: 유효한 액션 중에서 랜덤 선택
                    vector<SimulationResult> validActions = ActionSimulator::simulateAllActions(board, curBlock);
                    if (!validActions.empty())
                    {
                        targetAction = validActions[rand() % validActions.size()].action;
                    }
                    else
                    {
                        // 유효한 액션이 없으면 기본 액션 (게임 오버 상황)
                        targetAction = Action(GameConstants::DefaultAction::ROTATION, GameConstants::DefaultAction::COLUMN);
                    }
                }
                else
                {
                    // Exploitation: 최선의 액션 선택
                    auto [bestAction, _] = evaluator.selectBestActionWithLookAhead(board, curBlock, &nextBlock, GameConstants::LookAhead::DEFAULT_DISCOUNT);
                    targetAction = bestAction;
                }
                
                targetRotation = targetAction.rotation;
                targetColumn = targetAction.column;
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
                while (localGameOver == GameConstants::GameState::CONTINUE)
                {
                    localGameOver = mover.move_block(curBlock, nextBlock);
                    if (localGameOver != GameConstants::GameState::CONTINUE) break;
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
            localGameOver = mover.move_block(curBlock, nextBlock);
            gamestate.show_gamestat(false);
            
            // 자동 낙하로 블록이 바닥에 닿았는지 체크
            if (localGameOver == GameConstants::GameState::BLOCK_LANDED)  // 바닥에 닿음
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

        if (localGameOver == GameConstants::GameState::GAME_OVER)
        {
            winner = GameConstants::Winner::PLAYER;
            is_gameover = GameConstants::GameState::GAME_OVER;
            Utils::rightPlayerBoard = nullptr;
            return;
        }

        {
            std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex);
            Utils::gotoxy(GameConstants::UI::AI_CURSOR_X, GameConstants::UI::AI_CURSOR_Y, true);
        }

        if (is_gameover == GameConstants::GameState::GAME_OVER) {
            return;
        }
        
        Sleep(GameConstants::Delay::GAME_LOOP);
    }
}

