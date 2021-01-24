
#pragma once

#include <stdlib.h>

#include "gba/gba.h"

#define CONST_DATA __attribute__((section(".data")))

#include "types.h"
#include "variables.h"
#include "functions.h"

// helper macros

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define ABS(aValue) ((aValue) >= 0 ? (aValue) : -(aValue))

#define RECT_DISTANCE(aXA, aYA, aXB, aYB) (ABS((aXA) - (aXB)) + ABS((aYA) - (aYB)))
