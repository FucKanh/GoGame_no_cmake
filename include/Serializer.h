#pragma once
#include <string>
#include "Game.h"
namespace Serializer {
    bool save(const Game& game, const std::string& path);
    bool load(Game& game, const std::string& path);
}
