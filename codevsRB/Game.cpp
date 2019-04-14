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
                    // ���̎��_��, i��None���w���Ă���
                    // i�ȉ���None�ƂȂ�u���b�N�͑��݂��Ȃ�
                    // i�����y������Ȃ�΁Cy�̈ʒu�̃u���b�N��i�Ɏ����Ă���
                    if (i < y) std::swap(at(i, x), at(y, x));
                }
            }
            ok &= (i < HLimit);
        }
        return ok;
    }


    bool Field::partialFall(array<bool,W>& enabledCols) {
        bool ok = true;
        repeat(x, W) {
            if (!enabledCols[x]) continue;
            enabledCols[x] = false; // ��U���������Ȃ��������Ƃɂ���
            int i = 0;
            repeat(y, H) {
                if (at(y, x) != None) {
                    while (i < H && at(i, x) != None) ++i;
                    // ���̎��_��, i��None���w���Ă���
                    // i�ȉ���None�ƂȂ�u���b�N�͑��݂��Ȃ�
                    // i�����y������Ȃ�΁Cy�̈ʒu�̃u���b�N��i�Ɏ����Ă���
                    if (i < y)
                        std::swap(at(i, x), at(y, x)),
                        enabledCols[x] = true; // ������������̂�true�ɂ���
                }
            }
            ok &= (i < HLimit);
        }
        return ok;
    }


    // Pack��u���i���łɗ��Ƃ�����ԂŁj
    bool Field::insert(const Pack& pack, int x) {

        // �p�b�N�������鍂����T��
        int y1 = 0;
        while (y1 < H - 1 && at(y1, x) != None) ++y1;
        int y2 = 0;
        while (y2 < H - 1 && at(y2, x + 1) != None) ++y2;

        // �u���Ȃ�
        if (y1 >= H - 1 || y2 >= H - 1) return false;

        // �u��
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
        static vector<pair<int8_t, int8_t>> flag; flag.clear();
        // �������u���b�N���}�[�N����
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

        // �}�[�N�����_������
        int cnt = 0;
        for (auto p : flag) {
            cnt += at(p.first, p.second) > 0;
            at(p.first, p.second) = 0;
        }

        return cnt;
    }


    //
    int Field::partialEliminate(array<bool, W>& enabledCols) {
        static vector<pair<int8_t, int8_t>> flag; flag.clear();
        // �������u���b�N���}�[�N����
        repeat(x, W - 1) {
            // �����Ă���x�܂��͂��̉E�ׂ肪�}�[�N����Ă�����C����������\��������
            if (!enabledCols[x] && !enabledCols[x + 1]) continue;
            repeat(y, H) {
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (y > 0 && at(y - 1, x + 1) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y - 1, x + 1);
                if (at(y, x + 1) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y, x + 1);
                if (y < H - 1 && at(y + 1, x + 1) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x + 1);
                if (y < H - 1 && at(y + 1, x) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x);
            }
        }
        // W-1 ��
        if (enabledCols[W - 1]){
            const int x = W - 1;
            repeat(y, H - 1) {
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (at(y + 1, x) == 10 - a)
                    flag.emplace_back(y, x),
                    flag.emplace_back(y + 1, x);
            }
        }

        // reset
        enabledCols.fill(false);

        // �}�[�N�����_������
        int cnt = 0;
        for (auto p : flag) {
            cnt += at(p.first, p.second) > 0;
            at(p.first, p.second) = 0;
            enabledCols[p.second] = true;
        }

        return cnt;
    }


    int Field::explode() {
        Matrix<int8_t, H, W> flag;

        // �������u���b�N���}�[�N����
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

        // �}�[�N�����_������
        int cnt = 0;
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == 5 || flag(y, x) == 1) at(y, x) = 0, ++cnt;
            }
        }

        return cnt;
    }
    
    
    pair<int, bool> Field::chain() {
        array<bool, W> flag; flag.fill(true);
        int cnt = 0;
        bool ok = true;
        //ok = partialFall(flag);
        ok = fall();
        while (partialEliminate(flag) > 0) {
            ++cnt;
            ok = partialFall(flag);
        }
        return make_pair(cnt, ok);
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