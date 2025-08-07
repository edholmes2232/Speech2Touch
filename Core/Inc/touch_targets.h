#ifndef TOUCH_TARGETS_H
#define TOUCH_TARGETS_H

typedef enum
{
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
} TARGET_T;

typedef struct
{
  uint8_t page;
  uint8_t x;
  uint8_t y;
} TOUCH_TARGET_T;

static TOUCH_TARGET_T touch_targets[] = {
    // clang-format off
    /* TARGET_ESPRESSO */           { 0, 0, 0 }, // page 0 [0,0]
    /* TARGET_DOUBLE_ESPRESSO */    { 0, 1, 0 }, // page 0 [1,0]
    /* TARGET_ESPRESSO_MACCHIATO */ { 0, 2, 0 }, // page 0 [2,0]
    /* TARGET_LONG_BLACK */         { 0, 3, 0 }, // page 0 [3,0]
    /* TARGET_CAFFE_LATTE */        { 0, 0, 1 }, // page 0 [0,1]
    /* TARGET_CAPPUCCINO */         { 0, 1, 1 }, // page 0 [1,1]
    /* TARGET_FLAT_WHITE */         { 0, 2, 1 }, // page 0 [2,1]
    /* TARGET_MAGIC */              { 0, 3, 1 }, // page 0 [3,1]
    /* TARGET_PICCOLO_LATTE */      { 0, 0, 2 }, // page 0 [0,2]
    /* TARGET_HOT_CHOCOLATE */      { 0, 1, 2 }, // page 0 [1,2]
    /* TARGET_CHAI_LATTE */         { 0, 2, 2 }, // page 0 [2,2]
    /* TARGET_MOCHA */              { 0, 3, 2 }, // page 0 [3,2]
    /* TARGET_DIRTY_CHAI */         { 1, 0, 0 }, // page 1 [0,0]
    /* TARGET_STRONG_LATTE */       { 1, 1, 0 }, // page 1 [1,0]
    /* TARGET_STRONG_CAPPUCCINO */  { 1, 2, 0 }, // page 1 [2,0]
    /* TARGET_HOT_MILK */           { 1, 3, 0 }, // page 1 [3,0]
    /* TARGET_ICED_COFFEE */        { 1, 0, 1 }, // page 1 [0,1]
    /* TARGET_WEAK_LATTE */         { 1, 1, 1 }, // page 1 [1,1]
    /* TARGET_WEAK_CAPPUCCINO */    { 1, 2, 1 }, // page 1 [2,1]
    /* TARGET_HOT_MILK_FOAM */      { 1, 3, 1 }, // page 1 [3,1]
    /* TARGET_ICED_CHOCOLATE */     { 1, 0, 2 }, // page 1 [0,2]
    /* TARGET_HOT_LATTE */          { 1, 1, 2 }, // page 1 [1,2]
    /* TARGET_HOT_WATER */          { 1, 2, 2 }, // page 1 [2,2]
    /* TARGET_COLD_MILK */          { 1, 3, 2 }, // page 1 [3,2]
    /* TARGET_WARM_MILK_FOAM */     { 2, 0, 0 }, // page 2 [0,0]
    /* TARGET_COLD_MILK_FOAM */     { 2, 1, 0 }, // page 2 [1,0]
    // clang-format on
};

#endif