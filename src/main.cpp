#include "UI.h"
#include "Game.h"
int main(){
    GameConfig cfg;
    cfg.boardSize = 9;
    cfg.aiLevel = 0;
    Game game(cfg);
    UI ui(game);
    ui.run();
    return 0;
}
