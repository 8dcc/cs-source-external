
#ifndef GLOBALS_H_
#define GLOBALS_H_ 1

#include "sdk.h"

extern int g_pid;
extern void* g_playerList;
extern void* g_viewMatrix;
extern char g_localName[MAX_NAME];

/*----------------------------------------------------------------------------*/

void globalsInit(void);

#endif /* GLOBALS_H_ */
