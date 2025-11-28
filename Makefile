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

# Executables
TETRIS_EXE = tetris.exe
FEATURE_TEST_EXE = test_features.exe
ACTION_TEST_EXE = test_action_simulator.exe
EVALUATOR_TEST_EXE = test_evaluator.exe

# Default target
all: $(TETRIS_EXE) $(FEATURE_TEST_EXE) $(ACTION_TEST_EXE) $(EVALUATOR_TEST_EXE)

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

# Build only tetris
tetris: $(TETRIS_EXE)

# Build only feature test
test: $(FEATURE_TEST_EXE)

# Build only action simulator test
test-action: $(ACTION_TEST_EXE)

# Build only evaluator test
test-evaluator: $(EVALUATOR_TEST_EXE)

# Run feature test
run-test: $(FEATURE_TEST_EXE)
	./$(FEATURE_TEST_EXE)

# Run action simulator test
run-action-test: $(ACTION_TEST_EXE)
	./$(ACTION_TEST_EXE)

# Run evaluator test
run-evaluator-test: $(EVALUATOR_TEST_EXE)
	./$(EVALUATOR_TEST_EXE)

# Run tetris game
run-tetris: $(TETRIS_EXE)
	./$(TETRIS_EXE)

# Clean build artifacts
clean:
	rm -f $(TETRIS_EXE) $(FEATURE_TEST_EXE) $(ACTION_TEST_EXE) $(EVALUATOR_TEST_EXE) test_weights.txt
	@echo "Cleaned build artifacts"

# Help target
help:
	@echo "Available targets:"
	@echo "  all                - Build everything (default)"
	@echo "  tetris             - Build tetris game only"
	@echo "  test               - Build feature test only"
	@echo "  test-action        - Build action simulator test only"
	@echo "  test-evaluator     - Build evaluator test only"
	@echo "  run-test           - Build and run feature test"
	@echo "  run-action-test    - Build and run action simulator test"
	@echo "  run-evaluator-test - Build and run evaluator test"
	@echo "  run-tetris         - Build and run tetris game"
	@echo "  clean              - Remove all executables"
	@echo "  help               - Show this help message"

.PHONY: all tetris test test-action test-evaluator run-test run-action-test run-evaluator-test run-tetris clean help

