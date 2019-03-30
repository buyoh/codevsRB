#include "Macro.h"
#include "Game.h"



namespace Game {


    const Command Command::Skill = Command(1 << 6);

    constexpr ChainScoreT ChainScore;
    constexpr ChainSkillScoreT ChainSkillScore;
    constexpr BombScoreT BombScore;


    bool Field::fall() {
        bool ok = true;
        repeat(x, W) {
            int i = 0;
            repeat(y, H) {
                if (at(y, x) != None) {
                    while (i < H && at(i, x) != None) ++i;
                    // ���̎��_��, i��None���w���Ă���
                    // i�ȉ���None�ƂȂ�u���b�N�͑��݂��Ȃ�
                    // i�����y������Ȃ�΁Cy�̈ʒu�̃u���b�N��i�Ɏ����Ă���
                    if (i < y) std::swap(at(i, x), at(y, x));
                }
            }
            ok &= (i < HLimit);
        }
        return ok;
    }


    // ���[��
    // int Field::eliminate() {
    //     Matrix<int8_t, H, W> flag;
    // 
    //     // y�����̏���
    //     repeat(x, W) {
    //         // ���Ⴍ�Ƃ�@
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(y, H) {
    //             // ��̏ꍇ
    //             if (at(y, x) == 0) {
    //                 sum = 0;
    //                 l = y + 1;
    //                 continue;
    //             }
    //             // �V���ɔ͈͂ɒǉ�
    //             sum += at(y, x);
    //             // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
    //             while (sum > 10) {
    //                 sum -= at(l, x);
    //                 ++l;
    //             }
    //             // 10�Ȃ�΁C�͈͂�S�ď���
    //             if (sum == 10) {
    //                 while (l <= y) {
    //                     flag(l, x) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // x�����̏���
    //     repeat(y, H) {
    //         // ���Ⴍ�Ƃ�@
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(x, W) {
    //             // ��̏ꍇ
    //             if (at(y, x) == 0) {
    //                 sum = 0;
    //                 l = x + 1;
    //                 continue;
    //             }
    //             // �V���ɔ͈͂ɒǉ�
    //             sum += at(y, x);
    //             // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
    //             while (sum > 10) {
    //                 sum -= at(y, l);
    //                 ++l;
    //             }
    //             // 10�Ȃ�΁C�͈͂�S�ď���
    //             if (sum == 10) {
    //                 while (l <= x) {
    //                     flag(y, l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // �E��1
    //     iterate(u, 0, H - 1) {
    //         // ���Ⴍ�Ƃ�@
    //         int l = 0;
    //         int8_t sum = 0;
    //         int n = min(W, H - u);
    //         repeat(i, n) {
    //             // ��̏ꍇ
    //             if (at(u + i, i) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // �V���ɔ͈͂ɒǉ�
    //             sum += at(u + i, i);
    //             // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
    //             while (sum > 10) {
    //                 sum -= at(u + l, l);
    //                 ++l;
    //             }
    //             // 10�Ȃ�΁C�͈͂�S�ď���
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(u + l, l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // �E��2
    //     iterate(v, 1, W - 1) {
    //         // ���Ⴍ�Ƃ�@
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(i, W - v) {
    //             // ��̏ꍇ
    //             if (at(i, v + i) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // �V���ɔ͈͂ɒǉ�
    //             sum += at(i, v + i);
    //             // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
    //             while (sum > 10) {
    //                 sum -= at(l, v + l);
    //                 ++l;
    //             }
    //             // 10�Ȃ�΁C�͈͂�S�ď���
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(l, v + l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // �E��1
    //     iterate(u, 1, H) {
    //         // ���Ⴍ�Ƃ�@
    //         int l = 0;
    //         int8_t sum = 0;
    //         int n = min(W, u + 1);
    //         repeat(i, n) {
    //             // ��̏ꍇ
    //             if (at(u - i, l) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // �V���ɔ͈͂ɒǉ�
    //             sum += at(u-i, i);
    //             // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
    //             while (sum > 10) {
    //                 sum -= at(u-l, l);
    //                 ++l;
    //             }
    //             // 10�Ȃ�΁C�͈͂�S�ď���
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(u - l, l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    //     // �E��2
    //     iterate(v, 1, W-1) {
    //         // ���Ⴍ�Ƃ�@
    //         int l = 0;
    //         int8_t sum = 0;
    //         repeat(i, W - v) {
    //             // ��̏ꍇ
    //             if (at(H - 1 - i, v + i) == 0) {
    //                 sum = 0;
    //                 l = i + 1;
    //                 continue;
    //             }
    //             // �V���ɔ͈͂ɒǉ�
    //             sum += at(H - 1 - i, v + i);
    //             // 10�ȉ��ɂȂ�܂Ŕ͈͖��������
    //             while (sum > 10) {
    //                 sum -= at(H - 1 - l, v + l);
    //                 ++l;
    //             }
    //             // 10�Ȃ�΁C�͈͂�S�ď���
    //             if (sum == 10) {
    //                 while (l <= i) {
    //                     flag(H - 1 - l, v + l) = 1;
    //                     ++l;
    //                 }
    //                 sum = 0;
    //             }
    //         }
    //     }
    // 
    //     int cnt = 0;
    // 
    //     // �}�[�N�����_������
    //     repeat(x, W) {
    //         repeat(y, H) {
    //             if (flag(y, x)) at(y, x) = 0, ++cnt;
    //         }
    //     }
    // 
    // 
    //     return cnt;
    // }

    int Field::eliminate() {
        Matrix<int8_t, H, W> flag;
        // �������u���b�N���}�[�N����
        repeat(x, W) {
            repeat(y, H) {
                auto a = at(y, x);
                if (a == Ojama) continue;
                if (safeat(y, x + 1) + a == 10)
                    flag(y, x) = 1,
                    flag(y, x + 1) = 1;
                if (safeat(y + 1, x + 1) + a == 10)
                    flag(y, x) = 1,
                    flag(y + 1, x + 1) = 1;
                if (safeat(y + 1, x) + a == 10)
                    flag(y, x) = 1,
                    flag(y + 1, x) = 1;
                if (safeat(y + 1, x - 1) + a == 10)
                    flag(y, x) = 1,
                    flag(y + 1, x - 1) = 1;
            }
        }

        // �}�[�N�����_������
        int cnt = 0;
        repeat(x, W) {
            repeat(y, H) {
                if (flag(y, x) == 1) at(y, x) = 0, ++cnt;
            }
        }

        return cnt;
        return cnt;
    }


    // TODO:
    int Field::explode() {
        Matrix<int8_t, H, W> flag;

        // �������u���b�N���}�[�N����
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == Ojama || at(y, x) == 0 || at(y, x) == 5) continue;
                iterate(i, -1, 2) {
                    iterate(j, -1, 2) {
                        if (safeat(y + j, x + i) == 5) flag(y, x) = 1; 
                    }
                }
            }
        }

        // �}�[�N�����_������
        int cnt = 0;
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == 5 || flag(y, x) == 1) at(y, x) = 0, ++cnt;
            }
        }

        return cnt;
    }


    bool Field::stackOjama() {
        repeat(x, W) {
            repeat(y, H) {
                if (at(y, x) == 0) {
                    if (y >= HLimit) return true;
                    at(y, x) = Ojama;
                    break;
                }
            }
        }
        return false;
    }

    // �R�}���h��K�����āC�t�B�[���h��ω�������D
    // �X�L���Q�[�W���͈�ؕω������Ȃ�
    // ��]�O�̃p�b�N��^����
    // @return: score, chain, sender, success
    tuple<int, int, int, bool> Player::apply(Command cmd, const Pack& turnPack) {

        int score = 0;
        int chain = 0;
        int sender = 0;
        bool success = true;

        if (cmd.skill()) {
            // ����
            int bcnt = field.explode();
            success = field.fall();
            // �����X�R�A
            int sb = BombScore[bcnt];

            // �`�F�C��
            while (field.eliminate() > 0) {
                ++chain;
                success = field.fall();
            }
            // �X�L���`�F�C���X�R�A
            int ss = ChainSkillScore[chain];

            sender = sb / 2 + ss / 2;
            score = sb + ss;
        }
        else {
            // �p�b�N����
            field.insert(turnPack.rotated(cmd.rot()), cmd.xPos());
            success = field.fall();

            // �`�F�C��
            while (field.eliminate() > 0) {
                ++chain;
                success = field.fall();
            }
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
        turnCount += 1;

        return mesuccess && ensuccess && turnCount < N;

    }
}