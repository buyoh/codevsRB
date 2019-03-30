#include "Macro.h"
#include "Game.h"
#include "AI.h"

#include "MainAI.h"


// fin，foutを通じて実行する
// fin  : stdin
// fout : stdout
void execAI(istream& fin, ostream& fout) {

    BattleAI ai;

    fout << ai.aiName() << endl;

    Game::FirstInput fi; fi.input(fin);
    ai.setup(fi);

    while (true) {
        Game::Input i; i.input(fin);
        auto cmd = ai.loop(i, fi.packs[i.turnCount]);

        fout << cmd << endl;
    }

}


// シミュレータの動作確認
// fin : 自分への入力データを指すfd
// fme : 自分の出力結果ファイルのfd
// fen : 相手の出力結果ファイルのfd
void validateSimulator(istream& fin, istream& fme, istream& fen) {
    using namespace Game;

    // AI NAME
    // string myName, yourName;
    // fme >> myName;
    // fen >> yourName;

    // first input (packs)
    FirstInput firstInput;
    firstInput.input(fin);

    bool first = true;

    Command mycmd, encmd;
    Input input, localinput, prev;

    auto dumpcmd = [](const Command& cmd, const Pack& unrotatedPack) {
        if (cmd.skill()) {
            cerr << "skill" << endl;
        }
        else {
            Pack p = unrotatedPack.rotated(cmd.rot());
            repeat(i, 2) {
                repeat(j, 2) {
                    cerr << (int)p(i, j);
                }
                cerr << endl;
            }
            cerr << "to: " << (int)cmd.xPos() << endl;
        }
    };

    auto dumpfield = [](const Field& field) {
        rrepeat(i, H){
            repeat(j, W){
                cerr << setw(2) << (int)field(i, j);
            }
            cerr << '\n';
        }
    };

    auto dumpme = [&]() {
        cerr << "turn: " << (input.turnCount) << endl;
        cerr << "last mycmd (rotated)" << endl;
        dumpcmd(mycmd, firstInput.packs[localinput.turnCount - 1]);
        cerr << "last encmd (rotated)" << endl;
        dumpcmd(encmd, firstInput.packs[localinput.turnCount - 1]);
        cerr << "PREVFIELD(me): " << endl;
        dumpfield(prev.me.field);
        cerr << "ACTUAL(me): " << endl;
        dumpfield(localinput.me.field);
        cerr << "EXPECTED(me): " << endl;
        dumpfield(input.me.field);
    };

    auto dumpen = [&]() {
        cerr << "turn: " << (input.turnCount) << endl;
        cerr << "last mycmd (rotated)" << endl;
        dumpcmd(mycmd, firstInput.packs[localinput.turnCount - 1]);
        cerr << "last encmd (rotated)" << endl;
        dumpcmd(encmd, firstInput.packs[localinput.turnCount - 1]);
        cerr << "PREVFIELD(en): " << endl;
        dumpfield(prev.en.field);
        cerr << "ACTUAL(en): " << endl;
        dumpfield(localinput.en.field);
        cerr << "EXPECTED(en): " << endl;
        dumpfield(input.en.field);
    };

    while (true) {
        // turn input
        input.input(fin);

        if (!first) {
            if (input.turnCount != localinput.turnCount) {
                // EXIT
                break;
            }
            if (!(input.me.field == localinput.me.field)) {
                dumpme();
                prev.apply(mycmd, encmd, firstInput.packs[prev.turnCount]);
                abort();
            }
            if (!(input.en.field == localinput.en.field)) {
                dumpen();
                abort();
            }
            assert(input.me.ojama == localinput.me.ojama);
            assert(input.me.skill == localinput.me.skill);
            assert(input.en.ojama == localinput.en.ojama);
            assert(input.en.skill == localinput.en.skill);
        }
        else {
            first = false;
        }

        fme >> mycmd;
        fen >> encmd;

        localinput = input;
        prev = input;
        localinput.apply(mycmd, encmd, firstInput.packs[localinput.turnCount]);

    }

    cout << "success" << endl;
}