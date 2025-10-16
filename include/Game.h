#pragma once
#include <vector>
#include <string>
#include <optional>
#include <stack>
#include <utility>
#include <memory>

enum class Player { Black = 1, White = -1, None = 0 };

struct Move {
    int x, y;
    Player player;
    bool isPass = false;
    std::vector<std::pair<int,int>> captures;
};

struct GameConfig {
    int boardSize = 9;
    bool soundOn = true;
    int aiLevel = 0; // 0=human-human, 1=easy, 2=medium, 3=hard
};

class Board;

class Game {
public:
    Game(const GameConfig& cfg);

        // Kiểm tra còn nước hợp lệ cho người chơi p (dựa vào luật hiện tại + ko)
    bool hasLegalMove(Player p) const;


    Game(const Game& other);
    Game& operator=(const Game& other);
    Game(Game&&) noexcept = default;
    Game& operator=(Game&&) noexcept = default;

    const GameConfig& config() const { return cfg; }

    const Board& board() const { return *brd; }
    Board& board() { return *brd; }

    Player toPlay() const { return curPlayer; }
    bool isFinished() const { return finished; }
    std::string resultString() const;

    bool playMove(int x, int y);
    void pass();
    bool undo();
    bool redo();
    void reset();

    const std::vector<Move>& history() const { return moveList; }
    void tryAutoEnd();
    int scoreArea(Player p) const;
    int blackPrisoners() const { return blackPrisonersCount; }
    int whitePrisoners() const { return whitePrisonersCount; }

    std::optional<std::pair<int,int>> koPoint() const { return ko; }

    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

private:

    void copyFrom(const Game& other);
    GameConfig cfg;
    Player curPlayer;
    bool finished=false;
    int passesInRow=0;
    int blackPrisonersCount=0, whitePrisonersCount=0;

    std::unique_ptr<Board> brd;
    std::vector<Move> moveList;
    std::stack<Move> redoStack;
    std::optional<std::pair<int,int>> ko;

    bool placeStoneInternal(int x, int y, Player p, Move& out);
    void switchTurn();
};
