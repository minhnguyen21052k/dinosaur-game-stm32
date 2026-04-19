/**
 * ============================================================
 *  CHROME DINO GAME — LUMI Extension Kit (V10)
 *  - No ucg_ClearScreen transitions (overlay instead)
 *  - Speed setting only changes cactus move speed
 *  - Frame rate and dino speed constant regardless of setting
 * ============================================================
 */

#include "stm32f401re_rcc.h"
#include "stm32f401re_gpio.h"
#include "timer.h"
#include "ucg.h"
#include "Ucglib.h"
#include "buzzer.h"
#include "led.h"
#include "button.h"

#define SW          128
#define SH          128
#define GROUND_Y    100
#define DINO_W       12
#define DINO_H       14
#define DINO_X       15
#define DINO_GY     (GROUND_Y - DINO_H)
#define CACT_W        8
#define CACT_H       12
#define CACT_GY     (GROUND_Y - CACT_H)
#define JUMP_H       35
#define JUMP_FR      14
#define FRAME_MS     60      /* constant frame rate for all speeds */
#define CACT_SPD_SLOW 3      /* pixels per frame: slow */
#define CACT_SPD_FAST 6      /* pixels per frame: fast */
#define MENU_ITEMS    2

#define MENU_Y0     52
#define MENU_Y1     68

typedef enum { ST_MENU, ST_PLAY, ST_OVER } State;

static ucg_t ucg;
static State    state = ST_MENU;
static uint8_t  menuSel = 0;
static uint8_t  prevMenuSel = 0;
static uint8_t  speedFast = 0;
static uint16_t score = 0;
static uint16_t hiScore = 0;
static uint16_t prevScore = 0xFFFF;
static int16_t  dinoY, prevDinoY;
static uint8_t  jumping = 0, jumpFr = 0;
static int16_t  cactX, prevCactX;
static uint8_t  cactSpeed = CACT_SPD_SLOW;
static uint32_t lastFrMs;
static uint32_t overTime;
static uint8_t  pB1=1, pB3=1, pB5=1;
static uint8_t  buzzOn=0;
static uint32_t buzzOffT=0;

/* ── Draw a number using ucg_DrawGlyph ── */
static void DrawNumber(int16_t x, int16_t y, uint16_t val) {
    char c;
    uint8_t started = 0;
    if (val >= 100) {
        c = '0';
        while (val >= 100) { c++; val -= 100; }
        ucg_DrawGlyph(&ucg, x, y, 0, c);
        x += 8;
        started = 1;
    }
    if (val >= 10 || started) {
        c = '0';
        while (val >= 10) { c++; val -= 10; }
        ucg_DrawGlyph(&ucg, x, y, 0, c);
        x += 8;
    }
    c = '0';
    while (val >= 1) { c++; val -= 1; }
    ucg_DrawGlyph(&ucg, x, y, 0, c);
}

static uint8_t B1(void){uint8_t c=Button_GetLogicInputPin(BUTTON_KIT_ID1);uint8_t r=(c==0&&pB1==1);pB1=c;return r;}
static uint8_t B3(void){uint8_t c=Button_GetLogicInputPin(BUTTON_KIT_ID3);uint8_t r=(c==0&&pB3==1);pB3=c;return r;}
static uint8_t B5(void){uint8_t c=Button_GetLogicInputPin(BUTTON_KIT_ID5);uint8_t r=(c==0&&pB5==1);pB5=c;return r;}

static void PrepText(void) {
    ucg_SetFont(&ucg, ucg_font_helvR12_tr);
    ucg_SetFontMode(&ucg, UCG_FONT_MODE_TRANSPARENT);
}

/* ── Dino ── */
static void DrawDinoFrame(int16_t oldY, int16_t newY) {
    if (oldY != newY) {
        ucg_SetColor(&ucg,0,0,0,0);
        if (newY < oldY) {
            int16_t h = (oldY+DINO_H)-(newY+DINO_H);
            if (h>0) ucg_DrawBox(&ucg, DINO_X, newY+DINO_H, DINO_W, h);
        } else {
            int16_t h = newY-oldY;
            if (h>0) ucg_DrawBox(&ucg, DINO_X, oldY, DINO_W, h);
        }
    }
    ucg_SetColor(&ucg,0,30,200,30);
    ucg_DrawBox(&ucg, DINO_X, newY+3, 10, 8);
    ucg_DrawBox(&ucg, DINO_X+4, newY, 8, 4);
    ucg_SetColor(&ucg,0,0,0,0);
    ucg_DrawPixel(&ucg, DINO_X+9, newY+1);
    ucg_SetColor(&ucg,0,30,200,30);
    ucg_DrawBox(&ucg, DINO_X+1, newY+11, 2, 3);
    ucg_DrawBox(&ucg, DINO_X+6, newY+11, 2, 3);
}

/* ── Cactus ── */
static void DrawCactFrame(int16_t oldX, int16_t newX) {
    if (oldX!=newX && oldX>=-CACT_W && oldX<=SW) {
        ucg_SetColor(&ucg,0,0,0,0);
        int16_t eX = newX+CACT_W;
        int16_t eW = (oldX+CACT_W)-eX;
        if (eW>0 && eX>=0 && eX<SW)
            ucg_DrawBox(&ucg, eX, CACT_GY, eW, CACT_H);
    }
    if (newX>=-CACT_W && newX<=SW) {
        ucg_SetColor(&ucg,0,0,130,0);
        ucg_DrawBox(&ucg, newX+2, CACT_GY, 4, CACT_H);
        ucg_DrawBox(&ucg, newX, CACT_GY+3, 2, 4);
        ucg_DrawBox(&ucg, newX+6, CACT_GY+5, 2, 4);
    }
}

static void EraseCactFull(int16_t x) {
    if (x>=-CACT_W && x<=SW) {
        ucg_SetColor(&ucg,0,0,0,0);
        ucg_DrawBox(&ucg, x, CACT_GY, CACT_W, CACT_H);
    }
}

/* ══════════════════════════════════════════
 *  MENU
 * ══════════════════════════════════════════ */
static void MenuDrawCursor(void) {
    PrepText();
    ucg_SetColor(&ucg,0,0,0,0);
    ucg_DrawBox(&ucg, 3, (prevMenuSel==0 ? MENU_Y0 : MENU_Y1) - 12, 12, 15);
    ucg_SetColor(&ucg, 0, 30, 200, 30);
    ucg_DrawString(&ucg, 5, menuSel==0 ? MENU_Y0 : MENU_Y1, 0, ">");
}

static void MenuDrawSpeed(void) {
    ucg_SetColor(&ucg,0,0,0,0);
    ucg_DrawBox(&ucg, 72, MENU_Y1 - 12, 50, 15);
    PrepText();
    ucg_SetColor(&ucg, 0, 255, 255, 255);
    ucg_DrawString(&ucg, 72, MENU_Y1, 0, speedFast ? "Fast" : "Slow");
}

static void MenuDrawBest(void) {
    ucg_SetColor(&ucg,0,0,0,0);
    ucg_DrawBox(&ucg, 60, 17, 60, 15);
    PrepText();
    ucg_SetColor(&ucg, 0, 90, 90, 90);
    DrawNumber(62, 30, hiScore);
}

static void MenuFull(void) {
    /* Fill screen black without ucg_ClearScreen for speed */
    ucg_SetColor(&ucg,0,0,0,0);
    ucg_DrawBox(&ucg, 0, 0, SW, SH);

    PrepText();

    ucg_SetColor(&ucg, 0, 255, 220, 0);
    ucg_DrawString(&ucg, 15, 14, 0, "DINO GAME");

    ucg_SetColor(&ucg, 0, 90, 90, 90);
    ucg_DrawString(&ucg, 20, 30, 0, "Best: ");
    DrawNumber(62, 30, hiScore);

    ucg_SetColor(&ucg, 0, 255, 255, 255);
    ucg_DrawString(&ucg, 18, MENU_Y0, 0, "Start Game");
    ucg_DrawString(&ucg, 18, MENU_Y1, 0, "Speed:");
    ucg_DrawString(&ucg, 72, MENU_Y1, 0, speedFast ? "Fast" : "Slow");

    prevMenuSel = menuSel;
    ucg_SetColor(&ucg, 0, 30, 200, 30);
    ucg_DrawString(&ucg, 5, menuSel==0 ? MENU_Y0 : MENU_Y1, 0, ">");

    ucg_SetColor(&ucg, 0, 90, 90, 90);
    ucg_DrawString(&ucg, 5, 110, 0, "B1/5:Sel B3:OK");
}

/* ══════════════════════════════════════════
 *  GAME START — no ClearScreen, draw over menu
 * ══════════════════════════════════════════ */
static void GameStart(void) {
    state=ST_PLAY; score=0; prevScore=0xFFFF;
    dinoY=DINO_GY; prevDinoY=DINO_GY;
    jumping=0; jumpFr=0;
    cactX=SW+20; prevCactX=cactX;
    cactSpeed=speedFast?CACT_SPD_FAST:CACT_SPD_SLOW;
    lastFrMs=GetMilSecTick();

    /* Black fill instead of ClearScreen */
    ucg_SetColor(&ucg,0,0,0,0);
    ucg_DrawBox(&ucg, 0, 0, SW, SH);

    ucg_SetColor(&ucg,0,90,90,90);
    ucg_DrawHLine(&ucg, 0, GROUND_Y, SW);

    PrepText();
    ucg_SetColor(&ucg,0,90,90,90);
    ucg_DrawString(&ucg, 2, 12, 0, speedFast ? "FAST" : "SLOW");

    DrawDinoFrame(DINO_GY, DINO_GY);
}

/* ══════════════════════════════════════════
 *  GAME OVER — overlay on top, no clear
 * ══════════════════════════════════════════ */
static void GameOver(void) {
    state=ST_OVER;
    overTime=GetMilSecTick();
    if (score>hiScore) hiScore=score;

    LedControl_SetColorIndividual(LED_KIT_ID0, LED_COLOR_RED, 100);
    BuzzerControl_SetDutyCycle(30);
    buzzOn=0;

    /* Dark red overlay */
    ucg_SetColor(&ucg, 0, 40, 0, 0);
    ucg_DrawBox(&ucg, 4, 18, 120, 68);

    PrepText();

    ucg_SetColor(&ucg, 0, 255, 40, 40);
    ucg_DrawString(&ucg, 14, 34, 0, "GAME OVER!");

    ucg_SetColor(&ucg, 0, 255, 255, 255);
    ucg_DrawString(&ucg, 18, 52, 0, "Score: ");
    DrawNumber(68, 52, score);

    ucg_SetColor(&ucg, 0, 255, 220, 0);
    ucg_DrawString(&ucg, 25, 68, 0, "Best: ");
    DrawNumber(67, 68, hiScore);
}

static uint8_t Hit(void) {
    return (DINO_X<cactX+CACT_W && DINO_X+DINO_W>cactX &&
            dinoY<CACT_GY+CACT_H && dinoY+DINO_H>CACT_GY);
}

static void ClkInit(void) {
    RCC_DeInit(); RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY)==RESET);
    FLASH->ACR=FLASH_ACR_LATENCY_2WS|FLASH_ACR_PRFTEN|FLASH_ACR_ICEN|FLASH_ACR_DCEN;
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PLLConfig(RCC_PLLSource_HSI,16,336,4,7);
    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==RESET);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource()!=0x08);
    SystemCoreClock=84000000;
}

int main(void) {
    ClkInit();
    TimerInit();
    LedControl_Init();
    BuzzerControl_Init();
    Button_Init();

    Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID);
    ucg_ClearScreen(&ucg);
    ucg_SetRotate180(&ucg);

    state=ST_MENU;
    MenuFull();

    while (1) {
        processTimerScheduler();
        uint8_t b1=B1(), b3=B3(), b5=B5();

        if (buzzOn && GetMilSecTick()-buzzOffT>=80) {
            BuzzerControl_SetDutyCycle(0); buzzOn=0;
        }

        /* ── MENU ── */
        if (state==ST_MENU) {
            if (b1) {
                prevMenuSel=menuSel;
                menuSel=(menuSel==0)?MENU_ITEMS-1:menuSel-1;
                MenuDrawCursor();
            }
            if (b5) {
                prevMenuSel=menuSel;
                menuSel=(menuSel+1)%MENU_ITEMS;
                MenuDrawCursor();
            }
            if (b3) {
                if (menuSel==0) GameStart();
                else { speedFast^=1; MenuDrawSpeed(); }
            }
            continue;
        }

        /* ── GAME OVER ── */
        if (state==ST_OVER) {
            if (GetMilSecTick()-overTime>=2000) {
                LedControl_SetColorIndividual(LED_KIT_ID0, LED_COLOR_RED, 0);
                BuzzerControl_SetDutyCycle(0); buzzOn=0;
                state=ST_MENU; MenuFull();
            }
            continue;
        }

        /* ── PLAYING ── */
        if (b3 && !jumping) {
            jumping=1; jumpFr=0;
            BuzzerControl_SetDutyCycle(50);
            buzzOn=1; buzzOffT=GetMilSecTick();
        }

        /* Constant frame rate */
        uint32_t now=GetMilSecTick();
        if (now-lastFrMs<FRAME_MS) continue;
        lastFrMs=now;

        prevDinoY=dinoY;
        prevCactX=cactX;

        /* Dino jump — same speed regardless of setting */
        if (jumping) {
            jumpFr++;
            uint8_t half=JUMP_FR/2;
            if (jumpFr<=half)
                dinoY=DINO_GY-(JUMP_H*jumpFr)/half;
            else if (jumpFr<=JUMP_FR)
                dinoY=(DINO_GY-JUMP_H)+(JUMP_H*(jumpFr-half))/half;
            else { dinoY=DINO_GY; jumping=0; jumpFr=0; }
        }

        /* Cactus — speed setting only changes this */
        cactX -= cactSpeed;
        if (cactX<-CACT_W) {
            EraseCactFull(prevCactX);
            cactX=SW+10+(now%25);
            prevCactX=cactX;
            score++;
        }

        if (Hit()) {
            DrawDinoFrame(prevDinoY, dinoY);
            DrawCactFrame(prevCactX, cactX);
            GameOver();
            continue;
        }

        DrawCactFrame(prevCactX, cactX);
        DrawDinoFrame(prevDinoY, dinoY);

        ucg_SetColor(&ucg,0,90,90,90);
        ucg_DrawHLine(&ucg, 0, GROUND_Y, SW);

        if (score!=prevScore) {
            prevScore=score;
            ucg_SetColor(&ucg,0,0,0,0);
            ucg_DrawBox(&ucg, 85, 0, 43, 15);
            PrepText();
            ucg_SetColor(&ucg,0,255,255,255);
            DrawNumber(100, 12, score);
        }
    }
}
