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

# Executables
TETRIS_EXE = tetris.exe
FEATURE_TEST_EXE = test_features.exe

# Default target
all: $(TETRIS_EXE) $(FEATURE_TEST_EXE)

# Build tetris game
$(TETRIS_EXE): $(TETRIS_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Tetris game compiled successfully!"

# Build feature extractor test
$(FEATURE_TEST_EXE): $(FEATURE_TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Feature test compiled successfully!"

# Build only tetris
tetris: $(TETRIS_EXE)

# Build only feature test
test: $(FEATURE_TEST_EXE)

# Run feature test
run-test: $(FEATURE_TEST_EXE)
	./$(FEATURE_TEST_EXE)

# Run tetris game
run-tetris: $(TETRIS_EXE)
	./$(TETRIS_EXE)

# Clean build artifacts
clean:
	rm -f $(TETRIS_EXE) $(FEATURE_TEST_EXE)
	@echo "Cleaned build artifacts"

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build everything (default)"
	@echo "  tetris     - Build tetris game only"
	@echo "  test       - Build feature test only"
	@echo "  run-test   - Build and run feature test"
	@echo "  run-tetris - Build and run tetris game"
	@echo "  clean      - Remove all executables"
	@echo "  help       - Show this help message"

.PHONY: all tetris test run-test run-tetris clean help

