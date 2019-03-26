#pragma once
#include "Macro.h"
#include "Util.h"

namespace Game {

    //

    const int H = 19; // field height
    const int HLimit = 17; // field height Limit
    const int W = 10; // field width
    const int L = 2; // pack width

    const int8_t Ojama = 11; // ���ז��u���b�N�̐F
    const int8_t None = 0; // �󋕂̐F


    //


    // Field, Pack�̒��ۃN���X
    // ��������ɂ�y�������ɐ��񂳂�Ă���_���ʏ�ƈقȂ�D
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

        // ����]��cnt�񐔍s�����p�b�N��Ԃ�
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

        // ����]��cnt�񐔍s��
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


    // -y���d�͕���
    class Field : public Matrix<int8_t, H, W> {
        //

    public:
        inline Field() :Matrix() { }


        // �ŏ�ʂ�Pack��u��
        void insert(const Pack& pack, int x) {
            at(H - 1, x) = pack(0, 0);
            at(H - 2, x) = pack(1, 0);
            at(H - 1, x + 1) = pack(0, 1);
            at(H - 2, x + 1) = pack(1, 1);
        }


        // �u���b�N�𗎂Ƃ�
        // @return success ? true : false
        bool fall() {
            repeat(x, W) {
                int i = 0;
                repeat(y, H) {
                    if (at(y, x) != None) {
                        while (i < H && at(i, x) != None) ++i;
                        // ���̎��_��, i��None���w���Ă���
                        // i�ȉ���None�ƂȂ�u���b�N�͑��݂��Ȃ�
                        if (i >= H) return false;
                        // i�����y������Ȃ�΁Cy�̈ʒu�̃u���b�N��i�Ɏ����Ă���
                        if (i < y) std::swap(at(i, x), at(y, x));
                    }
                }
            }
        }


        // �u���b�N����������
        // @return score
        int eliminate() {
            int score = 0;
            Matrix<bool, H, W> flag;

            // y�����̏���
            repeat(x, W) {
                // ���Ⴍ�Ƃ�@
                int l = 0;
                int8_t sum = 0;
                repeat(y, H) {
                    // �V���ɔ͈͂ɒǉ�
                    sum += at(y, x);
                    // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
                    while (sum > 10) {
                        sum -= at(l, x);
                        ++l;
                    }
                    // 10�Ȃ�΁C�͈͂�S�ď���
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


        // 5�𔚔�������
        // @return score
        int explode() {

            // TODO:
            return 0;
        }

    };

}