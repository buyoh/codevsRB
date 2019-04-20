
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
    // int skill;
    
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

// ticket #14 1つの数字ブロックを落としてスコア計算？
// 試験実装
static inline int calcHeuristic_one(const Field& field, int milestonePackIndexBegin, int milestonePackIndexEnd) {
    int best = 0;
    repeat(x, W) {
        int y1 = 0;
        while (y1 < H - 1 && field(y1, x) != None) ++y1;
        for (int8_t v = 1; v <= 9; v += 1) {
            Field f = field;
            f(y1, x) = v;
            chmax(best, ChainScore[f.chain().first]);
        }
    }
    return best;
}


// 
static vector<Command> solveSequence(const Input& input, const int stackedOjama) {

    cerr << "solve: " << input.turn << " ";

    // 先読み探索の深さ
    const int MaxDepth = 12;
    // ちょくだいさーち
    priority_queue<SearchState> stackedStates[MaxDepth + 1];

    const int milestoneIdxBegin = input.turn + MaxDepth - 3;
    const int milestoneIdxEnd = input.turn + MaxDepth;

	Tag<int, vector<Command>> best(-1, vector<Command>());

	// firstTurn
	{
		Field field = input.me.field;
		if (stackedOjama > 0) field.stackOjama();

		// 今のターンのコマンドを総当たり
		repeat(r, 4) {
			auto pack = packs[input.turn].rotated(r);
			repeat(x, W - 1) {
				Command cmd(x, r);
				SearchState ss{ field, vector<Command>{cmd}, 0, 0 };
				if (!ss.field.insert(pack, x)) continue;
				int score = ChainScore[ss.field.chain().first];
				int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
				ss.score = score;
				ss.heuristic = heuristic;

				chmax(best, decltype(best)(score, ss.commands));

				stackedStates[0].push(move(ss));
			}
		}
		// スキルが発動可能ならば入れる．
		// スキルによって積み上がることは無いので，連鎖作成には使えない．よって1手目のみ考慮する．
		if (input.me.skillable()) {
			SearchState ss{ field, vector<Command>{Command::Skill}, 0, 0 };

			int bombcnt = ss.field.explode();
			int skillscore = BombScore[bombcnt] + ChainScore[ss.field.chain().first];
			int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
			ss.score = skillscore;
			ss.heuristic += heuristic;

			chmax(best, decltype(best)(skillscore, ss.commands));

			stackedStates[0].push(move(ss));
		}
	}

    // 時間が許す限り探索する
    int loopcount = 0;
    static int totaloppcount = 0;
    for (auto timer = TIME; MILLISEC(TIME - timer) < 4000; ) {

        repeat(depth, MaxDepth) {
            if (stackedStates[depth].empty()) continue;
            const SearchState& currss = stackedStates[depth].top();

            if (execOptions.enableMultiThread) {
				// マルチスレッド実行
                list<thread> threads;
                mutex mtx;

				auto copied_currss = currss;

				if (stackedOjama > depth + 1) copied_currss.field.stackOjama();


				array<Pack, 4> rotatedPack = {
					packs[input.turn + depth + 1],
					packs[input.turn + depth + 1].rotated(1),
					packs[input.turn + depth + 1].rotated(2),
					packs[input.turn + depth + 1].rotated(3)
				};

                repeat(x, W - 1) {

					threads.emplace_back([&](int xPos, array<SearchState, 4> ssl) {
                        // memo: スレッドセーフであること！
                        // <score, cmd.r>
                        pair<int, int> localBest( -1, 0 );
                        bool ok[] = { true, true, true, true };
                        repeat(r, 4) {
                            SearchState& ss = ssl[r];

                            if (!ss.field.insert(rotatedPack[r], xPos)) { ok[r] = false; continue; }
                            int chainscore = ChainScore[ss.field.chain().first];
                            if (!ss.field.fall()) { ok[r] = false; continue; } // あふれた？
                            int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);

                            // ss.commands.push_back(Command(x, r)); // NG. 排他ロックのスコープでpushする

                            // chmax(ss.score, cs);
                            ss.heuristic += heuristic;
                            // ss.skill += (ss.score > 0 ? 8 : 0);

                            chmax(localBest, decltype(localBest)(chainscore, r));
                        }
                        {
							// ここのブロックは排他ロックするので、スレッドセーフでなくても良い
                            lock_guard<mutex> lock(mtx);

                            repeat(r, 4) {
                                if (!ok[r]) continue;

                                ssl[r].commands.push_back(Command(xPos, r));

                                if (r == localBest.second && localBest.first > 0)
                                    chmax(best, Tag<int, vector<Command>>(localBest.first, ssl[localBest.second].commands));

                                stackedStates[depth + 1].push(move(ssl[r]));
                            }
                            
                        }
                        }, x, array<SearchState, 4>{copied_currss, copied_currss, copied_currss, copied_currss});
                }
                for (auto& t : threads) t.join();
            }
            else {
				// マルチスレッドでない

                repeat(r, 4) {
                    auto pack = packs[input.turn + depth + 1].rotated(r);
                    repeat(x, W - 1) {
                        SearchState ss = currss;
						if (stackedOjama > depth + 1) ss.field.stackOjama();

                        if (!ss.field.insert(pack, x)) continue;
                        int chainscore = ChainScore[ss.field.chain().first];
                        if (!ss.field.fall()) continue; // あふれた？
                        int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
                        ss.commands.push_back(Command(x, r));
                        // chmax(ss.score, cs);
                        ss.heuristic += heuristic;
                        // ss.skill += (ss.score > 0 ? 8 : 0);
                        chmax(best, decltype(best)(chainscore, ss.commands));

                        stackedStates[depth + 1].push(move(ss));
                    }
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


	if (execOptions.checkOutputCommands) {
		Player me = input.me;
		int t = 0;
		int bestscore = 0;
		bool fail = false;
		for (auto cmd : best.second) {
			auto res = me.apply(cmd, packs[input.turn + t]);
			chmax(bestscore, get<0>(res));
			fail |= get<3>(res);
			++t;
		}
		if (!fail && bestscore < best.first) {
			abort();
		}
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
            return Command((uniform_int_distribution<int>(0, W - 2))(r), (uniform_int_distribution<int>(0, 3))(r));
        }
    }

    static vector<Command> pool;
    static int stackedOjama = 0;
    if (pool.empty() ||
        (checkStackedOjama(input.me.field) && stackedOjama-- <= 0) ||
        (pool.back().skill() && !input.me.skillable())) {
        pool = solveSequence(input, stackedOjama = input.me.ojama/10);
        reverse(ALL(pool));
    }
    if (!pool.empty()) {
        auto c = pool.back();
        pool.pop_back();
        return c;
    }

    cerr << "No commands" << endl;

    return Command::Skill;
}
