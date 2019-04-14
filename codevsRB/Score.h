#pragma once

#include "Macro.h"

namespace Game {


    //
    // Score
    // ===========================


    class ChainScoreT {
        static constexpr int Limit = 50;
        int data_[Limit];
    public:
        constexpr ChainScoreT() :data_() {
            for (int i = 1; i < Limit; ++i)
                data_[i] = data_[i - 1] + static_cast<int>(cpow(1.3, double(i)));
        }
        constexpr int operator[](int i) const { return data_[i]; }
    };

    class ChainSkillScoreT {
        static constexpr int Limit = 50;
        int data_[Limit];
    public:
        constexpr ChainSkillScoreT() :data_() {
            for (int i = 1; i < Limit; ++i)
                data_[i] = data_[i - 1] + static_cast<int>(cpow(1.3, double(i)));
        }
        constexpr int operator[](int i) const { return data_[i]; }
    };

    class BombScoreT {
        static constexpr int Limit = 170;
        int data_[Limit];
    public:
        constexpr BombScoreT() :data_() {
            for (int i = 1; i < Limit; ++i)
                data_[i] = static_cast<int>(25.0 * cpow(2.0, double(i) / 12.0));
        }
        constexpr int operator[](int i) const { return data_[i]; }
    };

    // ‚È‚ñ‚à‚í‚©‚ç‚ñ
    constexpr ChainScoreT ChainScore;
    constexpr ChainSkillScoreT ChainSkillScore;
    constexpr BombScoreT BombScore;

}