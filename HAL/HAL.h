/*
 * HAL.h
 *
 *  Created on: Dec 29, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef HAL_HAL_H_
#define HAL_HAL_H_

#include <HAL/Button.h>
#include <HAL/LED.h>
#include <HAL/Timer.h>
#include <HAL/UART.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>  // Need to include this in order to use Graphics functions!
#include "LcdDriver/Crystalfontz128x128_ST7735.h"  // Need these drivers to use the LCD!


/**============================================================================
 * The main HAL struct. This struct encapsulates all of the other input structs
 * in this application as individual members. This includes all LEDs, all
 * Buttons, one HWTimer from which all software timers should reference, the
 * Joystick, and any other peripherals with which you wish to interface.
 * ============================================================================
 * USAGE WARNINGS
 * ============================================================================
 * YOU SHOULD HAVE EXACTLY ONE HAL STRUCT IN YOUR ENTIRE PROJECT. We recommend
 * you put this struct inside of a main [Application] object so that every
 * single function in your application has access to the main inputs and outputs
 * which interface with the hardware on the MSP432.
 */

#define DOWN_THRESHOLD  3000
#define UP_THRESHOLD  15000

#define LEFT_THRESHOLD  3000
#define RIGHT_THRESHOLD  15000

struct _Joystick {
    uint_fast16_t x;
    uint_fast16_t y;
};
typedef struct _Joystick Joystick;

struct _HAL {
  // LEDs - Left Launchpad LED (the LED is only a single color - red)
  LED launchpadLED1;

  // LEDs - Right Launchpad LEDs (one LED struct for red, one for blue, etc.)
  LED launchpadLED2Red;
  LED launchpadLED2Blue;
  LED launchpadLED2Green;

  // LEDs - Boosterpack LED (one LED struct for red, one for blue, etc.)
  LED boosterpackRed;
  LED boosterpackBlue;
  LED boosterpackGreen;

  // Buttons - Launchpad S1 and S2
  Button launchpadS1;
  Button launchpadS2;

  // Buttons - Boosterpack S1, S2, and JS (press down on the joystick)
  Button boosterpackS1;
  Button boosterpackS2;
  Button boosterpackJS;

  // UART - Construct a new UART instance
  UART uart;

  SWTimer timer;
  SWTimer obst_timer;
  Joystick joystick;

  Graphics_Context g_sContext;

};
typedef struct _HAL HAL;


typedef enum {title, main_menu, instructions, game, game_over, high_scores} screen;
typedef enum {NOT_LEFT, LEFT, NOT_RIGHT, RIGHT} Horizontal;
typedef enum {NOT_UP, UP, NOT_DOWN, DOWN} Vertical;
typedef enum {SMALL, SMALL_UP, BIG} obstacle;
typedef enum {GAME, INSTRUCTIONS, HIGH_SCORES} main_positions;



// Constructs an HAL object by calling the constructor of each individual member
HAL HAL_construct();

// Refreshes all necessary inputs in the HAL
void HAL_refresh(HAL* api);

void initializeGraphics(Graphics_Context *g_sContext_p);
void initialize_Peripherals();
void initADC();
void startADC();
void initJoyStick();
void getSampleJoyStick(Joystick* joystick_p);
bool Joystick_isTappedToLeft(Joystick* joystick_p);
bool Joystick_isTappedToRight(Joystick* joystick_p);
bool Joystick_isTappedUp(Joystick* joystick_p);
bool Joystick_isTappedDown(Joystick* joystick_p);
bool Joystick_isPressedToLeft(Joystick* joystick_p);
bool Joystick_isPressedToRight(Joystick* joystick_p);
bool Joystick_isPressedUp(Joystick* joystick_p);
bool Joystick_isPressedDown(Joystick* joystick_p);


#endif /* HAL_HAL_H_ */
