
#include "Macro.h"
#include "Game.h"
#include "AI.h"
#include "Time.h"

using namespace Game;


//


struct SearchState {
    Field field;
    Command firstcmd;
    int score;
    int heuristic;
    
    inline bool operator<(const SearchState& ss) const noexcept {
        return heuristic < ss.heuristic;
    }

    inline void swap(SearchState& ss) {
        field.swap(ss.field);
        firstcmd.swap(ss.firstcmd);
        std::swap(score, ss.score);
        std::swap(heuristic, ss.heuristic);
    }
};


//


static decltype(FirstInput::packs) packs;


//


// field: 対象
// milestonePack: 発火対象
static inline int calcHeuristic(const Field& field, const Pack& milestonePack) {
    int best = 0;
    repeat(i, W - 1) {
        Field f = field;
        f.insert(milestonePack, i);
        chmax(best, ChainScore[f.chain()]);
    }
    return best;
}


//


void BattleAI::setup(const Game::FirstInput& fi) {
    packs = fi.packs;

}


Command BattleAI::loop(const Input& input, const Pack& turnPack) {

    // 先読み探索の深さ
    const int MaxDepth = 10;
    // ちょくだいさーち
    priority_queue<SearchState> stackedStates[MaxDepth + 1];

    const Pack& milestonePack = packs[(input.turn+15)/15*15];

    // 今のターンのコマンドを総当たり
    repeat(r, 4) {
        auto pack = packs[input.turn].rotated(r);
        repeat(x, W - 1) {
            Command cmd(x, r);
            SearchState ss{ input.me.field, cmd, 0 };
            ss.field.insert(pack, x);
            ss.score += ChainScore[ss.field.chain()];
            ss.heuristic += calcHeuristic(ss.field, milestonePack);
            stackedStates[0].push(move(ss));
        }
    }
    
    // 時間が許す限り探索する
    int loopcount = 0;
    for (auto timer = TIME; MILLISEC(TIME - timer) < 600; ){

        repeat(depth, MaxDepth) {
            if (stackedStates[depth].empty()) continue;
            const SearchState& currss = stackedStates[depth].top();

            repeat(r, 4) {
                auto pack = packs[input.turn].rotated(r);
                repeat(x, W - 1) {
                    SearchState ss = currss;

                    ss.field.insert(pack, x);
                    chmax(ss.score, ChainScore[ss.field.chain()]);
                    ss.heuristic += calcHeuristic(ss.field, milestonePack);

                    stackedStates[depth + 1].push(move(ss));
                }
            }

            stackedStates[depth].pop();
        }
        ++loopcount;
    }

    Tag<int, Command> best(-1, Command::Skill);

    repeat(depth, MaxDepth + 1) {
        repeat(_, 100) {
            if (stackedStates[depth].empty()) continue;
            const SearchState& ss = stackedStates[depth].top();
            chmax(best, Tag<int, Command>(ss.score, ss.firstcmd));
            stackedStates[depth].pop();
        }
    }

    clog << "loop:" << loopcount << ", best:" << best.first << "\n";

    return best.second;
}
