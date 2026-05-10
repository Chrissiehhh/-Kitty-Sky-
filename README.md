# Kitty Sky

Kitty Sky is a vertical-scrolling action shooter built with SFML and CMake.

The player controls Hello Kitty through a scrolling battlefield, fights fruit enemies, collects pickups, and uses bullets, guided missiles, and bombs to clear the stage. The project also includes title, menu, settings, pause, and game-over states, along with custom cute audio and bloom effects.

`SFMLDemo-summary.pdf` contains a short project summary.

## Experiment Report

The current experiment report is available here:

- [Download the report](./高级程序设计实验报告（1）.docx)

GitHub usually lets visitors download the `.docx` file, but it does not reliably render Word documents directly in the browser. If you want inline preview later, a PDF version is the safest choice.

## Current Gameplay

- Player character: Hello Kitty
- Regular enemies: Mangosteen and Watermelon
- Boss enemy: Pineapple Boss
- Weapons:
  - normal shot
  - guided missile
  - bomb
- Pickups:
  - health refill
  - missile refill
  - fire spread upgrade
  - fire rate upgrade
  - bomb pickup

### Difficulty

Simple mode:

- starts with 2 bombs
- drops pickups more often
- has lighter enemy pacing
- ends with 1 pineapple boss

Hard mode:

- starts with 1 bomb
- has denser enemy waves that ramp up over time
- is less generous with pickups
- ends with 2 pineapple bosses

### Bomb Behavior

- Bombs trigger their attack immediately when thrown.
- Regular fruit enemies in the attack area are cleared instantly.
- Bosses in the attack area lose about half of their current HP.
- The bomb sprite still flies out for visual feedback, but the damage effect does not depend on a direct hit first.

## Build

1. Install SFML 2.x.
2. Configure CMake with the SFML root path if needed.
3. Build the project from the repository root so the `Media` folder stays beside the executable.

Example:

```powershell
cmake -S . -B build -DSFML_ROOT="C:/Path/To/SFML"
cmake --build build --config Release --target SFMLDemo
```

Run the built executable:

```powershell
.\build\Source\Release\SFMLDemo.exe
```

## Controls

Player 1:

- Arrow keys: move
- Space: fire
- M: launch missile
- B: launch bomb
- Esc: pause

Player 2 bindings available in Settings:

- W/A/S/D: move
- F: fire
- R: launch missile
- G: launch bomb

## Features

- Title screen, menu, settings, pause, and game-over states
- Vertical-scrolling stage with mixed fruit enemy waves
- Two difficulty modes with different pacing, drops, and boss count
- Pickups for health, missiles, fire spread, fire rate, and bombs
- Guided missiles and immediate-effect bombs
- Pineapple boss battle at the end of the stage
- Custom cute / princess-style music and sound effects
- Bloom and sound effects

## Audio

The current build uses custom generated audio files in:

- `Media/Music/CuteMenuTheme.wav`
- `Media/Music/CuteMissionTheme.wav`
- `Media/Sound/CuteAlliedGunfire.wav`
- `Media/Sound/CuteEnemyGunfire.wav`
- `Media/Sound/CuteExplosion1.wav`
- `Media/Sound/CuteExplosion2.wav`
- `Media/Sound/CuteLaunch.wav`
- `Media/Sound/CuteCollectPickup.wav`
- `Media/Sound/CuteButton.wav`

If you want to regenerate the current audio set:

```powershell
C:\Users\65151\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe tools\generate_cute_audio.py
```
