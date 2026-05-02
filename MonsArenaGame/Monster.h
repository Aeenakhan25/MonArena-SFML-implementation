#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Attack.h"

enum class MonsterType
{
    Fire,
    Water,
    Earth
};

class Monster
{
public:
    Monster(std::string name, int maxHealth, int level);
    virtual ~Monster() = default;

    const std::string& getName() const;
    int getHealth() const;
    int getMaxHealth() const;
    int getLevel() const;
    int getExperience() const;
    int getExperienceToNextLevel() const;
    bool isAlive() const;

    static int getTotalMonstersCreated();

    void addAttack(std::unique_ptr<Attack> attack);
    size_t getAttackCount() const;
    Attack* getAttack(size_t index);
    const Attack* getAttack(size_t index) const;

    virtual MonsterType getType() const = 0;
    virtual void useSpecialAbility(Monster& target) = 0;
    virtual void takeDamage(int amount);
    virtual void performAttack(size_t attackIndex, Monster& target);

    void processTurnStart();
    bool isStunned() const;
    bool canAct() const;
    bool hasStatusEffect(StatusEffectType type) const;

    void applyStatusEffect(StatusEffectType type, int duration);
    void addTemporaryAttackBuff(int amount, int duration);
    void heal(int amount);
    void heal();
    void gainExperience(int amount);

    // Overload example: raw damage vs type-aware damage.
    void takeDamage(int amount, MonsterType sourceType);
    int getScaledAttackValue(int baseValue) const;

protected:
    double getTypeMultiplier(MonsterType sourceType) const;
    bool hasAttackNamed(const std::string& attackName) const;
    virtual void onLevelUp(int newLevel);

private:
    struct ActiveStatusEffect
    {
        StatusEffectType type;
        int remainingTurns;
    };

    std::string name;
    int health;
    int maxHealth;
    int level;
    int experience;
    int experienceToNextLevel;
    int temporaryAttackBuff;
    int attackBuffDuration;
    bool stunnedThisTurn;
    std::vector<ActiveStatusEffect> activeEffects;
    std::vector<std::unique_ptr<Attack>> attacks;

    static int totalMonstersCreated;
};

