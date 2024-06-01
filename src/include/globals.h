
#ifndef GLOBALS_H_
#define GLOBALS_H_ 1

#include "sdk.h"

extern int g_pid;
extern void* g_playerList;
extern void* g_localPlayer;
extern void* g_localPlayerPtr;
extern void* g_viewMatrix;

/*----------------------------------------------------------------------------*/

void globalsInit(void);

#endif /* GLOBALS_H_ */
