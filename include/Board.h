#pragma once
#include <vector>
#include <utility>
#include <optional>
#include "Game.h"

class Board {
public:
    explicit Board(int n);
    int size() const { return N; }

    Player at(int x, int y) const;
    void set(int x, int y, Player p);

    int countLiberties(int x, int y) const;
    std::vector<std::pair<int,int>> captureIfNoLiberties(int x, int y, Player p);
    std::vector<std::pair<int,int>> listGroup(const std::vector<std::vector<Player>>& grid, int x, int y) const;

    bool isLegal(int x, int y, Player p, std::optional<std::pair<int,int>> koPoint) const;
    void removeStones(const std::vector<std::pair<int,int>>& stones);
    int estimateArea(Player p) const;

    const std::vector<std::vector<Player>>& grid() const { return g; }
    std::vector<std::vector<Player>>& grid() { return g; }

private:
    int N;
    std::vector<std::vector<Player>> g;
};
