#pragma once

#include <memory>
#include <string>
#include <vector>

#include "BattleManager.h"
#include "Player.h"

enum class UiMonsterChoice
{
    Fire,
    Water,
    Earth
};

struct AttackView
{
    std::string name;
    bool ready;
};

struct BattleView
{
    std::string playerName;
    std::string aiName;
    std::string playerMonsterName;
    std::string aiMonsterName;
    int playerHealth;
    int playerMaxHealth;
    int aiHealth;
    int aiMaxHealth;
    int lastDamageToPlayer;
    int lastDamageToAi;
    bool playerBurned;
    bool playerStunned;
    bool aiBurned;
    bool aiStunned;
    bool criticalHit;
    bool superEffective;
    BattleOutcome outcome;
    std::vector<AttackView> attacks;
};

class GameSession
{
public:
    GameSession();

    void start(const std::string& playerName, UiMonsterChoice choice);
    void performPlayerAttack(size_t attackIndex);
    BattleView getView() const;
    bool isRunning() const;

private:
    std::unique_ptr<Monster> createMonster(UiMonsterChoice choice, const std::string& customName) const;
    void ensureBattleCreated();

    std::unique_ptr<Player> player;
    std::unique_ptr<Player> ai;
    std::unique_ptr<BattleManager> battle;
    int lastDamageToPlayer;
    int lastDamageToAi;
    bool lastCriticalHit;
    bool lastSuperEffective;
};
