
#include "Macro.h"
#include "Game.h"
#include "Input.h"
#include "Score.h"

namespace Game {

    // �R�}���h��K�����āC�t�B�[���h��ω�������D
    // �X�L���Q�[�W���͈�ؕω������Ȃ�
    // ��]�O�̃p�b�N��^����
    // @return: score, chain, sender, success
    tuple<int, int, int, bool> Player::apply(Command cmd, const Pack& turnPack) {

        int score = 0;
        int chain = 0;
        int sender = 0;
        bool success = true;

        array<bool, W> changedFlag; changedFlag.fill(true);

        if (cmd.skill()) {
            // ����
            int bcnt = field.explode();
            // �����X�R�A
            int sb = BombScore[bcnt];

            // �����E�`�F�C��
            tie(chain, success) = field.chain();

            // �X�L���`�F�C���X�R�A
            int ss = ChainSkillScore[chain];

            sender = sb / 2 + ss / 2;
            score = sb + ss;
        }
        else {
            // �p�b�N����
            success = field.insert(turnPack.rotated(cmd.rot()), cmd.xPos());

            // �`�F�C��
            tie(chain, success) = field.chain();

            // �`�F�C���X�R�A
            int ss = ChainScore[chain];

            sender = ss / 2;
            score = ss;
        }

        return make_tuple(score, chain, sender, success);
    }


    // �R�}���h��K�����āC���̏�Ԃɂ���D
    // ��]�O�̃p�b�N��^����
    // @return: success?
    bool Input::apply(Command cmdMe, Command cmdEn, const Pack& turnPack) {

        // ���ז��u���b�N�̓�������
        if (me.ojama >= 10) {
            me.field.stackOjama();
            me.ojama -= 10;
        }
        if (en.ojama >= 10) {
            en.field.stackOjama();
            en.ojama -= 10;
        }

        // �l���X�R�A
        int mescore = 0, enscore = 0;
        // �`�F�C����
        int mechain = 0, enchain = 0;
        // ���ז��u���b�N�𑗂��
        int mesender = 0, ensender = 0;
        // �l�܂Ȃ�����
        bool mesuccess = true, ensuccess = true;

        // �t�B�[���h�̕ω�
        tie(mescore, mechain, mesender, mesuccess) = me.apply(cmdMe, turnPack);
        tie(enscore, enchain, ensender, ensuccess) = en.apply(cmdEn, turnPack);

        // �X�L���Q�[�W�̏���
        if (cmdMe.skill()) {
            assert(me.skill >= 80);
            me.skill = 0;
        }
        if (cmdEn.skill()) {
            assert(en.skill >= 80);
            en.skill = 0;
        }

        // �X�L���Q�[�W�̕ω�
        if (mechain >= 1) me.skill += 8;
        if (enchain >= 1) en.skill += 8;
        chlim(me.skill, 0, 100);
        chlim(en.skill, 0, 100);
        if (mechain >= 3) en.skill -= 12 + 2 * mechain;
        if (enchain >= 3) me.skill -= 12 + 2 * enchain;
        chlim(me.skill, 0, 100);
        chlim(en.skill, 0, 100);

        // ���ז��̕ω�
        me.ojama += ensender;
        en.ojama += mesender;
        if (me.ojama > en.ojama)
            me.ojama -= en.ojama,
            en.ojama = 0;
        else
            en.ojama -= me.ojama,
            me.ojama = 0;

        // �^�[������
        turn += 1;

        return mesuccess && ensuccess && turn < N;
    }
}