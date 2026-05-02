
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "IAiController.h"
#include "IBattleEvents.h"
#include "Player.h"

enum class BattleOutcome
{
    Ongoing,
    PlayerWin,
    AiWin,
    Draw
};

class BattleManager
{
public:
    BattleManager(Player& player, Player& aiPlayer, const IAiController* aiController = nullptr);

    BattleOutcome getOutcome() const;
    int getTurnCount() const;

    // Executes one full round: player turn then AI turn.
    void playTurn(
        size_t playerAttackIndex,
        bool playerUsesSpecial = false,
        bool aiUsesSpecial = false,
        bool playerUsesItem = false,
        size_t playerItemIndex = 0);

    // Optional loop-based battle runner for scripted player decisions.
    BattleOutcome runBattle(const std::vector<size_t>& scriptedPlayerAttacks, int maxTurns);
    void addEventListener(IBattleEvents& listener);
    void removeEventListener(IBattleEvents& listener);

private:
    void executePlayerTurn(size_t playerAttackIndex, bool playerUsesSpecial, bool playerUsesItem, size_t playerItemIndex);
    void executeAiTurn(bool aiUsesSpecial);
    size_t chooseAiAttackIndex() const;
    void refreshOutcome();
    void awardExperience();
    void emitTurnStart() const;
    void emitTurnSkipped(const Monster& monster) const;
    void emitAttackUsed(const Monster& attacker, const Monster& target, const std::string& attackName) const;
    void emitSpecialUsed(const Monster& user, const Monster& target) const;
    void emitItemUsed(const std::string& playerName, const std::string& itemName) const;
    void emitDamage(const Monster& target, int damage) const;
    void emitBattleFinished() const;

    Player& player;
    Player& aiPlayer;
    const IAiController* aiController;
    BattleOutcome outcome;
    int turnCount;
    std::vector<IBattleEvents*> listeners;
};
