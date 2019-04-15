#pragma once
#include "Macro.h"
#include "Game.h"

namespace Game {

    // IO構造体
    // ===========================


    // ゲーム開始時の入力
    struct FirstInput {
        array<Pack, N> packs;

        template<typename istream>
        void input(istream& is) {
            for (auto& p : packs) is >> p;
        }
    };


    // ターンごとのプレイヤーの状態
    struct Player {
        int timeMsec;
        int ojama;
        int skill;
        Field field;

        template<typename istream>
        void input(istream& is) {
            int score; // TODO: 仕様変更したかな？
            is >> timeMsec >> ojama >> skill >> score;
            rrepeat(y, HLimit) {
                repeat(x, W) {
                    int v; is >> v;
                    field(y, x) = v;
                }
            }
            repeat(x, W) {
                iterate(y, HLimit, H) {
                    field(y, x) = 0;
                }
            }
            string str; is >> str;
            assert(str == "END");
        }

        // コマンドを適応して，フィールドを変化させる．
        // スキルゲージ等は一切変化させない
        // @return: score, chain, sender, success
        tuple<int, int, int, bool> apply(Command, const Pack&);
    };


    // ターンごとの入力
    struct Input {
        int turn;
        Player me; // わたくし
        Player en; // あいて

        template<typename istream>
        void input(istream& is) {
            is >> turn;
            if (is.eof()) return;
            me.input(is);
            en.input(is);
        }

        bool apply(Command, Command, const Pack&);
    };

}