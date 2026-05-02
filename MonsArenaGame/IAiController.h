#pragma once

#include <cstddef>

class Monster;

class IAiController
{
public:
    virtual ~IAiController() = default;
    virtual size_t chooseAttackIndex(const Monster& aiMonster, const Monster& opponentMonster) const = 0;
};
