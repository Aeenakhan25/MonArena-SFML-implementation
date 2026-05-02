#include "GameSession.h"
#include "FireMonster.h"
#include "WaterMonster.h"
#include "EarthMonster.h"
#include "Attack.h"

GameSession::GameSession()
    : lastDamageToPlayer(0), lastDamageToAi(0), lastCriticalHit(false), lastSuperEffective(false)
{
}

std::unique_ptr<Monster> GameSession::createMonster(UiMonsterChoice choice, const std::string& customName) const
{
    std::unique_ptr<Monster> m;
    switch (choice)
    {
        case UiMonsterChoice::Fire:
            m = std::make_unique<FireMonster>(customName, 100, 5);
            m->addAttack(std::make_unique<DamageAttack>("Ember", 15, 0));
            m->addAttack(std::make_unique<DamageAttack>("Flamethrower", 30, 2));
            break;
        case UiMonsterChoice::Water:
            m = std::make_unique<WaterMonster>(customName, 110, 5);
            m->addAttack(std::make_unique<DamageAttack>("Water Gun", 15, 0));
            m->addAttack(std::make_unique<DamageAttack>("Hydro Pump", 30, 2));
            break;
        case UiMonsterChoice::Earth:
            m = std::make_unique<EarthMonster>(customName, 120, 5);
            m->addAttack(std::make_unique<DamageAttack>("Tackle", 15, 0));
            m->addAttack(std::make_unique<DamageAttack>("Earthquake", 30, 2));
            break;
    }
    return m;
}

void GameSession::start(const std::string& playerName, UiMonsterChoice choice)
{
    player = std::make_unique<Player>(playerName, createMonster(choice, "Hero"));
    
    UiMonsterChoice aiChoice = UiMonsterChoice::Fire;
    if (choice == UiMonsterChoice::Fire) aiChoice = UiMonsterChoice::Water;
    else if (choice == UiMonsterChoice::Water) aiChoice = UiMonsterChoice::Earth;
    else if (choice == UiMonsterChoice::Earth) aiChoice = UiMonsterChoice::Fire;
    
    ai = std::make_unique<Player>("AI Nemesis", createMonster(aiChoice, "Villain"));
    
    battle.reset();
    ensureBattleCreated();
}

void GameSession::ensureBattleCreated()
{
    if (!battle)
    {
        battle = std::make_unique<BattleManager>(*player, *ai, nullptr);
    }
}

void GameSession::performPlayerAttack(size_t attackIndex)
{
    if (battle)
    {
        int aiHpBefore = ai->getMonster().getHealth();
        int playerHpBefore = player->getMonster().getHealth();
        
        battle->playTurn(attackIndex);
        
        lastDamageToAi = aiHpBefore - ai->getMonster().getHealth();
        lastDamageToPlayer = playerHpBefore - player->getMonster().getHealth();
    }
}

BattleView GameSession::getView() const
{
    BattleView view;
    if (player && ai)
    {
        view.playerName = player->getName();
        view.aiName = ai->getName();
        view.playerMonsterName = player->getMonster().getName();
        view.aiMonsterName = ai->getMonster().getName();
        view.playerHealth = player->getMonster().getHealth();
        view.playerMaxHealth = player->getMonster().getMaxHealth();
        view.aiHealth = ai->getMonster().getHealth();
        view.aiMaxHealth = ai->getMonster().getMaxHealth();
        view.lastDamageToPlayer = lastDamageToPlayer;
        view.lastDamageToAi = lastDamageToAi;
        view.playerBurned = player->getMonster().hasStatusEffect(StatusEffectType::Burn);
        view.playerStunned = player->getMonster().hasStatusEffect(StatusEffectType::Stun);
        view.aiBurned = ai->getMonster().hasStatusEffect(StatusEffectType::Burn);
        view.aiStunned = ai->getMonster().hasStatusEffect(StatusEffectType::Stun);
        view.criticalHit = lastCriticalHit;
        view.superEffective = lastSuperEffective;
        
        if (battle)
        {
            view.outcome = battle->getOutcome();
        }
        else
        {
            view.outcome = BattleOutcome::Ongoing;
        }
            
        for (size_t i = 0; i < player->getMonster().getAttackCount(); ++i)
        {
            const Attack* atk = player->getMonster().getAttack(i);
            if (atk)
            {
                view.attacks.push_back({atk->getName(), atk->isReady()});
            }
        }
    }
    return view;
}

bool GameSession::isRunning() const
{
    return battle && battle->getOutcome() == BattleOutcome::Ongoing;
}
