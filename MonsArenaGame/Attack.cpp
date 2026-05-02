#include "Attack.h"
#include <algorithm>
#include "Monster.h"

Attack::Attack(std::string name, int damage, int cooldownTurns)
    : name(std::move(name)), damage(damage), cooldown(cooldownTurns), remainingCooldown(0)
{
}

const std::string& Attack::getName() const
{
    return name;
}

int Attack::getDamage() const
{
    return damage;
}

int Attack::getCooldown() const
{
    return cooldown;
}

int Attack::getRemainingCooldown() const
{
    return remainingCooldown;
}

bool Attack::isReady() const
{
    return remainingCooldown == 0;
}

void Attack::startCooldown()
{
    remainingCooldown = cooldown;
}

void Attack::tickCooldown()
{
    remainingCooldown = std::max(0, remainingCooldown - 1);
}

DamageAttack::DamageAttack(std::string name, int damage, int cooldownTurns)
    : Attack(std::move(name), damage, cooldownTurns)
{
}

void DamageAttack::execute(Monster& attacker, Monster& target)
{
    target.takeDamage(attacker.getScaledAttackValue(damage), attacker.getType());
}

HealAttack::HealAttack(std::string name, int healAmount, int cooldownTurns)
    : Attack(std::move(name), healAmount, cooldownTurns)
{
}

void HealAttack::execute(Monster& attacker, Monster&)
{
    attacker.heal(attacker.getScaledAttackValue(damage));
}

StatusAttack::StatusAttack(std::string name, int baseDamage, int cooldownTurns, StatusEffectType effectType, int effectDuration)
    : Attack(std::move(name), baseDamage, cooldownTurns), effectType(effectType), effectDuration(effectDuration)
{
}

void StatusAttack::execute(Monster& attacker, Monster& target)
{
    if (damage > 0)
    {
        target.takeDamage(attacker.getScaledAttackValue(damage), attacker.getType());
    }

    target.applyStatusEffect(effectType, effectDuration);
}
