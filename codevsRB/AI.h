#pragma once
#include "Macro.h"
#include "Game.h"


// ƒQ[ƒ€AI‚Ì’ŠÛƒNƒ‰ƒX
// À‘•‚·‚×‚«ŠÖ”‚½‚¿
class AI {
public:

    virtual string aiName() const = 0;
    virtual void setup(const Game::FirstInput&) = 0;
    virtual Game::Command loop(const Game::Input&, const Game::Pack&) = 0;
};


//


class BattleAI : public AI {
    // sigleton
public:
    inline string aiName() const override { return "mai(" __DATE__ ")"; };
    void setup(const Game::FirstInput&) override;
    Game::Command loop(const Game::Input&, const Game::Pack&) override;
};
