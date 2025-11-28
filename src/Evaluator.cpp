#include "Evaluator.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>

using namespace std;

// 기본 휴리스틱 가중치
// 문헌 기반 초기값 (Pierre Dellacherie's algorithm 참고)
Evaluator::Weights::Weights()
    : aggregateHeight(-0.510066)  // 높이는 낮을수록 좋음 (음수)
    , completeLines(0.760666)     // 라인 제거는 많을수록 좋음 (양수)
    , holes(-0.35663)             // 구멍은 적을수록 좋음 (음수)
    , bumpiness(-0.184483)        // 울퉁불퉁함은 적을수록 좋음 (음수)
    , maxHeight(-0.5)             // 최대 높이는 낮을수록 좋음 (음수)
    , minHeight(0.0)              // 최소 높이는 영향 적음
{
}

Evaluator::Weights::Weights(double agg, double lines, double holes, double bump, double maxH, double minH)
    : aggregateHeight(agg)
    , completeLines(lines)
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
    score += weights_.completeLines * features.completeLines;
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
    file << "completeLines " << weights_.completeLines << endl;
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
        {"completeLines", &weights_.completeLines},
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
    cout << "Complete Lines:   " << weights_.completeLines << endl;
    cout << "Holes:            " << weights_.holes << endl;
    cout << "Bumpiness:        " << weights_.bumpiness << endl;
    cout << "Max Height:       " << weights_.maxHeight << endl;
    cout << "Min Height:       " << weights_.minHeight << endl;
    cout << "=========================" << endl;
}

