#pragma once

#include "Monster.h"

class FireMonster : public Monster
{
public:
    FireMonster(const std::string& name, int maxHealth, int level);

    MonsterType getType() const override;
    void useSpecialAbility(Monster& target) override;
    void takeDamage(int amount) override;

protected:
    void onLevelUp(int newLevel) override;
};
