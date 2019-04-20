
#include "Macro.h"
#include "Game.h"
#include "Input.h"
#include "Score.h"

namespace Game {

    // コマンドを適応して，フィールドを変化させる．
    // スキルゲージ等は一切変化させない
    // 回転前のパックを与える
    // @return: score, chain, sender, success
    tuple<int, int, int, bool> Player::apply(Command cmd, const Pack& turnPack) {

        int score = 0;
        int chain = 0;
        int sender = 0;
        bool success = true;

        array<bool, W> changedFlag; changedFlag.fill(true);

        if (cmd.skill()) {
            // 爆発
            int bcnt = field.explode();
            // 爆発スコア
            int sb = BombScore[bcnt];

            // 落下・チェイン
            tie(chain, success) = field.chain();

            // スキルチェインスコア
            int ss = ChainSkillScore[chain];

            sender = sb / 2 + ss / 2;
            score = sb + ss;
        }
        else {
            // パック投下
            success = field.insert(turnPack.rotated(cmd.rot()), cmd.xPos());

            // チェイン
            tie(chain, success) = field.chain();

            // チェインスコア
            int ss = ChainScore[chain];

            sender = ss / 2;
            score = ss;
        }

        return make_tuple(score, chain, sender, success);
    }


    // コマンドを適応して，次の状態にする．
    // 回転前のパックを与える
    // @return: success?
    bool Input::apply(Command cmdMe, Command cmdEn, const Pack& turnPack) {

        // お邪魔ブロックの投下処理
        if (me.ojama >= 10) {
            me.field.stackOjama();
            me.ojama -= 10;
        }
        if (en.ojama >= 10) {
            en.field.stackOjama();
            en.ojama -= 10;
        }

        // 獲得スコア
        int mescore = 0, enscore = 0;
        // チェイン数
        int mechain = 0, enchain = 0;
        // お邪魔ブロックを送る個数
        int mesender = 0, ensender = 0;
        // 詰まなかった
        bool mesuccess = true, ensuccess = true;

        // フィールドの変化
        tie(mescore, mechain, mesender, mesuccess) = me.apply(cmdMe, turnPack);
        tie(enscore, enchain, ensender, ensuccess) = en.apply(cmdEn, turnPack);

        // スキルゲージの消耗
        if (cmdMe.skill()) {
            assert(me.skill >= 80);
            me.skill = 0;
        }
        if (cmdEn.skill()) {
            assert(en.skill >= 80);
            en.skill = 0;
        }

        // スキルゲージの変化
        if (mechain >= 1) me.skill += 8;
        if (enchain >= 1) en.skill += 8;
        chlim(me.skill, 0, 100);
        chlim(en.skill, 0, 100);
        if (mechain >= 3) en.skill -= 12 + 2 * mechain;
        if (enchain >= 3) me.skill -= 12 + 2 * enchain;
        chlim(me.skill, 0, 100);
        chlim(en.skill, 0, 100);

        // お邪魔の変化
        me.ojama += ensender;
        en.ojama += mesender;
        if (me.ojama > en.ojama)
            me.ojama -= en.ojama,
            en.ojama = 0;
        else
            en.ojama -= me.ojama,
            me.ojama = 0;

        // ターン増加
        turn += 1;

		// スコア加算
		me.score += mescore;
		en.score += enscore;

        return mesuccess && ensuccess && turn < N;
    }
}