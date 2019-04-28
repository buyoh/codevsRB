
#include "Macro.h"
#include "Game.h"
#include "Input.h"
#include "AI.h"
#include "Time.h"
#include "Score.h"
#include "PriorityQueue.h"

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
// vstatic array<array<Pack, 4>, N> rotatedPacks; // 回転済みのパック TODO: 


//


// ojamaが積もったか？
// 予めfallされていること
// deprecated
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
// workField: 作業用Field
static inline int calcHeuristic(const Field& field, int milestonePackIndexBegin, int milestonePackIndexEnd, Field& tempField) {
	int best = 0;
	iterate(mpi, milestonePackIndexBegin, milestonePackIndexEnd) {
		repeat(r, 4) {
			auto pack = packs[mpi].rotated(r);
			repeat(i, W - 1) {
				tempField = field;
				tempField.insert(pack, i);
				chmax(best, ChainScore[tempField.chain().first]);
			}
		}
	}
	return best;
}


// field: 対象
// milestonePack: 発火対象
static inline int calcHeuristic(const Field& field, int milestonePackIndexBegin, int milestonePackIndexEnd) {
	Field f;
    return calcHeuristic(field, milestonePackIndexBegin, milestonePackIndexEnd, f);
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

// 先読み探索の深さ
static const int MaxDepth = 12;

static const int NumOfThreads = 12;
static const int TimeLimit = 7500;

// 
static vector<Command> solveSequence(const Input& input, const int stackedOjama) {

    cerr << "solve: " << input.turn << " ";

    // ちょくだいさーち
	static PriorityQueue<SearchState> stackedStates[MaxDepth + 1];
	for (auto& ss : stackedStates) ss.clear(), ss.reserve(10000);

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
			
				ss.field.insert(pack, x); // 置く
				int score = ChainScore[ss.field.chain().first];
				if (ss.field.isOverFlow()) continue; // オーバーフローしたら無効
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

			int bombcnt = ss.field.explode(); ss.field.fall();
			int skillscore = BombScore[bombcnt] + ChainScore[ss.field.chain().first];
			int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
			ss.score = skillscore;
			ss.heuristic = heuristic;

			chmax(best, decltype(best)(skillscore, ss.commands));

			stackedStates[0].push(move(ss));
		}
	}

    // 時間が許す限り探索する
    int loopcount = 0;
    static int totaloppcount = 0;

	if (execOptions.enableMultiThread) {
		// マルチスレッドによる探索
		mutex mtx;

		// マルチスレッド実行
		list<thread> threads;

		repeat(_, NumOfThreads) threads.emplace_back([&input, &mtx, &best, stackedOjama, milestoneIdxBegin, milestoneIdxEnd]() {
			// memo: スレッドセーフであること！

			mtx.lock();
			SearchState ssStocker[W][4];
			bool ok[W][4];
			SearchState currss;
			Field tempField;

			for (auto timer = TIME; MILLISEC(TIME - timer) < TimeLimit; ) {

				repeat(depth, MaxDepth) {

					// 扱うSearchStateを取得
					{
						// 排他ロック
						if (stackedStates[depth].empty()) continue;
						currss = stackedStates[depth].top();
						stackedStates[depth].pop();
						mtx.unlock();
					}

					// ojamaを降らせる
					if (stackedOjama > depth + 1) currss.field.stackOjama();


					array<Pack, 4> rotatedPack = {
						packs[input.turn + depth + 1],
						packs[input.turn + depth + 1].rotated(1),
						packs[input.turn + depth + 1].rotated(2),
						packs[input.turn + depth + 1].rotated(3)
					};

					Tag<int, Command> localBest(-1, 0);
					repeat(x, W - 1) repeat(r, 4) ok[x][r] = true;

					// コマンド探索
					repeat(x, W - 1) {
						repeat(r, 4) {
							ssStocker[x][r] = currss;
							SearchState& ss = ssStocker[x][r];

							ss.field.insert(rotatedPack[r], x);
							int chainscore = ChainScore[ss.field.chain().first];
							if (ss.field.isOverFlow()) { ok[x][r] = false; continue; } // オーバーフローしたら無効
							int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd, tempField);
							//int height = ss.field.getHighest();
							//heuristic -= height / 2;

							// ss.commands.push_back(Command(x, r)); // 非推奨。排他ロックのスコープでpushする

							ss.heuristic = heuristic;
							// ss.skill += (ss.score > 0 ? 8 : 0);

							chmax(localBest, decltype(localBest)(chainscore, Command(x, r)));
						}
					}
					{
						// 排他ロック
						mtx.lock();

						repeat(x, W - 1) {
							repeat(r, 4) { // TODO:
								if (!ok[x][r]) continue;
								ssStocker[x][r].commands.push_back(Command(x, r));

								if (Command(x, r) == localBest.second && localBest.first > best.first)
									best = Tag<int, vector<Command>>(localBest.first, ssStocker[x][r].commands);

								stackedStates[depth + 1].push(ssStocker[x][r]);
							}
						}

					}
				}
				// ++loopcount;
			}
			mtx.unlock();


			});
		for (auto& t : threads) t.join();
	}
	else {
		// マルチスレッドでない探索
		for (auto timer = TIME; MILLISEC(TIME - timer) < TimeLimit; ) {

			repeat(depth, MaxDepth) {
				if (stackedStates[depth].empty()) continue;
				const SearchState& currss = stackedStates[depth].top();

				repeat(r, 4) {
					auto pack = packs[input.turn + depth + 1].rotated(r);
					repeat(x, W - 1) {
						SearchState ss = currss;
						if (stackedOjama > depth + 1) ss.field.stackOjama();

						ss.field.insert(pack, x);
						int chainscore = ChainScore[ss.field.chain().first];
						if (ss.field.isOverFlow()) continue; // オーバーフローしたら無効
						int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
						// int height = ss.field.getHighest();
						// heuristic -= height / 2;

						ss.commands.push_back(Command(x, r));
						// chmax(ss.score, cs);
						ss.heuristic = heuristic;
						// ss.skill += (ss.score > 0 ? 8 : 0);
						chmax(best, decltype(best)(chainscore, ss.commands));

						stackedStates[depth + 1].push(move(ss));
					}
				}

				stackedStates[depth].pop();
			}
			++loopcount;
		}
	}

    clog << "loop:" << loopcount << ", best:" << best.first << "\n";
    clog << "totaloop: " << (totaloppcount += loopcount) << "\n";
    repeat(depth, MaxDepth + 1) {
        clog << stackedStates[depth].size() << "\n";
    }

	for (auto cmd : best.second) {
		clog << cmd << "\n";
	}
	clog << "cmd.size = " << best.second.size() << endl;


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
			cerr << "Wrong commands" << endl;
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
        (input.me.ojama >= 10 && stackedOjama-- <= 0) ||
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
