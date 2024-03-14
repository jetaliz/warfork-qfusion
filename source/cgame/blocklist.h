#pragma once
#define	MAX_BLOCKS	1024
#include <stdint.h>
typedef struct
{
	uint64_t steamid;
	char name[MAX_STRING_CHARS];
} blockentry_t;


