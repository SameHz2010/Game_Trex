/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SH1106.h"      // Include display driver for OLED display
#include "fonts.h"       // Include font definitions for text display
#include "dino_frames.h" // Include sprite data for dinosaur, obstacles, ground, cloud.
#include <stdbool.h>     // Include standard boolean type
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
#define GROUND_HEIGHT 4          // Height of the ground in pixels
#define DINO_X 10                // Fixed X position of dinosaur from left edge
#define DINO_WIDTH 16            // Width of dinosaur sprite
#define DINO_HEIGHT 16           // Height of dinosaur sprite
#define MAX_CLOUDS 5             // Maximum number of clouds in the sky

// Obstacle dimensions
#define BIRD_WIDTH 16            // Width of bird obstacle
#define BIRD_HEIGHT 12           // Height of bird obstacle
#define TREE_SMALL_WIDTH 16      // Width of small cactus
#define TREE_SMALL_HEIGHT 14     // Height of small cactus
#define TREE_TALL_WIDTH 16       // Width of tall cactus
#define TREE_TALL_HEIGHT 16      // Height of tall cactus
#define TREE_SMALL_TALL_WIDTH 16 // Width of combined cactus
#define TREE_SMALL_TALL_HEIGHT 16 // Height of combined cactus

// Screen dimensions
#define SCREEN_WIDTH 128         // Width of the OLED display
#define SCREEN_HEIGHT 64         // Height of the OLED display

// Physics constants
static const float GRAVITY = 1.0f;        // Gravity force pulling dinosaur down
static const float JUMP_STRENGTH = 7.0f;  // Initial upward velocity when jumping
bool settingMenuOpen = false;

// Game state structure to keep track of all game variables
typedef struct {
	bool Menu;        // Flag for menu display state
	bool GameOver;    // Flag for game over state
	uint16_t Score;   // Current game score
	uint16_t Hscore;  // High score

	bool Jumping;     // Flag for dinosaur jumping state
	bool Ducking;     // Flag for dinosaur ducking state

	int16_t Dino;             // Y position of dinosaur
	int16_t ObstacleX;        // X position of current obstacle
	uint8_t ObstacleType;     // Type of current obstacle (0-3)
	uint8_t FrameCount;       // Counter for animation frames

	int16_t V_jump;           // Current vertical velocity for jump
	int16_t S_Obstacle;       // Obstacle movement speed
	int16_t Obstacle_Y;       // Y offset for obstacle (used for flying birds)

	int color;                // Current drawing color
	int bgColor;              // Current background color
	bool colorChanged;        // Flag to track color inversion
	int16_t initialObstacleSpeed;
} GameState;

// Initialize default game state
static GameState game = {
		.Menu = true,                                              // Start in menu state
		.GameOver = false,                                         // Not game over initially
		.Score = 0,                                                // Start with zero score
		.Hscore = 0,                                               // Start with zero high score

		.Jumping = false,                                          // Not jumping initially
		.Ducking = false,                                          // Not ducking initially

		.Dino = SCREEN_HEIGHT - GROUND_HEIGHT - DINO_HEIGHT,       // Position dinosaur on ground
		.ObstacleX = SCREEN_WIDTH,                                 // Start obstacle at right edge
		.ObstacleType = 0,                                         // Default to first obstacle type
		.FrameCount = 0,                                           // Initialize animation counter

		.V_jump = 0,                                               // No initial jump velocity
		.S_Obstacle = 4,                                           // Initial obstacle speed

		.color = SH1106_COLOR_WHITE,                               // Default draw color
		.bgColor = SH1106_COLOR_BLACK,                             // Default background color
		.colorChanged = false                                      // No color inversion yet
};

// Sprite structure to hold graphical elements
typedef struct {
	const unsigned char *Bitmap;  // Pointer to bitmap data
	int Width;                    // Width of sprite
	int Height;                   // Height of sprite
} Sprite;

// Cloud object structure
typedef struct {
	int16_t x;         // X position
	int16_t y;         // Y position
	uint8_t type;      // Cloud type (0-2)
	int16_t moveCount; // Movement counter
	bool active;       // Whether cloud is visible
} Cloud;

static Cloud clouds[MAX_CLOUDS];  // Array of cloud objects
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Update_Game_Logic() {
	// Jump control - button 0 pressed, not already jumping, and not ducking
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET && !game.Jumping && !game.Ducking) {
		game.Jumping = true;
		game.V_jump = JUMP_STRENGTH;  // Set initial upward velocity
	}
	// Duck control - button 1 pressed and not jumping
	else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
		game.Ducking = true;
	} else {
		game.Ducking = false;
	}

	// Jumping physics
	if (game.Jumping) {
		game.Dino -= game.V_jump;          // Move dinosaur vertically by velocity
		game.V_jump -= GRAVITY;            // Reduce velocity by gravity
		if (game.Dino >= SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT) {  // If hit ground
			game.Dino = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;     // Set to ground position
			game.Jumping = false;                                        // End jump
		}
		//nếu đang nhảy và nhấn nút duck
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
		{
			game.Dino = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
			game.Jumping = false;
			game.V_jump = 0;
			game.Ducking = true;
		}
	}

	// Obstacle movement
	game.ObstacleX -= game.S_Obstacle;  // Move obstacle left by speed
	if (game.ObstacleX < -TREE_SMALL_TALL_WIDTH) {  // If obstacle moves off screen
		game.ObstacleX = SCREEN_WIDTH + (rand() % 50 + 30);  // Reset to right with random offset
		game.ObstacleType = rand() % 4;                      // Random obstacle type
		game.Score += 10;                                   // Increase score
		// Increase difficulty every 50 points up to maximum speed of 10
		if (game.Score % 50 == 0 && game.S_Obstacle <= 100) {
			game.S_Obstacle++;
		}
		// Set vertical position for bird obstacle (either high or low)
		game.Obstacle_Y = (game.ObstacleType == 3) ? ((rand() % 2 == 0) ? 2 : 14) : 0;
	}
}

// Function to draw the ground
void Draw_Ground(int color) {
	uint8_t groundFrame = game.FrameCount / 3 % 3;  // Cycle through 3 ground frames
	const uint8_t *groundSprite;
	switch (groundFrame) {
	case 0: groundSprite = ground_1; break;  // First ground sprite
	case 1: groundSprite = ground_2; break;  // Second ground sprite
	case 2: groundSprite = ground_3; break;  // Third ground sprite
	}
	// Draw ground at bottom of screen
	SH1106_DrawBitmap(0, SCREEN_HEIGHT - GROUND_HEIGHT, groundSprite, SCREEN_WIDTH, GROUND_HEIGHT, color);
}

// Function to draw clouds in the sky
void Draw_Clouds(int color) {
	const unsigned char *cloudSprites[3] = {cloud1, cloud2, cloud3};  // Three cloud sprites
	static int16_t nextCloudSpawn = 0;  // Counter for when to spawn next cloud

	// Update and draw each active cloud
	for (int i = 0; i < MAX_CLOUDS; i++) {
		if (clouds[i].active) {
			// Draw cloud sprite
			SH1106_DrawBitmap(clouds[i].x, clouds[i].y, cloudSprites[clouds[i].type], 16, 8, color);
			clouds[i].x -= 1;      // Move cloud left slowly
			clouds[i].moveCount++;

			// Deactivate cloud if it moves off screen
			if (clouds[i].x < -16) {
				clouds[i].active = false;
			}
		}
	}

	// Spawn new cloud when counter reaches threshold
	if (nextCloudSpawn >= 50) {
		for (int i = 0; i < MAX_CLOUDS; i++) {
			if (!clouds[i].active) {
				clouds[i].x = SCREEN_WIDTH;           // Start at right edge
				clouds[i].y = 10 + (rand() % 3) * 10; // Random height
				clouds[i].type = rand() % 3;          // Random cloud type
				clouds[i].moveCount = 0;
				clouds[i].active = true;
				nextCloudSpawn = 0;                  // Reset spawn counter
				break;
			}
		}
	}

	nextCloudSpawn++;  // Increment spawn counter
}

// Function to update color scheme (inversions at certain score thresholds)
void Update_Color() {
	// Invert colors every 30 points
	if (game.Score % 100 == 0 && game.Score != 0 && !game.colorChanged) {
		if (game.color == SH1106_COLOR_WHITE) {
			game.color = SH1106_COLOR_BLACK;        // Invert to black drawing
			game.bgColor = SH1106_COLOR_WHITE;      // On white background
		} else {
			game.color = SH1106_COLOR_WHITE;        // Invert to white drawing
			game.bgColor = SH1106_COLOR_BLACK;      // On black background
		}
		game.colorChanged = true;                  // Set flag to prevent multiple inversions
	} else if (game.Score % 30 != 0) {
		game.colorChanged = false;                 // Reset flag when not at score threshold
	}
}

// Function to select appropriate dinosaur sprite based on state
Sprite Selec_Sprite_Player() {
	Sprite pl;
	if (game.Ducking) {
		// Ducking animation - alternate between two ducking frames
		pl.Bitmap = (game.FrameCount / 3 % 2 == 0 ? trex_duck1 : trex_duck2);
		pl.Width = 32;      // Ducking dinosaur is wider
		pl.Height = 16;
	} else {
		// Running animation - alternate between two running frames
		pl.Bitmap = (game.FrameCount / 3 % 2 == 0 ? trex_run1 : trex_run2);
		pl.Width = 16;
		pl.Height = 16;
	}
	return pl;
}

// Function to select appropriate obstacle sprite based on type
Sprite Selec_Sprite_Obstacle() {
	Sprite ob;

	switch (game.ObstacleType) {
	case 0:  // Small cactus
		ob.Width = TREE_SMALL_WIDTH;
		ob.Height = TREE_SMALL_HEIGHT;
		ob.Bitmap = tree_small;
		break;
	case 1:  // Tall cactus
		ob.Width = TREE_TALL_WIDTH;
		ob.Height = TREE_TALL_HEIGHT;
		ob.Bitmap = tree_tall;
		break;
	case 2:  // Combined cactus
		ob.Width = TREE_SMALL_TALL_WIDTH;
		ob.Height = TREE_SMALL_TALL_HEIGHT;
		ob.Bitmap = tree_small_tall;
		break;
	default:  // Bird (type 3) - animated with two frames
		ob.Width = BIRD_WIDTH;
		ob.Height = BIRD_HEIGHT;
		ob.Bitmap = (game.FrameCount / 3 % 2 == 0 ? bird_1 : bird_2);
		break;
	}

	return ob;
}

// Main rendering function for game state
void Draw_Game() {
	Update_Color();  // Update color scheme

	SH1106_Fill(game.bgColor);  // Fill screen with background color
	Draw_Ground(game.color);    // Draw ground
	Draw_Clouds(game.color);    // Draw clouds

	// Draw dinosaur
	Sprite pl = Selec_Sprite_Player();
	SH1106_DrawBitmap(DINO_X, game.Dino, pl.Bitmap, pl.Width, pl.Height, game.color);

	// Draw obstacle
	Sprite ob = Selec_Sprite_Obstacle();
	// Position obstacle at proper height based on type and offset
	SH1106_DrawBitmap(game.ObstacleX, SCREEN_HEIGHT - GROUND_HEIGHT - ob.Height - game.Obstacle_Y,
			ob.Bitmap, ob.Width, ob.Height, game.color);

	// Display scores
	char scoreStr[12];
	if(game.Score >= game.Hscore) {
		game.Hscore = game.Score;  // Update high score if needed
	}
	sprintf(scoreStr, "HI %d  %d", game.Hscore, game.Score);
	SH1106_GotoXY(50, 1);  // Position score at top of screen
	SH1106_Puts(scoreStr, &Font_7x10, game.color);
	SH1106_UpdateScreen();  // Update the display
}

// Function to check for collision between dinosaur and obstacle
bool Check_Collision() {
	int16_t obstacleWidth, obstacleHeight;
	int16_t obstacleY = SCREEN_HEIGHT - GROUND_HEIGHT;

	// Set dimensions based on obstacle type
	switch (game.ObstacleType) {
	case 0:  // Small cactus
		obstacleWidth = TREE_SMALL_WIDTH;
		obstacleHeight = TREE_SMALL_HEIGHT;
		obstacleY -= TREE_SMALL_HEIGHT;
		break;
	case 1:  // Tall cactus
		obstacleWidth = TREE_TALL_WIDTH;
		obstacleHeight = TREE_TALL_HEIGHT;
		obstacleY -= TREE_TALL_HEIGHT;
		break;
	case 2:  // Combined cactus
		obstacleWidth = TREE_SMALL_TALL_WIDTH;
		obstacleHeight = TREE_SMALL_TALL_HEIGHT;
		obstacleY -= TREE_SMALL_TALL_HEIGHT;
		break;
	case 3:  // Bird
		obstacleWidth = BIRD_WIDTH;
		obstacleHeight = BIRD_HEIGHT;
		obstacleY -= BIRD_HEIGHT + game.Obstacle_Y;  // Apply height offset for bird
		break;
	default:
		return false;
	}

	// Set dinosaur dimensions based on ducking state
	int16_t dinoWidth = DINO_WIDTH;
	int16_t dinoHeight = game.Ducking ? 10 : DINO_HEIGHT;  // Ducking has reduced height
	int16_t dinoY = game.Dino + (game.Ducking ? 6 : 0);    // Adjust Y for ducking

	// Check for overlap in both X and Y directions
	bool xOverlap = (DINO_X < game.ObstacleX + obstacleWidth) && (DINO_X + dinoWidth > game.ObstacleX);
	bool yOverlap = (dinoY < obstacleY + obstacleHeight) && (dinoY + dinoHeight > obstacleY);

	// Collision occurs when there's overlap in both directions
	return xOverlap && yOverlap;
}

// Reset game to initial state after game over
void Reset_Game(void) {
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) != GPIO_PIN_SET) {
		HAL_Delay(100);  // Debounce
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) != GPIO_PIN_SET) {}  // Wait for button press
		game.GameOver = false;
		game.Score = 0;  // Reset score
		game.Dino = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;  // Reset dinosaur position
		game.ObstacleX = SCREEN_WIDTH;  // Reset obstacle position
		game.S_Obstacle = game.initialObstacleSpeed;
		game.Jumping = false;
		game.Ducking = false;
		game.color = SH1106_COLOR_WHITE;
		game.bgColor = SH1106_COLOR_BLACK;

		// Clear all clouds
		for (int i = 0; i < MAX_CLOUDS; i++) {
			clouds[i].active = false;
		}

		Dino_Game();  // Restart game
	}
}

// Draw the game menu
void Draw_Menu() {
	SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen
	int currentSelection = 0; // 0 for Play, 1 for Setting
	bool inMainMenu = true; // Flag to track which menu we're in

	while (game.Menu) {
		if (inMainMenu) {
			// Draw main menu
			SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen

			// Display title
			SH1106_GotoXY(14, 1);
			SH1106_Puts("T-REX GAME", &Font_11x18, SH1106_COLOR_WHITE);

			// Display menu options
			SH1106_GotoXY(30, 20);
			SH1106_Puts("Play", &Font_7x10, SH1106_COLOR_WHITE);

			SH1106_GotoXY(30, 30);
			SH1106_Puts("Setting", &Font_7x10, SH1106_COLOR_WHITE);

			// Display dinosaur logo
			SH1106_DrawBitmap(46, 40, logo_trex, 36, 36, SH1106_COLOR_WHITE);

			// Display arrow to indicate main menu selection
			if (currentSelection == 0) {
				SH1106_DrawBitmap(1, 20, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 30, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			} else {
				SH1106_DrawBitmap(1, 30, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 20, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			}

			// Handle button 1 (down/select)
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {} // Wait for release
				currentSelection = 1 - currentSelection; // Toggle selection
			}

			// Handle button 0 (confirm/start)
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {} // Wait for release

				if (currentSelection == 0) { // Play selected
					game.Menu = false;
					game.Jumping = false;
					// Start game
				} else { // Setting selected
					inMainMenu = false; // Exit main menu, enter settings menu
					SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen for settings menu
				}
			}
		} else {
			// Display title
			SH1106_GotoXY(14, 1);
			SH1106_Puts("T-REX GAME", &Font_11x18, SH1106_COLOR_WHITE);

			// Display dinosaur logo
			SH1106_DrawBitmap(46, 40, logo_trex, 36, 36, SH1106_COLOR_WHITE);

			// Settings menu
			static int settingSelection = 0; // 0 for Speed, 1 for Exit

			// Display settings options
			SH1106_GotoXY(20, 20);
			SH1106_Puts("Speed: ", &Font_7x10, SH1106_COLOR_WHITE);

			// Show current speed
			char speed[10];
			if (game.S_Obstacle == 4) sprintf(speed, "Easy");
			else if (game.S_Obstacle == 7) sprintf(speed, "Normal");
			else sprintf(speed, "Hard");

			SH1106_GotoXY(70, 20);
			SH1106_Puts(speed, &Font_7x10, SH1106_COLOR_WHITE);

			SH1106_GotoXY(20, 30);
			SH1106_Puts("Exit", &Font_7x10, SH1106_COLOR_WHITE);

			// Display arrow selection for settings
			if (settingSelection == 0) {
				SH1106_DrawBitmap(1, 20, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 30, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			} else {
				SH1106_DrawBitmap(1, 30, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 20, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			}

			// Handle button 1 (down/select) in settings
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {} // Wait for release
				settingSelection = 1 - settingSelection; // Toggle setting option
			}

			// Handle button 0 (confirm/action) in settings
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {} // Wait for release

				if (settingSelection == 0) {
					// Store the previous speed setting to compare
					uint8_t prevSpeed = game.S_Obstacle;

					// Update speed (cycle through options)
					if (game.S_Obstacle == 4) game.S_Obstacle = 7;
					else if (game.S_Obstacle == 7) game.S_Obstacle = 10;
					else game.S_Obstacle = 4;

					// Only redraw if the speed has changed
					if (prevSpeed != game.S_Obstacle) {
						// Clear the entire speed text area (x1=70, y1=20, x2=110, y2=28)
						// Font_7x10 is approximately 8 pixels high and "Normal" is ~40 pixels wide
						SH1106_ClearArea(70, 20, 110, 28, SH1106_COLOR_BLACK);

						// Get the new speed text
						char newSpeed[10];
						if (game.S_Obstacle == 4) sprintf(newSpeed, "Easy");
						else if (game.S_Obstacle == 7) sprintf(newSpeed, "Normal");
						else sprintf(newSpeed, "Hard");

						// Display the new speed
						SH1106_GotoXY(70, 20);
						SH1106_Puts(newSpeed, &Font_7x10, SH1106_COLOR_WHITE);
					}

					game.initialObstacleSpeed = game.S_Obstacle;
				} else {
					// Exit selected - return to main menu
					inMainMenu = true;
					settingSelection = 0; // Reset settings selection for next time
				}
			}
		}

		SH1106_UpdateScreen(); // Update screen after each change
	}
}

// Draw game over screen
void Draw_Game_Over() {
	SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen
	int currentSelection = 0; // 0 for Restart, 1 for Setting
	bool inMainOverMenu = true; // Flag to track which menu we're in

	while (game.GameOver) {
		if (inMainOverMenu) {
			// Display Game Over main screen
			SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen

			// Display "GAME OVER" text
			SH1106_GotoXY(15, 1);
			SH1106_Puts("GAME OVER", &Font_11x18, SH1106_COLOR_WHITE);

			// Display high score
			char scoreStr[12];
			if (game.Score >= game.Hscore) {
				game.Hscore = game.Score;  // Update high score if needed
			}
			sprintf(scoreStr, "HI: %d", game.Hscore);
			SH1106_GotoXY(30, 20);
			SH1106_Puts(scoreStr, &Font_7x10, SH1106_COLOR_WHITE);

			// Display current score
			sprintf(scoreStr, "SCORE: %d", game.Score);
			SH1106_GotoXY(30, 31);
			SH1106_Puts(scoreStr, &Font_7x10, SH1106_COLOR_WHITE);

			// Display restart option
			SH1106_GotoXY(30, 40);
			SH1106_Puts("Restart", &Font_7x10, SH1106_COLOR_WHITE);

			// Display setting option
			SH1106_GotoXY(30, 50);
			SH1106_Puts("Setting", &Font_7x10, SH1106_COLOR_WHITE);

			// Display arrow to indicate selection
			if (currentSelection == 0) {
				SH1106_DrawBitmap(1, 40, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 50, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			} else {
				SH1106_DrawBitmap(1, 50, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 40, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			}

			// Handle button 1 (down/select)
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {} // Wait for release
				currentSelection = 1 - currentSelection; // Toggle selection
			}

			// Handle button 0 (confirm/start)
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {} // Wait for release

				if (currentSelection == 0) { // Restart selected
					Reset_Game(); // Restart game
					return; // Exit game over screen
				} else { // Setting selected
					inMainOverMenu = false; // Switch to settings menu
					SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen for settings menu
				}
			}
		} else {
			// Settings menu
			static int settingSelection = 0; // 0 for Speed, 1 for Exit

			// Display settings options
			SH1106_GotoXY(20, 20);
			SH1106_Puts("Speed: ", &Font_7x10, SH1106_COLOR_WHITE);

			// Show current speed
			char speed[10];
			if (game.S_Obstacle == 4) sprintf(speed, "Easy");
			else if (game.S_Obstacle == 7) sprintf(speed, "Normal");
			else sprintf(speed, "Hard");

			SH1106_GotoXY(70, 20);
			SH1106_Puts(speed, &Font_7x10, SH1106_COLOR_WHITE);

			SH1106_GotoXY(20, 30);
			SH1106_Puts("Exit", &Font_7x10, SH1106_COLOR_WHITE);

			// Display arrow selection for settings
			if (settingSelection == 0) {
				SH1106_DrawBitmap(1, 20, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 30, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			} else {
				SH1106_DrawBitmap(1, 30, Arrow, 16, 9, SH1106_COLOR_WHITE);
				SH1106_DrawBitmap(1, 20, Arrow, 16, 9, SH1106_COLOR_BLACK); // Clear other arrow
			}

			// Handle button 1 (down/select) in settings
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {} // Wait for release
				settingSelection = 1 - settingSelection; // Toggle setting option
			}

			// Handle button 0 (confirm/action) in settings
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
				HAL_Delay(100); // Debounce
				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {} // Wait for release

				if (settingSelection == 0) {
					// Store the previous speed setting to compare
					uint8_t prevSpeed = game.S_Obstacle;

					// Update speed (cycle through options)
					if (game.S_Obstacle == 4) game.S_Obstacle = 7;
					else if (game.S_Obstacle == 7) game.S_Obstacle = 10;
					else game.S_Obstacle = 4;

					// Only redraw if the speed has changed
					if (prevSpeed != game.S_Obstacle) {
						// Clear the entire speed text area (x1=70, y1=20, x2=110, y2=28)
						SH1106_ClearArea(70, 20, 110, 28, SH1106_COLOR_BLACK);

						// Get the new speed text
						char newSpeed[10];
						if (game.S_Obstacle == 4) sprintf(newSpeed, "Easy");
						else if (game.S_Obstacle == 7) sprintf(newSpeed, "Normal");
						else sprintf(newSpeed, "Hard");

						// Display the new speed
						SH1106_GotoXY(70, 20);
						SH1106_Puts(newSpeed, &Font_7x10, SH1106_COLOR_WHITE);
					}

					game.initialObstacleSpeed = game.S_Obstacle;
				} else {
					// Exit selected - return to game over main menu
					inMainOverMenu = true;
					SH1106_Fill(SH1106_COLOR_BLACK);  // Clear screen
				}
			}
		}

		SH1106_UpdateScreen(); // Update screen after each change
	}
}

// Main game function
void Dino_Game() {
	Draw_Menu();  // Show menu first

	// Main game loop
	while (!game.GameOver) {
		Update_Game_Logic();  // Update game state
		Draw_Game();          // Render game
		if (Check_Collision()) {  // Check for collision
			game.GameOver = true;  // End game if collision detected
		}
		game.FrameCount++;    // Increment animation counter
		HAL_Delay(50);        // Frame delay (20 fps)
	}

	Draw_Game_Over();  // Show game over screen
	Reset_Game();      // Wait for restart
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	SH1106_Init();
	Dino_Game();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pins : PB0 PB1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
