#pragma once

#include "IAiController.h"

class SimpleAiController : public IAiController
{
public:
    size_t chooseAttackIndex(const Monster& aiMonster, const Monster& opponentMonster) const override;
};

