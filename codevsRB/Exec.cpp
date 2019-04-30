#include "Macro.h"
#include "Game.h"
#include "Input.h"
#include "AI.h"
#include "CaseGenerator.h"

#include "Exec.h"


// global
ExecOptions execOptions;


// fin，foutを通じて実行する
// fin  : stdin
// fout : stdout
void execAI(istream& fin, ostream& fout) {

    BattleAI ai;

    fout << ai.aiName() << endl;

    Game::FirstInput fi; fi.input(fin);
    ai.setup(fi);

	for (int cnt = 0; ; ++cnt) {
		Game::Input i;
		atomic_bool loaded = false;
		auto t2 = thread([cnt, &ai, &loaded]() { ai.background(cnt, loaded); });
		i.input(fin); loaded = true;
		t2.join();
         
        auto cmd = ai.loop(i, fi.packs[i.turn]);

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
        cerr << "turn: " << (input.turn) << endl;
        cerr << "last mycmd (rotated)" << endl;
        dumpcmd(mycmd, firstInput.packs[localinput.turn - 1]);
        cerr << "last encmd (rotated)" << endl;
        dumpcmd(encmd, firstInput.packs[localinput.turn - 1]);
        cerr << "PREVFIELD(me): " << endl;
        dumpfield(prev.me.field);
        cerr << "ACTUAL(me): " << endl;
        dumpfield(localinput.me.field);
        cerr << "EXPECTED(me): " << endl;
        dumpfield(input.me.field);
    };

    auto dumpen = [&]() {
        cerr << "turn: " << (input.turn) << endl;
        cerr << "last mycmd (rotated)" << endl;
        dumpcmd(mycmd, firstInput.packs[localinput.turn - 1]);
        cerr << "last encmd (rotated)" << endl;
        dumpcmd(encmd, firstInput.packs[localinput.turn - 1]);
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
            if (input.turn != localinput.turn) {
                // EXIT
                break;
            }
            if (!(input.me.field == localinput.me.field)) {
                dumpme();
                prev.apply(mycmd, encmd, firstInput.packs[prev.turn]);
                abort();
            }
            if (!(input.en.field == localinput.en.field)) {
                dumpen();
                abort();
            }
            assert(input.me.ojama == localinput.me.ojama);
            assert(input.me.skill == localinput.me.skill);
			assert(input.me.score == localinput.me.score);
            assert(input.en.ojama == localinput.en.ojama);
            assert(input.en.skill == localinput.en.skill);
			assert(input.en.score == localinput.en.score);
        }
        else {
            first = false;
        }

        fme >> mycmd;
        fen >> encmd;

        localinput = input;
        prev = input;
        localinput.apply(mycmd, encmd, firstInput.packs[localinput.turn]);

    }

    cout << "success" << endl;
}



// 入力を自動生成して実行
// 敵AIなし，敵フィールドは常にリセット
// 実行時間確認など
void selfExecution() {

    BattleAI ai;

    Game::FirstInput firstinput;
    Generator::generateFirstInput(firstinput);
    ai.setup(firstinput);

    Game::Input input; Generator::setDefaultInput(input);

    repeat(turn, 150) {
        auto cmd = ai.loop(input, firstinput.packs[turn]);
        input.apply(cmd, Game::Command(0, 0), firstinput.packs[turn]);

        Generator::setDefaultPlayer(input.en);
    }
}
