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

    // class BombScoreT {
    //     static constexpr int Limit = 170;
    //     int data_[Limit];
    // public:
    //     constexpr BombScoreT() :data_() {
    //         for (int i = 1; i < Limit; ++i)
    //             data_[i] = static_cast<int>(25.0 * cpow<double>(2.0, double(i) / 12.0)); // 有理数だから二分法で簡単に出来なくもない
    //     }
    //     constexpr int operator[](int i) const { return data_[i]; }
    // };

    // なんもわからん
    constexpr ChainScoreT ChainScore;
    constexpr ChainSkillScoreT ChainSkillScore;
    constexpr int BombScore[] = { 0,26,28,29,31,33,35,37,39,42,44,47,50,52,56,59,62,66,70,74,79,84,89,94,100,105,112,118,125,133,141,149,158,168,178,188,200,211,224,237,251,266,282,299,317,336,356,377,400,423,448,475,503,533,565,599,634,672,712,755,800,847,897,951,1007,1067,1131,1198,1269,1345,1425,1510,1600,1695,1795,1902,2015,2135,2262,2397,2539,2690,2850,3020,3200,3390,3591,3805,4031,4271,4525,4794,5079,5381,5701,6040,6400,6780,7183,7610};

}