#ifndef TOUCH_TARGETS_H
#define TOUCH_TARGETS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Time taken to transition between pages
#define PAGE_TRANSITION_DELAY_MS 1000

#define MAX_PAGE_COUNT 3

typedef enum
{
  TARGET_NEXT_PAGE,
  TARGET_PREV_PAGE,

  // Page 0
  TARGET_ESPRESSO,
  TARGET_DOUBLE_ESPRESSO,
  TARGET_ESPRESSO_MACCHIATO,
  TARGET_LONG_BLACK,
  TARGET_CAFFE_LATTE,
  TARGET_CAPPUCCINO,
  TARGET_FLAT_WHITE,
  TARGET_MAGIC,
  TARGET_PICCOLO_LATTE,
  TARGET_HOT_CHOCOLATE,
  TARGET_CHAI_LATTE,
  TARGET_MOCHA,
  // Page 1
  TARGET_DIRTY_CHAI,
  TARGET_STRONG_LATTE,
  TARGET_STRONG_CAPPUCCINO,
  TARGET_HOT_MILK,
  TARGET_ICED_COFFEE,
  TARGET_WEAK_LATTE,
  TARGET_WEAK_CAPPUCCINO,
  TARGET_HOT_MILK_FOAM,
  TARGET_ICED_CHOCOLATE,
  TARGET_HOT_LATTE,
  TARGET_HOT_WATER,
  TARGET_COLD_MILK,
  // Page 2
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
  char name[24];
} TOUCH_TARGET_T;

// Arranged Top to Bottom, Left to Right
extern const TOUCH_TARGET_T touch_targets[TARGET_COUNT];

#ifdef __cplusplus
}
#endif

#endif