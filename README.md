# Chrome Dino Game — STM32F401RE + LUMI Extension Kit

A recreation of Chrome's offline dinosaur game running on an STM32 NUCLEO-F401RE with the LUMI IoT Extension Board, displayed on a 128x128 ST7735 SPI LCD.

## Features

- Menu screen with cursor navigation
- Two speed levels (Slow / Fast) — affects cactus speed only
- High score tracking across games
- Buzzer beep on jump
- Red LED flash on game over
- 2-second game over screen with score display, then auto-return to menu

## Hardware

- **Board:** STM32 NUCLEO-F401RE
- **Extension:** LUMI IoT STM Board Kit (LM-ISBK V1.0)
- **Display:** ST7735 128x128 SPI LCD (onboard)
- **Buzzer:** PC9 (transistor-driven)
- **LED:** RGB LEDs (transistor-driven)
- **Buttons:** SW1 (PB5), SW3 (PA4), SW5 (PB4)

## Controls

| Button | Menu | In Game |
|--------|------|---------|
| B1 (SW1, PB5) | Move cursor up | — |
| B5 (SW5, PB4) | Move cursor down | — |
| B3 (SW3, PA4) | Select / confirm | Jump |

## Required Library

This project uses the LUMI SDK for the NUCLEO-F401RE extension kit:

**https://github.com/HD-Nam/ThuVien_SDK_1.0.3_NUCLEO-F401RE**

Clone it and place it alongside the project directory:

```
D:/STM32CubeIDE_2.1.1/
├── ThuVien_SDK_1.0.3_NUCLEO-F401RE/   ← SDK library
│   ├── lib_stm/
│   └── shared/
└── DinoGame/                          ← this project
    ├── Src/
    │   └── main.c
    ├── Inc/
    ├── Startup/
    ├── STM32F401RETX_FLASH.ld
    └── STM32F401RETX_RAM.ld
```

## Building

### Prerequisites

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [LUMI SDK](https://github.com/HD-Nam/ThuVien_SDK_1.0.3_NUCLEO-F401RE) cloned locally

### Setup

1. Clone this repo and the SDK:
   ```
   git clone https://github.com/HD-Nam/ThuVien_SDK_1.0.3_NUCLEO-F401RE.git
   git clone <this-repo-url>
   ```

2. Open STM32CubeIDE, import the project via **File > Import > Existing Projects into Workspace**

3. Configure include paths in **Project Properties > C/C++ Build > Settings > MCU GCC Compiler > Include paths**:
   ```
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Drivers/CMSIS/Include
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Drivers/STM32F401RE_StdPeriph_Driver/inc
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/button
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/buzzer
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/led
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/sensor
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/serial
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/ucglib
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/flash
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Middle/rtos
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/shared/Utilities
   ```

4. Configure library path in **MCU GCC Linker > Library search paths**:
   ```
   ThuVien_SDK_1.0.3_NUCLEO-F401RE/lib_stm
   ```

5. Build with **Ctrl+B**

6. Flash to board via **Run > Debug** or the ST-LINK programmer

## Project Structure

```
├── Src/
│   └── main.c              ← game source code
├── Inc/                     ← project headers (if any)
├── Startup/
│   └── startup_stm32f401retx.s
├── STM32F401RETX_FLASH.ld   ← linker script
├── STM32F401RETX_RAM.ld
├── .gitignore
└── README.md
```

## How It Works

The game runs at a constant 60ms frame rate. The dino is drawn as a green sprite on the left side of the LCD. Cacti scroll from right to left — their speed depends on the selected difficulty. Collision is detected via axis-aligned bounding box overlap.

All graphics use the Ucglib library (ST7735 driver) with partial screen updates — only changed pixels are redrawn each frame to minimize SPI traffic and avoid flicker.

Numbers are rendered using `ucg_DrawGlyph` character-by-character since the nano specs C library on this toolchain does not support formatted printing (`snprintf`/`printf` with `%d`/`%u`).

## License

MIT
