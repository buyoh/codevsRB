#pragma once
#include "Macro.h"
#include "Util.h"

namespace Game {


    // �萔
    // ===========================


    const int N = 500; // gameturn count;
    const int H = 19; // field height
    const int HLimit = 16; // field height dead line
    const int W = 10; // field width
    // const int L = 2; // pack width

    const int8_t Ojama = 11; // ���ז��u���b�N�̐F
    const int8_t None = 0; // �󋕂̐F



    // ���ۍ\����
    // ===========================



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
        inline Matrix() :data_(Height*Width) { }
        inline Matrix(container_type&& d) : data_(move(d)) { data_.resize(Height*Width); }
        inline Matrix(initializer_list<T>&& d) : data_(move(d)) { data_.resize(Height*Width); }

        // �͈͓��Ȃ�true
        constexpr bool safe(int y, int x) const noexcept { return 0 <= y && y < Height && 0 <= x && x < Width; }

        inline T& at(int y, int x) noexcept { assert(safe(y, x)); return data_[y + x * Height]; }
        inline T at(int y, int x) const noexcept { assert(safe(y, x)); return data_[y + x * Height]; }
        inline T operator()(int y, int x) const noexcept { return data_[y + x*Height]; }
        inline T operator()(P p) const noexcept { return data_[p.y + p.x*Height]; }
        inline T& operator()(int y, int x) noexcept { return data_[y + x * Height]; }
        inline T& operator()(P p) noexcept { return data_[p.y + p.x*Height]; }

        // �͈͓��Ȃ�l�C�����łȂ��Ȃ�-1
        inline T safeat(int y, int x) const noexcept { return safe(y,x) ? at(y, x) : -1; }

        inline void swap(Matrix& f) {
            data_.swap(f.data_);
        }

        inline void fill(const T& val) {
            std::fill(data_.begin(), data_.end(), val);
        }

        inline bool operator==(const container_type& d) const {
            return data_ == d;
        }
        inline bool operator==(const Matrix<T, Height, Width>& m) const {
            return data_ == m.data_;
        }

    };



    // �\����
    // ===========================



    // y���d�͕����DField�ƈقȂ��
    class Pack : public Matrix<int8_t, 2, 2> {

    public:
        inline Pack() :Matrix() { }
        inline Pack(vector<int8_t>&& d) :Matrix(move(d)) { }
        inline Pack(initializer_list<int8_t>&& d) : Matrix(move(d)) { data_.resize(2*2); }

        //

        // �E��]��cnt�񐔍s�����p�b�N��Ԃ�
        inline Pack rotated(int cnt) const {
            const static int pat[][4] = { {0,1,2,3},{1,3,0,2},{3,2,1,0},{2,0,3,1} };
            vector<int8_t> d(4);
            cnt &= 3;
            d[0] = data_[pat[cnt][0]];
            d[1] = data_[pat[cnt][1]];
            d[2] = data_[pat[cnt][2]];
            d[3] = data_[pat[cnt][3]];
            return Pack(move(d));
        }

        // �E��]��cnt�񐔍s��
        inline void rotate(int cnt) {
            const static int pat[][3] = { {0, 1, 2},{1,3,3} ,{3,2, 2},{2, 2, 3} };
            cnt &= 3;
            std::swap(data_[0], data_[pat[cnt][0]]);
            std::swap(data_[1], data_[pat[cnt][1]]);
            std::swap(data_[2], data_[pat[cnt][2]]);
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
        bool fall();

        // �u���b�N����������
        // @return num of destroyed
        int eliminate();

        // 5�𔚔�������
        // @return num of destroyed
        int explode();

        // ���ז��u���b�N��1�i�~�点��
        // ���O��fall����Ă��邱��
        // overflow�������CField�̏�Ԃ͕s��
        // @return overflowed?
        bool stackOjama();


        inline bool operator==(const Field& f) const {
            return data_ == f.data_;
        }
    };


    //


    // �R�}���h
    class Command {
        // [0..3] xPos, [4..5] rot, [6] useSkill
        int8_t data_;

    public:

        constexpr Command(int8_t _xPos, int8_t _rot):
            data_((_xPos&15) | ((_rot&3) << 4)) {}
        constexpr Command(int8_t _raw) :
            data_(_raw) {}
        constexpr Command() : data_(0) {}

        constexpr int8_t xPos() const noexcept { return data_ & 15; }
        // �����E��]
        constexpr int8_t rot() const noexcept { return (data_ >> 4) & 3; }
        constexpr bool skill() const noexcept { return (data_ >> 6); }

        static const Command Skill;
    };


    //
    // Score
    // ===========================


    class ChainScoreT {
        static constexpr int Limit = 50;
        int data_[Limit];
    public:
        constexpr ChainScoreT() :data_() {
            for (int i = 1; i < Limit; ++i)
                data_[i] = data_[i-1] + static_cast<int>(cpow(1.3, double(i)));
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
                data_[i] = static_cast<int>(25.0*cpow(2.0, double(i)/12.0));
        }
        constexpr int operator[](int i) const { return data_[i]; }
    };

    // �Ȃ���킩���
    // extern constexpr ChainScoreT chainScore;
    // extern constexpr BombScoreT bombScore;



    // operator >> <<
    // ===========================


    template<typename T, int Height, int Width>
    istream& operator >>(istream& is, Matrix<T, Height, Width>& mat) {
        for (int i = 0; i < Height; ++i) {
            for (int j = 0; j < Width; ++j) {
                int x; is >> x;
                mat(i, j) = x;
            }
        }
        string str; is >> str;
        assert(str == "END");
        return is;
    }

    template<typename T, int Height, int Width>
    ostream& operator <<(ostream& os, Matrix<T, Height, Width>& mat) {
        for (int i = 0; i < Height; ++i) {
            for (int j = 0; j < Width; ++j) {
                os << setw(2) << (int)mat(i, j);
            }
            os << '\n';
        }
        return os;
    }

    inline ostream& operator <<(ostream& os, Command cmd) {
        if (cmd.skill()) os << 'S';
        else os << (int)cmd.xPos() << ' ' << (int)cmd.rot();
        return os;
    }

    inline istream& operator >>(istream& is, Command& cmd) {
        string c; is >> c;
        assert(c.size() == 1);
        if (c[0] == 'S') cmd = Command::Skill;
        else {
            int p; is >> p;
            cmd = Command(c[0] - '0', p);
        }
        return is;
    }


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
            is >> timeMsec >> ojama >> skill;
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
        int turnCount;
        Player me; // �킽����
        Player en; // ������

        template<typename istream>
        void input(istream& is) {
            is >> turnCount;
            if (is.eof()) return;
            me.input(is);
            en.input(is);
        }

        bool apply(Command, Command, const Pack&);
    };

}