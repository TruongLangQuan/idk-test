#pragma once
#include "GameSettings.h"

#define CORE

#define TEXTURES_COUNT 	3

#define TEX_OBJECTS 	0
#define TEX_GHZ_TILES 	1
#define TEX_GHZ_GIMM 	2

#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"
#include <list>

#ifdef OPTIMAL_RESOLUTION
	#define SCREEN_WIDTH    211
	#define SCREEN_HEIGHT   119
#endif
#ifdef FULL_RESOLUTION
	#define SCREEN_WIDTH    240
	#define SCREEN_HEIGHT   135
#endif
#ifdef MAX_FPS_RESOLUTION
	#define SCREEN_WIDTH    120
	#define SCREEN_HEIGHT   68
#endif
