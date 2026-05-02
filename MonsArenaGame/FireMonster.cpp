#include "FireMonster.h"

FireMonster::FireMonster(const std::string& name, int maxHealth, int level)
    : Monster(name, maxHealth, level)
{
}

MonsterType FireMonster::getType() const
{
    return MonsterType::Fire;
}

void FireMonster::useSpecialAbility(Monster& target)
{
    target.takeDamage(25, MonsterType::Fire);
    target.applyStatusEffect(StatusEffectType::Burn, 3);
}

void FireMonster::takeDamage(int amount)
{
    Monster::takeDamage(amount);
}

void FireMonster::onLevelUp(int newLevel)
{
    Monster::onLevelUp(newLevel);
}
