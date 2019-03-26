#pragma once
#include "Macro.h"
#include "Util.h"

namespace Game {

    //

    const int H = 19; // field height
    const int HLimit = 17; // field height Limit
    const int W = 10; // field width
    const int L = 2; // pack width

    const int8_t Ojama = 11; // お邪魔ブロックの色
    const int8_t None = 0; // 空虚の色


    //


    // Field, Packの抽象クラス
    // メモリ上にはy軸方向に整列されている点が通常と異なる．
    template<typename T, int Height, int Width>
    class Matrix {

    public:
        using container_type = vector<T>;

    protected:
        container_type data_;

        //

    public:
        inline Matrix() :data_(H*W) { }
        inline Matrix(container_type&& d) : data_(move(d)) { data_.resize(H*W); }

        inline T& at(int y, int x) noexcept { return data_[y + x * Height]; }
        inline T operator()(int y, int x) const noexcept { return data_[y + x*Height]; }
        inline T operator()(P p) const noexcept { return data_[p.y + p.x*Height]; }
        inline T& operator()(int y, int x) noexcept { return data_[y + x * Height]; }
        inline T& operator()(P p) noexcept { return data_[p.y + p.x*Height]; }

        inline bool safe(int y, int x) const noexcept { return 0 <= y && y < H && 0 <= x && x < W; }

        inline void swap(Matrix& f) {
            data_.swap(f.data_);
        }

        inline void fill(const T& val) {
            std::fill(data_.begin(), data_.end(), val);
        }

    };


    //


    class Pack : public Matrix<int8_t, L, L> {


        //



        //

    public:
        inline Pack() :Matrix() { }
        inline Pack(vector<int8_t>&& d) :Matrix(move(d)) { }

        //

        // 左回転をcnt回数行ったパックを返す
        Pack rotated(int cnt) const {
            const static int pat[][4] = { {0,1,2,3},{2,0,3,1},{3,2,1,0},{1,3,0,2} };
            vector<int8_t> d(4);
            cnt &= 3;
            d[0] = data_[pat[cnt][0]];
            d[1] = data_[pat[cnt][1]];
            d[2] = data_[pat[cnt][2]];
            d[3] = data_[pat[cnt][3]];
            return Pack(move(d));
        }

        // 左回転をcnt回数行う
        void rotate(int cnt) {
            const static int pat[][4] = { {0,1,2,3},{2,0,3,1},{3,2,1,0},{1,3,0,2} };
            array<int8_t, 4> d;
            cnt &= 3;
            d[0] = data_[pat[cnt][0]];
            d[1] = data_[pat[cnt][1]];
            d[2] = data_[pat[cnt][2]];
            d[3] = data_[pat[cnt][3]];
            copy(d.begin(), d.end(), data_.begin());
        }

    };


    //


    // -yが重力方向
    class Field : public Matrix<int8_t, H, W> {
        //

    public:
        inline Field() :Matrix() { }


        // 最上位にPackを置く
        void insert(const Pack& pack, int x) {
            at(H - 1, x) = pack(0, 0);
            at(H - 2, x) = pack(1, 0);
            at(H - 1, x + 1) = pack(0, 1);
            at(H - 2, x + 1) = pack(1, 1);
        }


        // ブロックを落とす
        // @return success ? true : false
        bool fall() {
            repeat(x, W) {
                int i = 0;
                repeat(y, H) {
                    if (at(y, x) != None) {
                        while (i < H && at(i, x) != None) ++i;
                        // この時点で, iはNoneを指している
                        // i以下にNoneとなるブロックは存在しない
                        if (i >= H) return false;
                        // iより上にyがあるならば，yの位置のブロックをiに持っていく
                        if (i < y) std::swap(at(i, x), at(y, x));
                    }
                }
            }
        }


        // ブロックを消去する
        // @return score
        int eliminate() {
            int score = 0;
            Matrix<bool, H, W> flag;

            // y方向の消去
            repeat(x, W) {
                // しゃくとり法
                int l = 0;
                int8_t sum = 0;
                repeat(y, H) {
                    // 新たに範囲に追加
                    sum += at(y, x);
                    // 10以下になるまで範囲末尾を削る
                    while (sum > 10) {
                        sum -= at(l, x);
                        ++l;
                    }
                    // 10ならば，範囲を全て消去
                    if (sum == 10) {
                        while (l <= y) {
                            flag (l, x) = 1;
                            ++l;
                        }
                        sum = 0;
                    }
                }
            }
            // TODO:
            return 0;
        }


        // 5を爆発させる
        // @return score
        int explode() {

            // TODO:
            return 0;
        }

    };

}