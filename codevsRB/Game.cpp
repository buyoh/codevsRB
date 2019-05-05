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
			// iは境界を指しているが、重力側、空側のどちらかは不明なので、(at(i, x) == None)で調べる
            ok &= (i - (at(i, x) == None) < HLimit);
        }
        return ok;
    }


    bool Field::partialFall(array<int, W>& enabledCols) {
        bool ok = true;
        repeat(x, W) {
            if (enabledCols[x] <= 0) continue;
            enabledCols[x] = 0; // 一旦書き換わらなかったことにする
            int i = 0;
            repeat(y, H) {
                if (at(y, x) != None) {
                    while (i < H && at(i, x) != None) ++i;
                    // この時点で, iはNoneを指している
                    // i以下にNoneとなるブロックは存在しない
                    // iより上にyがあるならば，yの位置のブロックをiに持っていく
                    if (i < y)
                        std::swap(at(i, x), at(y, x)),
                        enabledCols[x] = y + 1; // 書き換わったのでtrueにする
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
        vector<pair<int8_t, int8_t>> flag; flag.clear();
        // 消されるブロックをマークする
        repeat(x, W) {
            repeat(y, H) {
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (safeat(y, x + 1) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y, x + 1);
                if (safeat(y + 1, x + 1) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x + 1);
                if (safeat(y + 1, x) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x);
                if (safeat(y + 1, x - 1) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x - 1);
            }
        }

        // マークした点を消去
        int cnt = 0;
        for (auto p : flag) {
            cnt += at(p.first, p.second) > 0;
            at(p.first, p.second) = 0;
        }

        return cnt;
    }


    //
    int Field::partialEliminate(array<int, W>& enabledCols) {
        array<P, 32> erased;
        int erasedPtr = 0;
        // 消されるブロックをマークする
        repeat(x, W - 1) {
            // 今見ているxまたはその右隣りがマークされていたら，書き換える可能性がある
            if (enabledCols[x] <= 0 && enabledCols[x + 1] <= 0) continue;
            repeat(y, H) {
                if (enabledCols[x] < y && enabledCols[x + 1] < y) break;
                auto a = at(y, x);
                if (a == Ojama) continue;
                bool b = false;
                if (y > 0 && at(y - 1, x + 1) == 10 - a)
                    b = true,
                    erased[erasedPtr].y = y - 1,
					erased[erasedPtr++].x = x + 1;
                if (at(y, x + 1) == 10 - a)
                    b = true,
					erased[erasedPtr].y = y,
					erased[erasedPtr++].x = x + 1;
                if (y < H - 1 && at(y + 1, x + 1) == 10 - a)
                    b = true,
					erased[erasedPtr].y = y + 1,
					erased[erasedPtr++].x = x + 1;
                if (y < H - 1 && at(y + 1, x) == 10 - a)
                    b = true,
					erased[erasedPtr].y = y + 1,
					erased[erasedPtr++].x = x;
                if (b)
					erased[erasedPtr].y = y,
					erased[erasedPtr++].x = x;

            }
        }
        // W-1 列
        if (enabledCols[W - 1] > 0){
            const int x = W - 1;
            repeat(y, H - 1) {
                if (enabledCols[W - 1] < y) break;
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (at(y + 1, x) == 10 - a)
					erased[erasedPtr].y = y,
					erased[erasedPtr++].x = x,
					erased[erasedPtr].y = y + 1,
					erased[erasedPtr++].x = x;
            }
        }

        // reset
        enabledCols.fill(0);

        // マークした点を消去
        int cnt = 0;
        repeat(i, erasedPtr) {
            auto p = erased[i];
            cnt += at(p.y, p.x) > 0;
            at(p.y, p.x) = 0;
            chmax<int>(enabledCols[p.x], p.y + 1);
        }

        return cnt;
    }


    int Field::explode() {
		array<array<bool, H>, W> flag;
		for (auto& v : flag) v.fill(false);

        // 消されるブロックをマークする
        repeat(x, W - 1) {
            repeat(y, H) {
                if (at(y, x) == Ojama || at(y, x) == 0) continue;

				if (at(y, x) == 5) {
					flag[x][y] = true;
					if (0 < y) flag[x][y - 1] = true;
					if (0 < y) flag[x + 1][y - 1] = true;
					flag[x + 1][y] = true;
					if (y < H - 1) flag[x + 1][y + 1] = true;
				}
				else {
					if (0 < y && at(y - 1, x) == 5) flag[x][y] = true;
					else if (0 < y && at(y - 1, x + 1) == 5) flag[x][y] = true;
					else if (at(y, x + 1) == 5) flag[x][y] = true;
					else if (y < H - 1 && at(y + 1, x + 1) == 5) flag[x][y] = true;
				}
            }
        }
		{
			const int x = W - 1;
			repeat(y, H) {
				if (at(y, x) == Ojama || at(y, x) == 0) continue;

				if (at(y, x) == 5) {
					flag[x][y] = true;
					if (0 < y) flag[x][y - 1] = true;
				}
				else {
					if (0 < y && at(y - 1, x) == 5) flag[x][y] = true;
				}
			}
		}

        // マークした点を消去
        int cnt = 0;
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) != Ojama && at(y, x) != 0 && flag[x][y] == 1)
					at(y, x) = 0, ++cnt;
            }
        }

        return cnt;
    }
    
    
    pair<int, bool> Field::chain() {
        array<int, W> flag; flag.fill(H);
        int cnt = 0;
        bool ok = true;
        //ok = partialFall(flag);
        // ok = fall();
        while (partialEliminate(flag) > 0) {
            ++cnt;
            ok = partialFall(flag);
        }
        return make_pair(cnt, ok);
    }


    void Field::stackOjama() {
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == 0) {
                    at(y, x) = Ojama;
                    break;
                }
            }
        }
    }


	int Field::countWithountOjama() const {
		int k = 0;
		repeat(i, W * H) k += 1 <= data_[i] && data_[i] <= 9;
		return k;
	}

}