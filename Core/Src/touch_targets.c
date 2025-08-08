#include "touch_targets.h"

const TOUCH_TARGET_T touch_targets[] = {
    // clang-format off
    /* -------------------------------------------------------------------------- */
    /*                                   Page 0                                   */
    /* -------------------------------------------------------------------------- */
    /* ---------------------------------- Row 0 --------------------------------- */
    /* TARGET_ESPRESSO */           { 0, 0, 0, "Espresso" },
    /* TARGET_DOUBLE_ESPRESSO */    { 0, 1, 0, "Double Espresso" },
    /* TARGET_ESPRESSO_MACCHIATO */ { 0, 2, 0, "Espresso Macchiato" },
    /* TARGET_LONG_BLACK */         { 0, 3, 0, "Long Black" },
    /* ---------------------------------- Row 1 --------------------------------- */
    /* TARGET_CAFFE_LATTE */        { 0, 0, 1, "Caffe Latte" },
    /* TARGET_CAPPUCCINO */         { 0, 1, 1, "Cappuccino" },
    /* TARGET_FLAT_WHITE */         { 0, 2, 1, "Flat White" },
    /* TARGET_MAGIC */              { 0, 3, 1, "Magic" },
    /* ---------------------------------- Row 2 --------------------------------- */
    /* TARGET_PICCOLO_LATTE */      { 0, 0, 2, "Piccolo Latte" },
    /* TARGET_HOT_CHOCOLATE */      { 0, 1, 2, "Hot Chocolate" },
    /* TARGET_CHAI_LATTE */         { 0, 2, 2, "Chai Latte" },
    /* TARGET_MOCHA */              { 0, 3, 2, "Mocha" },

    /* -------------------------------------------------------------------------- */
    /*                                   Page 1                                   */
    /* -------------------------------------------------------------------------- */
    /* ---------------------------------- Row 0 --------------------------------- */
    /* TARGET_DIRTY_CHAI */         { 1, 0, 0, "Dirty Chai" },
    /* TARGET_STRONG_LATTE */       { 1, 1, 0, "Strong Latte" },
    /* TARGET_STRONG_CAPPUCCINO */  { 1, 2, 0, "Strong Cappuccino" },
    /* TARGET_HOT_MILK */           { 1, 3, 0, "Hot Milk" },
    /* ---------------------------------- Row 1 --------------------------------- */
    /* TARGET_ICED_COFFEE */        { 1, 0, 1, "Iced Coffee" },
    /* TARGET_WEAK_LATTE */         { 1, 1, 1, "Weak Latte" },
    /* TARGET_WEAK_CAPPUCCINO */    { 1, 2, 1, "Weak Cappuccino" },
    /* TARGET_HOT_MILK_FOAM */      { 1, 3, 1, "Hot Milk Foam" },
    /* ---------------------------------- Row 2 --------------------------------- */
    /* TARGET_ICED_CHOCOLATE */     { 1, 0, 2, "Iced Chocolate" },
    /* TARGET_HOT_LATTE */          { 1, 1, 2, "Hot Latte" },
    /* TARGET_HOT_WATER */          { 1, 2, 2, "Hot Water" },
    /* TARGET_COLD_MILK */          { 1, 3, 2, "Cold Milk" },
    /* -------------------------------------------------------------------------- */
    /*                                   Page 2                                   */
    /* -------------------------------------------------------------------------- */
    /* ---------------------------------- Row 0 --------------------------------- */
    /* TARGET_WARM_MILK_FOAM */     { 2, 0, 0, "Warm Milk Foam" },
    /* TARGET_COLD_MILK_FOAM */     { 2, 1, 0, "Cold Milk Foam" },
    // clang-format on
};
