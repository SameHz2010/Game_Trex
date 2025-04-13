/*
 * game_config.h
 *
 *  Created on: Apr 14, 2025
 *      Author: HuPo
 */

#ifndef SRC_GAME_CONFIG_H_
#define SRC_GAME_CONFIG_H_

// Display dimensions
#define SCREEN_WIDTH           128
#define SCREEN_HEIGHT          64

// Game elements dimensions
#define GROUND_HEIGHT          4
#define DINO_X                 10
#define DINO_WIDTH             16
#define DINO_HEIGHT            16
#define BIRD_WIDTH             16
#define BIRD_HEIGHT            12
#define TREE_SMALL_WIDTH       16
#define TREE_SMALL_HEIGHT      14
#define TREE_TALL_WIDTH        16
#define TREE_TALL_HEIGHT       16
#define TREE_SMALL_TALL_WIDTH  16
#define TREE_SMALL_TALL_HEIGHT 16
#define MAX_CLOUDS             5

// Physics constants
#define GRAVITY                1.0f
#define JUMP_STRENGTH          7.0f

// Game speed settings
#define SPEED_EASY             4
#define SPEED_NORMAL           7
#define SPEED_HARD             10

// Constants for UI positions to avoid magic numbers
#define MENU_TITLE_X         14
#define MENU_TITLE_Y         1
#define MENU_OPTION_X        30
#define MENU_PLAY_Y          20
#define MENU_SETTING_Y       30
#define MENU_LOGO_X          46
#define MENU_LOGO_Y          40
#define MENU_ARROW_X         1
#define SETTINGS_LABEL_X     20
#define SETTINGS_VALUE_X     70
#define SETTINGS_EXIT_Y      30
#define GAME_OVER_TITLE_X    15
#define SCORE_DISPLAY_X      30
#define HI_SCORE_Y           20
#define CURRENT_SCORE_Y      31
#define RESTART_OPTION_Y     40
#define SETTING_OPTION_Y     50
#define SCORE_TEXT_X         50
#define SCORE_TEXT_Y         1

// Button debounce delay
#define BUTTON_DEBOUNCE_DELAY 100

// Enum for obstacle types
typedef enum {
    OBSTACLE_SMALL_CACTUS,
    OBSTACLE_TALL_CACTUS,
    OBSTACLE_COMBINED_CACTUS,
    OBSTACLE_BIRD
} ObstacleType;

#endif /* SRC_GAME_CONFIG_H_ */
