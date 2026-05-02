#include "SimpleAiController.h"
#include "Monster.h"

size_t
SimpleAiController::chooseAttackIndex(const Monster &aiMonster,
                                      const Monster &opponentMonster) const {
  for (size_t i = 0; i < aiMonster.getAttackCount(); ++i) {
    const Attack *attack = aiMonster.getAttack(i);
    if (attack != nullptr && attack->isReady()) {
      return i;
    }
  }
  return 0;
}
