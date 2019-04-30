
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


//


// 先読み探索の深さ
static const int MaxDepth = 12;
// 探索に割り当てるスレッド数（MainThreadも含めるとNumOfThreadsを超える）
static const int NumOfThreads = 12;
// 探索に割り当てる時間
static const int TimeLimit = 7500;
// 評価に使うパックの個数
static const int milestoneIdxRange = 3;


//


// ちょくだいさーち
// 自作PQにする意味はあまり無い(消費メモリの大半はFieldのvector<T>で、これは別のタイミングで確保・解放されるため)
static PriorityQueue<SearchState> stackedStates[MaxDepth + 1];

// backgroundで使う。次に読み込まれるはずの、自身のField情報
static Player savedMyPlayer;


static int lastMilestoneIdxBegin = MaxDepth - milestoneIdxRange;
static int lastMilestoneIdxEnd = MaxDepth;
static int lastPredictedScore = 0;

//


// 
static vector<Command> solveSequence(const Input& input, const int stackedOjama, const atomic_bool& timekeeper) {

    cerr << "solve: " << input.turn << " ";

	// 初期化
	for (auto& ss : stackedStates) ss.clear(), ss.reserve(100000);

	// 評価に使うパックのindex
    const int milestoneIdxBegin = input.turn + MaxDepth - milestoneIdxRange;
    const int milestoneIdxEnd = input.turn + MaxDepth;

	Tag<int, vector<Command>> best(-1, vector<Command>());

	const bool enableSkill = input.me.skillable();

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
		// スキルゲージを稼ぎつつ積み上げることは難しいので，連鎖とスキルは共存出来ない。
		if (enableSkill) {
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

    int loopcount = 0;

    // 時間が許す限り探索する
	{
		// マルチスレッドによる探索
		mutex mtx;

		// マルチスレッド実行
		list<thread> threads;

		const int numOfThreads = execOptions.enableMultiThread ? NumOfThreads : 1;

		repeat(_, numOfThreads) threads.emplace_back(
			[
				&input, &timekeeper, &mtx, &best, &loopcount, 
				stackedOjama, milestoneIdxBegin, milestoneIdxEnd, enableSkill
			]() {
			// memo: スレッドセーフであること！

			// 排他ロック
			mtx.lock();
			SearchState ssStocker[W][4];
			bool ok[W][4];
			SearchState currss; // stackedStatesから取り出したもの。
			Field tempField; // calcHeuristic用
			int localLoopcount = 0;

			while (timekeeper) {

				repeat(depth, MaxDepth) {
					// 扱うSearchStateを取得
					{
						// 排他ロック
						if (stackedStates[depth].empty()) continue;
						currss = move(stackedStates[depth].top());
						stackedStates[depth].pop();
						mtx.unlock();
					}

					// ojamaを降らせる
					if (stackedOjama > depth + 1) currss.field.stackOjama();

					// 回転済みパック
					// ヒープではないので、この確保はスレッドセーフ。
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
				++localLoopcount;
			}

			// 排他ロック
			loopcount += localLoopcount;
			mtx.unlock();


			});
		for (auto& t : threads) t.join();
	}

	// output logs
    clog << "loop:" << loopcount << ", best:" << best.first << "\n";
    repeat(depth, MaxDepth + 1) {
        clog << stackedStates[depth].size() << "\n";
    }

	for (auto cmd : best.second) {
		clog << cmd << "\n";
	}
	clog << "cmd.size = " << best.second.size() << endl;

	// check commands
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

	// 後処理
	lastMilestoneIdxBegin = milestoneIdxBegin;
	lastMilestoneIdxEnd = milestoneIdxEnd;
	lastPredictedScore = best.first;

    return best.second;
}


//


void BattleAI::setup(const Game::FirstInput& fi) {
    packs = fi.packs;
}


//


static void applyCommandToSavedMyField(const Pack& turnPack, Command cmd) {
	int score, chain, sender; bool success;
	tie(score, chain, sender, success) = savedMyPlayer.apply(cmd, turnPack);

	if (chain > 0) savedMyPlayer.skill += 8;
}


Command BattleAI::loop(const Input& input, const Pack& turnPack) {

	savedMyPlayer = input.me;

    if (input.turn == 0) {
		// shuffleコマンドが有効
        if (execOptions.shuffleFirstCommand) {
            random_device r;
			Command cmd((uniform_int_distribution<int>(0, W - 2))(r), (uniform_int_distribution<int>(0, 3))(r));
			applyCommandToSavedMyField(turnPack, cmd);
            return cmd;
        }
    }

    static vector<Command> pool; // コマンドのリスト(末尾が次に実行するコマンド)
    static int stackedOjama = 0; // 想定されるお邪魔の数

    if (pool.empty() || // リストが空になった
        (input.me.ojama/10 != stackedOjama) || // お邪魔の数が一致していない
        (pool.back().skill() && !input.me.skillable())) { // スキルを使うコマンドだが使えない

		atomic_bool timekeeper(true);
		thread th([&input, &timekeeper]() {
			pool = solveSequence(input, stackedOjama = input.me.ojama / 10, timekeeper); // 再計算する
			});
		
		this_thread::sleep_for(chrono::milliseconds(TimeLimit)); // 待つ。
		timekeeper = false; // 通知。
		th.join(); // 通知後、完了を待つ。

        reverse(ALL(pool));
    }

	// お邪魔を計算
	if (stackedOjama > 0) --stackedOjama;

	// コマンドのリストを実行する
    if (!pool.empty()) {
        auto c = pool.back();
        pool.pop_back();
        return c;
    }
	// あかん
    cerr << "No commands" << endl;
    return Command::Skill;
}


void BattleAI::background(int turncount, const atomic_bool& isfinished) {
	// 最初は待ち時間が無いはず
	if (turncount == 0) {
		return;
	}


}
