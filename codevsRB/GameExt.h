#pragma once

#include "Macro.h"
#include "Game.h"

// AI用途のGameの拡張

namespace Game {

    class PField {
        unique_ptr<Field> instance_;
        shared_ptr<PField> parent_;
        vector<pair<P, int8_t>> changed_;
    public:

        // FieldクラスからPartialFieldを作る
        inline PField(const Field& f)
            :instance_(make_unique<Field>(f)) { }
        inline PField(shared_ptr<PField>& p, vector<pair<P, int8_t>>&& c)
            : parent_(p), changed_(c) { }

        // Fieldを生成する
        Field generate() const {
            if (instance_) return *instance_;
            Field f = parent_->generate();
            for (auto cmd : changed_)
                f(cmd.first.y, cmd.first.x) = cmd.second;
            return move(f);
        }
        // PartialFieldとFieldの差分であるような新たなPartialFieldを作る
        inline shared_ptr<PField> diff(const Field& f) {
            vector<pair<P, int8_t>> new_changed;
            Field g = generate();
            repeat(y, H) {
                repeat(x, W) {
                    if (f(y, x) != g(y, x)) new_changed.emplace_back(P(y, x), f(y, x)); // TODO: あんまり効率的ではない気がする
                }
            }
            new_changed.shrink_to_fit();
            return make_shared<PField>(f);
        }
    };
}
