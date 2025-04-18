/**
 * This file contains bitmap data for various game elements, such as the ground,
 * the T-Rex character, birds, trees, clouds, and other UI elements.
 *
 * Each array represents a bitmap image, where each byte or word corresponds to
 * a row of pixels. The bits within each byte or word represent the pixel values.
 *
 * The bitmaps are designed for monochrome displays, where each bit represents
 * either a pixel being on (1) or off (0).
 *
 * The dimensions of each bitmap are indicated in the comments before the array
 * declaration (e.g., 128x4, 16x16, 16x12, 36x36).
 */

// 128x4 bitmap for the ground, divided into three sections.
const unsigned char ground_1[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // First row: solid white (all pixels on).
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Second row: solid black (all pixels off).
    0x07, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, // Third row: pattern of small white segments.
    0x00, 0x3C, 0xE0, 0x00, 0xC0, 0x00, 0x00, 0x78, 0x00, 0x3C, 0xE0, 0x00, 0xC0, 0x00, 0x00, 0x78  // Fourth row: pattern of larger white segments.
};

const unsigned char ground_2[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // First row: solid white.
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, // Second row: pattern with small white sections
    0x18, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, // Third row: pattern with small white sections
    0x00, 0x00, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00  // Fourth row: pattern with small white sections
};

const unsigned char ground_3[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // First row: solid white
    0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, // Second row: pattern with small white sections
    0x18, 0x00, 0x00, 0x1C, 0x00, 0x3C, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x00, // Third row: pattern with small white sections
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00  // Fourth row: pattern with small white sections
};

// 16x16 bitmaps for the T-Rex running animation.
const unsigned char trex_run1[] = {
    0x00, 0x3C, // Row 1
    0x00, 0xFE, // Row 2
    0x01, 0xDE, // Row 3
    0x03, 0xFE, // Row 4
    0x03, 0xF0, // Row 5
    0x03, 0xFC, // Row 6
    0x83, 0xE0, // Row 7
    0xC7, 0xC0, // Row 8
    0xFF, 0xF0, // Row 9
    0xFF, 0xD0, // Row 10
    0xFF, 0xC0, // Row 11
    0x7F, 0xC0, // Row 12
    0x3F, 0x80, // Row 13
    0x1F, 0x00, // Row 14
    0x13, 0x00, // Row 15
    0x03, 0x80  // Row 16
};

const unsigned char trex_run2[] = {
    0x00, 0x3C,
    0x00, 0xFE,
    0x01, 0xDE,
    0x03, 0xFE,
    0x03, 0xF0,
    0x03, 0xFC,
    0x83, 0xE0,
    0xC7, 0xC0,
    0xFF, 0xF0,
    0xFF, 0xD0,
    0xFF, 0xC0,
    0x7F, 0xC0,
    0x3F, 0x80,
    0x1F, 0x00,
    0x19, 0x00,
    0x1C, 0x00
};

// 16x16 bitmaps for the T-Rex ducking animation.
const unsigned char trex_duck1[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x0C, 0x3F, 0x8F, 0xC0,
    0x0F, 0xFF, 0xFB, 0xE0,
    0x07, 0xFF, 0xFF, 0xE0,
    0x03, 0xFF, 0xFF, 0xE0,
    0x01, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xDF, 0xC0,
    0x00, 0x7F, 0x40, 0x00,
    0x00, 0x5C, 0x00, 0x00,
    0x00, 0x48, 0x00, 0x00,
    0x00, 0x60, 0x00, 0x00
};

const unsigned char trex_duck2[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x0C, 0x3F, 0x8F, 0xC0,
    0x0F, 0xFF, 0xFB, 0xE0,
    0x07, 0xFF, 0xFF, 0xE0,
    0x03, 0xFF, 0xFF, 0xE0,
    0x01, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xDF, 0xC0,
    0x00, 0x7F, 0x40, 0x00,
    0x00, 0x5C, 0x00, 0x00,
    0x00, 0x48, 0x00, 0x00,
    0x00, 0x0C, 0x00, 0x00
};

// 16x12 bitmaps for the bird animation.
const unsigned char bird_1[] = {
    0x01, 0x00,
    0x01, 0x80,
    0x09, 0xC0,
    0x19, 0xE0,
    0x3F, 0xF0,
    0x7F, 0xF8,
    0xFF, 0xFF,
    0x0F, 0xFC,
    0x03, 0xFE,
    0x00, 0xF8,
    0x00, 0x00,
    0x00, 0x00
};

const unsigned char bird_2[] = {
    0x00, 0x00,
    0x00, 0x00,
    0x08, 0x00,
    0x18, 0x00,
    0x3F, 0x00,
    0x7F, 0xF8,
    0xFF, 0xFF,
    0x0F, 0xFC,
    0x03, 0xFE,
    0x03, 0xF8,
    0x03, 0x00,
    0x02, 0x00
};

// 16x14 bitmap for small trees.
const unsigned char tree_small[] = {
    0x08, 0x00,
    0x0C, 0x00,
    0x0C, 0x00,
    0x0C, 0x00,
    0x4C, 0x80,
    0x6D, 0x80,
    0x6D, 0x80,
    0x6D, 0x80,
    0x7F, 0x80,
    0x3F, 0x00,
    0x0C, 0x00,
    0x0C, 0x00,
    0x0C, 0x00,
    0xDF, 0x80
};

// 16x16 bitmap for tall trees.
const unsigned char tree_tall[] = {
    0x08, 0x00,
    0x1C, 0x80,
    0x1D, 0x80,
    0x9D, 0x80,
    0xDD, 0x80,
    0xDD, 0x80,
    0xDD, 0x80,
    0xDD, 0x80,
    0xFF, 0x80,
    0x7F, 0x00,
    0x1C, 0x00,
    0x1C, 0x00,
    0x1C, 0x00,
    0x1C, 0x00,
    0x1C, 0x00,
    0xBF, 0x80
};

// 16x16 bitmap for small and tall trees combined.
const unsigned char tree_small_tall[] = {
    0x00, 0x10, 0x00, 0x38, 0x00, 0x39, 0x20, 0xBB,
    0x21, 0xBB, 0x25, 0xBB, 0xAD, 0xBB, 0xAD, 0xBB,
    0xAD, 0xBF, 0xF8, 0xF8, 0x60, 0x38, 0x60, 0x38,
    0x60, 0x38, 0x60, 0x38, 0xFC, 0xBE
};

// 36x36 bitmap for the T-Rex logo.
const uint8_t logo_trex[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1f, 0x80,
    0x00, 0x00, 0x00, 0x3f, 0xC0,
    0x00, 0x00, 0x0a, 0x7e, 0x00,
    0x00, 0x01, 0xff, 0xfc, 0x00,
    0x00, 0xff, 0xff, 0xff, 0x00,
    0x0f, 0xff, 0xff, 0xf1, 0x80,
    0x15, 0x7f, 0xff, 0xC0, 0x00,
    0x00, 0x0f, 0xff, 0x80, 0x00,
    0x00, 0x03, 0xfe, 0x40, 0x00,
    0x00, 0x00, 0x78, 0x00, 0x00,
    0x00, 0x00, 0x68, 0x00, 0x00,
    0x00, 0x00, 0x48, 0x00, 0x00,
    0x00, 0x00, 0x90, 0x00, 0x00,
    0x00, 0x00, 0x48, 0x00, 0x00,
    0x00, 0x00, 0x46, 0x00, 0x00,
    0x00, 0x00, 0x74, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
};

// Bitmap for the restart icon.
const uint8_t restart[] = {
    0xe0, 0x00, 0xe0,
    0xff, 0xff, 0x60,
    0xbf, 0xff, 0xe0,
    0x7e, 0xff, 0xC0,
    0x7e, 0x7f, 0xC0,
    0x78, 0x33, 0xC0,
    0x7a, 0x7b, 0xC0,
    0x7a, 0xfb, 0xC0,
    0x7b, 0xfb, 0xC0,
    0x7b, 0xfb, 0xC0,
    0x78, 0x03, 0xC0,
    0xbf, 0xff, 0xa0,
    0xdf, 0xff, 0x60,
    0xe0, 0x00, 0xe0,
};

// 8x8 bitmaps for clouds.
const unsigned char cloud1[] = {
    0x00, 0x38,
    0x00, 0x7C,
    0x38, 0xFE,
    0x7D, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF
};

const unsigned char cloud2[] = {
    0x00, 0x38,
    0x00, 0x7C,
    0x1C, 0xFE,
    0xBE, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF
};

const unsigned char cloud3[] = {
    0x00, 0x00,
    0x00, 0x00,
    0x3E, 0xFE,
    0x7F, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF
};

// 9x9 bitmap for an arrow.
const unsigned char Arrow[] = {
    0x01, 0x00,
    0x01, 0xC0,
    0x01, 0xF0,
    0xFF, 0xFC,
    0xFF, 0xFF,
    0xFF, 0xFC,
    0x01, 0xF0,
    0x01, 0xC0,
    0x01, 0x00
};
