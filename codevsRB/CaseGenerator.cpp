#include "Game.h"

namespace Generator {
    static mt19937_64 mt;

    void setSeed(decltype(mt19937_64::default_seed) seed) {
        mt.seed(seed);
    }

    void generateFirstInput(Game::FirstInput& fi) {
        uniform_int_distribution<int> udist1_9(1, 9);
        uniform_int_distribution<int> udist0_3(0, 3);
        for (int i = 0; i < Game::N; ++i) {
            fi.packs[i](0, 0) = udist1_9(mt);
            fi.packs[i](1, 0) = udist1_9(mt);
            fi.packs[i](0, 1) = udist1_9(mt);
            fi.packs[i](1, 1) = udist1_9(mt) < 5 ? udist1_9(mt) : 0;
            fi.packs[i].rotate(udist0_3(mt));
        }
    }

    void setDefaultPlayer(Game::Player& p) {
        p.ojama = 0;
        p.skill = 0;
        p.timeMsec = 100000;
        p.field.fill(0);
    }

    void setDefaultInput(Game::Input& i) {
        i.turn = 0;
        setDefaultPlayer(i.me);
        setDefaultPlayer(i.en);
    }
}