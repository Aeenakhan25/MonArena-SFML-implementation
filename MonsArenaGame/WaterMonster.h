#pragma once

#include "Monster.h"

class WaterMonster : public Monster
{
public:
    WaterMonster(const std::string& name, int maxHealth, int level);

    MonsterType getType() const override;
    void useSpecialAbility(Monster& target) override;

protected:
    void onLevelUp(int newLevel) override;
};

