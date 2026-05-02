#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstdlib>
#include <ctime>

using namespace sf;
using namespace std;

// ============= STRUCTS =============
struct GameMonster {
    string name;
    int health;
    int maxHealth;
    int level;
    int colorR, colorG, colorB;
    string type;

    GameMonster(string n, int hp, int lvl, int r, int g, int b, string t)
        : name(n), health(hp), maxHealth(hp), level(lvl), colorR(r), colorG(g), colorB(b), type(t) {}

    bool isAlive() const { return health > 0; }
    float getHealthPercent() const { return (float)health / maxHealth; }
    void takeDamage(int dmg) { health = max(0, health - dmg); }
    void heal(int amount) { health = min(maxHealth, health + amount); }
};

struct GameAttack {
    string name;
    int damage;
    int cooldown;
    int currentCooldown;

    GameAttack(string n, int d, int cd)
        : name(n), damage(d), cooldown(cd), currentCooldown(0) {}

    bool isReady() const { return currentCooldown == 0; }
    void use() { currentCooldown = cooldown; }
    void tick() { if (currentCooldown > 0) currentCooldown--; }
};

struct GameParticle {
    CircleShape shape;
    Vector2f velocity;
    float lifetime;

    GameParticle(Vector2f pos, Color color, Vector2f vel)
        : shape(4), velocity(vel), lifetime(1.0f) {
        shape.setPosition(pos);
        shape.setFillColor(color);
        shape.setOrigin({ 2, 2 });
    }

    bool update(float dt) {
        lifetime -= dt;
        shape.move(velocity * dt);
        Color col = shape.getFillColor();
        col.a = static_cast<uint8_t>(255 * max(0.0f, lifetime));
        shape.setFillColor(col);
        return lifetime > 0;
    }
};

struct GameDamageNumber {
    Text text;
    float lifetime;
    Vector2f velocity;

    GameDamageNumber(Font& font, int damage, Vector2f pos, bool crit)
        : text(font), lifetime(1.0f), velocity({ 0, -80 }) {
        text.setString("-" + to_string(damage));
        text.setCharacterSize(crit ? 32 : 24);
        text.setFillColor(crit ? Color::Red : Color::White);
        if (crit) text.setStyle(Text::Bold);
        text.setPosition(pos);
    }

    bool update(float dt) {
        lifetime -= dt;
        text.move(velocity * dt);
        velocity.y -= 30 * dt;
        Color col = text.getFillColor();
        col.a = static_cast<uint8_t>(255 * max(0.0f, lifetime));
        text.setFillColor(col);
        return lifetime > 0;
    }
};

// ============= MAIN GAME =============
int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    RenderWindow window(VideoMode({ 1280, 720 }), "MONS ARENA");
    window.setFramerateLimit(60);

    // Font
    Font font;
    font.openFromFile("C:/Windows/Fonts/Arial.ttf");

    // Game state
    enum GameScreen { StartMenu, MonsterSelect, Battle };
    GameScreen currentScreen = StartMenu;

    // Player data
    string playerName = "";
    string selectedMonster = "";
    int selectedColorR = 0, selectedColorG = 0, selectedColorB = 0;
    vector<GameAttack> playerAttacks;
    vector<GameAttack> enemyAttacks;

    // Enemy monster (always Earth type)
    GameMonster enemyMon("TERRA", 200, 5, 90, 70, 50, "Earth");
    enemyAttacks.push_back(GameAttack("Rock Throw", 18, 1));
    enemyAttacks.push_back(GameAttack("Earthquake", 28, 2));
    enemyAttacks.push_back(GameAttack("Stone Shield", 15, 2));
    enemyAttacks.push_back(GameAttack("Mountain Crush", 35, 3));

    // Animation vars
    float screenShakeX = 0, screenShakeY = 0;
    float shakeIntensity = 0;
    float playerDash = 0, enemyDash = 0;
    float flashAlpha = 0;
    float hitStop = 0;

    vector<GameParticle> particles;
    vector<GameDamageNumber> damageNumbers;

    // Battle state
    enum BattleState { PlayerTurn, AITurn, Animating };
    BattleState battleState = PlayerTurn;
    string battleMessage = "";
    float messageTimer = 0;
    GameMonster* currentPlayerMon = nullptr;

    Clock clock;

    // ============= START MENU UI =============
    Text titleText(font);
    titleText.setString("M O N S   A R E N A");
    titleText.setCharacterSize(64);
    titleText.setFillColor(Color::Yellow);
    titleText.setStyle(Text::Bold);
    titleText.setPosition({ 640, 150 });
    FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin({ titleBounds.size.x / 2, 0 });

    Text promptText(font);
    promptText.setString("Enter Your Name:");
    promptText.setCharacterSize(28);
    promptText.setFillColor(Color::White);
    promptText.setPosition({ 640, 280 });
    FloatRect promptBounds = promptText.getLocalBounds();
    promptText.setOrigin({ promptBounds.size.x / 2, 0 });

    RectangleShape nameBox({ 400, 50 });
    nameBox.setFillColor(Color(40, 40, 60));
    nameBox.setOutlineThickness(2);
    nameBox.setOutlineColor(Color::White);
    nameBox.setPosition({ 440, 330 });

    Text nameInput(font);
    nameInput.setCharacterSize(24);
    nameInput.setFillColor(Color::White);
    nameInput.setPosition({ 450, 338 });

    RectangleShape startButton({ 200, 50 });
    startButton.setFillColor(Color(50, 100, 50));
    startButton.setOutlineThickness(2);
    startButton.setOutlineColor(Color::White);
    startButton.setPosition({ 540, 420 });

    Text startText(font);
    startText.setString("START");
    startText.setCharacterSize(24);
    startText.setFillColor(Color::White);
    startText.setStyle(Text::Bold);
    startText.setPosition({ 640, 435 });
    FloatRect startTextBounds = startText.getLocalBounds();
    startText.setOrigin({ startTextBounds.size.x / 2, 0 });

    bool isTyping = true;

    // ============= MONSTER SELECT UI =============
    Text selectTitle(font);
    selectTitle.setString("CHOOSE YOUR MONSTER");
    selectTitle.setCharacterSize(48);
    selectTitle.setFillColor(Color::Yellow);
    selectTitle.setStyle(Text::Bold);
    selectTitle.setPosition({ 640, 80 });
    FloatRect selectTitleBounds = selectTitle.getLocalBounds();
    selectTitle.setOrigin({ selectTitleBounds.size.x / 2, 0 });

    // Monster cards struct with constructor
    struct MonsterCard {
        RectangleShape rect;
        CircleShape circle;
        Text name;
        Text type;
        Text hp;
        bool selected;

        MonsterCard(Font& f)
            : rect({ 300, 400 })
            , circle(80)
            , name(f)
            , type(f)
            , hp(f)
            , selected(false) {}
    };

    vector<MonsterCard> monsterCards;

    // Fire Monster
    MonsterCard fireCard(font);
    fireCard.rect.setFillColor(Color(40, 40, 60));
    fireCard.rect.setOutlineThickness(3);
    fireCard.rect.setOutlineColor(Color(220, 60, 50));
    fireCard.rect.setPosition({ 150, 200 });

    fireCard.circle.setFillColor(Color(220, 60, 50));
    fireCard.circle.setPosition({ 230, 240 });
    fireCard.circle.setOutlineThickness(3);
    fireCard.circle.setOutlineColor(Color::White);

    fireCard.name.setString("IGNIS");
    fireCard.name.setCharacterSize(28);
    fireCard.name.setFillColor(Color::White);
    fireCard.name.setStyle(Text::Bold);
    fireCard.name.setPosition({ 300, 380 });
    FloatRect fireNameBounds = fireCard.name.getLocalBounds();
    fireCard.name.setOrigin({ fireNameBounds.size.x / 2, 0 });

    fireCard.type.setString("Fire Type");
    fireCard.type.setCharacterSize(18);
    fireCard.type.setFillColor(Color(255, 150, 100));
    fireCard.type.setPosition({ 300, 420 });
    FloatRect fireTypeBounds = fireCard.type.getLocalBounds();
    fireCard.type.setOrigin({ fireTypeBounds.size.x / 2, 0 });

    fireCard.hp.setString("HP: 180");
    fireCard.hp.setCharacterSize(16);
    fireCard.hp.setFillColor(Color::White);
    fireCard.hp.setPosition({ 300, 460 });
    FloatRect fireHpBounds = fireCard.hp.getLocalBounds();
    fireCard.hp.setOrigin({ fireHpBounds.size.x / 2, 0 });

    monsterCards.push_back(fireCard);

    // Water Monster
    MonsterCard waterCard(font);
    waterCard.rect.setFillColor(Color(40, 40, 60));
    waterCard.rect.setOutlineThickness(3);
    waterCard.rect.setOutlineColor(Color(50, 100, 220));
    waterCard.rect.setPosition({ 490, 200 });

    waterCard.circle.setFillColor(Color(50, 100, 220));
    waterCard.circle.setPosition({ 570, 240 });
    waterCard.circle.setOutlineThickness(3);
    waterCard.circle.setOutlineColor(Color::White);

    waterCard.name.setString("AQUA");
    waterCard.name.setCharacterSize(28);
    waterCard.name.setFillColor(Color::White);
    waterCard.name.setStyle(Text::Bold);
    waterCard.name.setPosition({ 640, 380 });
    FloatRect waterNameBounds = waterCard.name.getLocalBounds();
    waterCard.name.setOrigin({ waterNameBounds.size.x / 2, 0 });

    waterCard.type.setString("Water Type");
    waterCard.type.setCharacterSize(18);
    waterCard.type.setFillColor(Color(100, 150, 255));
    waterCard.type.setPosition({ 640, 420 });
    FloatRect waterTypeBounds = waterCard.type.getLocalBounds();
    waterCard.type.setOrigin({ waterTypeBounds.size.x / 2, 0 });

    waterCard.hp.setString("HP: 170");
    waterCard.hp.setCharacterSize(16);
    waterCard.hp.setFillColor(Color::White);
    waterCard.hp.setPosition({ 640, 460 });
    FloatRect waterHpBounds = waterCard.hp.getLocalBounds();
    waterCard.hp.setOrigin({ waterHpBounds.size.x / 2, 0 });

    monsterCards.push_back(waterCard);

    // Earth Monster
    MonsterCard earthCard(font);
    earthCard.rect.setFillColor(Color(40, 40, 60));
    earthCard.rect.setOutlineThickness(3);
    earthCard.rect.setOutlineColor(Color(90, 70, 50));
    earthCard.rect.setPosition({ 830, 200 });

    earthCard.circle.setFillColor(Color(90, 70, 50));
    earthCard.circle.setPosition({ 910, 240 });
    earthCard.circle.setOutlineThickness(3);
    earthCard.circle.setOutlineColor(Color::White);

    earthCard.name.setString("TERRA");
    earthCard.name.setCharacterSize(28);
    earthCard.name.setFillColor(Color::White);
    earthCard.name.setStyle(Text::Bold);
    earthCard.name.setPosition({ 980, 380 });
    FloatRect earthNameBounds = earthCard.name.getLocalBounds();
    earthCard.name.setOrigin({ earthNameBounds.size.x / 2, 0 });

    earthCard.type.setString("Earth Type");
    earthCard.type.setCharacterSize(18);
    earthCard.type.setFillColor(Color(150, 120, 80));
    earthCard.type.setPosition({ 980, 420 });
    FloatRect earthTypeBounds = earthCard.type.getLocalBounds();
    earthCard.type.setOrigin({ earthTypeBounds.size.x / 2, 0 });

    earthCard.hp.setString("HP: 200");
    earthCard.hp.setCharacterSize(16);
    earthCard.hp.setFillColor(Color::White);
    earthCard.hp.setPosition({ 980, 460 });
    FloatRect earthHpBounds = earthCard.hp.getLocalBounds();
    earthCard.hp.setOrigin({ earthHpBounds.size.x / 2, 0 });

    monsterCards.push_back(earthCard);

    RectangleShape confirmButton({ 200, 50 });
    confirmButton.setFillColor(Color(50, 100, 50));
    confirmButton.setOutlineThickness(2);
    confirmButton.setOutlineColor(Color::White);
    confirmButton.setPosition({ 540, 640 });

    Text confirmText(font);
    confirmText.setString("FIGHT!");
    confirmText.setCharacterSize(24);
    confirmText.setFillColor(Color::White);
    confirmText.setStyle(Text::Bold);
    confirmText.setPosition({ 640, 650 });
    FloatRect confirmTextBounds = confirmText.getLocalBounds();
    confirmText.setOrigin({ confirmTextBounds.size.x / 2, 0 });

    // ============= BATTLE UI =============
    RectangleShape playerHealthBg({ 400, 28 });
    playerHealthBg.setFillColor(Color(30, 30, 40));
    playerHealthBg.setPosition({ 80, 100 });
    playerHealthBg.setOutlineThickness(2);
    playerHealthBg.setOutlineColor(Color::White);

    RectangleShape playerHealth({ 400, 28 });
    playerHealth.setFillColor(Color::Green);
    playerHealth.setPosition({ 80, 100 });

    RectangleShape enemyHealthBg({ 400, 28 });
    enemyHealthBg.setFillColor(Color(30, 30, 40));
    enemyHealthBg.setPosition({ 800, 100 });
    enemyHealthBg.setOutlineThickness(2);
    enemyHealthBg.setOutlineColor(Color::White);

    RectangleShape enemyHealth({ 400, 28 });
    enemyHealth.setFillColor(Color::Red);
    enemyHealth.setPosition({ 800, 100 });

    CircleShape playerCircle(90);
    playerCircle.setPosition({ 150, 280 });
    playerCircle.setOutlineThickness(4);
    playerCircle.setOutlineColor(Color::White);

    CircleShape enemyCircle(90);
    enemyCircle.setPosition({ 1040, 280 });
    enemyCircle.setFillColor(Color(enemyMon.colorR, enemyMon.colorG, enemyMon.colorB));
    enemyCircle.setOutlineThickness(4);
    enemyCircle.setOutlineColor(Color::White);

    CircleShape playerGlow(100);
    playerGlow.setFillColor(Color(255, 255, 255, 30));
    playerGlow.setPosition({ 140, 270 });

    CircleShape enemyGlow(100);
    enemyGlow.setFillColor(Color(255, 255, 255, 30));
    enemyGlow.setPosition({ 1030, 270 });

    Text playerNameText(font);
    playerNameText.setCharacterSize(24);
    playerNameText.setFillColor(Color::White);
    playerNameText.setStyle(Text::Bold);
    playerNameText.setPosition({ 80, 60 });

    Text playerLevelText(font);
    playerLevelText.setCharacterSize(14);
    playerLevelText.setFillColor(Color::Yellow);
    playerLevelText.setPosition({ 80, 85 });

    Text enemyNameText(font);
    enemyNameText.setString(enemyMon.name);
    enemyNameText.setCharacterSize(24);
    enemyNameText.setFillColor(Color::White);
    enemyNameText.setStyle(Text::Bold);
    enemyNameText.setPosition({ 800, 60 });

    Text enemyLevelText(font);
    enemyLevelText.setString("Lv." + to_string(enemyMon.level));
    enemyLevelText.setCharacterSize(14);
    enemyLevelText.setFillColor(Color::Yellow);
    enemyLevelText.setPosition({ 800, 85 });

    Text playerHealthText(font);
    playerHealthText.setCharacterSize(14);
    playerHealthText.setFillColor(Color(200, 200, 200));
    playerHealthText.setPosition({ 80, 132 });

    Text enemyHealthText(font);
    enemyHealthText.setCharacterSize(14);
    enemyHealthText.setFillColor(Color(200, 200, 200));
    enemyHealthText.setPosition({ 800, 132 });

    Text battleMessageText(font);
    battleMessageText.setCharacterSize(20);
    battleMessageText.setFillColor(Color::Cyan);
    battleMessageText.setStyle(Text::Bold);
    battleMessageText.setPosition({ 640, 450 });

    vector<RectangleShape> buttons;
    vector<Text> buttonTexts;

    for (int i = 0; i < 4; i++) {
        RectangleShape rect({ 280, 55 });
        rect.setPosition({ 500, static_cast<float>(530 + i * 65) });
        rect.setFillColor(Color(45, 50, 70));
        rect.setOutlineThickness(2);
        rect.setOutlineColor(Color(100, 110, 140));
        buttons.push_back(rect);

        Text txt(font);
        txt.setCharacterSize(18);
        txt.setFillColor(Color::White);
        txt.setStyle(Text::Bold);
        txt.setPosition({ 510, static_cast<float>(538 + i * 65) });
        buttonTexts.push_back(txt);
    }

    // ============= MAIN LOOP =============
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = min(dt, 0.033f);

        // Update animations (only in battle)
        if (currentScreen == Battle && currentPlayerMon != nullptr) {
            if (shakeIntensity > 0) {
                screenShakeX = ((rand() % 100) / 50.0f - 1.0f) * shakeIntensity;
                screenShakeY = ((rand() % 100) / 50.0f - 1.0f) * shakeIntensity;
                shakeIntensity -= dt * 15;
                if (shakeIntensity < 0) shakeIntensity = 0;
            }
            else {
                screenShakeX = 0;
                screenShakeY = 0;
            }

            playerDash = max(0.0f, playerDash - dt * 8);
            enemyDash = max(0.0f, enemyDash - dt * 8);
            flashAlpha = max(0.0f, flashAlpha - dt * 5);
            if (hitStop > 0) hitStop -= dt;

            if (messageTimer > 0) {
                messageTimer -= dt;
                if (messageTimer <= 0 && battleState == PlayerTurn) {
                    battleMessage = "Your turn! Choose an attack...";
                }
            }

            for (auto it = particles.begin(); it != particles.end();) {
                if (!it->update(dt)) it = particles.erase(it);
                else ++it;
            }

            for (auto it = damageNumbers.begin(); it != damageNumbers.end();) {
                if (!it->update(dt)) it = damageNumbers.erase(it);
                else ++it;
            }

            if (hitStop <= 0 && battleState != Animating) {
                for (auto& a : playerAttacks) a.tick();
                for (auto& a : enemyAttacks) a.tick();
            }
        }

        // Handle events
        while (auto event = window.pollEvent()) {
            if (event->is<Event::Closed>()) window.close();

            if (auto pressed = event->getIf<Event::MouseButtonPressed>()) {
                if (pressed->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(pressed->position.x), static_cast<float>(pressed->position.y));

                    if (currentScreen == StartMenu) {
                        if (nameBox.getGlobalBounds().contains(mousePos)) {
                            isTyping = true;
                        }
                        else if (startButton.getGlobalBounds().contains(mousePos) && !playerName.empty()) {
                            currentScreen = MonsterSelect;
                        }
                        else {
                            isTyping = false;
                        }
                    }
                    else if (currentScreen == MonsterSelect) {
                        for (size_t i = 0; i < monsterCards.size(); i++) {
                            if (monsterCards[i].rect.getGlobalBounds().contains(mousePos)) {
                                for (auto& card : monsterCards) card.selected = false;
                                monsterCards[i].selected = true;

                                if (i == 0) {
                                    selectedMonster = "IGNIS";
                                    selectedColorR = 220; selectedColorG = 60; selectedColorB = 50;
                                    playerAttacks.clear();
                                    playerAttacks.push_back(GameAttack("Flame Strike", 22, 1));
                                    playerAttacks.push_back(GameAttack("Fireball", 30, 2));
                                    playerAttacks.push_back(GameAttack("Heat Wave", 18, 1));
                                    playerAttacks.push_back(GameAttack("Inferno", 40, 3));
                                }
                                else if (i == 1) {
                                    selectedMonster = "AQUA";
                                    selectedColorR = 50; selectedColorG = 100; selectedColorB = 220;
                                    playerAttacks.clear();
                                    playerAttacks.push_back(GameAttack("Water Jet", 20, 1));
                                    playerAttacks.push_back(GameAttack("Tidal Wave", 28, 2));
                                    playerAttacks.push_back(GameAttack("Healing Rain", 15, 2));
                                    playerAttacks.push_back(GameAttack("Tsunami", 38, 3));
                                }
                                else {
                                    selectedMonster = "TERRA";
                                    selectedColorR = 90; selectedColorG = 70; selectedColorB = 50;
                                    playerAttacks.clear();
                                    playerAttacks.push_back(GameAttack("Rock Throw", 18, 1));
                                    playerAttacks.push_back(GameAttack("Earthquake", 28, 2));
                                    playerAttacks.push_back(GameAttack("Stone Shield", 15, 2));
                                    playerAttacks.push_back(GameAttack("Mountain Crush", 35, 3));
                                }
                                break;
                            }
                        }

                        if (confirmButton.getGlobalBounds().contains(mousePos) && !selectedMonster.empty()) {
                            int health = (selectedMonster == "IGNIS") ? 180 : (selectedMonster == "AQUA") ? 170 : 200;
                            currentPlayerMon = new GameMonster(selectedMonster, health, 5, selectedColorR, selectedColorG, selectedColorB,
                                (selectedMonster == "IGNIS") ? "Fire" : (selectedMonster == "AQUA") ? "Water" : "Earth");

                            playerCircle.setFillColor(Color(selectedColorR, selectedColorG, selectedColorB));
                            playerNameText.setString(selectedMonster);

                            battleState = PlayerTurn;
                            battleMessage = "Battle started! " + playerName + ", choose an attack...";
                            currentScreen = Battle;
                        }
                    }
                    else if (currentScreen == Battle && battleState == PlayerTurn && currentPlayerMon != nullptr && currentPlayerMon->isAlive() && enemyMon.isAlive()) {
                        for (size_t i = 0; i < buttons.size(); i++) {
                            if (buttons[i].getGlobalBounds().contains(mousePos) && i < playerAttacks.size()) {
                                if (playerAttacks[i].isReady()) {
                                    battleState = Animating;
                                    battleMessage = currentPlayerMon->name + " used " + playerAttacks[i].name + "!";
                                    messageTimer = 1.5f;

                                    playerDash = 15;
                                    shakeIntensity = 8;
                                    flashAlpha = 0.6f;
                                    hitStop = 0.1f;

                                    Vector2f targetPos(enemyCircle.getPosition().x + 90, enemyCircle.getPosition().y + 90);
                                    for (int p = 0; p < 20; p++) {
                                        float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
                                        float speed = static_cast<float>(rand() % 200 + 100);
                                        particles.push_back(GameParticle(targetPos, Color(255, 100, 100),
                                            Vector2f(cos(angle) * speed, sin(angle) * speed - 100)));
                                    }

                                    int damage = playerAttacks[i].damage;
                                    bool crit = (rand() % 100) < 15;
                                    if (crit) damage = static_cast<int>(damage * 1.5f);

                                    int oldHealth = enemyMon.health;
                                    enemyMon.takeDamage(damage);
                                    int actualDamage = oldHealth - enemyMon.health;

                                    damageNumbers.push_back(GameDamageNumber(font, actualDamage,
                                        Vector2f(targetPos.x - 30, targetPos.y - 20), crit));

                                    if (crit) battleMessage += " CRITICAL HIT!";

                                    playerAttacks[i].use();

                                    if (enemyMon.isAlive()) battleState = AITurn;
                                    else battleState = PlayerTurn;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (auto textEntered = event->getIf<Event::TextEntered>()) {
                if (currentScreen == StartMenu && isTyping) {
                    if (textEntered->unicode < 128 && textEntered->unicode != 8 && textEntered->unicode != 13) {
                        if (playerName.length() < 20) playerName += static_cast<char>(textEntered->unicode);
                    }
                    else if (textEntered->unicode == 8 && !playerName.empty()) {
                        playerName.pop_back();
                    }
                    else if (textEntered->unicode == 13 && !playerName.empty()) {
                        currentScreen = MonsterSelect;
                        isTyping = false;
                    }
                    nameInput.setString(playerName + (isTyping ? "_" : ""));
                }
            }
        }

        // AI Turn
        if (currentScreen == Battle && battleState == AITurn && enemyMon.isAlive() && currentPlayerMon != nullptr && currentPlayerMon->isAlive() && hitStop <= 0) {
            int attackIndex = -1;
            for (size_t i = 0; i < enemyAttacks.size(); i++) {
                if (enemyAttacks[i].isReady()) {
                    attackIndex = static_cast<int>(i);
                    break;
                }
            }

            if (attackIndex != -1) {
                battleMessage = enemyMon.name + " used " + enemyAttacks[attackIndex].name + "!";
                messageTimer = 1.5f;
                enemyDash = 15;
                shakeIntensity = 8;
                flashAlpha = 0.6f;
                hitStop = 0.1f;

                Vector2f targetPos(playerCircle.getPosition().x + 90, playerCircle.getPosition().y + 90);
                for (int p = 0; p < 20; p++) {
                    float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
                    float speed = static_cast<float>(rand() % 200 + 100);
                    particles.push_back(GameParticle(targetPos, Color(150, 100, 50),
                        Vector2f(cos(angle) * speed, sin(angle) * speed - 100)));
                }

                int oldHealth = currentPlayerMon->health;
                currentPlayerMon->takeDamage(enemyAttacks[attackIndex].damage);
                int actualDamage = oldHealth - currentPlayerMon->health;
                damageNumbers.push_back(GameDamageNumber(font, actualDamage,
                    Vector2f(targetPos.x - 30, targetPos.y - 20), false));

                enemyAttacks[attackIndex].use();
            }
            battleState = PlayerTurn;
        }

        // Update UI for battle
        if (currentScreen == Battle && currentPlayerMon != nullptr) {
            float targetPlayerWidth = 400 * currentPlayerMon->getHealthPercent();
            float curPlayerWidth = playerHealth.getSize().x;
            playerHealth.setSize({ curPlayerWidth + (targetPlayerWidth - curPlayerWidth) * 0.2f, 28 });

            float targetEnemyWidth = 400 * enemyMon.getHealthPercent();
            float curEnemyWidth = enemyHealth.getSize().x;
            enemyHealth.setSize({ curEnemyWidth + (targetEnemyWidth - curEnemyWidth) * 0.2f, 28 });

            if (currentPlayerMon->getHealthPercent() > 0.5f) playerHealth.setFillColor(Color::Green);
            else if (currentPlayerMon->getHealthPercent() > 0.25f) playerHealth.setFillColor(Color::Yellow);
            else playerHealth.setFillColor(Color::Red);

            if (enemyMon.getHealthPercent() > 0.5f) enemyHealth.setFillColor(Color::Green);
            else if (enemyMon.getHealthPercent() > 0.25f) enemyHealth.setFillColor(Color::Yellow);
            else enemyHealth.setFillColor(Color::Red);

            playerLevelText.setString("Lv." + to_string(currentPlayerMon->level));
            playerHealthText.setString("HP: " + to_string(currentPlayerMon->health) + "/" + to_string(currentPlayerMon->maxHealth));
            enemyHealthText.setString("HP: " + to_string(enemyMon.health) + "/" + to_string(enemyMon.maxHealth));

            for (size_t i = 0; i < buttonTexts.size() && i < playerAttacks.size(); i++) {
                buttonTexts[i].setString(playerAttacks[i].name);
                if (!playerAttacks[i].isReady()) {
                    buttonTexts[i].setString(playerAttacks[i].name + " (" + to_string(playerAttacks[i].currentCooldown) + ")");
                }
            }

            battleMessageText.setString(battleMessage);
            FloatRect msgBounds = battleMessageText.getLocalBounds();
            battleMessageText.setOrigin({ msgBounds.size.x / 2, 0 });
        }

        // ============ DRAW EVERYTHING ============
        window.clear(Color(20, 25, 45));

        if (currentScreen == StartMenu) {
            RectangleShape bg({ 1280, 720 });
            bg.setFillColor(Color(20, 25, 45));
            window.draw(bg);

            window.draw(titleText);
            window.draw(promptText);
            window.draw(nameBox);
            window.draw(nameInput);
            window.draw(startButton);
            window.draw(startText);
        }
        else if (currentScreen == MonsterSelect) {
            RectangleShape bg({ 1280, 720 });
            bg.setFillColor(Color(20, 25, 45));
            window.draw(bg);

            window.draw(selectTitle);

            for (auto& card : monsterCards) {
                if (card.selected) {
                    card.rect.setOutlineThickness(5);
                    card.rect.setOutlineColor(Color::Yellow);
                }
                else {
                    card.rect.setOutlineThickness(3);
                }
                window.draw(card.rect);
                window.draw(card.circle);
                window.draw(card.name);
                window.draw(card.type);
                window.draw(card.hp);
            }

            window.draw(confirmButton);
            window.draw(confirmText);
        }
        else if (currentScreen == Battle && currentPlayerMon != nullptr) {
            View view = window.getDefaultView();
            view.setCenter({ 640 + screenShakeX, 360 + screenShakeY });
            window.setView(view);

            float playerXOffset = playerDash * 5;
            float enemyXOffset = enemyDash * -5;

            RectangleShape bg({ 1280, 720 });
            bg.setFillColor(Color(20, 25, 45));
            window.draw(bg);

            window.draw(playerHealthBg);
            window.draw(playerHealth);
            window.draw(enemyHealthBg);
            window.draw(enemyHealth);

            window.draw(playerGlow);
            window.draw(enemyGlow);

            playerCircle.setPosition({ 150 + playerXOffset, 280 });
            enemyCircle.setPosition({ 1040 + enemyXOffset, 280 });

            window.draw(playerCircle);
            window.draw(enemyCircle);

            window.draw(playerNameText);
            window.draw(playerLevelText);
            window.draw(enemyNameText);
            window.draw(enemyLevelText);
            window.draw(playerHealthText);
            window.draw(enemyHealthText);
            window.draw(battleMessageText);

            for (size_t i = 0; i < buttons.size(); i++) {
                window.draw(buttons[i]);
                window.draw(buttonTexts[i]);
            }

            for (auto& p : particles) window.draw(p.shape);
            for (auto& dn : damageNumbers) window.draw(dn.text);

            if (flashAlpha > 0) {
                RectangleShape flash({ 1280, 720 });
                flash.setFillColor(Color(255, 255, 255, static_cast<uint8_t>(flashAlpha * 200)));
                window.draw(flash);
            }

            if (!currentPlayerMon->isAlive()) {
                Text endText(font);
                endText.setString("DEFEAT! Close window to exit...");
                endText.setCharacterSize(32);
                endText.setFillColor(Color::Red);
                endText.setStyle(Text::Bold);
                endText.setPosition({ 640, 360 });
                FloatRect endBounds = endText.getLocalBounds();
                endText.setOrigin({ endBounds.size.x / 2, 0 });
                window.draw(endText);
            }
            else if (!enemyMon.isAlive()) {
                Text endText(font);
                endText.setString("VICTORY! Close window to exit...");
                endText.setCharacterSize(32);
                endText.setFillColor(Color::Green);
                endText.setStyle(Text::Bold);
                endText.setPosition({ 640, 360 });
                FloatRect endBounds = endText.getLocalBounds();
                endText.setOrigin({ endBounds.size.x / 2, 0 });
                window.draw(endText);
            }
        }

        window.display();

        if (hitStop > 0 && currentScreen == Battle) {
            sf::sleep(milliseconds(static_cast<int>(hitStop * 100)));
        }
    }

    delete currentPlayerMon;
    return 0;
}