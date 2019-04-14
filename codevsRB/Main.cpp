#include "Macro.h"
#include "Exec.h"

//


void exit_help() {
    cout <<
        "usage: ./game [options]" "\n"
        "���[�h�I��" "\n"
        " --assertion : assertion." "\n"
        "             : --stdin, --stdout, --stdout2 ���K�v." "\n"
        " --self      : �����_���ȓ��͂�����Ő����C���s�D���o�͂Ȃ��D�x���`�}�[�N�p" "\n"
        "AI�I�v�V����" "\n"
        " --shuffle   : 1�^�[���ڂ������_����ɂ���" "\n"
        "����" "\n"
        " --stdin   <filename> : assertion �p. stdin �̑���� filename ���g�p." "\n"
        " --stdout  <filename> : assertion �p. stdout �̑���� filename ���g�p." "\n"
        " --stdout2 <filename> : assertion �p. ����v���C���[�̕W���o�͂� filename ���w��." "\n"
        "���̑�" "\n"
        " --help (-h) : �����\��" "\n";

    cout << endl;
    exit(0);
}


void exit_invalidOption() {
    cout <<
        "error: invalid option.";
    cout << endl;
    abort();
}


//


//


int main(int argc, char** argv) {

    int mode = 0;
    char* filename_cin = nullptr;
    char* filename_cout1 = nullptr;
    char* filename_cout2 = nullptr;

    for (int p = 1; p < argc; ++p) {

        if (argv[p][0] == '-' && argv[p][1] == '-') {
            // long flag

            if (strcmp(argv[p], "--help") == 0) {
                exit_help();
            }
            else if (strcmp(argv[p], "--assertion") == 0) {
                if (mode != 0) exit_invalidOption();
                mode = 'a';
            }
            else if (strcmp(argv[p], "--self") == 0) {
                if (mode != 0) exit_invalidOption();
                mode = 's';
            }
            else if (strcmp(argv[p], "--stdin") == 0) {
                if (p + 1 >= argc) exit_invalidOption();
                filename_cin = argv[++p];
            }
            else if (strcmp(argv[p], "--stdout") == 0) {
                if (p + 1 >= argc) exit_invalidOption();
                filename_cout1 = argv[++p];
            }
            else if (strcmp(argv[p], "--stdout2") == 0) {
                if (p + 1 >= argc) exit_invalidOption();
                filename_cout2 = argv[++p];
            }
            else if (strcmp(argv[p], "--shuffle") == 0) {
                execOptions.shuffleFirstCommand = true;
            }
        }
        else if (argv[p][0] == '-') {
            // short flag

            if (argv[p][1] == 'h') {
                exit_help();
            }

        }

    }

    switch (mode)
    {
    case 0: {
        // �ʏ�AI���[�h
        cin.tie(nullptr);

        execAI(cin, cout);
        break;
    }
    case 'a': {
        // validate���[�h
        if (filename_cin == nullptr ||
            filename_cout1 == nullptr ||
            filename_cout2 == nullptr) exit_invalidOption();
        ifstream fi(filename_cin);
        ifstream fo1(filename_cout1);
        ifstream fo2(filename_cout2);

        if (fi.fail() || fo1.fail() || fo2.fail()) {
            cout << "error: not found" << endl;
            abort();
        }

        validateSimulator(fi, fo1, fo2);
        break;
    }
    case 's': {
        // self���[�h

        selfExecution();
        break;
    }
    default:
        break;
    }

    return 0;
}