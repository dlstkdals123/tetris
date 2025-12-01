# Tetris Monte Carlo Learning Project Makefile
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
LDFLAGS = 

# Directories
SRC_DIR = src

# Common source files
COMMON_SRCS = $(SRC_DIR)/Board.cpp \
              $(SRC_DIR)/Utils.cpp \
              $(SRC_DIR)/position.cpp \
              $(SRC_DIR)/rotation.cpp \
              $(SRC_DIR)/STAGE.cpp \
              $(SRC_DIR)/gameState.cpp \
              $(SRC_DIR)/BlockGenerator.cpp \
              $(SRC_DIR)/BlockRender.cpp \
              $(SRC_DIR)/BlockMover.cpp

# AI/Learning source files
AI_SRCS = $(SRC_DIR)/Evaluator.cpp \
          $(SRC_DIR)/ActionSimulator.cpp \
          $(SRC_DIR)/FeatureExtractor.cpp

# Main executables source files
TETRIS_AI_SRCS = $(SRC_DIR)/tetris.cpp $(AI_SRCS) $(COMMON_SRCS)
TRAIN_SRCS = $(SRC_DIR)/train_multistage.cpp \
             $(SRC_DIR)/MCLearner.cpp \
             $(AI_SRCS) \
             $(SRC_DIR)/Board.cpp \
             $(SRC_DIR)/Utils.cpp \
             $(SRC_DIR)/position.cpp \
             $(SRC_DIR)/rotation.cpp

# Executables
TETRIS_AI_EXE = tetris_ai.exe
TRAIN_EXE = train_multistage.exe

# Default target
all: $(TETRIS_AI_EXE) $(TRAIN_EXE)

# Build tetris with AI (player vs AI)
$(TETRIS_AI_EXE): $(TETRIS_AI_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Tetris AI game compiled successfully!"

# Build Monte Carlo training program
$(TRAIN_EXE): $(TRAIN_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Monte Carlo training compiled successfully!"

# Individual build targets
tetris-ai: $(TETRIS_AI_EXE)

train: $(TRAIN_EXE)

# Run targets
run-tetris-ai: $(TETRIS_AI_EXE)
	./$(TETRIS_AI_EXE)

run-train: $(TRAIN_EXE)
	./$(TRAIN_EXE)

# Clean build artifacts
clean:
	rm -f $(TETRIS_AI_EXE) $(TRAIN_EXE)
	rm -f *.txt *.csv
	@echo "Cleaned build artifacts"

# Help target
help:
	@echo "========================================="
	@echo "Tetris Monte Carlo Learning Project"
	@echo "========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Build all executables (default)"
	@echo "  tetris-ai     - Build player vs AI tetris game"
	@echo "  train         - Build Monte Carlo training program"
	@echo ""
	@echo "Run targets:"
	@echo "  run-tetris-ai - Run player vs AI game"
	@echo "  run-train     - Run Monte Carlo training (20K episodes)"
	@echo ""
	@echo "Other:"
	@echo "  clean         - Remove all executables and output files"
	@echo "  help          - Show this help message"
	@echo ""

.PHONY: all tetris-ai train run-tetris-ai run-train clean help

