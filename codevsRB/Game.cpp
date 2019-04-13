#include "Macro.h"
#include "Game.h"



namespace Game {


    const Command Command::Skill = Command(1 << 6);


    bool Field::fall() {
        bool ok = true;
        repeat(x, W) {
            int i = 0;
            repeat(y, H) {
                if (at(y, x) != None) {
                    while (i < H && at(i, x) != None) ++i;
                    // この時点で, iはNoneを指している
                    // i以下にNoneとなるブロックは存在しない
                    // iより上にyがあるならば，yの位置のブロックをiに持っていく
                    if (i < y) std::swap(at(i, x), at(y, x));
                }
            }
            ok &= (i < HLimit);
        }
        return ok;
    }


    // Packを置く（すでに落とした状態で）
    bool Field::insert(const Pack& pack, int x) {

        // パックが落ちる高さを探す
        int y1 = 0;
        while (y1 < H - 1 && at(y1, x) != None) ++y1;
        int y2 = 0;
        while (y2 < H - 1 && at(y2, x + 1) != None) ++y2;

        // 置けない
        if (y1 >= H - 1 || y2 >= H - 1) return false;

        // 置く
        if (pack(1,0) != None){
            at(y1 + 1, x) = pack(0, 0);
            at(y1, x) = pack(1, 0);
        }
        else {
            at(y1, x) = pack(0, 0);
        }
        if (pack(1, 1) != None) {
            at(y2 + 1, x + 1) = pack(0, 1);
            at(y2, x + 1) = pack(1, 1);
        }
        else {
            at(y2, x + 1) = pack(0, 1);
        }

        return true;
    }


    int Field::eliminate() {
        vector<pair<int8_t, int8_t>> flag;
        // 消されるブロックをマークする
        repeat(x, W) {
            repeat(y, H) {
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (safeat(y, x + 1) + a == 10)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y, x + 1);
                if (safeat(y + 1, x + 1) + a == 10)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x + 1);
                if (safeat(y + 1, x) + a == 10)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x);
                if (safeat(y + 1, x - 1) + a == 10)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x - 1);
            }
        }

        // マークした点を消去
        int cnt = 0;
        for (auto p : flag) {
            if (at(p.first, p.second) > 0) {
                at(p.first, p.second) = 0;
                ++cnt;
            }
        }

        return cnt;
    }


    // TODO:
    int Field::explode() {
        Matrix<int8_t, H, W> flag;

        // 消されるブロックをマークする
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == Ojama || at(y, x) == 0 || at(y, x) == 5) continue;
                iterate(i, -1, 2) {
                    iterate(j, -1, 2) {
                        if (safeat(y + j, x + i) == 5) flag(y, x) = 1; 
                    }
                }
            }
        }

        // マークした点を消去
        int cnt = 0;
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == 5 || flag(y, x) == 1) at(y, x) = 0, ++cnt;
            }
        }

        return cnt;
    }


    bool Field::stackOjama() {
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == 0) {
                    if (y >= HLimit) return true;
                    at(y, x) = Ojama;
                    break;
                }
            }
        }
        return false;
    }

}