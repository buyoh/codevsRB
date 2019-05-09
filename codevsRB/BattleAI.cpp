
#include "Macro.h"
#include "Game.h"
#include "Input.h"
#include "AI.h"
#include "Time.h"
#include "Score.h"
#include "PriorityQueue.h"
#include "XorShift.h"

#include "Exec.h"

using namespace Game;


//


struct SearchState {
    Field field;
    vector<Command> commands;
	int16_t heuristic;
	int16_t skill;
    int16_t exploded;
    
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

// #define EnableAnalysis
#ifdef EnableAnalysis

namespace Ana {
	namespace He {
		static int cnt[20][W][4];
		inline void reset() {
			if (execOptions.enableMultiThread)
				cerr << "Warning: EnableAnalysis is unsafe" << endl;
			fill(cnt[0][0], cnt[20][W], 0);
		}
		inline void add(int i, int j, int k) {
			++cnt[i][j][k];
		}
		inline void dump() {
			priority_queue<array<int, 4>> pq;
			repeat(i, 20) repeat(j, W) repeat(k, 4) pq.push({ cnt[i][j][k], i,j,k });
			clog << "heuristic report:\n";
			repeat(i, 10) {
				const auto& a = pq.top();
				clog << a[0] << ':' << a[1] << ' ' << a[2] << ' ' << a[3] << '\n';
				pq.pop();
			}
		}
	}
}

#endif


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
				// tempField.stackOjama();
				tempField.insert(pack, i);
				const int cs = ChainScore[tempField.chain().first];
				chmax(best, cs);
#ifdef EnableAnalysis
				if (cs >= 20) Ana::He::add(milestonePackIndexEnd - mpi, r, i);
#endif
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


const int NumOfHeuristicStack = 8;

static int calcReducedHeuristic(const Field& field, int milestonePackIndexBegin, int milestonePackIndexEnd, Field& tempField, array<array<int, 3>, NumOfHeuristicStack>& memo) {
	int best = 0;
	if (memo.back()[0] < 0) {
		iterate(mpi, milestonePackIndexBegin, milestonePackIndexEnd) {
			repeat(r, 4) {
				auto pack = packs[mpi].rotated(r);
				repeat(i, W - 1) {
					tempField = field;
					//tempField.stackOjama();
					tempField.insert(pack, i);
					const int cs = ChainScore[tempField.chain().first];
					chmax(best, cs);
					if (cs >= 19) {
						for (auto& a : memo)
							if (a[0] < 0) {
								a[0] = mpi; a[1] = r; a[2] = i;
								break;
							}
							else if (a[0] == mpi && a[1] == r && a[2] == i)
								break;
					}
				}
			}
		}
	}
	else {
		for (auto& a : memo) {
			auto pack = packs[a[0]].rotated(a[1]);
			tempField = field;
			//tempField.stackOjama();
			tempField.insert(pack, a[2]);
			const int cs = ChainScore[tempField.chain().first];
			chmax(best, cs);
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
static const int TimeLimit = 10000;
static const int FirstTimeLimit = 18000;
// 評価に使うパックの個数
// static const int MilestoneIdxRange = 3;

static const int MilestoneIdxBegin = MaxDepth - 4;
static const int MilestoneIdxBeginWide = 4;
static const int MilestoneIdxEnd = MaxDepth;

// このスコア以上の連鎖が発生したらそれ以降の探索を行わない
static const int ThresholdBreakScore = 20; 
// このスコア以上は無意味とみなす
static const int SaturatedScore = 60;
// static const int SaturatedExplode = 10;
// このスコア以下は無意味とみなす
static const int SaturatedExplodeLower = 40;


//


// ちょくだいさーち
// 自作PQにする意味はあまり無い(消費メモリの大半はFieldのvector<T>で、これは別のタイミングで確保・解放されるため)
static PriorityQueue<SearchState> stackedStates[MaxDepth + 1];

// backgroundで使う。次に読み込まれるはずの情報
static Input savedInput;


static int lastMilestoneIdxBegin = MilestoneIdxBegin;
static int lastMilestoneIdxEnd = MilestoneIdxEnd;


//


// score: そのターンで獲得したスコア
// remainTurn: DepthTurn - turn
// exploded: これまでにボムで破壊したブロックの個数
inline int evaluateScore(int score, int remainTurn, int exploded) noexcept {
	return ((min(SaturatedScore, score) + remainTurn + max(0, exploded - SaturatedExplodeLower)) << 10) + score;
}

inline int evaluateHeuristic(int h, int exploded, uint64_t seed) noexcept {
    return ((h) << 8) | (seed & 0xFF); // 結局explodedは入れていない
}


// 
static Tag<int, vector<Command>> solveSequence(
	const Input& input, const int stackedOjama, const atomic_bool& timekeeper,
	const bool enableSkill, const int milestoneIdxBegin, const int milestoneIdxEnd, const int maxDepth) {
	// timekeeper: trueの間だけ実行

    cerr << "solve: " << input.turn << endl;

#ifdef EnableAnalysis
	Ana::He::reset();
#endif

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
                if (input.turn == 0 && 4 < x) break;
				Command cmd(x, r);
				SearchState ss{ field, vector<Command>{cmd}, 0, input.me.skill, 0 };
			
				ss.field.insert(pack, x); // 置く
				int score = ChainScore[ss.field.chain().first];
				if (ss.field.isOverFlow()) continue; // オーバーフローしたら無効
				// int cntblock = ss.field.countWithountOjama();
				int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
				// ss.score = score;
				ss.heuristic = heuristic;
				ss.skill += (score > 0) * SkillIncrement;

				chmax(best, decltype(best)(evaluateScore(score, maxDepth + 1, 0), ss.commands));

				if (score < ThresholdBreakScore)
					stackedStates[0].push(move(ss));
			}
		}
		// スキルが発動可能ならば入れる．
		// スキルゲージを稼ぎつつ積み上げることは難しいので，連鎖とスキルは共存出来ない。
		if (enableSkill && input.me.skillable()) {
			SearchState ss{ field, vector<Command>{Command::Skill}, 0, 0, 0 };

			int bombcnt = ss.field.explode(); ss.field.fall();
			int skillscore = BombScore[bombcnt] + ChainScore[ss.field.chain().first];
			int heuristic = calcHeuristic(ss.field, milestoneIdxBegin, milestoneIdxEnd);
			// int cntblock = ss.field.countWithountOjama();
			// ss.score = skillscore;
			ss.heuristic = heuristic;
            ss.exploded += bombcnt;

			chmax(best, decltype(best)(evaluateScore(skillscore, maxDepth + 1, ss.exploded), ss.commands));

			stackedStates[0].push(move(ss));
		}
	}

    // 時間が許す限り探索する
	{
		// マルチスレッドによる探索
		mutex mtx;

		// マルチスレッド実行
		list<thread> threads;

		const int numOfThreads = execOptions.enableMultiThread ? NumOfThreads : 1;

		mt19937_64 seedGen;

		repeat(_, numOfThreads) threads.emplace_back(
			[
				&input, &timekeeper, &mtx, &best, 
				stackedOjama, milestoneIdxBegin, milestoneIdxEnd, maxDepth, enableSkill
			](mt19937_64::result_type seed) {
			// memo: スレッドセーフであること！

			XorShift randdev(seed);

			// 排他ロック
			mtx.lock();
			SearchState ssStocker[W][4], ssStockerSkl;
			bool ok[W][4], okSkl;
			SearchState currss; // stackedStatesから取り出したもの。
			Field tempField; // calcHeuristic用
			array<array<int, 3>, NumOfHeuristicStack> heuristicmemo; // calcHeuristic用

			array<Pack, 4> rotatedPack;

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
					rotatedPack = {
						packs[currentTurn],
						packs[currentTurn].rotated(1),
						packs[currentTurn].rotated(2),
						packs[currentTurn].rotated(3)
					};

					Tag<int, Command> localBest(-1, 0);
					repeat(x, W - 1) repeat(r, 4) ok[x][r] = true;
					repeat(i, NumOfHeuristicStack) repeat(j, 3) heuristicmemo[i][j] = -1;
					okSkl = enableSkill;

					// コマンド探索
					repeat(x, W - 1) {
						repeat(r, 4) {
							ssStocker[x][r] = currss;
							SearchState& ss = ssStocker[x][r];

							ss.field.insert(rotatedPack[r], x);
							int chainscore = ChainScore[ss.field.chain().first];
							if (ss.field.isOverFlow()) { ok[x][r] = false; continue; } // オーバーフローしたら無効
							// int cntblock = ss.field.countWithountOjama();

							// localBest更新
							chmax(localBest, decltype(localBest)(evaluateScore(chainscore, maxDepth - depth, ss.exploded), Command(x, r)));

							// 続いて探索する価値がある(連鎖が終了した後でない)
							if (chainscore < ThresholdBreakScore) {
								int heuristic = evaluateHeuristic(
                                    calcReducedHeuristic(ss.field, max(currentTurn, milestoneIdxBegin), milestoneIdxEnd, tempField, heuristicmemo),
                                    ss.exploded, 
                                    randdev()
                                );
								ss.heuristic = heuristic;
								ss.skill += (chainscore > 0) * SkillIncrement;
							}
							else {
								ok[x][r] = false;
							}
								
						}
					}
                    if (enableSkill && currss.skillable()) {
						ssStockerSkl = currss;
						SearchState& ss = ssStockerSkl;

						int bombcnt = ss.field.explode(); ss.field.fall();
						int skillscore = BombScore[bombcnt] + ChainScore[ss.field.chain().first];
						if (ss.field.isOverFlow()) { okSkl = false; continue; } // オーバーフローしたら無効
						// int cntblock = ss.field.countWithountOjama();
                        int heuristic = evaluateHeuristic(
                            calcReducedHeuristic(ss.field, max(currentTurn, milestoneIdxBegin), milestoneIdxEnd, tempField, heuristicmemo),
                            bombcnt,
                            randdev()
                        );
						ss.heuristic = heuristic;
						ss.skill = 0;

						chmax(localBest, decltype(localBest)(evaluateScore(skillscore, maxDepth - depth, ss.exploded), Command::Skill));
                        ss.exploded += bombcnt;
					}
					{
						// 排他ロック
						mtx.lock();

						repeat(x, W - 1) {
							repeat(r, 4) {
								// ここでコマンドを追加する(allocateの発生)
								ssStocker[x][r].commands.push_back(Command(x, r));

								// 最良のコマンドならば、更新する
								if (localBest.first > best.first && Command(x, r) == localBest.second)
									best = Tag<int, vector<Command>>(localBest.first, ssStocker[x][r].commands);

								// 失敗したコマンドはqueueに追加しない
								if (!ok[x][r]) continue;

								// queueに追加
								stackedStates[depth + 1].push(ssStocker[x][r]);
							}
						}
						// スキルコマンドについて処理
						if (okSkl) {
							ssStockerSkl.commands.push_back(Command::Skill);
							if (Command::Skill == localBest.second && localBest.first > best.first)
								best = Tag<int, vector<Command>>(localBest.first, ssStockerSkl.commands);
							stackedStates[depth + 1].push(ssStockerSkl);
						}

					}
				}
				if (noAction) break;
			}

			// 排他ロック
			mtx.unlock();


			}, seedGen());
		for (auto& t : threads) t.join();
	}


	// output logs
	int stateSize = 0;
	repeat(depth, maxDepth + 1) stateSize += stackedStates[depth].size();
    clog << "score: " << best.first << ", statesize: " << stateSize << '\n';

	// for (auto cmd : best.second) clog << cmd << "\n";
	clog << "cmd.length = " << best.second.size() << endl;

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
#ifdef EnableAnalysis
	Ana::He::dump();
#endif

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

	this_thread::sleep_for(chrono::milliseconds(input.turn < 5 ? FirstTimeLimit : TimeLimit)); // 待つ。
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
