#pragma once


#define E_COMMON 1
#define E_RARE 2
#define E_USAGE 3


#define print(x) fputs((x), stdout)
#define min(x, y) ((x < y) ? (x) : (y))
#define lengthof(x) (sizeof(x)/sizeof((x)[0]))
