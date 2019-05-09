#pragma once

#include "Macro.h"
#include "Game.h"

// AI用途のGameの拡張

namespace Game {

    class PField {
        // instace
        // rootなら必ず存在．高速化のため，葉も持つ
        unique_ptr<Field> instance_;
        // 親．nullならばroot
        shared_ptr<PField> parent_;
        // 変更箇所
        vector<pair<P, int8_t>> changed_;

        vector<pair<P, int8_t>> diff(const Field& f, const Field& g) {
            vector<pair<P, int8_t>> new_changed;
            repeat(x, W) {
                repeat(y, H) {
                    if (f(y, x) != g(y, x)) new_changed.emplace_back(P(y, x), f(y, x));
                }
            }
            new_changed.shrink_to_fit();
            return new_changed;
        }
    public:

        // FieldクラスからPartialFieldを作る
        // make_sharedのためだけにpublic
        inline PField(const Field& f)
            :instance_(make_unique<Field>(f)) { }
        // pを親としたpartialfieldを作成
        // make_sharedのためだけにpublic
        inline PField(shared_ptr<PField>& p, vector<pair<P, int8_t>>&& c)
            : parent_(p), changed_(c) {
            if (p->parent_) // 親の親が存在するならば，親のインスタンスを消去
                p->instance_.release();
        }

        // Fieldを生成する
        Field generate() const {
            if (!!instance_) return *instance_;
            Field f = parent_->generate();
            for (auto cmd : changed_)
                f(cmd.first.y, cmd.first.x) = cmd.second;
            return move(f);
        }
        // PartialFieldとFieldの差分であるような新たなPartialFieldを作る
        static inline shared_ptr<PField> append(shared_ptr<PField>& parent, const Field& f) {
            return make_shared<PField>(parent, parent->diff(f, parent->generate()));
        }
        // PartialFieldとFieldの差分であるような新たなPartialFieldを作る
        // ただし，selfはgであることがわかっている
        static inline shared_ptr<PField> append_hint(shared_ptr<PField>& parent, const Field& f, const Field& g) {
            return make_shared<PField>(parent, parent->diff(f, g));
        }

        static inline shared_ptr<PField> create(const Field& f) {
            return make_shared<PField>(f);
        }
    };


    // compressed field
    class CField {
        array<uint64_t, W> data_;
        CField(const Field& field) :data_() {
            repeat(x, W) {
                uint64_t v = 0;
                repeat(y, 16) 
                    v |= ((uint64_t)field(y, x) & (uint64_t)15) << (uint64_t)(y*4);
                data_[x] = v;
            }
        }
        void putField(Field& field) const {
            repeat(x, W) {
                repeat(y, 16) {
                    field(y, x) |= ((uint64_t)(y, x) >> (uint64_t)(y * 4)) & (uint64_t)15;
                }
                iterate(y, 16, H) field(y, x) = 0;
            }
        }
        inline Field&& toField() const {
            Field f;
            putField(f);
            return move(f);
        }
    };
    

}
       