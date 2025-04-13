#ifndef FONTS_H
#define FONTS_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Font definition structure.
 */
typedef struct {
    uint8_t width;   /**< Font width in pixels. */
    uint8_t height;  /**< Font height in pixels. */
    const uint16_t* data; /**< Pointer to font data array. */
} FontDef_t;

/**
 * @brief String size (length and height) in pixels.
 */
typedef struct {
    uint16_t length; /**< String length in pixels. */
    uint16_t height; /**< String height in pixels. */
} FontSize_t;

/**
 * @brief 7x10 pixel font.
 */
extern FontDef_t Font_7x10;

/**
 * @brief 11x18 pixel font.
 */
extern FontDef_t Font_11x18;

/**
 * @brief 16x26 pixel font.
 */
extern FontDef_t Font_16x26;

/**
 * @brief Calculates string size in pixels for a given font.
 *
 * @param str       String to calculate size for.
 * @param size      Pointer to FontSize_t structure to store the result.
 * @param font      Pointer to FontDef_t font to use.
 *
 * @return Pointer to the input string.
 */
char* FONTS_GetStringSize(char* str, FontSize_t* size, FontDef_t* font);

#ifdef __cplusplus
}
#endif

#endif // FONTS_H
