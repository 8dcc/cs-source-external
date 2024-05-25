
#ifndef GLOBALS_H_
#define GLOBALS_H_ 1

extern int g_pid;
extern char g_localName[100];
extern void* g_playerList;
extern void* g_viewMatrix;

/*----------------------------------------------------------------------------*/

void globalsInit(void);

#endif /* GLOBALS_H_ */
