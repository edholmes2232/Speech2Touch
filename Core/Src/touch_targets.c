#include "touch_targets.h"

const TOUCH_TARGET_T touch_targets[] = {
    // clang-format off
    /* -------------------------------------------------------------------------- */
    /*                                   Page 0                                   */
    /* -------------------------------------------------------------------------- */
    /* ---------------------------------- Col 0 --------------------------------- */
    /* TARGET_ESPRESSO */           { 0, 0, 0, "espresso" },
    /* TARGET_CAFFE_LATTE */        { 0, 0, 1, "caffe latte" },
    /* TARGET_PICCOLO_LATTE */      { 0, 0, 2, "piccolo latte" },
    /* ---------------------------------- Col 1 --------------------------------- */
    /* TARGET_DOUBLE_ESPRESSO */    { 0, 1, 0, "double espresso" },
    /* TARGET_CAPPUCCINO */         { 0, 1, 1, "cappuccino" },
    /* TARGET_HOT_CHOCOLATE */      { 0, 1, 2, "hot chocolate" },
    /* ---------------------------------- Col 2 --------------------------------- */
    /* TARGET_ESPRESSO_MACCHIATO */ { 0, 2, 0, "espresso macchiato" },
    /* TARGET_FLAT_WHITE */         { 0, 2, 1, "flat white" },
    /* TARGET_CHAI_LATTE */         { 0, 2, 2, "chai latte" },
    /* ---------------------------------- Col 3 --------------------------------- */
    /* TARGET_LONG_BLACK */         { 0, 3, 0, "long black" },
    /* TARGET_MAGIC */              { 0, 3, 1, "magic" },
    /* TARGET_MOCHA */              { 0, 3, 2, "mocha" },

    /* -------------------------------------------------------------------------- */
    /*                                   Page 1                                   */
    /* -------------------------------------------------------------------------- */
    /* ---------------------------------- Col 0 --------------------------------- */
    /* TARGET_DIRTY_CHAI */         { 1, 0, 0, "dirty chai" },
    /* TARGET_ICED_COFFEE */        { 1, 0, 1, "iced coffee" },
    /* TARGET_ICED_CHOCOLATE */     { 1, 0, 2, "iced chocolate" },
    /* ---------------------------------- Col 1 --------------------------------- */
    /* TARGET_STRONG_LATTE */       { 1, 1, 0, "strong latte" },
    /* TARGET_WEAK_LATTE */         { 1, 1, 1, "weak latte" },
    /* TARGET_HOT_LATTE */          { 1, 1, 2, "hot latte" },
    /* ---------------------------------- Col 2 --------------------------------- */
    /* TARGET_STRONG_CAPPUCCINO */  { 1, 2, 0, "strong cappuccino" },
    /* TARGET_WEAK_CAPPUCCINO */    { 1, 2, 1, "weak cappuccino" },
    /* TARGET_HOT_WATER */          { 1, 2, 2, "hot water" },
    /* ---------------------------------- Col 3 --------------------------------- */
    /* TARGET_HOT_MILK */           { 1, 3, 0, "hot milk" },
    /* TARGET_HOT_MILK_FOAM */      { 1, 3, 1, "hot milk foam" },
    /* TARGET_COLD_MILK */          { 1, 3, 2, "cold milk" },

    /* -------------------------------------------------------------------------- */
    /*                                   Page 2                                   */
    /* -------------------------------------------------------------------------- */
    /* ---------------------------------- Col 0 --------------------------------- */
    /* TARGET_WARM_MILK_FOAM */     { 2, 0, 0, "warm milk foam" },
    /* TARGET_COLD_MILK_FOAM */     { 2, 0, 1, "cold milk foam" },
    // clang-format on
};
