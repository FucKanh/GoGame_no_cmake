#include "Board.h"
#include <queue>
#include <set>

Board::Board(int n): N(n), g(n, std::vector<Player>(n, Player::None)) {}

Player Board::at(int x, int y) const {
    if (x<0||y<0||x>=N||y>=N) return Player::None;
    return g[y][x];
}
void Board::set(int x, int y, Player p) {
    if (x<0||y<0||x>=N||y>=N) return;
    g[y][x]=p;
}

std::vector<std::pair<int,int>> Board::listGroup(const std::vector<std::vector<Player>>& grid, int sx, int sy) const {
    Player color = grid[sy][sx];
    std::vector<std::pair<int,int>> group;
    std::queue<std::pair<int,int>> q;
    std::set<long long> vis;
    q.push({sx,sy});
    vis.insert(((long long)sy<<32)|sx);
    int Nlocal = (int)grid.size();
    auto pushN = [&](int x,int y){
        if (x>=0&&y>=0&&x<Nlocal&&y<Nlocal && grid[y][x]==color) {
            long long key=((long long)y<<32)|x;
            if (!vis.count(key)) {vis.insert(key); q.push({x,y});}
        }
    };
    while(!q.empty()){
        auto [x,y]=q.front(); q.pop();
        group.push_back({x,y});
        pushN(x+1,y); pushN(x-1,y); pushN(x,y+1); pushN(x,y-1);
    }
    return group;
}

int Board::countLiberties(int sx, int sy) const {
    if (sx<0||sy<0||sx>=N||sy>=N) return 0;
    if (g[sy][sx]==Player::None) return 0;
    auto group = listGroup(g, sx, sy);
    std::set<long long> libs;
    for (auto [x,y]: group){
        const int dx[4]={1,-1,0,0};
        const int dy[4]={0,0,1,-1};
        for (int k=0;k<4;k++){
            int nx=x+dx[k], ny=y+dy[k];
            if (nx>=0&&ny>=0&&nx<N&&ny<N && g[ny][nx]==Player::None){
                libs.insert(((long long)ny<<32)|nx);
            }
        }
    }
    return (int)libs.size();
}

std::vector<std::pair<int,int>> Board::captureIfNoLiberties(int sx, int sy, Player p){
    std::vector<std::pair<int,int>> captured;
    if (sx<0||sy<0||sx>=N||sy>=N) return captured;

    Player enemy = (p==Player::Black? Player::White: Player::Black);
    const int dx[4]={1,-1,0,0};
    const int dy[4]={0,0,1,-1};

    // Đánh dấu các ô địch đã xử lý để không lặp nhóm qua nhiều hướng
    std::vector<std::vector<char>> seen(N, std::vector<char>(N, 0));

    auto bfsGroupAndLibs = [&](int gx, int gy){
        // BFS 1 nhóm bắt đầu tại (gx,gy). Trả về (group, liberties)
        std::vector<std::pair<int,int>> group;
        int liberties = 0;
        std::queue<std::pair<int,int>> q;
        q.push({gx,gy});
        seen[gy][gx] = 1;

        while(!q.empty()){
            auto [x,y]=q.front(); q.pop();
            group.push_back({x,y});
            for (int k=0;k<4;k++){
                int nx=x+dx[k], ny=y+dy[k];
                if (nx<0||ny<0||nx>=N||ny>=N) continue;
                if (g[ny][nx]==Player::None){
                    // đếm liberties theo ô trống tiếp giáp
                    liberties++;
                } else if (g[ny][nx]==enemy && !seen[ny][nx]){
                    seen[ny][nx]=1;
                    q.push({nx,ny});
                }
            }
        }
        return std::make_pair(group, liberties);
    };

    // Chỉ kiểm tra 4 nhóm địch kề cạnh nước đặt
    for (int k=0;k<4;k++){
        int nx=sx+dx[k], ny=sy+dy[k];
        if (nx<0||ny<0||nx>=N||ny>=N) continue;
        if (g[ny][nx]!=enemy) continue;
        if (seen[ny][nx]) continue;

        auto [group, libs] = bfsGroupAndLibs(nx, ny);
        if (libs==0){
            // gom vào danh sách bắt (KHÔNG xóa ngay để không ảnh hưởng nhóm kế)
            captured.insert(captured.end(), group.begin(), group.end());
        }
    }

    // Sau khi xác định xong, mới xóa tất cả một lượt
    removeStones(captured);
    return captured;
}


bool Board::isLegal(int x, int y, Player p, std::optional<std::pair<int,int>> koPoint) const {
    if (x<0||y<0||x>=N||y>=N) return false;
    if (g[y][x]!=Player::None) return false;
    if (koPoint && koPoint->first==x && koPoint->second==y) return false;

    auto sim = g;
    sim[y][x]=p;

    auto listGroupLocal = [&](int sx,int sy){ return listGroup(sim, sx, sy); };
    auto countLibsLocal = [&](int sx,int sy){
        std::set<long long> libs;
        if (sx<0||sy<0||sx>=N||sy>=N) return 0;
        if (sim[sy][sx]==Player::None) return 0;
        auto grp = listGroupLocal(sx,sy);
        const int dx[4]={1,-1,0,0};
        const int dy[4]={0,0,1,-1};
        for (auto [gx,gy]: grp){
            for (int k=0;k<4;k++){
                int nx=gx+dx[k], ny=gy+dy[k];
                if (nx>=0&&ny>=0&&nx<N&&ny<N && sim[ny][nx]==Player::None){
                    libs.insert(((long long)ny<<32)|nx);
                }
            }
        }
        return (int)libs.size();
    };
    Player enemy = (p==Player::Black? Player::White: Player::Black);
    const int dx[4]={1,-1,0,0};
    const int dy[4]={0,0,1,-1};
    for (int k=0;k<4;k++){
        int nx=x+dx[k], ny=y+dy[k];
        if (nx>=0&&ny>=0&&nx<N&&ny<N && sim[ny][nx]==enemy){
            if (countLibsLocal(nx,ny)==0) return true;
        }
    }
    return countLibsLocal(x,y)>0;
}

void Board::removeStones(const std::vector<std::pair<int,int>>& stones){
    for (auto [x,y]: stones){
        if (x>=0&&y>=0&&x<N&&y<N) g[y][x]=Player::None;
    }
}

int Board::estimateArea(Player p) const {
    int Nn = N;
    std::vector<std::vector<int>> vis(Nn, std::vector<int>(Nn,0));
    int score = 0;
    for (int y=0;y<Nn;y++){
        for (int x=0;x<Nn;x++){
            if (g[y][x]!=Player::None) {
                if (g[y][x]==p) score += 1;
                continue;
            }
            if (vis[y][x]) continue;
            std::vector<std::pair<int,int>> region;
            std::queue<std::pair<int,int>> q;
            q.push({x,y}); vis[y][x]=1;
            bool seenBlack=false, seenWhite=false;
            while(!q.empty()){
                auto [cx,cy]=q.front(); q.pop();
                region.push_back({cx,cy});
                const int dx[4]={1,-1,0,0};
                const int dy[4]={0,0,1,-1};
                for (int k=0;k<4;k++){
                    int nx=cx+dx[k], ny=cy+dy[k];
                    if (nx>=0&&ny>=0&&nx<Nn&&ny<Nn){
                        if (g[ny][nx]==Player::None && !vis[ny][nx]){
                            vis[ny][nx]=1; q.push({nx,ny});
                        } else if (g[ny][nx]==Player::Black) seenBlack=true;
                        else if (g[ny][nx]==Player::White) seenWhite=true;
                    }
                }
            }
            if (seenBlack && !seenWhite && p==Player::Black) score += (int)region.size();
            if (seenWhite && !seenBlack && p==Player::White) score += (int)region.size();
        }
    }
    return score;
}
