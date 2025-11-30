#include "Evaluator.h"
#include "BoardConstants.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>

using namespace std;

// 기본 휴리스틱 가중치
Evaluator::Weights::Weights()
    : aggregateHeight(-1)  // 높이는 낮을수록 좋음 (음수)
    , holes(-5)             // 구멍은 적을수록 좋음 (음수)
    , bumpiness(-1)        // 울퉁불퉁함은 적을수록 좋음 (음수)
    , maxHeight(-5)             // 최대 높이는 낮을수록 좋음 (음수)
    , minHeight(0.0)              // 최소 높이는 영향 적음
{ 
}

Evaluator::Weights::Weights(double agg, double holes, double bump, double maxH, double minH)
    : aggregateHeight(agg)
    , holes(holes)
    , bumpiness(bump)
    , maxHeight(maxH)
    , minHeight(minH)
{
}

Evaluator::Evaluator(const Weights& weights)
    : weights_(weights)
{
}

double Evaluator::evaluate(const FeatureExtractor::Features& features) const
{
    // Linear combination: V(s) = w1*f1 + w2*f2 + ... + wn*fn
    double score = 0.0;
    
    score += weights_.aggregateHeight * features.aggregateHeight;
    score += weights_.holes * features.holes;
    score += weights_.bumpiness * features.bumpiness;
    score += weights_.maxHeight * features.maxHeight;
    score += weights_.minHeight * features.minHeight;
    
    return score;
}

double Evaluator::evaluateBoard(const Board& board) const
{
    FeatureExtractor::Features features = FeatureExtractor::extractFeatures(board);
    return evaluate(features);
}

double Evaluator::evaluateResult(const SimulationResult& result) const
{
    // 유효하지 않거나 게임 오버인 경우 최악의 점수
    if (!result.isValid || result.gameOver)
    {
        return -std::numeric_limits<double>::infinity();
    }
    
    return evaluate(result.features);
}

std::pair<Action, double> Evaluator::selectBestAction(const Board& board, const Block& block) const
{
    // 모든 가능한 액션 시뮬레이션
    vector<SimulationResult> results = ActionSimulator::simulateAllActions(board, block);
    
    if (results.empty())
    {
        // 가능한 액션이 없으면 기본 액션 반환
        return {Action(0, 5), -std::numeric_limits<double>::infinity()};
    }
    
    // 최고 점수 찾기
    Action bestAction = results[0].action;
    double bestScore = evaluateResult(results[0]);
    
    for (size_t i = 1; i < results.size(); i++)
    {
        double score = evaluateResult(results[i]);
        if (score > bestScore)
        {
            bestScore = score;
            bestAction = results[i].action;
        }
    }
    
    return {bestAction, bestScore};
}

std::pair<Action, double> Evaluator::selectBestActionWithLookAhead(
    const Board& board, 
    const Block& currentBlock, 
    const Block* nextBlock,
    double lookAheadDiscount) const
{
    // 현재 블록의 모든 가능한 액션 시뮬레이션
    vector<SimulationResult> currentResults = ActionSimulator::simulateAllActions(board, currentBlock);
    
    if (currentResults.empty())
    {
        // 가능한 액션이 없으면 기본 액션 반환
        return {Action(0, 5), -std::numeric_limits<double>::infinity()};
    }
    
    Action bestAction = currentResults[0].action;
    double bestScore = -std::numeric_limits<double>::infinity();
    
    // 각 현재 블록 액션에 대해 평가
    for (const auto& currentResult : currentResults)
    {
        if (!currentResult.isValid || currentResult.gameOver)
        {
            continue;  // 유효하지 않은 액션은 스킵
        }
        
        // 현재 블록 배치 후 보드 상태 시뮬레이션
        Board simBoard(board);
        Block simBlock(currentBlock.getType(), Rotation(currentResult.action.rotation),
                      Position(currentResult.action.column, BoardConstants::BLOCK_START_Y));
        ActionSimulator::moveBlockToPosition(simBlock, currentResult.action.rotation, currentResult.action.column);
        ActionSimulator::dropBlock(simBoard, simBlock);
        simBoard.mergeBlock(simBlock);
        simBoard.deleteFullLine();  // 라인 삭제
        
        // 현재 블록 점수
        double currentScore = evaluateResult(currentResult);
        
        // 다음 블록까지 고려
        if (nextBlock != nullptr)
        {
            // 다음 블록의 모든 가능한 액션 시뮬레이션
            vector<SimulationResult> nextResults = ActionSimulator::simulateAllActions(simBoard, *nextBlock);
            
            if (!nextResults.empty())
            {
                // 다음 블록의 최고 점수 찾기
                double bestNextScore = -std::numeric_limits<double>::infinity();
                for (const auto& nextResult : nextResults)
                {
                    if (nextResult.isValid && !nextResult.gameOver)
                    {
                        double nextScore = evaluateResult(nextResult);
                        if (nextScore > bestNextScore)
                        {
                            bestNextScore = nextScore;
                        }
                    }
                }
                
                // 현재 점수 + (다음 블록 최고 점수 * 할인 인자)
                if (bestNextScore > -std::numeric_limits<double>::infinity())
                {
                    currentScore += bestNextScore * lookAheadDiscount;
                }
            }
        }
        
        // 최고 점수 업데이트
        if (currentScore > bestScore)
        {
            bestScore = currentScore;
            bestAction = currentResult.action;
        }
    }
    
    return {bestAction, bestScore};
}

std::vector<std::pair<Action, double>> Evaluator::evaluateAllActions(const Board& board, const Block& block) const
{
    vector<SimulationResult> results = ActionSimulator::simulateAllActions(board, block);
    vector<pair<Action, double>> actionScores;
    
    for (const auto& result : results)
    {
        double score = evaluateResult(result);
        actionScores.push_back({result.action, score});
    }
    
    // 점수 내림차순 정렬
    sort(actionScores.begin(), actionScores.end(),
         [](const pair<Action, double>& a, const pair<Action, double>& b) {
             return a.second > b.second;
         });
    
    return actionScores;
}

bool Evaluator::saveWeights(const string& filename) const
{
    ofstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file for writing: " << filename << endl;
        return false;
    }
    
    file << "# Tetris Evaluator Weights" << endl;
    file << "aggregateHeight " << weights_.aggregateHeight << endl;
    file << "holes " << weights_.holes << endl;
    file << "bumpiness " << weights_.bumpiness << endl;
    file << "maxHeight " << weights_.maxHeight << endl;
    file << "minHeight " << weights_.minHeight << endl;
    
    file.close();
    cout << "Weights saved to " << filename << endl;
    return true;
}

bool Evaluator::loadWeights(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file for reading: " << filename << endl;
        return false;
    }
    
    // 가중치 포인터 맵 (더 확장 가능하고 유지보수 쉬움)
    unordered_map<string, double*> weightMap = {
        {"aggregateHeight", &weights_.aggregateHeight},
        {"holes", &weights_.holes},
        {"bumpiness", &weights_.bumpiness},
        {"maxHeight", &weights_.maxHeight},
        {"minHeight", &weights_.minHeight}
    };
    
    string line, key;
    double value;
    
    while (file >> key)
    {
        if (key[0] == '#')
        {
            // 주석 라인 건너뛰기
            getline(file, line);
            continue;
        }
        
        file >> value;
        
        auto it = weightMap.find(key);
        if (it != weightMap.end())
        {
            *(it->second) = value;
        }
    }
    
    file.close();
    cout << "Weights loaded from " << filename << endl;
    return true;
}

void Evaluator::printWeights() const
{
    cout << "=== Evaluator Weights ===" << endl;
    cout << "Aggregate Height: " << weights_.aggregateHeight << endl;
    cout << "Holes:            " << weights_.holes << endl;
    cout << "Bumpiness:        " << weights_.bumpiness << endl;
    cout << "Max Height:       " << weights_.maxHeight << endl;
    cout << "Min Height:       " << weights_.minHeight << endl;
    cout << "=========================" << endl;
}

