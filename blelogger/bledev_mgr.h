#ifndef _BLEDEVMGR_MODULE_H
#define _BLEDEVMGR_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "at.h"
#include <sys/types.h>
#include <sys/wait.h>

typedef struct bledev_info_t
{
    time_t time;
    char addr[18];
    int rssi;
    double lat;
    double lon;
    struct bledev_info_t *next;
} bledev_info_t;

int bledev_init(void);
int bledev_add(char addr[18], int rssi);
bledev_info_t *bledev_listget(void);
int bledev_listrelease(void);
int bledev_listsize(void);

#endif