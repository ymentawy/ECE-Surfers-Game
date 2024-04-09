#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <HAL/HAL.h>

// Constants for the application

// Maximum length for strings
#define MAX_STRING_LENGTH 21

// Maximum number of lines
#define MAX_LINES 16

// Maximum number of lives
#define MAX_LIVES 3

// Maximum coordinate values
#define MAX_COORDINATE 127
#define MIN_COORDINATE 0

// Speed at which assets are loaded
#define LOAD_SPEED 2

// Increment value for Y position
#define Y_INCREMENTAL 8

// Maximum number of obstacles
#define MAX_OBSTACLES 22

// Maximum number of high scores
#define MAX_SCORES 3


// Structure for the Application object
struct _Application {
  // Application members and FSM state variables
  UART_Baudrate baudChoice;           // Selected baud rate
  bool firstCall;                     // Flag for first call to application
  screen screen_state;                // Enumeration for different screen states
  unsigned int vx;                    // Velocity in X direction
  unsigned int vy;                    // Velocity in Y direction
  bool joyStickPushedUp;              // Flag for joystick up input
  bool joyStickPushedDown;            // Flag for joystick down input
  bool joyStickPushedtoRight;         // Flag for joystick right input
  bool joyStickPushedtoLeft;          // Flag for joystick left input
  bool jump;                          // Flag for player jump action
  bool score_changed;                 // Flag indicating if score changed
  bool obstacle_collision[MAX_OBSTACLES]; // Array of obstacle collision flags
  obstacle obstacle_type[MAX_OBSTACLES];  // Array of obstacle types
  int main_pos;                       // Position in the main loop
  bool first_main;                    // Flag for first call to main loop
  int score;                          // Player score
  int lives;                          // Player lives
  int player_x;                       // Player X position
  int player_y;                       // Player Y position
  int temp_player_y;                  // Temporary player Y position
  int high_scores[MAX_SCORES];        // Array of high scores
  int obstacles_x[MAX_OBSTACLES];     // Array of obstacle X positions
  int obstacles_y[MAX_OBSTACLES];     // Array of obstacle Y positions
  SWTimer obst_speed;                 // Speed of obstacles
};
typedef struct _Application Application;

// Constructor for the Application object
Application Application_construct();

// Main super-loop function
void Application_loop(Application* app, HAL* hal);

// Updates communications settings
void Application_updateCommunications(Application* app, HAL* hal);

// Interprets incoming character and echoes back to terminal what kind of character was received
char Application_interpretIncomingChar(char);

// Generic circular increment function
uint32_t CircularIncrement(uint32_t value, uint32_t maximum);

// Finite state machine for game
void Game_FSM(Application* app_p, HAL* hal_p);

// Function declarations for printing different screens
void clear_screen(HAL* hal_p);
void print_title(Application* app_p, HAL* hal_p);
void print_main(Application* app_p, HAL* hal_p, bool moveUp, bool moveDown);
void print_game(Application* app_p, HAL* hal_p, bool moveToLeft, bool moveToRight, bool moveUp, bool moveDown);
void handle_player(Application* app_p, HAL* hal_p, bool moveToLeft, bool moveToRight, bool moveUp, bool moveDown);
void handle_lives(Application* app_p, HAL* hal_p);
void print_instructions(Application* app_p, HAL* hal_p);
void print_scores(Application* app_p, HAL* hal_p);
void print_rectangle(Application* app_p, HAL* hal_p, int min_x, int max_x, int min_y, int max_y);
void draw_obstacle(Application* app_p, HAL* hal_p, int x, int y, obstacle type);
obstacle generateRandomObstacle();
int generateRandomY(obstacle type);
int circularIncrement(int* current);
void handleObstacleGeneration(Application* app_p, HAL* hal_p);
bool checkCollision(Application* app_p, HAL* hal_p);
int clamp(int value, int min, int max);
void print_over(Application* app_p, HAL* hal_p);
void reset_game(Application* app_p, HAL* hal_p);
void make_5digit_NumString(unsigned int num, int8_t *string);
void updateHighScores(Application* app_p, int score);
void CheckDifficulty(Application* app_p);

#endif /* APPLICATION_H_ */
