#pragma once

#include "Monster.h"

class EarthMonster : public Monster
{
public:
    EarthMonster(const std::string& name, int maxHealth, int level);

    MonsterType getType() const override;
    void useSpecialAbility(Monster& target) override;

protected:
    void onLevelUp(int newLevel) override;
};

