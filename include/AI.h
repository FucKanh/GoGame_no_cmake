#pragma once
#include "Game.h"
#include "Board.h"
#include <vector>

struct AIMove { int x=-1, y=-1; bool isPass=false; };

class AIPlayer {
public:
    explicit AIPlayer(int level): level(level) {}
    AIMove chooseMove(const Game& game, Player me);
private:
    int level;
    int evaluate(const Board& b, Player me) const;
    int minimax(Game& game, int depth, int maxDepth, Player me, bool maximizing, int alpha, int beta);
    std::vector<std::pair<int,int>> generateMoves(const Game& game) const;
};
