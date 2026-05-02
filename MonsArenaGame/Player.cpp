#include "Player.h"

Player::Player(std::string name, std::unique_ptr<Monster> monster)
    : name(std::move(name)), monster(std::move(monster))
{
}

const std::string& Player::getName() const { return name; }

Monster& Player::getMonster() { return *monster; }
const Monster& Player::getMonster() const { return *monster; }

void Player::chooseAttack(size_t attackIndex, Monster& target)
{
    if (monster && monster->canAct())
    {
        monster->performAttack(attackIndex, target);
    }
}

void Player::useMonsterSpecial(Monster& target)
{
    if (monster && monster->canAct())
    {
        monster->useSpecialAbility(target);
    }
}

void Player::addItem(std::unique_ptr<Item> item)
{
    if (item)
    {
        inventory.push_back(std::move(item));
    }
}

bool Player::useItem(size_t itemIndex, Monster& target)
{
    if (itemIndex < inventory.size() && monster && monster->canAct())
    {
        inventory[itemIndex]->use(*this, target);
        inventory.erase(inventory.begin() + itemIndex);
        return true;
    }
    return false;
}

size_t Player::getItemCount() const { return inventory.size(); }

std::string Player::getItemName(size_t itemIndex) const
{
    if (itemIndex < inventory.size())
    {
        return inventory[itemIndex]->getName();
    }
    return "";
}

bool Player::hasAvailableActions() const
{
    return monster && monster->canAct();
}
