
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
    //int16_t score;
	int16_t heuristic;
	int16_t skill;
    
    inline bool operator<(const SearchState& ss) const noexcept {
        return heuristic < ss.heuristic;
    }

	inline bool skillable() const noexcept { return skill >= SkillIncrement; }

    inline void swap(SearchState& ss) {
        field.swap(ss.field);
        commands.swap(ss.commands);
        //std::swap(score, ss.score);
        std::swap(heuristic, ss.heuristic);
		std::swap(skill, ss.skill);
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
// static inline int calcHeuristic_one(const Field& field, int milestonePackIndexBegin, int milestonePackIndexEnd) {
//     int best = 0;
//     repeat(x, W) {
//         int y1 = 0;
//         while (y1 < H - 1 && field(y1, x) != None) ++y1;
//         for (int8_t v = 1; v <= 9; v += 1) {
//             Field f = field;
//             f(y1, x) = v;
//             chmax(best, ChainScore[f.chain().first]);
//         }
//     }
//     return best;
// }


//


// 先読み探索の深さ
static const int MaxDepth = 12;
// 探索に割り当てるスレッド数（MainThreadも含めるとNumOfThreadsを超える）
static const int NumOfThreads = 12;
// 探索に割り当てる時間
static const int TimeLimit = 10000;
// 評価に使うパックの個数
// static const int MilestoneIdxRange = 3;

static const int MilestoneIdxBegin = MaxDepth - 3;
static const int MilestoneIdxBeginWide = 4;
static const int MilestoneIdxEnd = MaxDepth;

//


// ちょくだいさーち
// 自作PQにする意味はあまり無い(消費メモリの大半はFieldのvector<T>で、これは別のタイミングで確保・解放されるため)
static PriorityQueue<SearchState> stackedStates[MaxDepth + 1];

// backgroundで使う。次に読み込まれるはずの情報
static Input savedInput;


static int lastMilestoneIdxBegin = MilestoneIdxBegin;
static int lastMilestoneIdxEnd = MilestoneIdxEnd;
// static int lastPredictedScore = 0;

//


// 
static Tag<int, vector<Command>> solveSequence(
	const Input& input, const int stackedOjama, const atomic_bool& timekeeper,
	const bool enableSkill, const int milestoneIdxBegin, const int milestoneIdxEnd, const int maxDepth) {
	// timekeeper: trueの間だけ実行

    cerr << "solve: " << input.turn << endl;

	// 初期化
	for (auto& ss : stackedStates) ss.clear(), ss.reserve(100000);

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
				SearchState ss{ field, vector<Command>{cmd}, 0, input.me.skill };
			
				ss.field.insert(pack, x); // 置く
				int score = ChainScore[ss.field.chain().first];
				if (ss.field.isOverFlow()) continue; // オーバーフローしたら無効
				int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
				// ss.score = score;
				ss.heuristic = heuristic;
				ss.skill += (score > 0) * SkillIncrement;

				chmax(best, decltype(best)(score, ss.commands));

				stackedStates[0].push(move(ss));
			}
		}
		// スキルが発動可能ならば入れる．
		// スキルゲージを稼ぎつつ積み上げることは難しいので，連鎖とスキルは共存出来ない。
		if (enableSkill && input.me.skillable()) {
			SearchState ss{ field, vector<Command>{Command::Skill}, 0, 0 };

			int bombcnt = ss.field.explode(); ss.field.fall();
			int skillscore = BombScore[bombcnt] + ChainScore[ss.field.chain().first];
			int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
			// ss.score = skillscore;
			ss.heuristic = heuristic;

			chmax(best, decltype(best)(skillscore, ss.commands));

			// stackedStates[0].push(move(ss));
			// pushしない。スキル使用は発火なので、探索する意味があるか？
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
				stackedOjama, milestoneIdxBegin, milestoneIdxEnd, maxDepth, enableSkill
			]() {
			// memo: スレッドセーフであること！

			// 排他ロック
			mtx.lock();
			SearchState ssStocker[W][4], ssStockerSkl;
			bool ok[W][4], okSkl;
			SearchState currss; // stackedStatesから取り出したもの。
			Field tempField; // calcHeuristic用
			int localLoopcount = 0;

			while (timekeeper) {
				// 何もしなかったら時間にかかわらず終了するフラグ
				bool noAction = true;

				repeat(depth, maxDepth) {
					// 扱うSearchStateを取得
					{
						// 排他ロック
						// スタックから状態を取り出す
						if (stackedStates[depth].empty()) continue;
						currss = move(stackedStates[depth].top());
						stackedStates[depth].pop();
						mtx.unlock();
					}
					noAction = false;

					// ojamaを降らせる
					if (stackedOjama > depth + 1) currss.field.stackOjama();

					// 今のターン
					int currentTurn = input.turn + depth + 1;

					// 回転済みパック
					// ヒープではないので、この確保はスレッドセーフ。
					array<Pack, 4> rotatedPack = {
						packs[currentTurn],
						packs[currentTurn].rotated(1),
						packs[currentTurn].rotated(2),
						packs[currentTurn].rotated(3)
					};

					Tag<int, Command> localBest(-1, 0);
					repeat(x, W - 1) repeat(r, 4) ok[x][r] = true;
					okSkl = enableSkill;

					// コマンド探索
					repeat(x, W - 1) {
						repeat(r, 4) {
							ssStocker[x][r] = currss;
							SearchState& ss = ssStocker[x][r];

							ss.field.insert(rotatedPack[r], x);
							int chainscore = ChainScore[ss.field.chain().first];
							if (ss.field.isOverFlow()) { ok[x][r] = false; continue; } // オーバーフローしたら無効
							int heuristic = calcHeuristic(ss.field, max(currentTurn, milestoneIdxBegin), milestoneIdxEnd, tempField);

							// ss.commands.push_back(Command(x, r)); // 非推奨。排他ロックのスコープでpushする

							ss.heuristic = heuristic;
							ss.skill += (chainscore > 0) * SkillIncrement;

							chmax(localBest, decltype(localBest)(chainscore, Command(x, r)));
						}
					}
					if (enableSkill && currss.skillable()) {
						ssStockerSkl = currss;
						SearchState& ss = ssStockerSkl;

						int bombcnt = ss.field.explode(); ss.field.fall();
						int skillscore = BombScore[bombcnt] + ChainScore[ss.field.chain().first];
						if (ss.field.isOverFlow()) { okSkl = false; continue; } // オーバーフローしたら無効
						int heuristic = calcHeuristic(ss.field, max(currentTurn, milestoneIdxBegin), milestoneIdxEnd, tempField);
						// ss.score = skillscore;
						ss.heuristic = heuristic;
						ss.skill = 0;

						chmax(localBest, decltype(localBest)(skillscore, Command::Skill));
					}
					{
						// 排他ロック
						mtx.lock();

						repeat(x, W - 1) {
							repeat(r, 4) {
								// 失敗したコマンドは何もしない
								if (!ok[x][r]) continue;
								// ここでコマンドを追加する(allocateの発生)
								ssStocker[x][r].commands.push_back(Command(x, r));

								// 最良のコマンドならば、更新する
								if (localBest.first > best.first && Command(x, r) == localBest.second)
									best = Tag<int, vector<Command>>(localBest.first, ssStocker[x][r].commands);
								// queueに追加
								stackedStates[depth + 1].push(ssStocker[x][r]);
							}
						}
						// スキルコマンドについて処理
						if (okSkl) {
							ssStockerSkl.commands.push_back(Command::Skill);
							if (Command::Skill == localBest.second && localBest.first > best.first)
								best = Tag<int, vector<Command>>(localBest.first, ssStockerSkl.commands);
							// stackedStates[depth + 1].push(ssStockerSkl); // pushしない(実装する場合、スキルゲージの消耗を考慮しないとダメ)
							// pushしない。スキル使用は発火なので、探索する意味があるか？
						}

					}
				}
				++localLoopcount;
				if (noAction) break;
			}

			// 排他ロック
			loopcount += localLoopcount;
			mtx.unlock();


			});
		for (auto& t : threads) t.join();
	}

	// output logs
    clog << "loop:" << loopcount << ", best:" << best.first << "\n";
    repeat(depth, maxDepth + 1) {
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

    return best;
}


//


// 大体あってる
static inline bool nearlyEqual(const Input& i1, const Input& i2) {
	return
		i1.me.ojama / 10 == i2.me.ojama / 10 &&
		i1.me.skillable() == i2.me.skillable();
}


//


// 次の入力を雑に予測して、savedInputに保管
// 相手が(1ターン先読みの範囲内で)最良のコマンドを叩くとは限らない
static void setPredicatedNextInput(const Input& currentInput, Command myCommand) {
	savedInput = currentInput;
	Tag<int, Command> best(-1, Command::Skill);

	// 相手のコマンドを雑に推定
	{
		Field field = savedInput.en.field;
		if (savedInput.en.ojama > 0) field.stackOjama();

		// 今のターンのコマンドを総当たり
		repeat(r, 4) {
			auto pack = packs[savedInput.turn].rotated(r);
			repeat(x, W - 1) {
				Command cmd(x, r);
				Field f = field;

				f.insert(pack, x); // 置く
				int score = ChainScore[f.chain().first];
				if (f.isOverFlow()) continue; // オーバーフローしたら無効

				chmax(best, decltype(best)(score, cmd));
			}
		}
	}
	if (best.first < 0); // コマンドが無かった場合、ほぼ勝ちだが、とりあえず Skill とみなす

	savedInput.apply(myCommand, best.second, packs[savedInput.turn]);
}


//


static Tag<int, vector<Command>> generateCommandPool(const Input& input, int mib, int mie) {
	// 評価に使うパックのindex
	const int milestoneIdxBegin = input.turn + mib;
	const int milestoneIdxEnd = input.turn + mie;
	// スキルコマンドも探索するか？
	const bool enableSkill = input.me.skillable();

	Tag<int, vector<Command>> result(-1, vector<Command>());

	atomic_bool timekeeper(true);
	thread th([&]() {
		result = solveSequence(input, input.me.ojama / 10, timekeeper,
			enableSkill, milestoneIdxBegin, milestoneIdxEnd, MaxDepth); // 再計算する
		});

	this_thread::sleep_for(chrono::milliseconds(TimeLimit)); // 待つ。
	timekeeper = false; // 通知。
	th.join(); // 通知後、完了を待つ。

	// 後処理
	lastMilestoneIdxBegin = milestoneIdxBegin;
	lastMilestoneIdxEnd = milestoneIdxEnd;

	reverse(ALL(result.second));
	return move(result);
}


//


void BattleAI::setup(const Game::FirstInput& fi) {
    packs = fi.packs;
}


//


static vector<Command> commandPool; // コマンドのリスト(末尾が次に実行するコマンド)
static int bestCommandScore;
static vector<Command> suggestedCommandPool; // コマンドのリスト(末尾が次に実行するコマンド)
static int suggestedCommandScore;


//


Command BattleAI::loop(const Input& input, const Pack& turnPack) {

    if (input.turn == 0) {
		// shuffleコマンドが有効
        if (execOptions.shuffleFirstCommand) {
            random_device r;
			Command cmd((uniform_int_distribution<int>(0, W - 2))(r), (uniform_int_distribution<int>(0, 3))(r));

			setPredicatedNextInput(input, cmd);
            return cmd;
        }
    }

	static int stackedOjama = 0;

	// リストが空になった
    if (commandPool.empty()) { // スキルを使うコマンドだが使えない
		stackedOjama = input.me.ojama / 10;

		// commandPoolを再構築する
		clog << "!reconstruct commands" << endl;
		auto pair = generateCommandPool(input, MilestoneIdxBegin, MilestoneIdxEnd);
		bestCommandScore = pair.first;
		commandPool = move(pair.second);
    }
	// お邪魔の数が一致していない(savedInputを使うと、中途半端な連鎖の予測によって毎回再構築が発生するのでダメ)
	// スキルを使うコマンドだが使えない
	else if (input.me.ojama / 10 != stackedOjama || 
		(commandPool.back().skill() && !input.me.skillable())) { 
		stackedOjama = input.me.ojama / 10;

		// commandPoolを再構築する
		clog << "!reconstruct commands(Wide)" << endl;
		auto pair = generateCommandPool(input, MilestoneIdxBeginWide, MilestoneIdxEnd);
		bestCommandScore = pair.first;
		commandPool = move(pair.second);
	}
	else {
		// リストが空になっていなくても、改善していたら置き換える

		if (nearlyEqual(input, savedInput) &&
			bestCommandScore < suggestedCommandScore) {
			clog << "!replace commands" << endl;
			commandPool = suggestedCommandPool;
			bestCommandScore = suggestedCommandScore;
		}
	}

	if (stackedOjama > 0) --stackedOjama;

    if (!commandPool.empty()) {
		// コマンドのリストを実行する
        auto c = commandPool.back();
		commandPool.pop_back();

		setPredicatedNextInput(input, c);

        return c;
    }


	// あかん
    cerr << "No commands" << endl;
    return Command::Skill;
}


//


void BattleAI::background(const atomic_bool& timekeeper) {
	// 最初は待ち時間が無いはず
	if (savedInput.turn == 0) {
		return;
	}

	// 探索面数が少ない
	if (lastMilestoneIdxBegin <= savedInput.turn) return;

	// 探索
	auto p = solveSequence(savedInput, savedInput.me.ojama / 10, timekeeper,
		savedInput.me.skillable(), lastMilestoneIdxBegin, lastMilestoneIdxEnd, lastMilestoneIdxEnd - savedInput.turn); // 再計算する
	auto score = p.first;
	auto pool = move(p.second);
	reverse(ALL(pool));

	// 候補として挙げておく
	suggestedCommandScore = score;
	suggestedCommandPool = pool;
}
