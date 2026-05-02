# MonsArena

## OVERVIEW
MonsArena is a fully playable turn-based monster battle game written in C++17
using the SFML graphics library. Players choose a custom name and one of three
elemental monsters — Fire, Water, or Earth — before entering a head-to-head
battle against an AI-controlled Earth monster named TERRA.

The project is structured as a dual-layer codebase:

  [1] A rich backend engine built with proper OOP — polymorphic Monster types,
      abstract interfaces, a BattleManager, item system, status effects, XP
      gain, and event-driven battle callbacks.

  [2] A self-contained graphical frontend in main.cpp that drives the SFML
      window, renders animated sprites, health bars, particles, floating damage
      numbers, screen shake, hit-stop, and all three game screens.

The game runs at 60 FPS in a 1280x720 window and is compiled as a Visual Studio
C++ project.

---

## HIGHLIGHTS
★  Three selectable elemental monsters (IGNIS / AQUA / TERRA), each with a
   unique attack set and type-advantage multipliers.

★  Polished visual feedback — screen shake, flash overlays, particle bursts,
   floating damage numbers, and smooth health bar interpolation.

★  Critical hit system with 15% proc chance and 1.5× damage multiplier,
   displayed with bold red text.

★  Attack cooldown system — each move has an individual cooldown shown live
   on the battle buttons.

★  Full OOP backend: abstract Monster, polymorphic Attack variants
   (Damage / Heal / Status), IAiController interface, IBattleEvents observer,
   Item hierarchy, and GameSession facade.

★  Status effect engine — Burn and Stun affect monster actions per turn.

★  AI opponent that cycles through ready attacks each round with hit-stop
   synchronisation before acting.

★  XP and level-up hooks on the Monster class for future progression features.
---

## FEATURES

```
FEATURES/
├── screens/
│   ├── StartMenu       — Name input with text cursor, keyboard & click support
│   ├── MonsterSelect   — Hoverable / selectable monster cards with stat preview
│   └── BattleArena     — Full real-time animated combat screen
│
├── battle-mechanics/
│   ├── PlayerTurn      — Click any of 4 attack buttons to act
│   ├── AiTurn          — Enemy auto-selects first available (non-cooldown) move
│   ├── CooldownSystem  — Per-attack turn counter displayed inline on buttons
│   ├── CriticalHits    — 15% chance → ×1.5 damage, red bold damage number
│   ├── TypeAdvantage   — Multipliers computed via getTypeMultiplier()
│   └── StatusEffects   — Burn (DoT) and Stun (skip turn) with duration tracking
│
├── visuals/
│   ├── Particles       — Circular burst spawned on hit with fade + gravity
│   ├── DamageNumbers   — Floating text (white = normal, red+bold = crit)
│   ├── ScreenShake     — Random offset applied to View on attack land
│   ├── FlashOverlay    — White translucent rectangle fade on impact
│   ├── HitStop         — Brief frame-freeze for punch feel
│   ├── MonsterDash     — Horizontal lunge animation toward target
│   └── HealthBars      — Smooth lerp + colour shift (green → yellow → red)
│
├── items/
│   ├── HealingItem     — Restores HP to monster
│   └── BuffItem        — Temporary attack stat boost with turn duration
│
└── audio/
    └── (planned — see Future Improvements)
```

---

## OBJECTIVES
[PRIMARY]
  > Demonstrate core C++ OOP principles in a real interactive game context.
  > Implement a clean separation between game logic and rendering layers.
  > Build a functional, playable turn-based combat loop with visual polish.

[SECONDARY]
  > Showcase polymorphism through Monster subclasses and Attack variants.
  > Apply interface-based design (IAiController, IBattleEvents) for extensibility.
  > Use the Observer pattern to decouple event handling from battle execution.
  > Implement a working AI that operates within the same ruleset as the player.

[ACADEMIC / LEARNING]
  > Practice abstract classes and pure virtual methods.
  > Explore operator overloading, method overloading, and static class members.
  > Understand resource management with smart pointers (unique_ptr).
  > Integrate a third-party graphics library (SFML) into a C++ project.

---

## ARCHITECTURE

```
┌──────────────────────────────────────────────────────────────────┐
│                        GAME ENTRY POINT                          │
│                          main.cpp                                │
│   (SFML window + UI structs + game loop + all screen rendering)  │
└───────────────────────────────┬──────────────────────────────────┘
                                │ instantiates
              ┌─────────────────▼─────────────────┐
              │            GameSession             │
              │  (facade: owns Player + AI + BM)   │
              └───────┬───────────────┬────────────┘
                      │               │
          ┌───────────▼──┐     ┌──────▼──────────────┐
          │    Player    │     │    BattleManager     │
          │  (name +     │     │  (turn loop, events, │
          │   Monster +  │     │   outcome, XP award) │
          │   Items)     │     └──┬──────────┬────────┘
          └──────┬───────┘        │          │
                 │         ┌──────▼──┐  ┌────▼────────────┐
          ┌──────▼──────┐  │IAiCtrl  │  │  IBattleEvents  │
          │   Monster   │  │interface│  │   (observer)    │
          │  (abstract) │  └──────┬──┘  └─────────────────┘
          └──┬──┬──┬────┘         │
             │  │  │        SimpleAiController
    ┌────────┘  │  └────────────────────────────────┐
    │           │                                   │
┌───▼────┐ ┌───▼────┐ ┌──────────┐           ┌─────▼──────┐
│  Fire  │ │ Water  │ │  Earth   │           │   Attack   │
│Monster │ │Monster │ │ Monster  │           │ (abstract) │
└────────┘ └────────┘ └──────────┘           └──┬──┬──┬───┘
                                                 │  │  │
                                        ┌────────┘  │  └──────────┐
                                        │           │             │
                                  ┌─────▼──┐  ┌────▼──┐  ┌───────▼───┐
                                  │Damage  │  │ Heal  │  │  Status   │
                                  │Attack  │  │Attack │  │  Attack   │
                                  └────────┘  └───────┘  └───────────┘
```

---

## CONCEPTS
OBJECT-ORIENTED PROGRAMMING
───────────────────────────
  [✓] Encapsulation      — All monster state is private; exposed via getters only
  [✓] Inheritance        — FireMonster / WaterMonster / EarthMonster extend Monster
                         — DamageAttack / HealAttack / StatusAttack extend Attack
                         — HealingItem / BuffItem extend Item
  [✓] Polymorphism       — Monster::getType(), useSpecialAbility(), takeDamage()
                           are pure/virtual and overridden per subclass
  [✓] Abstraction        — IAiController: pure interface for AI behaviour
                         — IBattleEvents: pure observer interface for events

DESIGN PATTERNS
────────────────
  [✓] Observer Pattern   — IBattleEvents listeners registered with BattleManager
                           to receive turn, damage, and outcome notifications
  [✓] Facade Pattern     — GameSession hides Player / BattleManager complexity
                           behind a single clean API for the UI layer
  [✓] Strategy Pattern   — IAiController allows swappable AI implementations
                           injected into BattleManager at construction

C++ LANGUAGE FEATURES
──────────────────────
  [✓] Abstract Classes   — Monster and Attack declare pure virtual methods
  [✓] Smart Pointers     — unique_ptr used for Monster, Attack, Item, Player
  [✓] Static Members     — Monster::totalMonstersCreated (class-level counter)
  [✓] Method Overloading — takeDamage(int) and takeDamage(int, MonsterType)
  [✓] Enums (scoped)     — MonsterType, BattleOutcome, StatusEffectType,
                           UiMonsterChoice, GameScreen, BattleState
  [✓] Operator overload  — (scoped for future extension)
  [✓] Delta-time loop    — Clock-based dt ensures frame-rate independent logic
  [✓] Event polling      — SFML event system for input, text, and window events

---

##  TECHNOLOGIES
LANGUAGE
  C++17
  Compiled with MSVC via Visual Studio 2022

GRAPHICS / WINDOWING
  SFML (Simple and Fast Multimedia Library)

BUILD SYSTEM
  Visual Studio 2022 
  Platform: Windows x64

STANDARD LIBRARY
  <string>    <vector>    <memory>
  <algorithm> <cmath>     <random>
  <cstdlib>   <ctime>     <cstddef>

---


## FUTURE_IMPROVEMENTS
GAMEPLAY
- Inventory system — let players use HealingItems / BuffItems mid-battle
- Multiple enemy encounters — enemy roster with different types and levels
- Difficulty selector — Easy / Normal / Hard changes AI attack priority logic

VISUALS & AUDIO
- Sprite sheets — replace circles with hand-drawn or generated monster art
- Sound effects — attack SFX, hit sounds, victory fanfare via SFML Audio
- Background music — looping battle OST tracks
  
---

