#ifndef TOUCH_TARGETS_H
#define TOUCH_TARGETS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TARGET_STRING_MAX_LENGTH 24

// Time taken to transition between pages
#define PAGE_TRANSITION_DELAY_MS 1000

#define MAX_PAGE_COUNT 3

typedef enum
{
  TARGET_NEXT_PAGE = -2,
  TARGET_PREV_PAGE = -1,

  // Page 0 (Column order)
  // Col 0
  TARGET_ESPRESSO = 0,
  TARGET_CAFFE_LATTE,
  TARGET_PICCOLO_LATTE,
  // Col 1
  TARGET_DOUBLE_ESPRESSO,
  TARGET_CAPPUCCINO,
  TARGET_HOT_CHOCOLATE,
  // Col 2
  TARGET_ESPRESSO_MACCHIATO,
  TARGET_FLAT_WHITE,
  TARGET_CHAI_LATTE,
  // Col 3
  TARGET_LONG_BLACK,
  TARGET_MAGIC,
  TARGET_MOCHA,

  // Page 1 (Column order)
  // Col 0
  TARGET_DIRTY_CHAI,
  TARGET_ICED_COFFEE,
  TARGET_ICED_CHOCOLATE,
  // Col 1
  TARGET_STRONG_LATTE,
  TARGET_WEAK_LATTE,
  TARGET_HOT_LATTE,
  // Col 2
  TARGET_STRONG_CAPPUCCINO,
  TARGET_WEAK_CAPPUCCINO,
  TARGET_HOT_WATER,
  // Col 3
  TARGET_HOT_MILK,
  TARGET_HOT_MILK_FOAM,
  TARGET_COLD_MILK,

  // Page 2 (Column order)
  // Col 0
  TARGET_WARM_MILK_FOAM,
  TARGET_COLD_MILK_FOAM,

  TARGET_COUNT,
} TARGET_T;

typedef struct
{
  uint16_t x;
  uint16_t y;
} TOUCH_COORD_T;

typedef struct
{
  uint8_t page;
  uint8_t col;
  uint8_t row;
  char name[TARGET_STRING_MAX_LENGTH];
} TOUCH_TARGET_T;

// Arranged Top to Bottom, Left to Right
extern const TOUCH_TARGET_T touch_targets[TARGET_COUNT];

#ifdef __cplusplus
}
#endif

#endif