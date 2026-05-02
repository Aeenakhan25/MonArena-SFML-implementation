#include "WaterMonster.h"

WaterMonster::WaterMonster(const std::string& name, int maxHealth, int level)
    : Monster(name, maxHealth, level)
{
}

MonsterType WaterMonster::getType() const
{
    return MonsterType::Water;
}

void WaterMonster::useSpecialAbility(Monster& target)
{
    heal(20);
    target.takeDamage(15, MonsterType::Water);
}

void WaterMonster::onLevelUp(int newLevel)
{
    Monster::onLevelUp(newLevel);
}
