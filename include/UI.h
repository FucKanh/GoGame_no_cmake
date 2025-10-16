#pragma once
#include <SFML/Audio.hpp>
#include <thread>
#include <atomic>
#include <optional>
#include "Game.h"
#include "Board.h"
#include "AI.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <functional>
#include <SFML/Audio.hpp>
#include <thread>
#include <atomic>
#include <optional>

class UI {
public:
    UI(Game& game);
    void run();
private:
    Game& game;
    sf::RenderWindow window;
    sf::Font font;
    sf::Text infoText;
    sf::RectangleShape boardRect;
    float cellSize;
    float margin = 60.f;

        // --- Right panel layout (no overlap) ---
    float panelLeft = 820.f;
    float panelWidth = 640.f;   // đủ cho 2 cột ~ 300px/cột
    float panelGap  = 12.f;

    float btnW = 300.f;
    float btnH = 36.f;

    sf::Text modeText;              // hiển thị "Mode: 2-Player / AI: Hard" bên dưới cụm AI


    struct Button {
        sf::RectangleShape rect;
        sf::Text label;
        std::function<void()> onClick;
        bool contains(sf::Vector2f p) const { return rect.getGlobalBounds().contains(p); }
    };
    std::vector<Button> buttons;

    sf::SoundBuffer placeBuf, captureBuf, endBuf;
    sf::Sound sPlace, sCapture, sEnd;

    AIPlayer ai{1};

    // AI threading
    std::unique_ptr<std::thread> aiWorker;
    std::atomic<bool> aiBusy{false};
    std::atomic<bool> aiCancel{false};
    std::optional<AIMove> aiResult;

    // Generation để vô hiệu hóa kết quả cũ
    uint64_t aiGen = 0;
    std::optional<uint64_t> aiResultGen;

    // History panel (scrollable) - dời lên tránh nút Reset
    float histX = 820.f, histY = 600.f, histW = 640.f, histH = 200.f;
    float histScroll = 0.f;
    float histLineH = 16.f;

    // Lưu index nút để highlight mode hiện tại
    int idxBtnPVP = -1, idxBtnEasy = -1, idxBtnMed = -1, idxBtnHard = -1;

    // Helpers
    void cancelAI();
    void restartLayout();       // gọi lại setupUI khi đổi board/mode
    void highlightModeButtons();


    void setupUI();
    void drawBoard();
    void drawStones();
    void drawHUD();
    void handleClick(float mx, float my);
    void handleWheel(float mx, float my, float delta);
    std::pair<int,int> mouseToCell(float mx, float my) const;
    void notifyEnd();
    void openSaveDialog();
    void openLoadDialog();
    void drawMoveHistory();
};
