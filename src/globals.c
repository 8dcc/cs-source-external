
#include <stdio.h>
#include <stdlib.h>

#include "include/sdk.h"
#include "include/util.h"

#define OFFSET_PLAYERLIST 0xBE9380
#define OFFSET_VIEWMATRIX 0xC7213C

/*----------------------------------------------------------------------------*/

int g_pid          = 0;
void* g_playerList = NULL;
void* g_viewMatrix = NULL;
char g_localName[MAX_NAME];

/*----------------------------------------------------------------------------*/

static inline void getLocalName(void) {
    int c;
    for (int i = 0; i < MAX_NAME && (c = getchar()) != '\n'; i++)
        g_localName[i] = c;
}

void globalsInit(void) {
    /* Get PID of the game, for read/write operations */
    g_pid = pidof("hl2_linux");
    if (g_pid < 0) {
        ERR("Failed to get the PID of hl2_linux");
        exit(1);
    }
    printf("Got PID of hl2_linux: %d\n", g_pid);

    void* client = getModuleBaseAddress(g_pid, "/bin/client\\.so");
    if (!client) {
        ERR("Can't open client.so");
        exit(1);
    }
    printf("Base address of client.so: %p\n", client);

    void* engine = getModuleBaseAddress(g_pid, "/bin/engine\\.so");
    if (!engine) {
        ERR("Can't open engine.so");
        exit(1);
    }
    printf("Base address of engine.so: %p\n", engine);

    /* TODO: This is a sketchy method */
    void* playerList_ptr = GET_OFFSET(client, OFFSET_PLAYERLIST);
    readProcessMemory(g_pid, playerList_ptr, &g_playerList, sizeof(void*));
    g_playerList += 0x28;

    g_viewMatrix = GET_OFFSET(engine, OFFSET_VIEWMATRIX);

    /* TODO: Get local player pointer, filter with that in ESP */
    printf("Enter your in-game name: ");
    getLocalName();
}
