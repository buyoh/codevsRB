#pragma once
#include "Macro.h"
#include "Game.h"


// ゲームAIの抽象クラス
// 実装すべき関数たち
class AI {
public:

    virtual string aiName() const = 0;
    virtual void setup(const Game::FirstInput&) = 0;
    virtual Game::Command loop(const Game::Input&, const Game::Pack&) = 0;
	virtual void background(const atomic_bool& finished) = 0;
};


//


class BattleAI : public AI {
    // sigleton
public:
    inline string aiName() const override { return "mai-" __DATE__; };
    void setup(const Game::FirstInput&) override;
    Game::Command loop(const Game::Input&, const Game::Pack&) override;
	void background(const atomic_bool&) override;
};
