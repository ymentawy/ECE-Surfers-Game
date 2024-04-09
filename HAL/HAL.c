/*
 * API.c
 *
 *  Created on: Dec 29, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#include <HAL/HAL.h>

/**
 * Constructs a new API object. The API constructor should simply call the
 * constructors of each of its sub-members with the proper inputs.
 *
 * @return a properly constructed API object.
 */
HAL HAL_construct() {
  // The API object which will be returned at the end of construction
  HAL hal;

  // Initialize all LEDs by calling their constructors with correctly-defined
  // arguments.
  hal.launchpadLED1 = LED_construct(LAUNCHPAD_LED1_PORT, LAUNCHPAD_LED1_PIN);

  hal.launchpadLED2Red =
      LED_construct(LAUNCHPAD_LED2_RED_PORT, LAUNCHPAD_LED2_RED_PIN);
  hal.launchpadLED2Green =
      LED_construct(LAUNCHPAD_LED2_GREEN_PORT, LAUNCHPAD_LED2_GREEN_PIN);
  hal.launchpadLED2Blue =
      LED_construct(LAUNCHPAD_LED2_BLUE_PORT, LAUNCHPAD_LED2_BLUE_PIN);

  hal.boosterpackRed =
      LED_construct(BOOSTERPACK_LED_RED_PORT, BOOSTERPACK_LED_RED_PIN);
  hal.boosterpackGreen =
      LED_construct(BOOSTERPACK_LED_GREEN_PORT, BOOSTERPACK_LED_GREEN_PIN);
  hal.boosterpackBlue =
      LED_construct(BOOSTERPACK_LED_BLUE_PORT, BOOSTERPACK_LED_BLUE_PIN);

  // Initialize all Buttons by calling their constructors with correctly-defined
  // arguments.
  hal.launchpadS1 =
      Button_construct(LAUNCHPAD_S1_PORT, LAUNCHPAD_S1_PIN);  // Launchpad S1
  hal.launchpadS2 =
      Button_construct(LAUNCHPAD_S2_PORT, LAUNCHPAD_S2_PIN);  // Launchpad S2

  hal.boosterpackS1 = Button_construct(BOOSTERPACK_S1_PORT,
                                       BOOSTERPACK_S1_PIN);  // Boosterpack S1
  hal.boosterpackS2 = Button_construct(BOOSTERPACK_S2_PORT,
                                       BOOSTERPACK_S2_PIN);  // Boosterpack S2
  hal.boosterpackJS = Button_construct(BOOSTERPACK_JS_PORT,
                                       BOOSTERPACK_JS_PIN);  // Joystick Button

  // Construct the UART module inside of this HAL struct
  hal.uart = UART_construct(USB_UART_INSTANCE, USB_UART_PORT, USB_UART_PINS);


  hal.timer = SWTimer_construct(WAIT_TIME);
  hal.obst_timer = SWTimer_construct(OBSTACLE_TIME);


  initializeGraphics(&hal.g_sContext);
  initialize_Peripherals();

  // Once we have finished building the API, return the completed struct.
  return hal;
}

/**
 * Upon every new cycle of the main super-loop, we MUST UPDATE the status of
 * all inputs. In this program, this function is called only once in the
 * Application_loop() function. Since the Application_loop() function is called
 * once per loop of the while (true) loop in main, we are effectively polling
 * all inputs once per loop.
 *
 * @param hal:  The API whose input modules we wish to refresh
 */
void HAL_refresh(HAL* hal) {
  // Refresh Launchpad buttons
  Button_refresh(&hal->launchpadS1);
  Button_refresh(&hal->launchpadS2);

  // Refresh Boosterpack buttons
  Button_refresh(&hal->boosterpackS1);
  Button_refresh(&hal->boosterpackS2);
  Button_refresh(&hal->boosterpackJS);

  getSampleJoyStick(&hal->joystick);

  // Not real TODO: No need to add anything for UART
}

void initializeGraphics(Graphics_Context *g_sContext_p) {
  // Initialize the LCD
  Crystalfontz128x128_Init();
  Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

  // Initialize context
  Graphics_initContext(g_sContext_p, &g_sCrystalfontz128x128,
                       &g_sCrystalfontz128x128_funcs);

  // Set colors and fonts
  Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
  Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
  Graphics_setFont(g_sContext_p, &g_sFontFixed6x8);

  // Clear the screen
  Graphics_clearDisplay(g_sContext_p);

}

void initialize_Peripherals()
{
    //WDT_A_hold(WDT_A_BASE);
    initADC();
    initJoyStick();
    startADC();
}


// Initializing the ADC which resides on SoC
void initADC() {
    ADC14_enableModule();

    ADC14_initModule(ADC_CLOCKSOURCE_SYSOSC,
                     ADC_PREDIVIDER_1,
                     ADC_DIVIDER_1,
                      0
                     );

    // This configures the ADC to store output results
    // in ADC_MEM0 for joystick X.
    // Todo: if we want to add joystick Y, then, we have to use more memory locations
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);


    // This configures the ADC in manual conversion mode
    // Software will start each conversion.
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
}


void startADC() {
   // Starts the ADC with the first conversion
   // in repeat-mode, subsequent conversions run automatically
   ADC14_enableConversion();
   ADC14_toggleConversionTrigger();
}


// Interfacing the Joystick with ADC (making the proper connections in software)
void initJoyStick() {

    // This configures ADC_MEM0 to store the result from
    // input channel A15 (Joystick X), in non-differential input mode
    // (non-differential means: only a single input pin)
    // The reference for Vref- and Vref+ are VSS and VCC respectively
    ADC14_configureConversionMemory(ADC_MEM0,
                                  ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                  ADC_INPUT_A15,
                                  ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input
    // A15 is multiplexed on GPIO port P6 pin PIN0
    // TODO: which one of GPIO_PRIMARY_MODULE_FUNCTION, or
    //                    GPIO_SECONDARY_MODULE_FUNCTION, or
    //                    GPIO_TERTIARY_MODULE_FUNCTION
    // should be used in place of 0 as the last argument?
    GPIO_setAsPeripheralModuleFunctionInputPin(BOOSTERPACK_JS_HORIZONTAL_PORT,
                                               BOOSTERPACK_JS_HORIZONTAL_PIN,
                                               BOOSTERPACK_JS_PERIPHERAL_MODULE);

    GPIO_setAsPeripheralModuleFunctionInputPin(BOOSTERPACK_JS_VERTICAL_PORT,
                                               BOOSTERPACK_JS_VERTICAL_PIN,
                                               BOOSTERPACK_JS_PERIPHERAL_MODULE);

}

void getSampleJoyStick(Joystick* joystick_p) {
    // ADC runs in continuous mode, we just read the conversion buffers
    joystick_p->x = ADC14_getResult(ADC_MEM0);
    joystick_p->y = ADC14_getResult(ADC_MEM1);
}


bool Joystick_isTappedToLeft(Joystick* joystick_p) {
    static Horizontal state = NOT_LEFT;
    bool output = false;

    switch(state) {
    case NOT_LEFT:
        if(joystick_p->x < LEFT_THRESHOLD){
            state = LEFT;
            output = true;
        }
        break;
    case LEFT:
        if (joystick_p->x > LEFT_THRESHOLD){
            state = NOT_LEFT;
        }
    }
    return (output);
}


bool Joystick_isTappedToRight(Joystick* joystick_p) {
    static Horizontal state = NOT_RIGHT;
    bool output = false;

    switch(state) {
    case NOT_RIGHT:
        if(joystick_p->x > RIGHT_THRESHOLD){
            state = RIGHT;
            output = true;
        }
        break;
    case RIGHT:
        if (joystick_p->x < RIGHT_THRESHOLD){
            state = NOT_RIGHT;
        }
    }
    return (output);
}

bool Joystick_isTappedUp(Joystick* joystick_p) {
    static Vertical state = NOT_UP;
    bool output = false;

    switch(state) {
    case NOT_UP:
        if(joystick_p->y > UP_THRESHOLD){
            state = UP;
            output = true;
        }
        break;
    case UP:
        if (joystick_p->y < UP_THRESHOLD){
            state = NOT_UP;
        }
    }
    return (output);
}

bool Joystick_isTappedDown(Joystick* joystick_p) {
    static Vertical state = NOT_DOWN;
    bool output = false;

    switch(state) {
    case NOT_DOWN:
        if(joystick_p->y < DOWN_THRESHOLD){
            state = DOWN;
            output = true;
        }
        break;
    case DOWN:
        if (joystick_p->y > DOWN_THRESHOLD){
            state = NOT_DOWN;
        }
    }
    return (output);
}

bool Joystick_isPressedToLeft(Joystick* joystick_p) {
    return (joystick_p->x < LEFT_THRESHOLD);
}

bool Joystick_isPressedToRight(Joystick* joystick_p) {
    return (joystick_p->x > RIGHT_THRESHOLD);
}

bool Joystick_isPressedUp(Joystick* joystick_p) {
    return (joystick_p->y > UP_THRESHOLD);
}

bool Joystick_isPressedDown(Joystick* joystick_p) {
    return (joystick_p->y < DOWN_THRESHOLD);
}
