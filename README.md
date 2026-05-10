# Kitty Sky

Kitty Sky is a vertical-scrolling action shooter built with SFML and CMake.
Kitty Sky 是一款基于 SFML 和 CMake 开发的纵向卷轴动作射击游戏。

The player controls Hello Kitty through a scrolling battlefield, fights fruit enemies, collects pickups, and uses bullets, guided missiles, and bombs to clear the stage.
玩家操控 Hello Kitty 在滚动战场中前进，对抗水果敌人，收集补给，并使用子弹、导弹和炸弹通关。

The project also includes title, menu, settings, pause, and game-over states.
项目还包含标题、菜单、设置、暂停和游戏结束界面。

`SFMLDemo-summary.pdf` contains a short project summary.
`SFMLDemo-summary.pdf` 是项目的简要说明文档。

## Experiment Report / 实验报告

The current experiment report is available here:
当前实验报告如下：

- [PDF version](./高级程序设计实验报告（1）.pdf)
- [Word version](./高级程序设计实验报告（1）.docx)

GitHub can preview the PDF directly in the browser, while the `.docx` file is kept for download.
GitHub 通常可以直接在浏览器里预览 PDF，而 `.docx` 文件主要用于下载。

## Current Gameplay / 当前玩法

- Player character: Hello Kitty
- 角色：Hello Kitty
- Regular enemies: Mangosteen and Watermelon
- 普通敌人：山竹和西瓜
- Boss enemy: Pineapple Boss
- Boss 敌人：菠萝 Boss
- Weapons:
  - normal shot
  - 普通射击
  - guided missile
  - 导弹
  - bomb
  - 炸弹
- Pickups:
  - health refill
  - 血量补给
  - missile refill
  - 导弹补给
  - fire spread upgrade
  - 扩散射击升级
  - fire rate upgrade
  - 射速升级
  - bomb pickup
  - 炸弹补给

### Difficulty / 难度

Simple mode:
简单模式：

- starts with 2 bombs
- 初始拥有 2 个炸弹
- drops pickups more often
- 补给掉落更频繁
- has lighter enemy pacing
- 敌人节奏更轻松
- ends with 1 pineapple boss
- 最后会出现 1 个菠萝 Boss

Hard mode:
困难模式：

- starts with 1 bomb
- 初始只有 1 个炸弹
- has denser enemy waves that ramp up over time
- 敌人波次更密集，并且会逐渐增强
- is less generous with pickups
- 补给掉落更少
- ends with 2 pineapple bosses
- 最后会出现 2 个菠萝 Boss

### Bomb Behavior / 炸弹效果

- Bombs trigger their attack immediately when thrown.
- 炸弹投出后会立刻触发攻击。
- Regular fruit enemies in the attack area are cleared instantly.
- 攻击范围内的普通水果敌人会被立即清除。
- Bosses in the attack area lose about half of their current HP.
- 攻击范围内的 Boss 会损失大约一半当前生命值。
- The bomb sprite still flies out for visual feedback, but the damage effect does not depend on a direct hit first.
- 炸弹图标仍会飞出作为视觉反馈，但伤害效果不依赖于先直接命中。

## Build / 构建

1. Install SFML 2.x.
1. 安装 SFML 2.x。
2. Configure CMake with the SFML root path if needed.
2. 如有需要，为 CMake 配置 SFML 根目录路径。
3. Build the project from the repository root so the `Media` folder stays beside the executable.
3. 请在仓库根目录下构建项目，这样 `Media` 文件夹才能和可执行文件保持同级。

Example / 示例：

```powershell
cmake -S . -B build -DSFML_ROOT="C:/Path/To/SFML"
cmake --build build --config Release --target SFMLDemo
```

Run the built executable:
运行生成的可执行文件：

```powershell
.\build\Source\Release\SFMLDemo.exe
```

## Controls / 操作说明

Player 1:
玩家 1：

- Arrow keys: move
- 方向键：移动
- Space: fire
- 空格：射击
- M: launch missile
- M：发射导弹
- B: launch bomb
- B：发射炸弹
- Esc: pause
- Esc：暂停

## Features / 特性

- Title screen, menu, settings, pause, and game-over states
- 标题界面、菜单、设置、暂停和游戏结束界面
- Vertical-scrolling stage with mixed fruit enemy waves
- 纵向卷轴关卡，包含多种水果敌人波次
- Two difficulty modes with different pacing, drops, and boss count
- 两种难度模式，节奏、掉落和 Boss 数量都不同
- Pickups for health, missiles, fire spread, fire rate, and bombs
- 提供血量、导弹、扩散射击、射速和炸弹补给
- Guided missiles and immediate-effect bombs
- 导弹和即时生效的炸弹
- Pineapple boss battle at the end of the stage
- 关卡末尾的菠萝 Boss 战
