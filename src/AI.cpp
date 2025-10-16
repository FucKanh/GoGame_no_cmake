#include "AI.h"
#include <algorithm>
#include <random>

static std::mt19937 rng(1337);

AIMove AIPlayer::chooseMove(const Game& game, Player me){
    if (level==1){
        std::vector<std::pair<int,int>> legal;
        int N = game.board().size();
        for (int y=0;y<N;y++){
            for (int x=0;x<N;x++){
                if (game.board().at(x,y)==Player::None){
                    if (game.board().isLegal(x,y, me, game.koPoint()))
                        legal.push_back({x,y});
                }
            }
        }
        if (!legal.empty()){
            std::uniform_int_distribution<int> dist(0,(int)legal.size()-1);
            auto [x,y]=legal[dist(rng)];
            return {x,y,false};
        }
        return { -1,-1,true };
    }

    int depth = (level==2? 2: 3);
    Game root = game;
    int bestScore = -1e9;
    AIMove best{ -1,-1,true };
    auto moves = generateMoves(game);
    if (moves.empty()) return { -1,-1,true };
    for (auto [x,y]: moves){
        Game child = root;
        if (child.toPlay()!=me){ /* assume AI called on its turn */ }
        if (child.playMove(x,y)){
            int score = minimax(child, 0, depth, me, false, -1000000, 1000000);
            if (score>bestScore){ bestScore=score; best = {x,y,false}; }
        }
    }
    return best.x==-1? AIMove{-1,-1,true} : best;
}

int AIPlayer::evaluate(const Board& b, Player me) const{
    int myArea = b.estimateArea(me);
    Player opp = (me==Player::Black? Player::White: Player::Black);
    int oppArea = b.estimateArea(opp);
    int N=b.size();
    int myLib=0, oppLib=0;
    for (int y=0;y<N;y++){
        for (int x=0;x<N;x++){
            if (b.at(x,y)==me) myLib += const_cast<Board&>(b).countLiberties(x,y);
            else if (b.at(x,y)==opp) oppLib += const_cast<Board&>(b).countLiberties(x,y);
        }
    }
    return (myArea-oppArea)*10 + (myLib-oppLib);
}

std::vector<std::pair<int,int>> AIPlayer::generateMoves(const Game& game) const{
    std::vector<std::pair<int,int>> mv;
    int N=game.board().size();
    for (int y=0;y<N;y++){
        for (int x=0;x<N;x++){
            if (game.board().at(x,y)==Player::None){
                if (game.board().isLegal(x,y, game.toPlay(), game.koPoint()))
                    mv.push_back({x,y});
            }
        }
    }
    std::sort(mv.begin(), mv.end(), [N](auto a, auto b){
        auto score = [N](std::pair<int,int> p){
            float cx=N/2.0f, cy=N/2.0f;
            float dx=p.first-cx, dy=p.second-cy;
            return dx*dx+dy*dy;
        };
        return score(a)<score(b);
    });
    if (mv.size()>60) mv.resize(60);
    return mv;
}

int AIPlayer::minimax(Game& game, int depth, int maxDepth, Player me, bool maximizing, int alpha, int beta){
    if (depth>=maxDepth) { return evaluate(game.board(), me); }
    auto moves = generateMoves(game);
    if (moves.empty()){
        game.pass();
        int sc = evaluate(game.board(), me);
        game.undo();
        return sc;
    }
    if (maximizing){
        int best=-1000000;
        for (auto [x,y]: moves){
            if (game.playMove(x,y)){
                int val = minimax(game, depth+1, maxDepth, me, false, alpha, beta);
                game.undo();
                if (val>best) best=val;
                if (best>alpha) alpha=best;
                if (beta<=alpha) break;
            }
        }
        return best;
    } else {
        int best=1000000;
        for (auto [x,y]: moves){
            if (game.playMove(x,y)){
                int val = minimax(game, depth+1, maxDepth, me, true, alpha, beta);
                game.undo();
                if (val<best) best=val;
                if (best<beta) beta=best;
                if (beta<=alpha) break;
            }
        }
        return best;
    }
}
