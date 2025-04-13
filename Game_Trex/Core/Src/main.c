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
#include "SH1106.h"
#include "fonts.h"
#include "dino_frames.h"
#include <stdbool.h>
#include "game_config.h"
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
// Game state structure
typedef struct {
    int16_t Dino;
    int16_t V_jump;
    int16_t ObstacleX;
    int16_t S_Obstacle;
    int16_t Obstacle_Y;
    int16_t initialObstacleSpeed;
    uint16_t Score;
    uint16_t Hscore;
    bool Menu;
    bool GameOver;
    bool Jumping;
    bool Ducking;
    bool colorChanged;
    uint8_t ObstacleType;
    uint8_t FrameCount;
    int color;
    int bgColor;
} GameState;

// Sprite structure
typedef struct {
    const unsigned char* Bitmap;
    int Width;
    int Height;
} Sprite;

// Cloud structure
typedef struct {
    int16_t x;
    int16_t y;
    int16_t moveCount;
    uint8_t type;
    bool active;
} Cloud;

/* Global variables */
static GameState game = {
    .Menu = true,
    .GameOver = false,
    .Score = 0,
    .Hscore = 0,
    .Jumping = false,
    .Ducking = false,
    .Dino = SCREEN_HEIGHT - GROUND_HEIGHT - DINO_HEIGHT,
    .ObstacleX = SCREEN_WIDTH,
    .ObstacleType = OBSTACLE_SMALL_CACTUS, // Initialize with enum
    .FrameCount = 0,
    .V_jump = 0,
    .S_Obstacle = SPEED_EASY,
    .color = SH1106_COLOR_WHITE,
    .bgColor = SH1106_COLOR_BLACK,
    .colorChanged = false,
    .initialObstacleSpeed = SPEED_EASY
};

static Cloud clouds[MAX_CLOUDS];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

void Draw_Settings_Menu(bool isGameOver);
void Handle_Button_Input(bool *buttonState);
void Update_Speed_Setting();
void Draw_Speed_Text();
void Reset_Game_State();
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Reset game to initial state
void Reset_Game(void) {
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) != GPIO_PIN_SET) {
        HAL_Delay(BUTTON_DEBOUNCE_DELAY);
        while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) != GPIO_PIN_SET) {}

        Reset_Game_State();
        Dino_Game();
    }
}

// Reset all game state variables
void Reset_Game_State(void) {
    game.GameOver = false;
    game.Score = 0;
    game.Dino = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
    game.ObstacleX = SCREEN_WIDTH;
    game.S_Obstacle = game.initialObstacleSpeed;
    game.Jumping = false;
    game.Ducking = false;
    game.color = SH1106_COLOR_WHITE;
    game.bgColor = SH1106_COLOR_BLACK;

    // Clear all clouds
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].active = false;
    }
}

// Function to select appropriate dinosaur sprite based on state
Sprite Select_Sprite_Player(void) {
    Sprite pl;
    bool isAnimationFrame1 = (game.FrameCount / 3 % 2 == 0);

    if (game.Ducking) {
        pl.Bitmap = isAnimationFrame1 ? trex_duck1 : trex_duck2;
        pl.Width = 32;
        pl.Height = 16;
    } else {
        pl.Bitmap = isAnimationFrame1 ? trex_run1 : trex_run2;
        pl.Width = 16;
        pl.Height = 16;
    }

    return pl;
}

// Function to select appropriate obstacle sprite based on type
Sprite Select_Sprite_Obstacle(void) {
    Sprite ob;
    bool isAnimationFrame1 = (game.FrameCount / 3 % 2 == 0);

    switch (game.ObstacleType) {
        case OBSTACLE_SMALL_CACTUS:
            ob.Width = TREE_SMALL_WIDTH;
            ob.Height = TREE_SMALL_HEIGHT;
            ob.Bitmap = tree_small;
            break;
        case OBSTACLE_TALL_CACTUS:
            ob.Width = TREE_TALL_WIDTH;
            ob.Height = TREE_TALL_HEIGHT;
            ob.Bitmap = tree_tall;
            break;
        case OBSTACLE_COMBINED_CACTUS:
            ob.Width = TREE_SMALL_TALL_WIDTH;
            ob.Height = TREE_SMALL_TALL_HEIGHT;
            ob.Bitmap = tree_small_tall;
            break;
        case OBSTACLE_BIRD:
            ob.Width = BIRD_WIDTH;
            ob.Height = BIRD_HEIGHT;
            ob.Bitmap = isAnimationFrame1 ? bird_1 : bird_2;
            break;
        default:
            return (Sprite){0}; // Return an empty sprite on error
    }

    return ob;
}

// Function to draw the ground
void Draw_Ground(int color) {
    uint8_t groundFrame = game.FrameCount / 3 % 3;
    const uint8_t *groundSprite;

    switch (groundFrame) {
        case 0: groundSprite = ground_1; break;
        case 1: groundSprite = ground_2; break;
        case 2: groundSprite = ground_3; break;
        default: return; // Do nothing on error
    }

    SH1106_DrawBitmap(0, SCREEN_HEIGHT - GROUND_HEIGHT, groundSprite, SCREEN_WIDTH, GROUND_HEIGHT, color);
}

// Function to draw clouds in the sky
void Draw_Clouds(int color) {
    const unsigned char *cloudSprites[3] = {cloud1, cloud2, cloud3};
    static int16_t nextCloudSpawn = 0;

    // Update and draw each active cloud
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (clouds[i].active) {
            SH1106_DrawBitmap(clouds[i].x, clouds[i].y, cloudSprites[clouds[i].type], 16, 8, color);
            clouds[i].x -= 1;
            clouds[i].moveCount++;

            if (clouds[i].x < -16) {
                clouds[i].active = false;
            }
        }
    }

    // Spawn new cloud when counter reaches threshold
    if (nextCloudSpawn >= 50) {
        for (int i = 0; i < MAX_CLOUDS; i++) {
            if (!clouds[i].active) {
                clouds[i].x = SCREEN_WIDTH;
                clouds[i].y = 10 + (rand() % 3) * 10;
                clouds[i].type = rand() % 3;
                clouds[i].moveCount = 0;
                clouds[i].active = true;
                nextCloudSpawn = 0;
                break;
            }
        }
    }

    nextCloudSpawn++;
}

// Function to update color scheme
void Update_Color(void) {
    // Invert colors every 100 points
    if (game.Score % 100 == 0 && game.Score != 0 && !game.colorChanged) {
        // Toggle colors
        if (game.color == SH1106_COLOR_WHITE) {
            game.color = SH1106_COLOR_BLACK;
            game.bgColor = SH1106_COLOR_WHITE;
        } else {
            game.color = SH1106_COLOR_WHITE;
            game.bgColor = SH1106_COLOR_BLACK;
        }
        game.colorChanged = true;
    } else if (game.Score % 30 != 0) {
        game.colorChanged = false;
    }
}

// Update game state logic
void Update_Game_Logic(void) {
    // Jump control
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET && !game.Jumping && !game.Ducking) {
        game.Jumping = true;
        game.V_jump = JUMP_STRENGTH;
    }
    // Duck control
    else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
        game.Ducking = true;
    } else {
        game.Ducking = false;
    }

    // Jumping physics
    if (game.Jumping) {
        game.Dino -= game.V_jump;
        game.V_jump -= GRAVITY;

        if (game.Dino >= SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT) {
            game.Dino = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
            game.Jumping = false;
        }

        // Cancel jump if duck pressed while jumping
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
            game.Dino = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
            game.Jumping = false;
            game.V_jump = 0;
            game.Ducking = true;
        }
    }

    // Obstacle movement
    game.ObstacleX -= game.S_Obstacle;

    if (game.ObstacleX < -TREE_SMALL_TALL_WIDTH) {
        game.ObstacleX = SCREEN_WIDTH + (rand() % 50 + 30);
        game.ObstacleType = rand() % 4;
        game.Score += 10;

        // Increase difficulty every 50 points
        if (game.Score % 50 == 0 && game.S_Obstacle <= 100) {
            game.S_Obstacle++;
        }

        // Set vertical position for bird obstacle
        game.Obstacle_Y = (game.ObstacleType == OBSTACLE_BIRD) ? ((rand() % 2 == 0) ? 2 : 14) : 0;
    }
}

// Check for collision between dinosaur and obstacle
bool Check_Collision(void) {
    int16_t obstacleWidth, obstacleHeight;
    int16_t obstacleY = SCREEN_HEIGHT - GROUND_HEIGHT;

    // Set dimensions based on obstacle type
    switch (game.ObstacleType) {
        case OBSTACLE_SMALL_CACTUS:
            obstacleWidth = TREE_SMALL_WIDTH;
            obstacleHeight = TREE_SMALL_HEIGHT;
            obstacleY -= TREE_SMALL_HEIGHT;
            break;
        case OBSTACLE_TALL_CACTUS:
            obstacleWidth = TREE_TALL_WIDTH;
            obstacleHeight = TREE_TALL_HEIGHT;
            obstacleY -= TREE_TALL_HEIGHT;
            break;
        case OBSTACLE_COMBINED_CACTUS:
            obstacleWidth = TREE_SMALL_TALL_WIDTH;
            obstacleHeight = TREE_SMALL_TALL_HEIGHT;
            obstacleY -= TREE_SMALL_TALL_HEIGHT;
            break;
        case OBSTACLE_BIRD:
            obstacleWidth = BIRD_WIDTH;
            obstacleHeight = BIRD_HEIGHT;
            obstacleY -= BIRD_HEIGHT + game.Obstacle_Y;
            break;
        default:
            return false;
    }

    // Set dinosaur dimensions based on ducking state
    int16_t dinoWidth = DINO_WIDTH;
    int16_t dinoHeight = game.Ducking ? 10 : DINO_HEIGHT;
    int16_t dinoY = game.Dino + (game.Ducking ? 6 : 0);

    // Check for overlap in both X and Y directions
    bool xOverlap = (DINO_X < game.ObstacleX + obstacleWidth) && (DINO_X + dinoWidth > game.ObstacleX);
    bool yOverlap = (dinoY < obstacleY + obstacleHeight) && (dinoY + dinoHeight > obstacleY);

    return xOverlap && yOverlap;
}

// Draw menu header with logo
void Draw_Menu_Header(const char* title) {
    // Display title
    SH1106_GotoXY(MENU_TITLE_X, MENU_TITLE_Y);
    SH1106_Puts(title, &Font_11x18, SH1106_COLOR_WHITE);

    // Display dinosaur logo
    SH1106_DrawBitmap(MENU_LOGO_X, MENU_LOGO_Y, logo_trex, 36, 36, SH1106_COLOR_WHITE);
}

// Draw a menu arrow at the specified position
void Draw_Menu_Arrow(int position, int selectedPosition) {
    if (position == selectedPosition) {
        SH1106_DrawBitmap(MENU_ARROW_X, position, Arrow, 16, 9, SH1106_COLOR_WHITE);
    } else {
        SH1106_DrawBitmap(MENU_ARROW_X, position, Arrow, 16, 9, SH1106_COLOR_BLACK);
    }
}

// Get string representation of current speed setting
const char* Get_Speed_Text(void) {
    if (game.S_Obstacle == SPEED_EASY) {
        return "Easy";
    } else if (game.S_Obstacle == SPEED_NORMAL) {
        return "Normal";
    } else {
        return "Hard";
    }
}

// Draw settings menu
void Draw_Settings_Menu(bool isGameOver) {
    static int settingSelection = 0;

    // Clear if coming from another screen
    SH1106_Fill(SH1106_COLOR_BLACK);

    // Display title and logo if in game over settings
    if (isGameOver) {
        Draw_Menu_Header("T-REX GAME");
    }

    // Display settings options
    SH1106_GotoXY(SETTINGS_LABEL_X, MENU_PLAY_Y);
    SH1106_Puts("Speed: ", &Font_7x10, SH1106_COLOR_WHITE);

    // Show current speed
    SH1106_GotoXY(SETTINGS_VALUE_X, MENU_PLAY_Y);
    SH1106_Puts(Get_Speed_Text(), &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_GotoXY(SETTINGS_LABEL_X, SETTINGS_EXIT_Y);
    SH1106_Puts("Exit", &Font_7x10, SH1106_COLOR_WHITE);

    // Display arrow selection for settings
    Draw_Menu_Arrow(MENU_PLAY_Y, settingSelection == 0 ? MENU_PLAY_Y : SETTINGS_EXIT_Y);
    Draw_Menu_Arrow(SETTINGS_EXIT_Y, settingSelection == 1 ? SETTINGS_EXIT_Y : MENU_PLAY_Y);

    bool buttonPressed = false;

    while (!buttonPressed) {
        // Handle selection change
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
            HAL_Delay(BUTTON_DEBOUNCE_DELAY);
            while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {}
            settingSelection = 1 - settingSelection;

            // Update arrows
            Draw_Menu_Arrow(MENU_PLAY_Y, settingSelection == 0 ? MENU_PLAY_Y : SETTINGS_EXIT_Y);
            Draw_Menu_Arrow(SETTINGS_EXIT_Y, settingSelection == 1 ? SETTINGS_EXIT_Y : MENU_PLAY_Y);

            SH1106_UpdateScreen();
        }

        // Handle selection
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
            HAL_Delay(BUTTON_DEBOUNCE_DELAY);
            while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {}

            if (settingSelection == 0) {
                // Store previous speed for comparison
                uint8_t prevSpeed = game.S_Obstacle;

                // Update speed (cycle through options)
                if (game.S_Obstacle == SPEED_EASY) {
                    game.S_Obstacle = SPEED_NORMAL;
                } else if (game.S_Obstacle == SPEED_NORMAL) {
                    game.S_Obstacle = SPEED_HARD;
                } else {
                    game.S_Obstacle = SPEED_EASY;
                }

                // Only redraw if the speed has changed
                if (prevSpeed != game.S_Obstacle) {
                    // Clear the speed text area
                    SH1106_ClearArea(SETTINGS_VALUE_X, MENU_PLAY_Y, 110, 28, SH1106_COLOR_BLACK);

                    // Show updated speed
                    SH1106_GotoXY(SETTINGS_VALUE_X, MENU_PLAY_Y);
                    SH1106_Puts(Get_Speed_Text(), &Font_7x10, SH1106_COLOR_WHITE);

                    SH1106_UpdateScreen();
                }

                game.initialObstacleSpeed = game.S_Obstacle;
            } else {
                // Exit selected
                buttonPressed = true;
                settingSelection = 0;
            }
        }
    }
}

// Draw the game menu
void Draw_Menu(void) {
    SH1106_Fill(SH1106_COLOR_BLACK);
    int currentSelection = 0;
    bool inMainMenu = true;

    while (game.Menu) {
        if (inMainMenu) {
            // Draw main menu
            SH1106_Fill(SH1106_COLOR_BLACK);

            // Display title and logo
            Draw_Menu_Header("T-REX GAME");

            // Display menu options
            SH1106_GotoXY(MENU_OPTION_X, MENU_PLAY_Y);
            SH1106_Puts("Play", &Font_7x10, SH1106_COLOR_WHITE);

            SH1106_GotoXY(MENU_OPTION_X, MENU_SETTING_Y);
            SH1106_Puts("Setting", &Font_7x10, SH1106_COLOR_WHITE);

            // Display arrow for selection
            Draw_Menu_Arrow(MENU_PLAY_Y, currentSelection == 0 ? MENU_PLAY_Y : MENU_SETTING_Y);
            Draw_Menu_Arrow(MENU_SETTING_Y, currentSelection == 1 ? MENU_SETTING_Y : MENU_PLAY_Y);

            // Handle button 1 (down/select)
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_Delay(BUTTON_DEBOUNCE_DELAY);
                while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {}
                currentSelection = 1 - currentSelection;
            }

            // Handle button 0 (confirm/start)
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
                HAL_Delay(BUTTON_DEBOUNCE_DELAY);
                while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {}

                if (currentSelection == 0) {
                    game.Menu = false;
                    game.Jumping = false;
                } else {
                    inMainMenu = false;
                    Draw_Settings_Menu(false);
                    inMainMenu = true;
                    SH1106_Fill(SH1106_COLOR_BLACK);
                }
            }
        }

        SH1106_UpdateScreen();
    }
}

// Draw game over screen
void Draw_Game_Over(void) {
    SH1106_Fill(SH1106_COLOR_BLACK);
    int currentSelection = 0;
    bool inMainOverMenu = true;

    while (game.GameOver) {
        if (inMainOverMenu) {
            // Display Game Over main screen
            SH1106_Fill(SH1106_COLOR_BLACK);

            // Display "GAME OVER" text
            SH1106_GotoXY(GAME_OVER_TITLE_X, MENU_TITLE_Y);
            SH1106_Puts("GAME OVER", &Font_11x18, SH1106_COLOR_WHITE);

            // Update high score if needed
            if (game.Score > game.Hscore) {
                game.Hscore = game.Score;
            }

            // Display high score
            char scoreStr[12];
            sprintf(scoreStr, "HI: %d", game.Hscore);
            SH1106_GotoXY(SCORE_DISPLAY_X, HI_SCORE_Y);
            SH1106_Puts(scoreStr, &Font_7x10, SH1106_COLOR_WHITE);

            // Display current score
            sprintf(scoreStr, "SCORE: %d", game.Score);
            SH1106_GotoXY(SCORE_DISPLAY_X, CURRENT_SCORE_Y);
            SH1106_Puts(scoreStr, &Font_7x10, SH1106_COLOR_WHITE);

            // Display restart option
            SH1106_GotoXY(MENU_OPTION_X, RESTART_OPTION_Y);
            SH1106_Puts("Restart", &Font_7x10, SH1106_COLOR_WHITE);

            // Display setting option
            SH1106_GotoXY(MENU_OPTION_X, SETTING_OPTION_Y);
            SH1106_Puts("Setting", &Font_7x10, SH1106_COLOR_WHITE);

            // Display arrow to indicate selection
            Draw_Menu_Arrow(RESTART_OPTION_Y, currentSelection == 0 ? RESTART_OPTION_Y : SETTING_OPTION_Y);
            Draw_Menu_Arrow(SETTING_OPTION_Y, currentSelection == 1 ? SETTING_OPTION_Y : RESTART_OPTION_Y);

            // Handle button 1 (down/select)
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_Delay(BUTTON_DEBOUNCE_DELAY);
                while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {}
                currentSelection = 1 - currentSelection;
            }

            // Handle button 0 (confirm/start)
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
                HAL_Delay(BUTTON_DEBOUNCE_DELAY);
                while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {}

                if (currentSelection == 0) {
                    Reset_Game();
                    return;
                } else {
                    inMainOverMenu = false;
                    Draw_Settings_Menu(true);
                    inMainOverMenu = true;
                    SH1106_Fill(SH1106_COLOR_BLACK);
                }
            }
        }

        SH1106_UpdateScreen();
    }
}

// Draw the game screen
void Draw_Game(void) {
    Update_Color();

    SH1106_Fill(game.bgColor);
    Draw_Ground(game.color);
    Draw_Clouds(game.color);

    // Draw dinosaur
    Sprite pl = Select_Sprite_Player();
    SH1106_DrawBitmap(DINO_X, game.Dino, pl.Bitmap, pl.Width, pl.Height, game.color);

    // Draw obstacle
    Sprite ob = Select_Sprite_Obstacle();
    SH1106_DrawBitmap(
        game.ObstacleX,
        SCREEN_HEIGHT - GROUND_HEIGHT - ob.Height - game.Obstacle_Y,
        ob.Bitmap, ob.Width, ob.Height, game.color
    );

    // Display scores
    char scoreStr[12];

    // Update high score if needed
    if (game.Score > game.Hscore) {
        game.Hscore = game.Score;
    }

    sprintf(scoreStr, "HI %d  %d", game.Hscore, game.Score);
    SH1106_GotoXY(SCORE_TEXT_X, SCORE_TEXT_Y);
    SH1106_Puts(scoreStr, &Font_7x10, game.color);

    SH1106_UpdateScreen();
}

// Main game function
void Dino_Game(void) {
    Draw_Menu();

    while (!game.GameOver) {
        Update_Game_Logic();
        Draw_Game();

        if (Check_Collision()) {
            game.GameOver = true;
        }

        game.FrameCount++;
        HAL_Delay(50);
    }

    Draw_Game_Over();
    Reset_Game();
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
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
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

#ifdef USE_FULL_ASSERT
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
