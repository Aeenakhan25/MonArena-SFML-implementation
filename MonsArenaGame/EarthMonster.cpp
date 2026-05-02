#include "EarthMonster.h"

EarthMonster::EarthMonster(const std::string& name, int maxHealth, int level)
    : Monster(name, maxHealth, level)
{
}

MonsterType EarthMonster::getType() const
{
    return MonsterType::Earth;
}

void EarthMonster::useSpecialAbility(Monster& target)
{
    target.takeDamage(20, MonsterType::Earth);
}

void EarthMonster::onLevelUp(int newLevel)
{
    Monster::onLevelUp(newLevel);
}
