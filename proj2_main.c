/**
 * Starter code for Project 2. Good luck!
 *
 * We recommending copy/pasting your HAL folder from Project 1
 * into this project.
 */

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


/* HAL and Application includes */
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>

extern const Graphics_Image colors8BPP_UNCOMP;


// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void InitNonBlockingLED() {
  GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void PollNonBlockingLED() {
  GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
  if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
  }
}

int main(void) {
  // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
  WDT_A_holdTimer();
  srand(time(NULL)); // Use the current time as the seed

  // Initialize the system clock and background hardware timer, used to enable
  // software timers to time their measurements properly.
  InitSystemTiming();

  // Initialize the main Application object and HAL object
  HAL hal = HAL_construct();
  Application app = Application_construct();
  app.obst_speed = hal.obst_timer;

  // Do not remove this line. This is your non-blocking check.
  InitNonBlockingLED();

  // Main super-loop! In a polling architecture, this function should call
  // your main FSM function over and over.
  while (true) {
    // Do not remove this line. This is your non-blocking check.
    PollNonBlockingLED();
    HAL_refresh(&hal);
    Application_loop(&app, &hal);
  }
}

/**
 * The main constructor for your application. This function should initialize
 * each of the FSMs which implement the application logic of your project.
 *
 * @return a completely initialized Application object
 */
Application Application_construct() {
  Application app;

  // Initialize local application state variables here!
  app.baudChoice = BAUD_9600;  // Default baud rate choice
  app.firstCall = true;         // Flag for first function call
  app.screen_state = title;     // Initial screen state
  app.joyStickPushedDown = false;    // Joystick state flags
  app.joyStickPushedUp = false;
  app.joyStickPushedtoLeft = false;
  app.joyStickPushedtoRight = false;
  app.score_changed = false;     // Flag to indicate if score changed
  app.jump = false;              // Flag for player jump action
  app.main_pos = 0;              // Main position variable
  app.first_main = false;        // Flag for first main function call
  app.score = 0;                 // Initial score
  app.lives = 3;                 // Initial number of lives
  app.player_x = 15;             // Initial player X position
  app.player_y = 93;             // Initial player Y position
  app.temp_player_y = 0;         // Temporary player Y position

  // Initialize high scores array
  int i;
  for (i = 0; i < MAX_SCORES; i++)
      app.high_scores[i] = 0;

  // Initialize obstacles arrays
  for (i = 0; i < MAX_OBSTACLES; i++) {
      app.obstacles_x[i] = 0;    // X positions
      app.obstacles_y[i] = -2;   // Y positions (off-screen initially)
  }

  // Initialize obstacle collision flags
  for (i = 0; i < MAX_OBSTACLES; i++)
      app.obstacle_collision[i] = false;

  // Initialize obstacle types
  for (i = 0; i < MAX_OBSTACLES; i++)
      app.obstacle_type[i] = SMALL;

  return app;  // Return initialized application state
}


// Function to clear the screen
void clear_screen(HAL* hal_p){
    // Define the screen rectangle
    static Graphics_Rectangle R;
    R.xMin = MIN_COORDINATE;
    R.xMax = MAX_COORDINATE;
    R.yMin = MIN_COORDINATE;
    R.yMax = MAX_COORDINATE;

    // Set foreground color to black and fill the rectangle
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&hal_p->g_sContext, &R);
    // Set foreground color back to white
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}

/*
 * Application_loop
 *
 * This function is called once per super-loop of the main application. It manages the main
 * application logic, including updating communications, handling button presses, and printing
 * output via UART.
 *
 * Parameters:
 *   - app_p: Pointer to the Application struct containing application state and variables.
 *   - hal_p: Pointer to the HAL struct containing hardware abstraction layer functions and variables.
 *
 * Description:
 *   - Restarts or updates communications if this is the first time the application is run or if
 *     BoosterPack S2 is pressed (which indicates a new baudrate is being set up).
 *   - Calls the Game_FSM function to manage the game's finite state machine.
 *   - Checks if BoosterPack S2 is tapped or if it's the first call to update communications accordingly.
 *   - Prints output via UART.
 */
void Application_loop(Application* app_p, HAL* hal_p) {
    // Reset joystick state flags
    app_p->joyStickPushedDown = false;
    app_p->joyStickPushedUp = false;
    app_p->joyStickPushedtoLeft = false;
    app_p->joyStickPushedtoRight = false;

    // Call the game finite state machine (FSM)
    Game_FSM(app_p, hal_p);
}

// This function handles the title state of the game.
// It prints the title screen and starts the timer if it's the first call.
// Otherwise, it simply prints the title screen.
void title_state(Application* app_p, HAL* hal_p) {
    if (app_p->firstCall) {
        // If it's the first call, print the title screen, draw an image, start the timer, and mark firstCall as false.
        print_title(app_p, hal_p);
        Graphics_drawImage(&hal_p->g_sContext, &colors8BPP_UNCOMP, 0, 0);
        SWTimer_start(&hal_p->timer);
        app_p->firstCall = false;
    } else {
        // If it's not the first call, simply print the title screen.
        print_title(app_p, hal_p);
    }
}


// This function handles the main menu state of the game.
// It checks for joystick and button inputs, updates the application state accordingly,
// and prints the main menu screen.
void main_menu_state(Application* app_p, HAL* hal_p) {
    if (Joystick_isTappedDown(&hal_p->joystick)) {
        app_p->joyStickPushedDown = true;
    } else if (Joystick_isTappedUp(&hal_p->joystick)) {
        app_p->joyStickPushedUp = true;
    }

    // Print the main menu screen with joystick input states.
    print_main(app_p, hal_p, app_p->joyStickPushedUp, app_p->joyStickPushedDown);

    // Check for button inputs and transition to different screens accordingly.
    if (Button_isTapped(&hal_p->boosterpackJS) && app_p->main_pos == GAME) {
        app_p->screen_state = game;
        SWTimer_start(&hal_p->obst_timer);
        clear_screen(hal_p);
        print_game(app_p, hal_p, app_p->joyStickPushedtoLeft, app_p->joyStickPushedtoRight, app_p->joyStickPushedUp, app_p->joyStickPushedDown);
        app_p->first_main = true;
    } else if (Button_isTapped(&hal_p->boosterpackJS) && app_p->main_pos == INSTRUCTIONS) {
        app_p->screen_state = instructions;
        clear_screen(hal_p);
        print_instructions(app_p, hal_p);
        app_p->first_main = true;
    } else if (Button_isTapped(&hal_p->boosterpackJS) && app_p->main_pos == HIGH_SCORES) {
        app_p->screen_state = high_scores;
        clear_screen(hal_p);
        print_scores(app_p, hal_p);
        app_p->first_main = true;
    }
}

// This function handles the instructions state of the game.
// It checks for button input to return to the main menu and updates the screen accordingly.
void instructions_state(Application* app_p, HAL* hal_p) {
    if (Button_isTapped(&hal_p->boosterpackJS)) {
        app_p->screen_state = main_menu;
        clear_screen(hal_p);
        print_main(app_p, hal_p, app_p->joyStickPushedUp, app_p->joyStickPushedDown);
    }
}

// This function handles the game state of the application.
// It checks for joystick and button inputs, updates the application state accordingly,
// and prints the game screen.
void game_state(Application* app_p, HAL* hal_p) {
    if (Joystick_isTappedDown(&hal_p->joystick)) {
        app_p->joyStickPushedDown = true;
    } else if (Joystick_isTappedUp(&hal_p->joystick)) {
        app_p->joyStickPushedUp = true;
    }

    if (Joystick_isPressedToLeft(&hal_p->joystick)) {
        app_p->joyStickPushedtoLeft = true;
    } else if (Joystick_isPressedToRight(&hal_p->joystick)) {
        app_p->joyStickPushedtoRight = true;
    } else if (Button_isTapped(&hal_p->boosterpackS1) && !app_p->jump) {
        app_p->jump = true;
        app_p->temp_player_y = app_p->player_y;
    }

    // Print the game screen with joystick input states.
    print_game(app_p, hal_p, app_p->joyStickPushedtoLeft, app_p->joyStickPushedtoRight, app_p->joyStickPushedUp, app_p->joyStickPushedDown);
}

// This function handles the game over state of the game.
// It checks for button input to return to the main menu, resets the game state, and updates the screen.
void game_over_state(Application* app_p, HAL* hal_p) {
    if (Button_isTapped(&hal_p->boosterpackJS)) {
        reset_game(app_p, hal_p);
        app_p->screen_state = main_menu;
        clear_screen(hal_p);
        print_main(app_p, hal_p, app_p->joyStickPushedUp, app_p->joyStickPushedDown);
    }
}

// This function handles the high scores state of the game.
// It checks for button input to return to the main menu and updates the screen accordingly.
void high_scores_state(Application* app_p, HAL* hal_p) {
    if (Button_isTapped(&hal_p->boosterpackJS)) {
        app_p->screen_state = main_menu;
        clear_screen(hal_p);
        print_main(app_p, hal_p, app_p->joyStickPushedUp, app_p->joyStickPushedDown);
    }
}


// This function implements the finite state machine (FSM) of the game.
// It switches between different game states based on the current screen state.
void Game_FSM(Application* app_p, HAL* hal_p) {
    switch (app_p->screen_state) {
        case title:
            title_state(app_p, hal_p);
            break;
        case main_menu:
            main_menu_state(app_p, hal_p);
            break;
        case instructions:
            instructions_state(app_p, hal_p);
            break;
        case game:
            game_state(app_p, hal_p);
            break;
        case game_over:
            game_over_state(app_p, hal_p);
            break;
        case high_scores:
            high_scores_state(app_p, hal_p);
            break;
    }
}

// This function prints the title screen of the game.
// It displays project information and animates the loading bar.
// Once the loading bar is complete, it transitions to the main menu state.
void print_title(Application* app_p, HAL* hal_p) {
    static int x = MIN_COORDINATE - 3; // Initial position of the loading bar

    // Static array to store lines of text
    static char lines[MAX_LINES][MAX_STRING_LENGTH];

    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);

    strcpy(lines[2], "Spring 2024 Project");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[2], -1, 12, 24, true);

    strcpy(lines[3], "ECE Surfers");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[3], -1, 12, 32, true);

    strcpy(lines[4], "Youssef Mentawy");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[4], -1, 12, 40, true);

    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
    print_rectangle(app_p, hal_p, x, x + 3, MAX_COORDINATE - 10, MAX_COORDINATE);
    x += LOAD_SPEED;

    // Check if the loading bar animation is complete and transition to the main menu state
    if (x >= MAX_COORDINATE) {
        app_p->screen_state = main_menu;
        clear_screen(hal_p);
        print_main(app_p, hal_p, app_p->joyStickPushedUp, app_p->joyStickPushedDown);
        app_p->first_main = true;
    }
}


void print_main(Application* app_p, HAL* hal_p, bool moveUp, bool moveDown){
    // Static array to store lines of text
    static char lines[MAX_LINES][MAX_STRING_LENGTH];
    static unsigned int x = 8;
    static unsigned int y = 40;
    static char cursor[] = ">";

    if (app_p->first_main){
        Graphics_drawString(&hal_p->g_sContext, (int8_t *) cursor, -1, x, y, true);
        app_p->first_main = false;
    }


    if ((moveDown && (y<50)) || (moveUp && (y>40)))
    {

        Graphics_drawString(&hal_p->g_sContext, (int8_t *) " ", -1, x, y, true);

        if (moveUp){
            y = y-Y_INCREMENTAL;
            app_p->main_pos -= 1;
        }
        else if (moveDown){
            y = y+Y_INCREMENTAL;
            app_p->main_pos += 1;
        }
        Graphics_drawString(&hal_p->g_sContext, (int8_t *) cursor, -1, x, y, true);

    }

    strcpy(lines[0], "Play ECE Surfers");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[0], -1, 15, 40, true);

    strcpy(lines[1], "Instructions");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[1], -1, 15, 48, true);

    strcpy(lines[2], "View high scores");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[2], -1, 15, 56, true);
}

// This function prints the game screen, including the score, lives, player, obstacles, and handles player movement.
void print_game(Application* app_p, HAL* hal_p, bool moveToLeft, bool moveToRight, bool moveUp, bool moveDown) {
    // Set foreground color to white
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);

    // Static text for score and lives
    static char score_text[] = "    Score ";
    static char score_number[7];
    static char lives_text[] = "    LIVES ";

    // Draw score text and number
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)score_text, -1, 0, 8, true);
    sprintf(score_number, "%06d", app_p->score);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)score_number, -1, 60, 8, true);

    // Draw lives text
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lives_text, -1, 0, 112, true);

    // Draw horizontal lines to separate sections
    Graphics_drawLine(&hal_p->g_sContext, 0, 24, 127, 24);
    Graphics_drawLine(&hal_p->g_sContext, 0, 48, 127, 48);
    Graphics_drawLine(&hal_p->g_sContext, 0, 72, 127, 72);
    Graphics_drawLine(&hal_p->g_sContext, 0, 96, 127, 96);

    // Handle player movement and obstacles
    handle_player(app_p, hal_p, moveToLeft, moveToRight, moveUp, moveDown);
    handleObstacleGeneration(app_p, hal_p);
    handle_lives(app_p, hal_p);
}

// This function handles the display of remaining lives and triggers game over if all lives are lost.
void handle_lives(Application* app_p, HAL* hal_p) {
    int i;
    // Check for collision with obstacles
    if (checkCollision(app_p, hal_p)) {
        // Set foreground color to black to update lives display
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
        // Erase hearts representing remaining lives
        for (i = 0; i < app_p->lives; i++)
            print_rectangle(app_p, hal_p, (60 + (8 * i)), (60 + (8 * i)) + 3, 113, 116);
        // Decrease remaining lives count
        app_p->lives -= 1;
        // Check if no lives remaining
        if (app_p->lives == 0) {
            // Set game state to game over
            app_p->screen_state = game_over;
            // Clear screen and display game over message
            clear_screen(hal_p);
            print_over(app_p, hal_p);
            // Update high scores
            updateHighScores(app_p, app_p->score);
        }
    }
    // Set foreground color to red for remaining lives display
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_RED);
    // Draw hearts representing remaining lives
    for (i = 0; i < app_p->lives; i++)
        print_rectangle(app_p, hal_p, (60 + (8 * i)), (60 + (8 * i)) + 3, 113, 116);
}

// This function handles the movement and animation of the player character.
void handle_player(Application* app_p, HAL* hal_p, bool moveToLeft, bool moveToRight, bool moveUp, bool moveDown) {
    // Set foreground color to blue for player character
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLUE);
    // Draw player character
    Graphics_fillCircle(&hal_p->g_sContext, app_p->player_x, app_p->player_y, 2);

    // Handle player movement
    if ((moveToLeft && (app_p->player_x > 3)) || (moveToRight && (app_p->player_x < 124)) ||
        (!app_p->jump && ((moveDown && (app_p->player_y < 95)) || (moveUp && (app_p->player_y > 47))))) {
        // Erase previous position of player character
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_fillCircle(&hal_p->g_sContext, app_p->player_x, app_p->player_y, 2);
        // Update player position based on movement
        if (moveToLeft) {
            app_p->player_x -= 1;
        } else if (moveToRight) {
            app_p->player_x += 1;
        } else if (moveUp) {
            app_p->player_y -= 24;
        } else if (moveDown) {
            app_p->player_y += 24;
        }
        // Draw player character at new position
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_fillCircle(&hal_p->g_sContext, app_p->player_x, app_p->player_y, 2);
    }

    // Handle player jump animation
    if (app_p->jump) {
        static int jump_height = 17;
        static bool jumping_up = true;
        // Erase previous position of player character
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_fillCircle(&hal_p->g_sContext, app_p->player_x, app_p->player_y, 2);

        // Perform jump animation
        if (jumping_up) {
            app_p->player_y -= 1;
            if (app_p->player_y <= app_p->temp_player_y - jump_height) {
                jumping_up = false;
            }
        } else {
            app_p->player_y += 1;
            if (app_p->player_y >= app_p->temp_player_y) {
                app_p->player_y = app_p->temp_player_y; // Ensure player returns to original position
                app_p->jump = false; // Reset jump flag
                jumping_up = true; // Reset jump direction for next jump
            }
        }
        // Draw player character at new position
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_fillCircle(&hal_p->g_sContext, app_p->player_x, app_p->player_y, 2);
    }
}

// This function prints the game over screen, including the final score and instructions to return to the main menu.
void print_over(Application* app_p, HAL* hal_p) {
    // Static array to store lines of text
    static char lines[MAX_LINES][MAX_STRING_LENGTH];
    static char score[7];

    // Draw "GAME OVER" message
    strcpy(lines[2], "GAME OVER");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[2], -1, 35, 40, true);

    // Draw final score
    strcpy(lines[3], "Score:");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[3], -1, 49, 56, true);
    sprintf(score, "%06d", app_p->score);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)score, -1, 49, 64, true);

    // Draw instructions to return to main menu
    strcpy(lines[4], "(Press JSB to return)");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[4], -1, 0, 104, true);
    strcpy(lines[5], " (to the main menu)");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[5], -1, 0, 112, true);
}

// This function prints a rectangle on the screen.
void print_rectangle(Application* app_p, HAL* hal_p, int min_x, int max_x, int min_y, int max_y) {
    // Define rectangle coordinates
    Graphics_Rectangle R;
    R.xMin = min_x;
    R.xMax = max_x;
    R.yMin = min_y;
    R.yMax = max_y;

    // Fill rectangle with current foreground color
    Graphics_fillRectangle(&hal_p->g_sContext, &R);
}

// Function to print the instructions screen
void print_instructions(Application* app_p, HAL* hal_p){
    // Static array to store lines of text
    static char lines[MAX_LINES][MAX_STRING_LENGTH];

    // Copy and draw each line of text
    strcpy(lines[0], "HOW TO PLAY");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[0], -1, 25, 8, true);
    strcpy(lines[1], "---------------------");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[1], -1, 0, 16, true);
    strcpy(lines[2], "Avoid the obstacles!");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[2], -1, 0, 24, true);
    strcpy(lines[3], "Score points by");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[3], -1, 0, 32, true);
    strcpy(lines[4], "surviving as long");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[4], -1, 0, 40, true);
    strcpy(lines[5], " as possible. If you");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[5], -1, 0, 48, true);
    strcpy(lines[6], "collide with an");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[6], -1, 0, 56, true);
    strcpy(lines[7], "obstacle, you will");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[7], -1, 0, 64, true);
    strcpy(lines[8], "lose a life. Try to");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[8], -1, 0, 72, true);
    strcpy(lines[9], "get a high score!");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[9], -1, 0, 80, true);
    strcpy(lines[10], "Use JOYSTICK to move");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[10], -1, 0, 96, true);
    strcpy(lines[11], "Press BB1 to jump");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[11], -1, 0, 104, true);
    strcpy(lines[12], "(Press JSB to return)");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *) lines[12], -1, 0, 120, true);
}

// This function prints the high scores screen, including the top three scores and instructions to return to the main menu.
void print_scores(Application* app_p, HAL* hal_p) {
    static char lines[MAX_LINES][MAX_STRING_LENGTH];
    static char score1[7];
    static char score2[7];
    static char score3[7];

    // Copy and draw each line of text
    strcpy(lines[0], "     HIGH SCORES");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[0], -1, 0, 8, true);
    strcpy(lines[1], "1:");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[1], -1, 55, 32, true);
    strcpy(lines[2], "2:");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[2], -1, 55, 56, true);
    strcpy(lines[3], "3:");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[3], -1, 55, 80, true);

    // Draw instructions to return to main menu
    strcpy(lines[4], "(Press JSB to return)");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[4], -1, 0, 104, true);
    strcpy(lines[5], " (to the main menu)");
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)lines[5], -1, 0, 112, true);

    // Display top three high scores
    sprintf(score1, "%06d", app_p->high_scores[0]);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)score1, -1, 43, 40, true);

    sprintf(score2, "%06d", app_p->high_scores[1]);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)score2, -1, 43, 64, true);

    sprintf(score3, "%06d", app_p->high_scores[2]);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)score3, -1, 43, 88, true);
}

// This function draws the x and y coordinates on the screen.
void drawXY(HAL* hal_p, unsigned int x, unsigned int y) {
    int8_t string[6];

    // Draw x coordinate
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)"x=", -1, 10, 5, true);
    make_5digit_NumString(x, string);
    Graphics_drawString(&hal_p->g_sContext, string, -1, 30, 5, true);

    // Draw y coordinate
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)"y=", -1, 10, 15, true);
    make_5digit_NumString(y, string);
    Graphics_drawString(&hal_p->g_sContext, string, -1, 30, 15, true);
}

// This function converts an unsigned integer to a 5-digit string.
void make_5digit_NumString(unsigned int num, int8_t *string) {
    string[0] = (num / 10000) + '0';
    string[1] = ((num % 10000) / 1000) + '0';
    string[2] = ((num % 1000) / 100) + '0';
    string[3] = ((num % 100) / 10) + '0';
    string[4] = ((num % 10) / 1) + '0';
    string[5] = 0;
}

// This function draws an obstacle on the screen based on its type and position.
void draw_obstacle(Application* app_p, HAL* hal_p, int x, int y, obstacle type) {
    switch (type) {
        case SMALL:
        case SMALL_UP:
            // Draw small obstacle
            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
            print_rectangle(app_p, hal_p, x, x + 6, y - 10, y);
            x -= 1;
            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_CYAN);
            print_rectangle(app_p, hal_p, x, x + 6, y - 10, y);
            break;
        case BIG:
            // Draw big obstacle
            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
            print_rectangle(app_p, hal_p, x, x + 6, y - 22, y);
            x -= 1;
            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_CYAN);
            print_rectangle(app_p, hal_p, x, x + 6, y - 22, y);
            break;
    }
}

// This function handles the generation and movement of obstacles on the screen.
void handleObstacleGeneration(Application* app_p, HAL* hal_p) {
    static int counter = 0;
    int i;
    CheckDifficulty(app_p);

    // Check if the obstacle timer has expired
    if (SWTimer_expired(&app_p->obst_speed)) {
        // Generate a new obstacle only if the current obstacle has passed a certain threshold
        if (app_p->obstacles_x[counter] <= -7) {
            app_p->obstacles_x[counter] = 127;
            app_p->obstacle_type[counter] = generateRandomObstacle();
            app_p->obstacles_y[counter] = generateRandomY(app_p->obstacle_type[counter]);
        }

        // Move to the next obstacle slot
        counter = circularIncrement(&counter);

        // Restart the obstacle timer for the next iteration
        SWTimer_start(&app_p->obst_speed);
    }

    // Update existing obstacles
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (app_p->obstacles_x[i] > -7) {
            // Draw and move obstacle
            draw_obstacle(app_p, hal_p, app_p->obstacles_x[i], app_p->obstacles_y[i], app_p->obstacle_type[i]);
            app_p->obstacles_x[i] -= 1; // Move the obstacles to the left

            // Increment score if an obstacle disappears off the screen
            if (app_p->obstacles_x[i] <= -7 && (app_p->obstacles_y[i] == 47 || app_p->obstacles_y[i] == 71 || app_p->obstacles_y[i] == 95
                    || app_p->obstacles_y[i] == 35 || app_p->obstacles_y[i] == 59 || app_p->obstacles_y[i] == 83)) {
                app_p->score += 500;
                if (app_p->score % 5000 == 0)
                    app_p->score_changed = true;
            }
        }
    }
}



// Function to perform circular increment from 0 to 3
int circularIncrement(int* current) {
    // Increment the current value and handle overflow
    *current = (*current + 1) % MAX_OBSTACLES;
    return *current;
}

// This function generates a random y-coordinate for obstacles based on their type.
int generateRandomY(obstacle type) {
    // Seed the random number generator
    srand(time(NULL));
    // Generate a random number between 0 and 2
    int rand_num = rand() % 3; // rand() % n generates a random number between 0 and n-1

    // Map the random number to one of the desired y-coordinate values
    if (type == SMALL || type == BIG)
        switch (rand_num) {
            case 0:
                return 47;
            case 1:
                return 71;
            case 2:
                return 95;
            default:
                return 47; // Default to 47 if an unexpected value is generated
        }
    else
        switch (rand_num) {
            case 0:
                return 35;
            case 1:
                return 59;
            case 2:
                return 83;
            default:
                return 35; // Default to 47 if an unexpected value is generated
        }
}

// This function generates a random obstacle type.
obstacle generateRandomObstacle() {
    // Seed the random number generator
    srand(time(NULL));
    // Generate a random number between 0 and 2
    int rand_num = rand() % 3; // rand() % n generates a random number between 0 and n-1

    // Map the random number to one of the desired obstacle types
    switch (rand_num) {
        case 0:
            return SMALL;
        case 1:
            return SMALL_UP;
        case 2:
            return BIG;
        default:
            return SMALL; // Default to SMALL if an unexpected value is generated
    }
}

// Function to check if the player touches an obstacle.
bool checkCollision(Application* app_p, HAL* hal_p) {
    int i;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        // Check if the obstacle has already caused a collision
        if (!app_p->obstacle_collision[i]) {
            // Check if the player's position overlaps with the position of the obstacle
            switch (app_p->obstacle_type[i]) {
                case SMALL:
                    if (app_p->player_x + 2 >= app_p->obstacles_x[i] && app_p->player_x - 2 <= app_p->obstacles_x[i] + 6 &&
                        app_p->player_y - 2 >= app_p->obstacles_y[i] - 10 && app_p->player_y + 2 <= app_p->obstacles_y[i]) {
                        app_p->obstacle_collision[i] = true; // Update the collision flag
                        return true; // Collision detected
                    }
                    break;
                case SMALL_UP:
                    if (app_p->player_x + 2 >= app_p->obstacles_x[i] && app_p->player_x - 2 <= app_p->obstacles_x[i] + 6 &&
                        app_p->player_y + 2 >= app_p->obstacles_y[i] - 10 && app_p->player_y - 2 <= app_p->obstacles_y[i]) {
                        app_p->obstacle_collision[i] = true; // Update the collision flag
                        return true; // Collision detected
                    }
                    break;
                case BIG:
                    if (app_p->player_x + 2 >= app_p->obstacles_x[i] && app_p->player_x - 2 <= app_p->obstacles_x[i] + 6 &&
                        app_p->player_y - 2 >= app_p->obstacles_y[i] - 22 && app_p->player_y + 2 <= app_p->obstacles_y[i]) {
                        app_p->obstacle_collision[i] = true; // Update the collision flag
                        return true; // Collision detected
                    }
                    break;
            }
        }
    }
    return false; // No collision detected
}


// Function to check and adjust the game difficulty based on the player's score.
void CheckDifficulty(Application* app_p){
    static int count = 100;

    // If the player's score has changed, adjust the obstacle speed
    if (app_p->score_changed){
        app_p->obst_speed = SWTimer_construct(OBSTACLE_TIME - count);
        if (count <= 900)
            count += 100;
        app_p->score_changed = false;
    }
}

// Function to reset the game state when restarting the game.
void reset_game(Application* app_p, HAL* hal_p) {
    // Reset player's position
    app_p->player_x = 15;
    app_p->player_y = 95;

    // Reset score and lives
    app_p->score = 0;
    app_p->lives = 3;

    // Reset obstacle positions and collision flags
    int i;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        app_p->obstacles_x[i] = -10;
        app_p->obstacles_y[i] = -10;
        app_p->obstacle_collision[i] = false;
    }

    // Reset any other relevant variables or states
}

// Function to update high scores after game over and sort them in descending order.
void updateHighScores(Application* app_p, int score) {
    int i, j, temp;

    // Update high scores array if the current score is higher than existing ones
    for (i = 0; i < MAX_SCORES; i++) {
        if (score > app_p->high_scores[i]) {
            for (j = MAX_SCORES - 1; j > i; j--) {
                app_p->high_scores[j] = app_p->high_scores[j - 1];
            }
            app_p->high_scores[i] = score;
            break;
        }
    }

    // Sort high scores array in descending order
    for (i = 0; i < MAX_SCORES - 1; i++) {
        for (j = 0; j < MAX_SCORES - i - 1; j++) {
            if (app_p->high_scores[j] < app_p->high_scores[j + 1]) {
                temp = app_p->high_scores[j];
                app_p->high_scores[j] = app_p->high_scores[j + 1];
                app_p->high_scores[j + 1] = temp;
            }
        }
    }
}

// Function to handle UART communication for sending and receiving characters.
void uart_print(Application* app_p, HAL* hal_p){
    if (UART_hasChar(&hal_p->uart)) {
        // The character received from the serial terminal
        char rxChar = UART_getChar(&hal_p->uart);

        // Interpret the incoming character and prepare a response
        char txChar = Application_interpretIncomingChar(rxChar);

        // Proceed if the interpreted character is not null
        if (txChar != '\0') {
            // Send the character back through UART if transmission is possible
            if (UART_canSend(&hal_p->uart))
                UART_sendChar(&hal_p->uart, txChar);
        }
    }
}

/**
 * Interprets a character which was incoming and returns an interpretation of
 * that character. If the input character is a letter, it returns 'L' for Letter,
 * if a number it returns 'N' for Number, and if something else, it returns '\0' for Other.
 *
 * @param rxChar: Input character
 * @return :  Output character interpretation
 */
char Application_interpretIncomingChar(char rxChar) {
    char txChar;

    // Numbers - if the character entered was a number, transfer back the number
    if (rxChar >= '0' && rxChar <= '9') {
        txChar = rxChar;
    }
    // Letters - if the character entered was a letter, transfer back the letter
    else if ((rxChar >= 'a' && rxChar <= 'z') || (rxChar >= 'A' && rxChar <= 'Z')) {
        txChar = rxChar;
    }
    else {
        return '\0'; // Return null character for any other character types
    }

    return txChar;
}
