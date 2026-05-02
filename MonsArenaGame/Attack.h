#pragma once

#include <string>

class Monster;

enum class StatusEffectType
{
    Burn,
    Stun
};

class Attack
{
public:
    Attack(std::string name, int damage, int cooldownTurns);
    virtual ~Attack() = default;

    const std::string& getName() const;
    int getDamage() const;
    int getCooldown() const;
    int getRemainingCooldown() const;
    bool isReady() const;

    void startCooldown();
    void tickCooldown();

    virtual void execute(Monster& attacker, Monster& target) = 0;

protected:
    std::string name;
    int damage;
    int cooldown;
    int remainingCooldown;
};

class DamageAttack : public Attack
{
public:
    DamageAttack(std::string name, int damage, int cooldownTurns);
    void execute(Monster& attacker, Monster& target) override;
};

class HealAttack : public Attack
{
public:
    HealAttack(std::string name, int healAmount, int cooldownTurns);
    void execute(Monster& attacker, Monster& target) override;
};

class StatusAttack : public Attack
{
public:
    StatusAttack(std::string name, int baseDamage, int cooldownTurns, StatusEffectType effectType, int effectDuration);
    void execute(Monster& attacker, Monster& target) override;

private:
    StatusEffectType effectType;
    int effectDuration;
};
