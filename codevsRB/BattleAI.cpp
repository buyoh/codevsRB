
#include "Macro.h"
#include "Game.h"
#include "AI.h"


// static int state = 0;


void BattleAI::setup(const Game::FirstInput&) {
    

}


Game::Command BattleAI::loop(const Game::Input& input, const Game::Pack& turnPack) {
    using namespace Game;

    Tag<int, Command> best(-1, Command::Skill);

    repeat(r, 4) {
        repeat(x, W - 1) {
            Player p = input.me;
            auto [score, chain, sender, success] = p.apply(Command(x, r), turnPack);
            chmax(best, Tag<int, Command>{score, Command(x, r)});
        }
    }

    return best.second;
}
