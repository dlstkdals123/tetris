# Tetris 프로젝트 Makefile
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
LDFLAGS = 

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
COMMON_SRCS = $(SRC_DIR)/Board.cpp \
              $(SRC_DIR)/Utils.cpp \
              $(SRC_DIR)/COLOR.cpp \
              $(SRC_DIR)/position.cpp \
              $(SRC_DIR)/rotation.cpp \
              $(SRC_DIR)/STAGE.cpp \
              $(SRC_DIR)/gameState.cpp \
              $(SRC_DIR)/BlockGenerator.cpp \
              $(SRC_DIR)/BlockRender.cpp \
              $(SRC_DIR)/BlockMover.cpp

TETRIS_SRCS = $(SRC_DIR)/tetris.cpp $(COMMON_SRCS)
FEATURE_TEST_SRCS = $(SRC_DIR)/test_features.cpp \
                    $(SRC_DIR)/FeatureExtractor.cpp \
                    $(SRC_DIR)/Board.cpp \
                    $(SRC_DIR)/Utils.cpp \
                    $(SRC_DIR)/COLOR.cpp \
                    $(SRC_DIR)/position.cpp \
                    $(SRC_DIR)/rotation.cpp

ACTION_TEST_SRCS = $(SRC_DIR)/test_action_simulator.cpp \
                   $(SRC_DIR)/ActionSimulator.cpp \
                   $(SRC_DIR)/FeatureExtractor.cpp \
                   $(SRC_DIR)/Board.cpp \
                   $(SRC_DIR)/Utils.cpp \
                   $(SRC_DIR)/COLOR.cpp \
                   $(SRC_DIR)/position.cpp \
                   $(SRC_DIR)/rotation.cpp

EVALUATOR_TEST_SRCS = $(SRC_DIR)/test_evaluator.cpp \
                      $(SRC_DIR)/Evaluator.cpp \
                      $(SRC_DIR)/ActionSimulator.cpp \
                      $(SRC_DIR)/FeatureExtractor.cpp \
                      $(SRC_DIR)/Board.cpp \
                      $(SRC_DIR)/Utils.cpp \
                      $(SRC_DIR)/COLOR.cpp \
                      $(SRC_DIR)/position.cpp \
                      $(SRC_DIR)/rotation.cpp

TD_LEARNER_TEST_SRCS = $(SRC_DIR)/test_td_learner.cpp \
                       $(SRC_DIR)/TDLearner.cpp \
                       $(SRC_DIR)/Evaluator.cpp \
                       $(SRC_DIR)/ActionSimulator.cpp \
                       $(SRC_DIR)/FeatureExtractor.cpp \
                       $(SRC_DIR)/Board.cpp \
                       $(SRC_DIR)/Utils.cpp \
                       $(SRC_DIR)/COLOR.cpp \
                       $(SRC_DIR)/position.cpp \
                       $(SRC_DIR)/rotation.cpp

TRAIN_MULTISTAGE_SRCS = $(SRC_DIR)/train_multistage.cpp \
                        $(SRC_DIR)/TDLearner.cpp \
                        $(SRC_DIR)/Evaluator.cpp \
                        $(SRC_DIR)/ActionSimulator.cpp \
                        $(SRC_DIR)/FeatureExtractor.cpp \
                        $(SRC_DIR)/Board.cpp \
                        $(SRC_DIR)/Utils.cpp \
                        $(SRC_DIR)/COLOR.cpp \
                        $(SRC_DIR)/position.cpp \
                        $(SRC_DIR)/rotation.cpp

# Executables
TETRIS_EXE = tetris.exe
FEATURE_TEST_EXE = test_features.exe
ACTION_TEST_EXE = test_action_simulator.exe
EVALUATOR_TEST_EXE = test_evaluator.exe
TD_LEARNER_TEST_EXE = test_td_learner.exe
TRAIN_MULTISTAGE_EXE = train_multistage.exe

# Default target
all: $(TETRIS_EXE) $(FEATURE_TEST_EXE) $(ACTION_TEST_EXE) $(EVALUATOR_TEST_EXE) $(TD_LEARNER_TEST_EXE)

# Build tetris game
$(TETRIS_EXE): $(TETRIS_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Tetris game compiled successfully!"

# Build feature extractor test
$(FEATURE_TEST_EXE): $(FEATURE_TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Feature test compiled successfully!"

# Build action simulator test
$(ACTION_TEST_EXE): $(ACTION_TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Action simulator test compiled successfully!"

# Build evaluator test
$(EVALUATOR_TEST_EXE): $(EVALUATOR_TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Evaluator test compiled successfully!"

# Build TD learner test
$(TD_LEARNER_TEST_EXE): $(TD_LEARNER_TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "TD learner test compiled successfully!"

# Build multi-stage training
$(TRAIN_MULTISTAGE_EXE): $(TRAIN_MULTISTAGE_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Multi-stage training compiled successfully!"

# Build only tetris
tetris: $(TETRIS_EXE)

# Build only feature test
test: $(FEATURE_TEST_EXE)

# Build only action simulator test
test-action: $(ACTION_TEST_EXE)

# Build only evaluator test
test-evaluator: $(EVALUATOR_TEST_EXE)

# Build only TD learner test
test-td: $(TD_LEARNER_TEST_EXE)

# Build multi-stage training
train: $(TRAIN_MULTISTAGE_EXE)

# Run feature test
run-test: $(FEATURE_TEST_EXE)
	./$(FEATURE_TEST_EXE)

# Run action simulator test
run-action-test: $(ACTION_TEST_EXE)
	./$(ACTION_TEST_EXE)

# Run evaluator test
run-evaluator-test: $(EVALUATOR_TEST_EXE)
	./$(EVALUATOR_TEST_EXE)

# Run TD learner test
run-td-test: $(TD_LEARNER_TEST_EXE)
	./$(TD_LEARNER_TEST_EXE)

# Run multi-stage training
run-train: $(TRAIN_MULTISTAGE_EXE)
	./$(TRAIN_MULTISTAGE_EXE)

# Run tetris game
run-tetris: $(TETRIS_EXE)
	./$(TETRIS_EXE)

# Clean build artifacts
clean:
	rm -f $(TETRIS_EXE) $(FEATURE_TEST_EXE) $(ACTION_TEST_EXE) $(EVALUATOR_TEST_EXE) $(TD_LEARNER_TEST_EXE) $(TRAIN_MULTISTAGE_EXE)
	rm -f test_weights.txt test_td_weights.txt test_training_progress.csv
	rm -f weights_episode_*.txt final_weights_multistage.txt training_progress_multistage.csv
	@echo "Cleaned build artifacts"

# Help target
help:
	@echo "Available targets:"
	@echo "  all                - Build everything (default)"
	@echo "  tetris             - Build tetris game only"
	@echo "  test               - Build feature test only"
	@echo "  test-action        - Build action simulator test only"
	@echo "  test-evaluator     - Build evaluator test only"
	@echo "  test-td            - Build TD learner test only"
	@echo "  train              - Build multi-stage training program"
	@echo "  run-test           - Build and run feature test"
	@echo "  run-action-test    - Build and run action simulator test"
	@echo "  run-evaluator-test - Build and run evaluator test"
	@echo "  run-td-test        - Build and run TD learner test"
	@echo "  run-train          - Build and run multi-stage training (30K episodes!)"
	@echo "  run-tetris         - Build and run tetris game"
	@echo "  clean              - Remove all executables"
	@echo "  help               - Show this help message"

.PHONY: all tetris test test-action test-evaluator test-td train run-test run-action-test run-evaluator-test run-td-test run-train run-tetris clean help

