#include "Macro.h"
#include "Game.h"



namespace Game {


    const Command Command::Skill = Command(1 << 6);

    constexpr ChainScoreT ChainScore;
    constexpr ChainSkillScoreT ChainSkillScore;
    constexpr BombScoreT BombScore;


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


    // えーん
    // int Field::eliminate() {
    //     Matrix<int8_t, H, W> flag;
    // 
    //     // y方向の消去
    //     repeat(x, W) {
    //         // しゃくとり法
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(y, H) {
    //             // 空の場合
    //             if (at(y, x) == 0) {
    //                 sum = 0;
    //                 l = y + 1;
    //                 continue;
    //             }
    //             // 新たに範囲に追加
    //             sum += at(y, x);
    //             // 10以下になるまで範囲末尾を削る
    //             while (sum > 10) {
    //                 sum -= at(l, x);
    //                 ++l;
    //             }
    //             // 10ならば，範囲を全て消去
    //             if (sum == 10) {
    //                 while (l <= y) {
    //                     flag(l, x) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // x方向の消去
    //     repeat(y, H) {
    //         // しゃくとり法
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(x, W) {
    //             // 空の場合
    //             if (at(y, x) == 0) {
    //                 sum = 0;
    //                 l = x + 1;
    //                 continue;
    //             }
    //             // 新たに範囲に追加
    //             sum += at(y, x);
    //             // 10以下になるまで範囲末尾を削る
    //             while (sum > 10) {
    //                 sum -= at(y, l);
    //                 ++l;
    //             }
    //             // 10ならば，範囲を全て消去
    //             if (sum == 10) {
    //                 while (l <= x) {
    //                     flag(y, l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // 右上1
    //     iterate(u, 0, H - 1) {
    //         // しゃくとり法
    //         int l = 0;
    //         int8_t sum = 0;
    //         int n = min(W, H - u);
    //         repeat(i, n) {
    //             // 空の場合
    //             if (at(u + i, i) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // 新たに範囲に追加
    //             sum += at(u + i, i);
    //             // 10以下になるまで範囲末尾を削る
    //             while (sum > 10) {
    //                 sum -= at(u + l, l);
    //                 ++l;
    //             }
    //             // 10ならば，範囲を全て消去
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(u + l, l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // 右上2
    //     iterate(v, 1, W - 1) {
    //         // しゃくとり法
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(i, W - v) {
    //             // 空の場合
    //             if (at(i, v + i) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // 新たに範囲に追加
    //             sum += at(i, v + i);
    //             // 10以下になるまで範囲末尾を削る
    //             while (sum > 10) {
    //                 sum -= at(l, v + l);
    //                 ++l;
    //             }
    //             // 10ならば，範囲を全て消去
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(l, v + l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // 右下1
    //     iterate(u, 1, H) {
    //         // しゃくとり法
    //         int l = 0;
    //         int8_t sum = 0;
    //         int n = min(W, u + 1);
    //         repeat(i, n) {
    //             // 空の場合
    //             if (at(u - i, l) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // 新たに範囲に追加
    //             sum += at(u-i, i);
    //             // 10以下になるまで範囲末尾を削る
    //             while (sum > 10) {
    //                 sum -= at(u-l, l);
    //                 ++l;
    //             }
    //             // 10ならば，範囲を全て消去
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(u - l, l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // 右下2
    //     iterate(v, 1, W-1) {
    //         // しゃくとり法
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(i, W - v) {
    //             // 空の場合
    //             if (at(H - 1 - i, v + i) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // 新たに範囲に追加
    //             sum += at(H - 1 - i, v + i);
    //             // 10以下になるまで範囲末尾を削る
    //             while (sum > 10) {
    //                 sum -= at(H - 1 - l, v + l);
    //                 ++l;
    //             }
    //             // 10ならば，範囲を全て消去
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(H - 1 - l, v + l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    // 
    //     int cnt = 0;
    // 
    //     // マークした点を消去
    //     repeat(x, W) {
    //         repeat(y, H) {
    //             if (flag(y, x)) at(y, x) = 0, ++cnt;
    //         }
    //     }
    // 
    // 
    //     return cnt;
    // }

    int Field::eliminate() {
        Matrix<int8_t, H, W> flag;
        // 消されるブロックをマークする
        repeat(x, W) {
            repeat(y, H) {
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (safeat(y, x + 1) + a == 10)
                    flag(y, x) = 1,
                    flag(y, x + 1) = 1;
                if (safeat(y + 1, x + 1) + a == 10)
                    flag(y, x) = 1,
                    flag(y + 1, x + 1) = 1;
                if (safeat(y + 1, x) + a == 10)
                    flag(y, x) = 1,
                    flag(y + 1, x) = 1;
                if (safeat(y + 1, x - 1) + a == 10)
                    flag(y, x) = 1,
                    flag(y + 1, x - 1) = 1;
            }
        }

        // マークした点を消去
        int cnt = 0;
        repeat(x, W) {
            repeat(y, H) {
                if (flag(y, x) == 1) at(y, x) = 0, ++cnt;
            }
        }

        return cnt;
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

    // コマンドを適応して，フィールドを変化させる．
    // スキルゲージ等は一切変化させない
    // 回転前のパックを与える
    // @return: score, chain, sender, success
    tuple<int, int, int, bool> Player::apply(Command cmd, const Pack& turnPack) {

        int score = 0;
        int chain = 0;
        int sender = 0;
        bool success = true;

        if (cmd.skill()) {
            // 爆発
            int bcnt = field.explode();
            success = field.fall();
            // 爆発スコア
            int sb = BombScore[bcnt];

            // チェイン
            while (field.eliminate() > 0) {
                ++chain;
                success = field.fall();
            }
            // スキルチェインスコア
            int ss = ChainSkillScore[chain];

            sender = sb / 2 + ss / 2;
            score = sb + ss;
        }
        else {
            // パック投下
            field.insert(turnPack.rotated(cmd.rot()), cmd.xPos());
            success = field.fall();

            // チェイン
            while (field.eliminate() > 0) {
                ++chain;
                success = field.fall();
            }
            // チェインスコア
            int ss = ChainScore[chain];

            sender = ss / 2;
            score = ss;
        }

        return make_tuple(score, chain, sender, success);
    }


    // コマンドを適応して，次の状態にする．
    // 回転前のパックを与える
    // @return: success?
    bool Input::apply(Command cmdMe, Command cmdEn, const Pack& turnPack) {
        
        // お邪魔ブロックの投下処理
        if (me.ojama >= 10) {
            me.field.stackOjama();
            me.ojama -= 10;
        }
        if (en.ojama >= 10) {
            en.field.stackOjama();
            en.ojama -= 10;
        }

        // 獲得スコア
        int mescore = 0, enscore = 0;
        // チェイン数
        int mechain = 0, enchain = 0;
        // お邪魔ブロックを送る個数
        int mesender = 0, ensender = 0;
        // 詰まなかった
        bool mesuccess = true, ensuccess = true;

        // フィールドの変化
        tie(mescore, mechain, mesender, mesuccess) = me.apply(cmdMe, turnPack);
        tie(enscore, enchain, ensender, ensuccess) = en.apply(cmdEn, turnPack);

        // スキルゲージの消耗
        if (cmdMe.skill()) {
            assert(me.skill >= 80);
            me.skill = 0;
        }
        if (cmdEn.skill()) {
            assert(en.skill >= 80);
            en.skill = 0;
        }

        // スキルゲージの変化
        if (mechain >= 1) me.skill += 8;
        if (enchain >= 1) en.skill += 8;
        chlim(me.skill, 0, 100);
        chlim(en.skill, 0, 100);
        if (mechain >= 3) en.skill -= 12 + 2 * mechain;
        if (enchain >= 3) me.skill -= 12 + 2 * enchain;
        chlim(me.skill, 0, 100);
        chlim(en.skill, 0, 100);

        // お邪魔の変化
        me.ojama += ensender;
        en.ojama += mesender;
        if (me.ojama > en.ojama)
            me.ojama -= en.ojama,
            en.ojama = 0;
        else
            en.ojama -= me.ojama,
            me.ojama = 0;

        // ターン増加
        turnCount += 1;

        return mesuccess && ensuccess && turnCount < N;

    }
}