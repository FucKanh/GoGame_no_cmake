#include "Serializer.h"
#include "Board.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string playerToStr(Player p){
    if (p==Player::Black) return "B";
    if (p==Player::White) return "W";
    return ".";
}
static Player strToPlayer(const std::string& s){
    if (s=="B") return Player::Black;
    if (s=="W") return Player::White;
    return Player::None;
}

bool Serializer::save(const Game& game, const std::string& path){
    json j;
    j["boardSize"]=game.board().size();
    j["toPlay"] = (game.toPlay()==Player::Black? "B":"W");
    j["moves"]=json::array();
    for (auto &m: game.history()){
        json jm;
        jm["x"]=m.x; jm["y"]=m.y; jm["player"]=playerToStr(m.player); jm["isPass"]=m.isPass;
        jm["captures"]=json::array();
        for (auto &c: m.captures){
            jm["captures"].push_back({{"x",c.first},{"y",c.second}});
        }
        j["moves"].push_back(jm);
    }
    json grid=json::array();
    for (int y=0;y<game.board().size();y++){
        json row=json::array();
        for (int x=0;x<game.board().size();x++){
            row.push_back(playerToStr(game.board().at(x,y)));
        }
        grid.push_back(row);
    }
    j["grid"]=grid;

    std::ofstream f(path);
    if (!f) return false;
    f << j.dump(2);
    return true;
}

bool Serializer::load(Game& game, const std::string& path){
    std::ifstream f(path);
    if (!f) return false;
    json j; f >> j;
    GameConfig cfg = game.config();
    if (j.contains("boardSize")) cfg.boardSize = j["boardSize"].get<int>();
    game = Game(cfg);
    if (j.contains("grid")){
        for (int y=0;y<game.board().size();y++){
            for (int x=0;x<game.board().size();x++){
                std::string s = j["grid"][y][x].get<std::string>();
                game.board().set(x,y, strToPlayer(s));
            }
        }
    }
    return true;
}
