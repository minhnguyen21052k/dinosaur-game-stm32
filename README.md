# Chrome Dino Game — STM32F401RE + LUMI Extension Kit

A recreation of Chrome's offline dinosaur game running on an STM32 NUCLEO-F401RE with the LUMI IoT Extension Board, displayed on a 128x128 ST7735 SPI LCD.

## Features

* Menu screen with cursor navigation
* Two speed levels (Slow / Fast) — affects cactus speed only
* High score tracking across games
* Buzzer beep on jump
* Red LED flash on game over
* 2-second game over screen with score display, then auto-return to menu

## Hardware

* **Board:** STM32 NUCLEO-F401RE
* **Extension:** LUMI IoT STM Board Kit (LM-ISBK V1.0)
* **Display:** ST7735 128x128 SPI LCD (onboard)
* **Buzzer:** PC9 (transistor-driven)
* **LED:** RGB LEDs (transistor-driven)
* **Buttons:** SW1 (PB5), SW3 (PA4), SW5 (PB4)

## Controls

|Button|Menu|In Game|
|-|-|-|
|B1 (SW1, PB5)|Move cursor up|—|
|B5 (SW5, PB4)|Move cursor down|—|
|B3 (SW3, PA4)|Select / confirm|Jump|

## Required Library

This project uses the LUMI SDK for the NUCLEO-F401RE extension kit:

**https://github.com/HD-Nam/ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE**

Clone it and place it alongside the project directory:

```
D:/STM32CubeIDE\_2.1.1/
├── ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/   ← SDK library
│   ├── lib\_stm/
│   └── shared/
└── DinoGame/                          ← this project
    ├── Src/
    │   └── main.c
    ├── Inc/
    ├── Startup/
    ├── STM32F401RETX\_FLASH.ld
    └── STM32F401RETX\_RAM.ld
```

## Building

### Prerequisites

* [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
* [LUMI SDK](https://github.com/HD-Nam/ThuVien_SDK_1.0.3_NUCLEO-F401RE) cloned locally

### Setup

1. Clone this repo and the SDK:

```
   git clone https://github.com/HD-Nam/ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE.git
   git clone <this-repo-url>
   ```

2. Open STM32CubeIDE, import the project via **File > Import > Existing Projects into Workspace**
3. Configure include paths in **Project Properties > C/C++ Build > Settings > MCU GCC Compiler > Include paths**:

```
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Drivers/CMSIS/Include
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Drivers/STM32F401RE\_StdPeriph\_Driver/inc
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/button
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/buzzer
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/led
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/sensor
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/serial
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/ucglib
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/flash
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Middle/rtos
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/shared/Utilities
   ```

4. Configure library path in **MCU GCC Linker > Library search paths**:

```
   ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE/lib\_stm
   ```

5. Build with **Ctrl+B**
6. Flash to board via **Run > Debug** or the ST-LINK programmer

## Project Structure

```
├── Src/
│   └── main.c              ← game source code
├── Inc/                     ← project headers (if any)
├── Startup/
│   └── startup\_stm32f401retx.s
├── STM32F401RETX\_FLASH.ld   ← linker script
├── STM32F401RETX\_RAM.ld
├── .gitignore
└── README.md
```

## How It Works

The player can choose the difficulty on the menu screen upon opening the game. After starting the game, the player will control the dinosaur with the objective of jumping over as many cactus as possible. The game will keep track of your score and show it on the game menu. Upon losing, the player will be show their current and best score then will be sent back to the game menu.

## License

MIT

