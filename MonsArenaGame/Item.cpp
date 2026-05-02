#include "Item.h"
#include "Player.h"
#include "Monster.h"

Item::Item(std::string name) : name(std::move(name)) {}

const std::string& Item::getName() const { return name; }

HealingItem::HealingItem(std::string name, int healingAmount)
    : Item(std::move(name)), healingAmount(healingAmount) {}

void HealingItem::use(Player& owner, Monster& target)
{
    owner.getMonster().heal(healingAmount);
}

BuffItem::BuffItem(std::string name, int buffAmount, int duration)
    : Item(std::move(name)), buffAmount(buffAmount), duration(duration) {}

void BuffItem::use(Player& owner, Monster& target)
{
    owner.getMonster().addTemporaryAttackBuff(buffAmount, duration);
}
