#pragma once

#include <string>

class Monster;
enum class BattleOutcome;

class IBattleEvents
{
public:
    virtual ~IBattleEvents() = default;

    virtual void onTurnStarted(int turnNumber) = 0;
    virtual void onTurnSkipped(const Monster& monster) = 0;
    virtual void onAttackUsed(const Monster& attacker, const Monster& target, const std::string& attackName) = 0;
    virtual void onSpecialAbilityUsed(const Monster& user, const Monster& target) = 0;
    virtual void onItemUsed(const std::string& playerName, const std::string& itemName) = 0;
    virtual void onDamageApplied(const Monster& target, int damageAmount) = 0;
    virtual void onBattleFinished(BattleOutcome outcome, int turnCount) = 0;
};
