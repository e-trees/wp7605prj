#ifndef _AT_CMD_MODULE_H
#define _AT_CMD_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_BUF_SIZE 256
#define DEFAULT_SPLIT_FILE_SIZE (2 * 1024)

int _at_port_fd;
struct termios _oldtio, _newtio;

int at_open(char *port);
int at_close(void);
int at_gpsinit(void);
int at_gpslocget(double *lat, double *lon, int *fix);

#endif