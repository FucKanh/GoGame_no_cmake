#include "UI.h"
#include "Audio.h"
#include "Serializer.h"
#include <sstream>

static std::string coordToGo(int x, int y, int N){
    std::string letters="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (x<0||x>=N||y<0||y>=N) return "PASS";
    std::string s;
    s += letters[x];
    s += std::to_string(N - y);
    return s;
}

void UI::cancelAI(){
    aiCancel.store(true);
    aiBusy.store(false);
    aiResult.reset();
    aiResultGen.reset();
    ++aiGen; // đổi thế hệ để kết quả cũ (nếu có) bị bỏ qua
}

void UI::restartLayout(){
    buttons.clear();
    setupUI();
}

void UI::highlightModeButtons(){
    // reset màu
    auto paint = [&](int i, sf::Color c){
        if (i>=0 && i<(int)buttons.size()){
            buttons[i].rect.setFillColor(c);
        }
    };
    sf::Color normal(50,50,60);
    paint(idxBtnPVP,  normal);
    paint(idxBtnEasy, normal);
    paint(idxBtnMed,  normal);
    paint(idxBtnHard, normal);

    // tô đậm nút đang active
    sf::Color active(70,120,200);  // xanh nhạt
    switch (game.config().aiLevel){
        case 0: paint(idxBtnPVP,  active); break;
        case 1: paint(idxBtnEasy, active); break;
        case 2: paint(idxBtnMed,  active); break;
        case 3: paint(idxBtnHard, active); break;
    }
}

UI::UI(Game& game): game(game), window(sf::VideoMode(1600, 900), "Go Game") {
    window.setFramerateLimit(60);
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")){
        // optional: bundle a font
    }
    placeBuf.loadFromFile(AudioUtil::assetPath("sounds/place.wav"));
    captureBuf.loadFromFile(AudioUtil::assetPath("sounds/capture.wav"));
    endBuf.loadFromFile(AudioUtil::assetPath("sounds/end.wav"));
    sPlace.setBuffer(placeBuf);
    sCapture.setBuffer(captureBuf);
    sEnd.setBuffer(endBuf);

    if (!placeBuf.loadFromFile(AudioUtil::assetPath("sounds/place.wav"))) {
        // std::cerr << "Failed to load place.wav\n";
    }
    if (!captureBuf.loadFromFile(AudioUtil::assetPath("sounds/capture.wav"))) {
        // std::cerr << "Failed to load capture.wav\n";
    }
    if (!endBuf.loadFromFile(AudioUtil::assetPath("sounds/end.wav"))) {
        // std::cerr << "Failed to load end.wav\n";
    }

    setupUI();
}

void UI::setupUI(){
    int N = game.board().size();

    // ===== Tính kích thước bàn =====
    // Bàn theo chiều cao (giới hạn tối đa 640)
    float boardPixels = std::min((float)window.getSize().y - 120.f, 640.f);

    // Chừa tối thiểu không gian panel phải: 2 cột ~300px + 20px đệm
    const float rightMinPanel = 2*300.f + 20.f;
    float maxBoardByWidth = (float)window.getSize().x
                          - (margin /*trái*/ + 40.f /*đệm*/ + rightMinPanel + margin /*lề phải*/);
    // Không để bàn quá nhỏ (>= 360)
    boardPixels = std::min(boardPixels, std::max(360.f, maxBoardByWidth));

    cellSize = boardPixels / (N-1);
    boardRect.setPosition(margin, margin + 60.f);
    boardRect.setSize(sf::Vector2f(boardPixels, boardPixels));
    boardRect.setFillColor(sf::Color(222,184,135));
    boardRect.setOutlineThickness(4.f);
    boardRect.setOutlineColor(sf::Color::Black);

    // Nếu cửa sổ co lại, đảm bảo bàn không lấn panel phải
    if (boardRect.getSize().x > maxBoardByWidth && maxBoardByWidth > 360.f) {
        boardRect.setSize({maxBoardByWidth, maxBoardByWidth});
        cellSize = boardRect.getSize().x / (N-1);
    }

    // ===== Right panel: 2 CỘT =====
    const float gap   = 12.f;
    const float colGap = 20.f;

    panelLeft  = boardRect.getPosition().x + boardRect.getSize().x + 40.f;
    panelWidth = std::max(rightMinPanel, (float)window.getSize().x - panelLeft - 20.f);

    float colW = (panelWidth - colGap) / 2.f;
    btnW = colW;
    btnH = 36.f;

    auto makeBtnIdx = [&](const std::string& text, float x, float y, std::function<void()> cb)->int{
        Button b;
        b.rect.setPosition(x,y);
        b.rect.setSize(sf::Vector2f(btnW, btnH));
        b.rect.setFillColor(sf::Color(50,50,60));
        b.rect.setOutlineThickness(1);
        b.rect.setOutlineColor(sf::Color::White);
        b.label.setFont(font);
        b.label.setString(text);
        b.label.setCharacterSize(16);
        b.label.setFillColor(sf::Color::White);
        b.label.setPosition(x+10, y+6);
        b.onClick = cb;
        buttons.push_back(std::move(b));
        return (int)buttons.size()-1;
    };

    // Toạ độ 2 cột
    float col1X = panelLeft;
    float col2X = panelLeft + colW + colGap;
    float y1 = 60.f; // cột 1
    float y2 = 60.f; // cột 2

    // ==== CỘT 1: New board + Mode ====
    makeBtnIdx("New 9x9",   col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.boardSize=9;  game=Game(c); restartLayout(); }); y1 += btnH + gap;
    makeBtnIdx("New 13x13", col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.boardSize=13; game=Game(c); restartLayout(); }); y1 += btnH + gap;
    makeBtnIdx("New 19x19", col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.boardSize=19; game=Game(c); restartLayout(); }); y1 += btnH + gap*2;

    idxBtnPVP  = makeBtnIdx("2-Player",   col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.aiLevel=0; game=Game(c); restartLayout(); }); y1 += btnH + gap;
    idxBtnEasy = makeBtnIdx("AI: Easy",   col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.aiLevel=1; game=Game(c); restartLayout(); }); y1 += btnH + gap;
    idxBtnMed  = makeBtnIdx("AI: Medium", col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.aiLevel=2; game=Game(c); restartLayout(); }); y1 += btnH + gap;
    idxBtnHard = makeBtnIdx("AI: Hard",   col1X, y1, [&]{ cancelAI(); auto c=game.config(); c.aiLevel=3; game=Game(c); restartLayout(); }); y1 += btnH + gap;

    // Mode label ngay dưới cụm AI
    y1 += gap;
    modeText.setFont(font);
    modeText.setCharacterSize(16);
    modeText.setFillColor(sf::Color(220,220,230));
    modeText.setString(
        game.config().aiLevel==0? "Mode: 2-Player" :
        game.config().aiLevel==1? "Mode: AI Easy" :
        game.config().aiLevel==2? "Mode: AI Medium" : "Mode: AI Hard"
    );
    modeText.setPosition(col1X, y1);
    y1 += 16.f + gap*2;

    // ==== CỘT 2: Undo -> Reset ====
    makeBtnIdx("Undo",  col2X, y2, [&]{ cancelAI(); game.undo(); }); y2 += btnH + gap;
    makeBtnIdx("Redo",  col2X, y2, [&]{ cancelAI(); game.redo(); }); y2 += btnH + gap;
    makeBtnIdx("Pass",  col2X, y2, [&]{ cancelAI(); game.pass(); sPlace.play(); }); y2 += btnH + gap;
    makeBtnIdx("Save",  col2X, y2, [&]{ cancelAI(); openSaveDialog(); }); y2 += btnH + gap;
    makeBtnIdx("Load",  col2X, y2, [&]{ cancelAI(); openLoadDialog(); restartLayout(); }); y2 += btnH + gap;
    makeBtnIdx("Reset", col2X, y2, [&]{ cancelAI(); game.reset(); restartLayout(); }); y2 += btnH + gap;

    // ==== HISTORY: chiếm toàn bộ chiều ngang dưới 2 cột ====
    float topAfterCols = std::max(y1, y2) + gap*2;
    histX = panelLeft;
    histW = panelWidth;                  // full 2 cột
    histY = topAfterCols;
    float bottomMargin = 40.f;
    histH = std::max(180.f, (float)window.getSize().y - histY - bottomMargin);

    highlightModeButtons();

    // --- Info text (Turn | Kết quả) đặt ngay TRÊN bàn cờ, dễ nhìn ---
    infoText.setFont(font);
    infoText.setCharacterSize(18);
    infoText.setFillColor(sf::Color(230,230,240));
    // đặt một dòng ngay phía trên mép trên của bàn
    infoText.setPosition(boardRect.getPosition().x, boardRect.getPosition().y - 70.f);

}


void UI::run(){
    while(window.isOpen()){
        sf::Event e;
        while(window.pollEvent(e)){
            if (e.type==sf::Event::Closed) window.close();

            if (e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left){
                sf::Vector2f mp(e.mouseButton.x, e.mouseButton.y);
                for (auto &b: buttons){ if (b.contains(mp)) { b.onClick(); highlightModeButtons(); } }
                handleClick(mp.x, mp.y);
            }
            if (e.type == sf::Event::MouseWheelScrolled){
                sf::Vector2f mp(e.mouseWheelScroll.x, e.mouseWheelScroll.y);
                handleWheel(mp.x, mp.y, e.mouseWheelScroll.delta);
            }
        }

        // 1) ƯU TIÊN: nếu đã có kết quả AI HỢP LỆ, áp dụng NGAY
        if (aiResult.has_value() && aiResultGen.has_value() &&
            *aiResultGen == aiGen &&
            game.config().aiLevel>0 && game.toPlay()==Player::White && !game.isFinished()){
            auto mv = *aiResult; 
            aiResult.reset(); 
            aiResultGen.reset();

            if (mv.isPass) {
                game.pass();
                // pass thì có thể phát âm nhẹ nếu muốn:
                // sPlace.play();
            } else {
                if (game.playMove(mv.x,mv.y)){
                    const auto& hist = game.history();
                    if (!hist.empty() && !hist.back().captures.empty()){
                        sCapture.play();
                    } else {
                        sPlace.play();
                    }
                }
            }
        }


        // 2) Nếu tới lượt AI mà chưa có job, spawn MỘT job (không tạo liên tục)
        if (!game.isFinished() && game.config().aiLevel>0 && game.toPlay()==Player::White){
            if (!aiBusy.load() && !aiResult.has_value()){
                aiBusy.store(true);
                aiCancel.store(false);
                ai = AIPlayer(game.config().aiLevel);
                aiResult.reset();
                aiResultGen.reset();
                uint64_t myGen = aiGen;
                Game snapshot = game;
                aiWorker.reset(new std::thread([this, snapshot, myGen]() mutable {
                    if (this->aiCancel.load()) { this->aiBusy.store(false); return; }
                    AIMove mv = ai.chooseMove(snapshot, Player::White);
                    if (!this->aiCancel.load()){ this->aiResult = mv; this->aiResultGen = myGen; }
                    this->aiBusy.store(false);
                }));
                aiWorker->detach();
            }
        } else {
            // không phải chế độ AI → chắc chắn tắt trạng thái bận
            if (game.config().aiLevel==0) cancelAI();
        }

                // Nếu đến lượt hiện tại mà không còn nước hợp lệ -> auto-pass
        if (!game.isFinished()) {
            Player turn = game.toPlay();
            if (!game.hasLegalMove(turn)) {
                game.pass();           // auto-pass
                // nếu pass này là lần thứ 2 liên tiếp thì Game sẽ finished=true
            }
        }


        // Vẽ
        window.clear(sf::Color(30,30,35));
        drawBoard();
        drawStones();
        drawHUD();
        drawMoveHistory();
        window.display();

        // Overlay loading của AI — không nhấp nháy nữa vì không còn spawn liên tục
        if (aiBusy.load()){
            sf::RectangleShape overlay(sf::Vector2f(760.f, 40.f));
            overlay.setPosition(40.f, 680.f);
            overlay.setFillColor(sf::Color(0,0,0,160));
            sf::Text t; t.setFont(font); t.setCharacterSize(18); t.setFillColor(sf::Color::White);
            t.setString("AI is thinking…");
            t.setPosition(60.f, 688.f);
            window.draw(overlay); window.draw(t);
        }

        

        if (game.isFinished()) {
            sf::RectangleShape over(sf::Vector2f(760.f, 80.f));
            over.setPosition(40.f, 660.f);
            over.setFillColor(sf::Color(0,0,0,180));
            sf::Text t; t.setFont(font); t.setCharacterSize(20); t.setFillColor(sf::Color::White);
            t.setString("Game Over - " + game.resultString() + " | New/Reset de choi lai");
            t.setPosition(60.f, 685.f);
            window.draw(over); window.draw(t);
        }

    }
}




void UI::drawBoard(){
    window.draw(boardRect);
    int N = game.board().size();
    float x0 = boardRect.getPosition().x;
    float y0 = boardRect.getPosition().y;
    float bp = boardRect.getSize().x;
    for (int i=0;i<N;i++){
        float x = x0 + i*cellSize;
        float y = y0 + i*cellSize;
        sf::Vertex line1[] = { sf::Vertex(sf::Vector2f(x, y0), sf::Color::Black),
                               sf::Vertex(sf::Vector2f(x, y0+bp), sf::Color::Black)};
        sf::Vertex line2[] = { sf::Vertex(sf::Vector2f(x0, y), sf::Color::Black),
                               sf::Vertex(sf::Vector2f(x0+bp, y), sf::Color::Black)};
        window.draw(line1, 2, sf::Lines);
        window.draw(line2, 2, sf::Lines);
    }
}

void UI::drawStones(){
    int N = game.board().size();
    float x0 = boardRect.getPosition().x;
    float y0 = boardRect.getPosition().y;
    for (int y=0;y<N;y++){
        for (int x=0;x<N;x++){
            Player p = game.board().at(x,y);
            if (p==Player::None) continue;
            sf::CircleShape c(cellSize*0.45f);
            c.setOrigin(c.getRadius(), c.getRadius());
            c.setPosition(x0 + x*cellSize, y0 + y*cellSize);
            c.setFillColor(p==Player::Black? sf::Color::Black: sf::Color::White);
            c.setOutlineThickness(2.f);
            c.setOutlineColor(sf::Color(0,0,0,100));
            window.draw(c);
        }
    }
}

void UI::drawHUD(){
    // Cập nhật nội dung HUD
    std::ostringstream oss;
    oss << (game.isFinished() ? "[Game Over] " : "")
        << "Turn: " << (game.toPlay()==Player::Black ? "Black" : "White")
        << " | " << game.resultString();
    infoText.setString(oss.str());

    // Vẽ dòng HUD
    window.draw(infoText);

    // Vẽ các nút (2 cột)
    for (auto &b: buttons) {
        window.draw(b.rect);
        window.draw(b.label);
    }

    // Vẽ nhãn Mode đã được đặt vị trí trong setupUI()
    window.draw(modeText);
}



std::pair<int,int> UI::mouseToCell(float mx, float my) const{
    auto pos = boardRect.getPosition();
    float x = (mx - pos.x)/cellSize;
    float y = (my - pos.y)/cellSize;
    int xi = int(x+0.5f);
    int yi = int(y+0.5f);
    if (xi<0||yi<0||xi>=game.board().size()||yi>=game.board().size()) return {-1,-1};
    return {xi, yi};
}

void UI::handleClick(float mx, float my){
    auto [cx,cy] = mouseToCell(mx,my);
    if (cx<0) return;
    if (game.isFinished()) { notifyEnd(); return; }
    if (game.config().aiLevel>0 && game.toPlay()==Player::White) return;

    // Trước đây là: if (game.playMove(cx,cy)){ sPlace.play(); }
    if (game.playMove(cx,cy)){
        // kiểm tra move vừa thêm có bắt quân không
        const auto& hist = game.history();
        if (!hist.empty() && !hist.back().captures.empty()){
            sCapture.play();
        } else {
            sPlace.play();
        }
    }
}


void UI::handleWheel(float mx, float my, float delta){
    sf::FloatRect panel(histX, histY, histW, histH);
    if (panel.contains(mx, my)){
        histScroll -= delta * histLineH * 3.f; // tốc độ cuộn
        int total = (int)game.history().size();
        float maxScroll = std::max(0.f, total*histLineH - histH);
        if (histScroll < 0.f) histScroll = 0.f;
        if (histScroll > maxScroll) histScroll = maxScroll;
    }
}


void UI::notifyEnd(){ sEnd.play(); }

void UI::openSaveDialog(){ Serializer::save(game, "savegame.json"); }
void UI::openLoadDialog(){ game.loadFromFile("savegame.json"); }

static std::string ellipsize(sf::Text& text, float maxWidth) {
    std::string s = text.getString();
    while (text.getLocalBounds().width > maxWidth && !s.empty()) {
        if (s.size() > 1) s.resize(s.size()-1);
        else break;
        text.setString(s + "…");
    }
    return text.getString();
}

void UI::drawMoveHistory(){
    // Panel nền
    sf::RectangleShape panel(sf::Vector2f(histW, histH));
    panel.setPosition(histX, histY);
    panel.setFillColor(sf::Color(40,40,48));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(90,90,110));
    window.draw(panel);

    // Title
    sf::Text title; title.setFont(font); title.setCharacterSize(16); title.setFillColor(sf::Color::White);
    title.setString("Move history");
    title.setPosition(histX, histY - 20.f);
    window.draw(title);

    // Nội dung (lọc theo Y để không vẽ ngoài khung)
    float startY = histY - histScroll;
    int N = game.board().size();
    int idx=1;
    float innerLeft = histX + 8.f;
    float innerWidth = histW - 16.f;

    for (auto &m: game.history()){
        std::string who = (m.player==Player::Black? "B":"W");
        std::string coord = m.isPass? "PASS": std::string(1, 'A'+m.x) + std::to_string(N - m.y);
        float y = startY + (idx-1)*histLineH;

        if (y + histLineH < histY){ idx++; continue; } // phía trên
        if (y > histY + histH){ break; }               // phía dưới

        sf::Text line; line.setFont(font); line.setCharacterSize(14);
        line.setFillColor(sf::Color(200,200,210));
        line.setString(std::to_string(idx) + ". " + who + " " + coord);
        line.setPosition(innerLeft, y);

        // Cắt bớt nếu quá rộng
        if (line.getLocalBounds().width > innerWidth){
            // thử giảm size chút
            line.setCharacterSize(13);
            if (line.getLocalBounds().width > innerWidth){
                // ellipsize
                ellipsize(line, innerWidth);
            }
        }

        window.draw(line);
        idx++;
    }

    // Scroll bar
    int total = (int)game.history().size();
    float contentH = std::max(histH, total*histLineH);
    if (contentH > histH + 1){
        float barH = std::max(20.f, histH * (histH / contentH));
        float barY = histY + (histScroll / (contentH - histH)) * (histH - barH);
        sf::RectangleShape bar(sf::Vector2f(6.f, barH));
        bar.setPosition(histX + histW - 10.f, barY);
        bar.setFillColor(sf::Color(160,160,180));
        window.draw(bar);
    }
}



