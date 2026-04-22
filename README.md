# Kitty Sky

Kitty Sky is a vertical-scrolling action shooter built with SFML and CMake.

The player controls Hello Kitty through a scrolling battlefield, fights fruit enemies, collects pickups, and uses missiles and bullets to clear each stage. The game also includes title, menu, settings, pause, and game-over states, plus sound and bloom effects.

`SFMLDemo-summary.pdf` contains a short project summary.

## Build

1. Install SFML 2.x.
2. Configure CMake with the SFML root path if needed.
3. Build the project from the repository root so the `Media` folder stays beside the executable.

Example:

```powershell
cmake -S . -B build -DSFML_ROOT="C:/Path/To/SFML"
cmake --build build --config Release
```
