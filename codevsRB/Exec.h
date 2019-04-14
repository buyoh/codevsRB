#pragma once

struct ExecOptions {
    bool shuffleFirstCommand;
};
extern ExecOptions execOptions;

// fin�Cfout��ʂ��Ď��s����
// fin  : stdin
// fout : stdout
void execAI(istream& fin, ostream& fout);

// �V�~�����[�^�̓���m�F
// fin : �����ւ̓��̓f�[�^���w��fd
// fme : �����̏o�͌��ʃt�@�C����fd
// fen : ����̏o�͌��ʃt�@�C����fd
void validateSimulator(istream& fin, istream& fme, istream& fen);

// ���͂������������Ď��s
// �GAI�Ȃ��C�G�t�B�[���h�͏�Ƀ��Z�b�g
// ���s���Ԋm�F�Ȃ�
void selfExecution();
