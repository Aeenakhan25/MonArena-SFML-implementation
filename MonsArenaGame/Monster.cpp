#include "Monster.h"
#include <algorithm>

int Monster::totalMonstersCreated = 0;

Monster::Monster(std::string name, int maxHealth, int level)
    : name(std::move(name)), health(maxHealth), maxHealth(maxHealth),
      level(level), experience(0), experienceToNextLevel(level * 100),
      temporaryAttackBuff(0), attackBuffDuration(0), stunnedThisTurn(false) {
  ++totalMonstersCreated;
}

const std::string &Monster::getName() const { return name; }
int Monster::getHealth() const { return health; }
int Monster::getMaxHealth() const { return maxHealth; }
int Monster::getLevel() const { return level; }
int Monster::getExperience() const { return experience; }
int Monster::getExperienceToNextLevel() const { return experienceToNextLevel; }
bool Monster::isAlive() const { return health > 0; }

int Monster::getTotalMonstersCreated() { return totalMonstersCreated; }

void Monster::addAttack(std::unique_ptr<Attack> attack) {
  if (attack) {
    attacks.push_back(std::move(attack));
  }
}

size_t Monster::getAttackCount() const { return attacks.size(); }

Attack *Monster::getAttack(size_t index) {
  if (index < attacks.size()) {
    return attacks[index].get();
  }
  return nullptr;
}

const Attack *Monster::getAttack(size_t index) const {
  if (index < attacks.size()) {
    return attacks[index].get();
  }
  return nullptr;
}

void Monster::takeDamage(int amount) {
  if (amount > 0) {
    health -= amount;
    if (health < 0) {
      health = 0;
    }
  }
}

void Monster::performAttack(size_t attackIndex, Monster &target) {
  if (attackIndex < attacks.size()) {
    Attack *attack = attacks[attackIndex].get();
    if (attack && attack->isReady()) {
      attack->execute(*this, target);
      attack->startCooldown();
    }
  }
}

void Monster::processTurnStart() {
  stunnedThisTurn = hasStatusEffect(StatusEffectType::Stun);

  if (attackBuffDuration > 0) {
    attackBuffDuration--;
    if (attackBuffDuration == 0) {
      temporaryAttackBuff = 0;
    }
  }

  for (auto it = activeEffects.begin(); it != activeEffects.end();) {
    if (it->type == StatusEffectType::Burn) {
      takeDamage(5); // Constant 5 damage for burn
    }

    it->remainingTurns--;
    if (it->remainingTurns <= 0) {
      it = activeEffects.erase(it);
    } else {
      ++it;
    }
  }

  for (auto &attack : attacks) {
    if (attack) {
      attack->tickCooldown();
    }
  }
}

bool Monster::isStunned() const { return stunnedThisTurn; }

bool Monster::canAct() const { return isAlive() && !isStunned(); }

bool Monster::hasStatusEffect(StatusEffectType type) const {
  return std::any_of(
      activeEffects.begin(), activeEffects.end(),
      [type](const ActiveStatusEffect &effect) { return effect.type == type; });
}

void Monster::applyStatusEffect(StatusEffectType type, int duration) {
  activeEffects.push_back({type, duration});
}

void Monster::addTemporaryAttackBuff(int amount, int duration) {
  temporaryAttackBuff = amount;
  attackBuffDuration = duration;
}

void Monster::heal(int amount) {
  if (amount > 0 && isAlive()) {
    health = std::min(maxHealth, health + amount);
  }
}

void Monster::heal() {
  if (isAlive()) {
    health = maxHealth;
  }
}

void Monster::gainExperience(int amount) {
  if (amount > 0) {
    experience += amount;
    while (experience >= experienceToNextLevel) {
      experience -= experienceToNextLevel;
      level++;
      onLevelUp(level);
    }
  }
}

void Monster::takeDamage(int amount, MonsterType sourceType) {
  double multiplier = getTypeMultiplier(sourceType);
  int finalDamage = static_cast<int>(amount * multiplier);
  takeDamage(finalDamage);
}

int Monster::getScaledAttackValue(int baseValue) const {
  return baseValue + temporaryAttackBuff + (level * 2);
}

double Monster::getTypeMultiplier(MonsterType sourceType) const {
  MonsterType myType = getType();
  if (sourceType == myType) {
    return 0.5;
  }

  if (sourceType == MonsterType::Fire && myType == MonsterType::Earth)
    return 2.0;
  if (sourceType == MonsterType::Water && myType == MonsterType::Fire)
    return 2.0;
  if (sourceType == MonsterType::Earth && myType == MonsterType::Water)
    return 2.0;

  if (myType == MonsterType::Fire && sourceType == MonsterType::Earth)
    return 0.5;
  if (myType == MonsterType::Water && sourceType == MonsterType::Fire)
    return 0.5;
  if (myType == MonsterType::Earth && sourceType == MonsterType::Water)
    return 0.5;

  return 1.0;
}

bool Monster::hasAttackNamed(const std::string &attackName) const {
  return std::any_of(attacks.begin(), attacks.end(),
                     [&attackName](const std::unique_ptr<Attack> &attack) {
                       return attack && attack->getName() == attackName;
                     });
}

void Monster::onLevelUp(int newLevel) {
  maxHealth += 10;
  health = maxHealth;
  experienceToNextLevel = newLevel * 100;
}
