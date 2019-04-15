#pragma once
#include "Macro.h"
#include "Game.h"

namespace Game {

    // IO�\����
    // ===========================


    // �Q�[���J�n���̓���
    struct FirstInput {
        array<Pack, N> packs;

        template<typename istream>
        void input(istream& is) {
            for (auto& p : packs) is >> p;
        }
    };


    // �^�[�����Ƃ̃v���C���[�̏��
    struct Player {
        int timeMsec;
        int ojama;
        int skill;
        Field field;

        template<typename istream>
        void input(istream& is) {
            int score; // TODO: �d�l�ύX�������ȁH
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

        // �R�}���h��K�����āC�t�B�[���h��ω�������D
        // �X�L���Q�[�W���͈�ؕω������Ȃ�
        // @return: score, chain, sender, success
        tuple<int, int, int, bool> apply(Command, const Pack&);
    };


    // �^�[�����Ƃ̓���
    struct Input {
        int turn;
        Player me; // �킽����
        Player en; // ������

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