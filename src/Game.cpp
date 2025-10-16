#include "Game.h"
#include "Board.h"
#include "Serializer.h"

Game::Game(const GameConfig& cfg): cfg(cfg), curPlayer(Player::Black) {
    brd = std::make_unique<Board>(cfg.boardSize);
}
void Game::copyFrom(const Game& o){
    cfg = o.cfg;
    curPlayer = o.curPlayer;
    finished = o.finished;
    passesInRow = o.passesInRow;
    blackPrisonersCount = o.blackPrisonersCount;
    whitePrisonersCount = o.whitePrisonersCount;
    // deep copy Board
    brd = std::make_unique<Board>(*o.brd);
    moveList = o.moveList;
    redoStack = o.redoStack;
    ko = o.ko;
}

Game::Game(const Game& o){
    copyFrom(o);
}

Game& Game::operator=(const Game& o){
    if (this != &o){
        copyFrom(o);
    }
    return *this;
}

void Game::switchTurn(){ curPlayer = (curPlayer==Player::Black? Player::White: Player::Black); }

bool Game::placeStoneInternal(int x, int y, Player p, Move& out){
    if (!brd->isLegal(x,y,p, ko)) return false;
    brd->set(x,y,p);
    auto captured = brd->captureIfNoLiberties(x,y,p);
    if (brd->countLiberties(x,y)==0){
        // không hợp lệ -> revert
        brd->set(x,y, Player::None);
        // trả lại các quân vừa bị bắt (nếu có)
        Player enemy = (p==Player::Black? Player::White: Player::Black);
        for (auto [cx,cy]: captured) brd->set(cx,cy, enemy);
        return false;
    }

    if (captured.size()==1){
        ko = std::make_pair(captured[0].first, captured[0].second);
    } else {
        ko.reset();
    }
    out = {x,y,p,false,captured};
    if (p==Player::Black) blackPrisonersCount += (int)captured.size();
    else whitePrisonersCount += (int)captured.size();
    return true;
}

bool Game::playMove(int x, int y){
    if (finished) return false;
    Move mv;
    if (!placeStoneInternal(x,y,curPlayer,mv)) return false;
    moveList.push_back(mv);
    while(!redoStack.empty()) redoStack.pop();
    passesInRow=0;
    switchTurn();
    return true;
}

void Game::pass(){
    if (finished) return;
    Move mv; mv.x=-1; mv.y=-1; mv.player=curPlayer; mv.isPass=true;
    moveList.push_back(mv);
    while(!redoStack.empty()) redoStack.pop();
    passesInRow++;
    if (passesInRow>=2){ finished=true; }
    switchTurn();
}

bool Game::undo(){
    if (moveList.empty()) return false;
    auto mv = moveList.back(); moveList.pop_back();
    if (!mv.isPass){
        brd->set(mv.x,mv.y, Player::None);
        Player enemy = (mv.player==Player::Black? Player::White: Player::Black);
        for (auto [x,y]: mv.captures) brd->set(x,y, enemy);
        if (mv.player==Player::Black) blackPrisonersCount -= (int)mv.captures.size();
        else whitePrisonersCount -= (int)mv.captures.size();
    }
    redoStack.push(mv);
    finished=false;
    passesInRow=0;
    switchTurn();
    return true;
}

bool Game::redo(){
    if (redoStack.empty()) return false;
    auto mv = redoStack.top(); redoStack.pop();
    if (mv.isPass){
        moveList.push_back(mv);
        passesInRow++;
        if (passesInRow>=2) finished=true;
        switchTurn();
        return true;
    } else {
        Move performed;
        if (!placeStoneInternal(mv.x,mv.y, mv.player, performed)) return false;
        moveList.push_back(performed);
        switchTurn();
        return true;
    }
}

bool Game::hasLegalMove(Player p) const {
    const int N = brd->size();
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            if (brd->at(x,y) == Player::None) {
                if (brd->isLegal(x, y, p, ko)) return true;
            }
        }
    }
    return false;
}

void Game::reset(){ *this = Game(cfg); }
void Game::tryAutoEnd(){ if (passesInRow>=2) finished=true; }
int Game::scoreArea(Player p) const{ return brd->estimateArea(p); }

std::string Game::resultString() const{
    int b = scoreArea(Player::Black);
    int w = scoreArea(Player::White);
    std::string s = "Area Score - Black: " + std::to_string(b) + "  White: " + std::to_string(w);
    if (finished){
        s += "  |  Result: ";
        if (b>w) s += "Black wins";
        else if (w>b) s += "White wins";
        else s += "Draw";
    }
    return s;
}

bool Game::saveToFile(const std::string& path) const{ return Serializer::save(*this, path); }
bool Game::loadFromFile(const std::string& path){ return Serializer::load(*this, path); }
