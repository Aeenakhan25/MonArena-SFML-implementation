#include "BattleManager.h"

#include <algorithm>

#include "SimpleAiController.h"

namespace
{
    const SimpleAiController kDefaultAiController;

    size_t chooseFirstReadyAttack(const Monster& monster)
    {
        for (size_t i = 0; i < monster.getAttackCount(); ++i)
        {
            const Attack* attack = monster.getAttack(i);
            if (attack != nullptr && attack->isReady())
            {
                return i;
            }
        }

        return 0;
    }
}

BattleManager::BattleManager(Player& player, Player& aiPlayer, const IAiController* aiController)
    : player(player),
    aiPlayer(aiPlayer),
    aiController(aiController == nullptr ? &kDefaultAiController : aiController),
    outcome(BattleOutcome::Ongoing),
    turnCount(0)
{
}

BattleOutcome BattleManager::getOutcome() const
{
    return outcome;
}

int BattleManager::getTurnCount() const
{
    return turnCount;
}

void BattleManager::addEventListener(IBattleEvents& listener)
{
    if (std::find(listeners.begin(), listeners.end(), &listener) == listeners.end())
    {
        listeners.push_back(&listener);
    }
}

void BattleManager::removeEventListener(IBattleEvents& listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), &listener), listeners.end());
}

void BattleManager::playTurn(
    size_t playerAttackIndex,
    bool playerUsesSpecial,
    bool aiUsesSpecial,
    bool playerUsesItem,
    size_t playerItemIndex)
{
    if (outcome != BattleOutcome::Ongoing)
    {
        return;
    }

    ++turnCount;
    emitTurnStart();
    executePlayerTurn(playerAttackIndex, playerUsesSpecial, playerUsesItem, playerItemIndex);
    refreshOutcome();
    if (outcome != BattleOutcome::Ongoing)
    {
        awardExperience();
        emitBattleFinished();
        return;
    }

    executeAiTurn(aiUsesSpecial);
    refreshOutcome();
    if (outcome != BattleOutcome::Ongoing)
    {
        awardExperience();
        emitBattleFinished();
    }
}

BattleOutcome BattleManager::runBattle(const std::vector<size_t>& scriptedPlayerAttacks, int maxTurns)
{
    const int cappedTurns = std::max(1, maxTurns);
    int index = 0;

    while (outcome == BattleOutcome::Ongoing && turnCount < cappedTurns)
    {
        const size_t playerAttackIndex = scriptedPlayerAttacks.empty()
            ? 0
            : scriptedPlayerAttacks[static_cast<size_t>(index) % scriptedPlayerAttacks.size()];

        playTurn(playerAttackIndex);
        ++index;
    }

    if (outcome == BattleOutcome::Ongoing)
    {
        outcome = BattleOutcome::Draw;
        awardExperience();
        emitBattleFinished();
    }

    return outcome;
}

void BattleManager::executePlayerTurn(
    size_t playerAttackIndex,
    bool playerUsesSpecial,
    bool playerUsesItem,
    size_t playerItemIndex)
{
    Monster& playerMonster = player.getMonster();
    Monster& aiMonster = aiPlayer.getMonster();

    const int healthBeforeTurnStart = playerMonster.getHealth();
    playerMonster.processTurnStart();
    emitDamage(playerMonster, healthBeforeTurnStart - playerMonster.getHealth());
    if (!player.hasAvailableActions())
    {
        emitTurnSkipped(playerMonster);
        return;
    }

    if (playerUsesSpecial)
    {
        const int targetHealthBefore = aiMonster.getHealth();
        player.useMonsterSpecial(aiMonster);
        emitSpecialUsed(playerMonster, aiMonster);
        emitDamage(aiMonster, targetHealthBefore - aiMonster.getHealth());
        return;
    }

    if (playerUsesItem && player.getItemCount() > 0)
    {
        const std::string itemName = player.getItemName(playerItemIndex);
        const int healthBeforeItem = playerMonster.getHealth();
        player.useItem(playerItemIndex, aiMonster);
        emitItemUsed(player.getName(), itemName);
        emitDamage(playerMonster, healthBeforeItem - playerMonster.getHealth());
        return;
    }

    Attack* chosen = playerMonster.getAttack(playerAttackIndex);
    if (chosen != nullptr && chosen->isReady())
    {
        const std::string attackName = chosen->getName();
        const int targetHealthBefore = aiMonster.getHealth();
        player.chooseAttack(playerAttackIndex, aiMonster);
        emitAttackUsed(playerMonster, aiMonster, attackName);
        emitDamage(aiMonster, targetHealthBefore - aiMonster.getHealth());
        return;
    }

    const size_t fallback = chooseFirstReadyAttack(playerMonster);
    Attack* fallbackAttack = playerMonster.getAttack(fallback);
    if (fallbackAttack != nullptr && fallbackAttack->isReady())
    {
        const std::string attackName = fallbackAttack->getName();
        const int targetHealthBefore = aiMonster.getHealth();
        player.chooseAttack(fallback, aiMonster);
        emitAttackUsed(playerMonster, aiMonster, attackName);
        emitDamage(aiMonster, targetHealthBefore - aiMonster.getHealth());
    }
}

void BattleManager::executeAiTurn(bool aiUsesSpecial)
{
    Monster& playerMonster = player.getMonster();
    Monster& aiMonster = aiPlayer.getMonster();

    const int healthBeforeTurnStart = aiMonster.getHealth();
    aiMonster.processTurnStart();
    emitDamage(aiMonster, healthBeforeTurnStart - aiMonster.getHealth());
    if (!aiPlayer.hasAvailableActions())
    {
        emitTurnSkipped(aiMonster);
        return;
    }

    if (aiUsesSpecial)
    {
        const int targetHealthBefore = playerMonster.getHealth();
        aiPlayer.useMonsterSpecial(playerMonster);
        emitSpecialUsed(aiMonster, playerMonster);
        emitDamage(playerMonster, targetHealthBefore - playerMonster.getHealth());
        return;
    }

    const size_t aiAttackIndex = chooseAiAttackIndex();
    Attack* chosen = aiMonster.getAttack(aiAttackIndex);
    if (chosen != nullptr && chosen->isReady())
    {
        const std::string attackName = chosen->getName();
        const int targetHealthBefore = playerMonster.getHealth();
        aiPlayer.chooseAttack(aiAttackIndex, playerMonster);
        emitAttackUsed(aiMonster, playerMonster, attackName);
        emitDamage(playerMonster, targetHealthBefore - playerMonster.getHealth());
    }
}

size_t BattleManager::chooseAiAttackIndex() const
{
    const Monster& aiMonster = aiPlayer.getMonster();
    const size_t selected = aiController->chooseAttackIndex(aiMonster, player.getMonster());
    const Attack* selectedAttack = aiMonster.getAttack(selected);
    if (selectedAttack != nullptr && selectedAttack->isReady())
    {
        return selected;
    }

    return chooseFirstReadyAttack(aiMonster);
}

void BattleManager::refreshOutcome()
{
    const bool playerAlive = player.getMonster().isAlive();
    const bool aiAlive = aiPlayer.getMonster().isAlive();

    if (playerAlive && aiAlive)
    {
        outcome = BattleOutcome::Ongoing;
    }
    else if (playerAlive && !aiAlive)
    {
        outcome = BattleOutcome::PlayerWin;
    }
    else if (!playerAlive && aiAlive)
    {
        outcome = BattleOutcome::AiWin;
    }
    else
    {
        outcome = BattleOutcome::Draw;
    }
}

void BattleManager::awardExperience()
{
    if (outcome == BattleOutcome::PlayerWin)
    {
        player.getMonster().gainExperience(120);
        aiPlayer.getMonster().gainExperience(40);
    }
    else if (outcome == BattleOutcome::AiWin)
    {
        aiPlayer.getMonster().gainExperience(120);
        player.getMonster().gainExperience(40);
    }
    else if (outcome == BattleOutcome::Draw)
    {
        player.getMonster().gainExperience(60);
        aiPlayer.getMonster().gainExperience(60);
    }
}

void BattleManager::emitTurnStart() const
{
    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onTurnStarted(turnCount);
        }
    }
}

void BattleManager::emitTurnSkipped(const Monster& monster) const
{
    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onTurnSkipped(monster);
        }
    }
}

void BattleManager::emitAttackUsed(const Monster& attacker, const Monster& target, const std::string& attackName) const
{
    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onAttackUsed(attacker, target, attackName);
        }
    }
}

void BattleManager::emitSpecialUsed(const Monster& user, const Monster& target) const
{
    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onSpecialAbilityUsed(user, target);
        }
    }
}

void BattleManager::emitItemUsed(const std::string& playerName, const std::string& itemName) const
{
    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onItemUsed(playerName, itemName);
        }
    }
}

void BattleManager::emitDamage(const Monster& target, int damage) const
{
    if (damage <= 0)
    {
        return;
    }

    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onDamageApplied(target, damage);
        }
    }
}

void BattleManager::emitBattleFinished() const
{
    for (IBattleEvents* listener : listeners)
    {
        if (listener != nullptr)
        {
            listener->onBattleFinished(outcome, turnCount);
        }
    }
}
