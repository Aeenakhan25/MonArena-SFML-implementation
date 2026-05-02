#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "Item.h"
#include "Monster.h"

class Player
{
public:
    Player(std::string name, std::unique_ptr<Monster> monster);

    const std::string& getName() const;
    Monster& getMonster();
    const Monster& getMonster() const;

    void chooseAttack(size_t attackIndex, Monster& target);
    void useMonsterSpecial(Monster& target);
    void addItem(std::unique_ptr<Item> item);
    bool useItem(size_t itemIndex, Monster& target);
    size_t getItemCount() const;
    std::string getItemName(size_t itemIndex) const;
    bool hasAvailableActions() const;

private:
    std::string name;
    std::unique_ptr<Monster> monster;
    std::vector<std::unique_ptr<Item>> inventory;
};

