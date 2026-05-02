#pragma once

#include <string>

class Player;
class Monster;

class Item
{
public:
    explicit Item(std::string name);
    virtual ~Item() = default;

    const std::string& getName() const;
    virtual void use(Player& owner, Monster& target) = 0;

protected:
    std::string name;
};

class HealingItem : public Item
{
public:
    HealingItem(std::string name, int healingAmount);
    void use(Player& owner, Monster& target) override;

private:
    int healingAmount;
};

class BuffItem : public Item
{
public:
    BuffItem(std::string name, int buffAmount, int duration);
    void use(Player& owner, Monster& target) override;

private:
    int buffAmount;
    int duration;
};

