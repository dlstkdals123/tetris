#pragma once

/**
 * GameConstants
 * 게임 관련 모든 상수 정의
 */

namespace GameConstants {
    // 게임 모드
    namespace GameMode {
        constexpr int SINGLE_PLAYER = 1;
        constexpr int VS_AI = 2;
        constexpr int VS_PLAYER = 3;
        
        constexpr int MIN = 1;
        constexpr int MAX = 3;
    }
    
    // AI 난이도
    namespace AIDifficulty {
        constexpr int EASY = 1;
        constexpr int NORMAL = 2;
        constexpr int HARD = 3;
        
        constexpr int MIN = 1;
        constexpr int MAX = 3;
        constexpr int DEFAULT = NORMAL;
        
        // 난이도별 Epsilon 값
        constexpr double EASY_EPSILON = 0.2;
        constexpr double NORMAL_EPSILON = 0.1;
        constexpr double HARD_EPSILON = 0.0;
        constexpr double DEFAULT_EPSILON = NORMAL_EPSILON;
        
        // 난이도별 Delay 값 (밀리초)
        constexpr int EASY_DELAY = 500;
        constexpr int NORMAL_DELAY = 250;
        constexpr int HARD_DELAY = 150;
        constexpr int DEFAULT_DELAY = NORMAL_DELAY;
    }
    
    // 게임 상태
    namespace GameState {
        constexpr int CONTINUE = 0;
        constexpr int GAME_OVER = 1;
        constexpr int BLOCK_LANDED = 2;
    }
    
    // 승자
    namespace Winner {
        constexpr int NONE = 0;
        constexpr int PLAYER1 = 1;
        constexpr int PLAYER2 = 2;
        constexpr int PLAYER = 1;
        constexpr int AI = 2;
        constexpr int CANCELLED = -1;
    }
    
    // 레벨
    namespace Level {
        constexpr int MIN = 1;
        constexpr int MAX = 8;
    }
    
    // 스테이지
    namespace Stage {
        constexpr int COUNT = 10;
    }
    
    // 로고 애니메이션
    namespace LogoAnimation {
        constexpr int ANIMATION_INTERVAL = 40;
        constexpr int BLOCK_COUNT = 4;
        constexpr int CLEAR_LINES = 5;
        constexpr int START_ROW = 14;
    }
    
    // 로고 UI 좌표
    namespace LogoUI {
        constexpr int LINE_OFFSET = 1;
        constexpr int LINE_COUNT = 5;
    }
    
    // 게임 오버 UI 좌표
    namespace GameOverUI {
        constexpr int BOX_TOP_Y = 8;
        constexpr int BOX_BOTTOM_Y = 12;
        constexpr int TEXT_Y = 10;
        constexpr int BOX_TOP_1_Y = 9;
        constexpr int BOX_TOP_2_Y = 11;
    }
    
    // UI 좌표
    namespace UI {
        constexpr int MENU_LEFT_X = 10;
        constexpr int MENU_TOP_Y = 3;
        
        constexpr int LOGO_LEFT_X = 13;
        constexpr int LOGO_TOP_Y = 3;
        constexpr int LOGO_TEXT_Y = 20;
        constexpr int LOGO_TEXT_X = 28;
        constexpr int LOGO_BLOCK_X = 17;
        
        constexpr int GAME_OVER_X = 15;
        constexpr int GAME_OVER_Y = 8;
        
        constexpr int AI_CURSOR_X = 77;
        constexpr int AI_CURSOR_Y = 23;
        
        constexpr int KEY_INFO_Y = 7;
        constexpr int LEVEL_INPUT_Y = 3;
        constexpr int VS_PLAYER_KEY_OFFSET = 45;
        
        // Pause 메뉴 좌표
        constexpr int PAUSE_MENU_X = 30;
        constexpr int PAUSE_MENU_Y = 10;
        constexpr int PAUSE_TEXT_X = 25;
        constexpr int PAUSE_TEXT_START_Y = 12;
        constexpr int PAUSE_TEXT_OFFSET = 1;
    }
    
    // 시간 지연 (밀리초)
    namespace Delay {
        constexpr int MENU_SLEEP = 10;
        constexpr int LOGO_SLEEP = 100;
        constexpr int LOGO_ANIMATION = 30;
        constexpr int GAME_OVER_WAIT = 2000;
        constexpr int GAME_LOOP = 15;
        constexpr int PAUSE_LOOP = 10;
        constexpr int INPUT_THREAD = 1;
    }
    
    // 입력 버퍼
    namespace Input {
        constexpr int BUFFER_SIZE = 1024;
        constexpr char SPACE = 32;
    }
    
    // 기본 액션
    namespace DefaultAction {
        constexpr int ROTATION = 0;
        constexpr int COLUMN = 5;
    }
    
    // 보드 오프셋
    namespace BoardOffset {
        constexpr int X = 5;
        constexpr int Y = 1;
    }
    
    // UI 유틸리티
    namespace Utils {
        constexpr int RIGHT_PLAYER_X_OFFSET = 60;
        constexpr int GOTOXY_SUCCESS = 0;
    }
    
    // 블록 회전
    namespace BlockRotation {
        constexpr int INITIAL_ROTATION = 0;
        constexpr int DEFAULT = 0;
        constexpr int MAX_ROTATIONS = 4;
        constexpr int O_BLOCK_ROTATIONS = 1;
        constexpr int I_S_Z_BLOCK_ROTATIONS = 2;
        constexpr int ROTATION_INCREMENT = 1;
    }
    
    // 시뮬레이션 결과 초기값
    namespace Simulation {
        constexpr int INITIAL_LINES_CLEARED = 0;
        constexpr int NO_COLLISION = 0;
        constexpr int GAME_OVER_Y_THRESHOLD = 0;
    }
    
    // 블록 타입 및 배열 크기
    namespace BlockType {
        constexpr int COUNT = 7;
        constexpr int TOTAL_BLOCKS = 7;
        constexpr int SHAPE_DIMENSION = 4;
        constexpr int RANDOM_RANGE = 6;
        constexpr int RANDOM_OFFSET = 1;
    }
    
    // 로고 블록 생성
    namespace LogoBlock {
        constexpr int POS_X_1 = 6;
        constexpr int POS_X_2 = 12;
        constexpr int POS_X_3 = 19;
        constexpr int POS_X_4 = 24;
        constexpr int POS_Y = 14;
    }
    
    // 확률 계산
    namespace Probability {
        constexpr int MAX_PERCENT = 100;
    }
    
    // 점수 계산
    namespace Score {
        constexpr int BASE_SCORE = 100;
        constexpr int LEVEL_MULTIPLIER = 10;
        constexpr int RANDOM_BONUS_MAX = 10;
    }
    
    // 블록 렌더링
    namespace BlockRender {
        constexpr int SHAPE_SIZE = 4;
        constexpr int X_COORD_MULTIPLIER = 2;
        constexpr int CURSOR_X = 77;
        constexpr int CURSOR_Y = 23;
    }
    
    // 다음 블록 표시 박스
    namespace NextBlockBox {
        constexpr int START_Y = 1;
        constexpr int END_Y = 6;
        constexpr int WIDTH = 6;
        constexpr int X_POS = 33;
        constexpr int BORDER_TOP_Y = 1;
        constexpr int BORDER_BOTTOM_Y = 6;
        constexpr int BORDER_LEFT_X = 0;
        constexpr int BORDER_RIGHT_X = 5;
    }
    
    // 다음 블록 위치
    namespace NextBlock {
        constexpr int POS_X = 15;
        constexpr int POS_Y = 1;
    }
    
    // 레벨 색상
    namespace LevelColor {
        constexpr int COLOR_COUNT = 6;
        constexpr int COLOR_OFFSET = 1;
    }
    
    // 보드 반환값
    namespace BoardReturn {
        constexpr int SUCCESS = 0;
        constexpr int GAME_OVER = -1;
    }
    
    // Feature 추출 관련
    namespace Feature {
        constexpr int COLUMN_COUNT = 12;
        constexpr int HEIGHT_DIFF_COUNT = 11;
        constexpr int TOTAL_FEATURES = 26;
        constexpr double NORMALIZATION_VALUE = 20.0;
        constexpr int INITIAL_VALUE = 0;
        constexpr int WALL_CELL_VALUE = 1;
    }
    
    // Evaluator 평가
    namespace Evaluator {
        constexpr double DEATH_PENALTY = -100000.0;
        constexpr double DEFAULT_LOOK_AHEAD_DISCOUNT = 1.0;
        
        // 초기 가중치
        namespace InitialWeight {
            constexpr double MAX_HEIGHT = -1.0;
            constexpr double HOLES = -40.0;
            constexpr double WELLS = -4.0;
            constexpr double COLUMN_HEIGHT = -0.5;
            constexpr double HEIGHT_DIFF = -8.0;
        }
    }
    
    // 우물 계산
    namespace WellCalculation {
        constexpr int WELL_FORMULA_DIVISOR = 2;
    }
    
    // 게임 상태 초기값
    namespace GameStateInitial {
        constexpr int INITIAL_LEVEL = 0;
        constexpr int INITIAL_SCORE = 0;
        constexpr int INITIAL_LINES = 0;
        constexpr int LEVEL_DISPLAY_OFFSET = 1;
        constexpr int LEVEL_INCREMENT = 1;
        constexpr int MIN_REMAINING_LINES = 0;
    }
    
    // 게임 상태 UI 좌표
    namespace GameStatUI {
        constexpr int STAT_LEFT_X = 35;
        constexpr int LEVEL_VALUE_X = 41;
        constexpr int STAGE_TEXT_Y = 7;
        constexpr int SCORE_TEXT_Y = 9;
        constexpr int SCORE_VALUE_Y = 10;
        constexpr int LINES_TEXT_Y = 12;
        constexpr int LINES_VALUE_Y = 13;
        constexpr int TOP_SCORE_Y = 15;
        constexpr int SCORE_FORMAT_WIDTH = 10;
        inline constexpr const char* SCORE_FORMAT = "%10d";
    }
    
    // 점수 관리
    namespace ScoreManager {
        constexpr int MAX_SCORES = 100;
        constexpr int RANK_OFFSET = 1;
        constexpr int TOP_SCORE_OFFSET = 1;
        constexpr int TOP_N_COUNT = 3;
        constexpr int LOGO_TOP_SCORE_X = GameStatUI::STAT_LEFT_X;
        constexpr int LOGO_TOP_SCORE_Y = 10;
        constexpr int SINGLE_PLAYER_TOP_SCORE_X = GameStatUI::STAT_LEFT_X;
        constexpr int SINGLE_PLAYER_TOP_SCORE_Y = GameStatUI::TOP_SCORE_Y;
        constexpr int VS_MODE_TOP_SCORE_X = GameStatUI::STAT_LEFT_X;
        constexpr int VS_MODE_TOP_SCORE_Y = GameStatUI::TOP_SCORE_Y;
        inline constexpr const char* SCORES_FILE_NAME = "scores.txt";
    }
    
    // 입력 초기값
    namespace InputInitial {
        constexpr int MODE_DEFAULT = -1;
        constexpr int DIFFICULTY_DEFAULT = -1;
        constexpr int LEVEL_DEFAULT = 0;
        constexpr char KEY_DEFAULT = 0;
        constexpr char PAUSE_KEY_DEFAULT = 0;
    }
    
    // 확률 분포
    namespace ProbabilityDistribution {
        constexpr double MIN = 0.0;
        constexpr double MAX = 1.0;
    }
    
    // Look-ahead
    namespace LookAhead {
        constexpr double DEFAULT_DISCOUNT = 1.0;
    }
    
    // 반복문 시작값
    namespace Loop {
        constexpr int GAME_LOOP_START = 1;
    }
    
    // 딜레이
    namespace Delay {
        constexpr int PAUSE_SLEEP = 10;
    }
}

