
#include "Macro.h"
#include "Game.h"
#include "Input.h"
#include "AI.h"
#include "Time.h"
#include "Score.h"

#include "Exec.h"

using namespace Game;


//


struct SearchState {
    Field field;
    vector<Command> commands;
    int score;
    int heuristic;
    
    inline bool operator<(const SearchState& ss) const noexcept {
        return heuristic < ss.heuristic;
    }

    inline void swap(SearchState& ss) {
        field.swap(ss.field);
        commands.swap(ss.commands);
        std::swap(score, ss.score);
        std::swap(heuristic, ss.heuristic);
    }
};


//


static decltype(FirstInput::packs) packs;


//


// ojamaが積もったか？
// 予めfallされていること
static bool checkStackedOjama(const Field& field) {
    repeat(x, W){
        bool lasto = false; // 直前がojamaだった
        repeat(y, H) {
            if (field(y, x) == None) { // 空虚に到達
                if (!lasto) return false; // 直前がojamaでないならば，積もっていない
                break;
            }
        }
    }
    return true;
}


//


// field: 対象
// milestonePack: 発火対象
static inline int calcHeuristic(const Field& field, int milestonePackIndexBegin, int milestonePackIndexEnd) {
    int best = 0;
    iterate(mpi, milestonePackIndexBegin, milestonePackIndexEnd){
        repeat(r, 4) {
            auto pack = packs[mpi].rotated(r);
            repeat(i, W / 2 - 1) {
                Field f = field;
                f.insert(pack, i*2);
                chmax(best, ChainScore[f.chain().first]);
            }
        }
    }
    return best;
}


// 
static vector<Command> solveSequence(const Input& input) {

    cerr << "solve: " << input.turn << " ";

    // 先読み探索の深さ
    const int MaxDepth = 14;
    // ちょくだいさーち
    priority_queue<SearchState> stackedStates[MaxDepth + 1];

    const int milestoneIdxBegin = input.turn + MaxDepth - 6;
    const int milestoneIdxEnd = input.turn + MaxDepth;

    // 今のターンのコマンドを総当たり
    repeat(r, 4) {
        auto pack = packs[input.turn].rotated(r);
        repeat(x, W - 1) {
            Command cmd(x, r);
            SearchState ss{ input.me.field, vector<Command>{cmd}, 0 };
            ss.field.insert(pack, x);
            ss.score = ChainScore[ss.field.chain().first];
            ss.heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
            stackedStates[0].push(move(ss));
        }
    }

    Tag<int, vector<Command>> best(-1, vector<Command>());

    // 時間が許す限り探索する
    int loopcount = 0;
    static int totaloppcount = 0;
    for (auto timer = TIME; MILLISEC(TIME - timer) < 4000; ) {

        repeat(depth, MaxDepth) {
            if (stackedStates[depth].empty()) continue;
            const SearchState& currss = stackedStates[depth].top();

            repeat(r, 4) {
                auto pack = packs[input.turn+depth+1].rotated(r);
                repeat(x, W - 1) {
                    SearchState ss = currss;

                    ss.field.insert(pack, x);
                    int chainscore = ChainScore[ss.field.chain().first];
                    if (!ss.field.fall()) continue;
                    int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
                    ss.commands.push_back(Command(x, r));
                    // chmax(ss.score, cs);
                    ss.heuristic += heuristic;
                    chmax(best, decltype(best)(chainscore, ss.commands));

                    stackedStates[depth + 1].push(move(ss));
                }
            }

            stackedStates[depth].pop();
        }
        ++loopcount;
    }

    clog << "loop:" << loopcount << ", best:" << best.first << "\n";
    clog << "totaloop: " << (totaloppcount += loopcount) << "\n";
    repeat(depth, MaxDepth + 1) {
        clog << stackedStates[depth].size() << endl;
    }


    return best.second;
}


//


void BattleAI::setup(const Game::FirstInput& fi) {
    packs = fi.packs;

}


Command BattleAI::loop(const Input& input, const Pack& turnPack) {

    if (input.turn == 0) {
        if (execOptions.shuffleFirstCommand) {
            random_device r;
            return Command((uniform_int_distribution<int>(0, W - 1))(r), 0);
        }
    }

    static vector<Command> pool;
    if (pool.empty() || checkStackedOjama(input.me.field)) {
        pool = solveSequence(input);
        reverse(ALL(pool));
    }
    if (!pool.empty()) {
        auto c = pool.back();
        pool.pop_back();
        return c;
    }

    return Command::Skill;
}
