# ECE Surfers Game - README

## Overview

Welcome to the ECE Surfers game! This README provides a comprehensive overview of the project, including its purpose, features, implementation details, testing procedures, code quality, and conclusion.

## Summary

The ECE Surfers game is a multiplayer game developed for the MSP432 microcontroller. It offers an immersive gaming experience that seamlessly integrates the LCD and GPIO button controls. This README serves as a resource for understanding the technical aspects, implementation strategies, and challenges faced during the development process.

## Project Description

The project meticulously implements the game, ensuring flawless functionality and a smooth user experience. From the title screen to the engaging game interface, each element is well-designed to provide a fluid and interactive journey for players. The implementation adheres to project specifications, employing finite-state machines (FSMs) to manage transitions between screens seamlessly.

## Implementation Details

The game consists of several screens, each managed by specific functions:

- **Title Screen:** `print_title`
- **Main Menu Screen:** `main_menu_state`
- **Instructions Screen:** `instructions_state`
- **Game Screen:** `game_state`
- **Game Over Screen:** `game_over_state`
- **High Scores Screen:** `high_scores_state`

Each screen utilizes various functions to handle player input, display information, and manage transitions between screens.

## Testing

To ensure the robustness and reliability of the implemented system, rigorous testing procedures were conducted at various stages of development. Unit testing and integration testing were performed to verify correctness, identify bugs, and assess the interactions between different components.

## Code Quality

The codebase adheres to best practices for code quality, ensuring readability, maintainability, and scalability. Comments are strategically placed throughout the codebase to provide clarity and context, while functions utilize parameter passing to promote modularity and encapsulation. Symbolic labels and macros are used consistently to enhance code readability, and a structured approach to function design improves code clarity and facilitates code reuse.

## Conclusion

In conclusion, the ECE Surfers game project has successfully realized a game experience that meets the specified requirements. Through diligent development, thorough testing, and adherence to code quality principles, the project demonstrates the practical application of software engineering in embedded systems and interactive applications.
