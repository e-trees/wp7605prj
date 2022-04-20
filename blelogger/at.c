#include "at.h"

int at_open(char *port)
{

    int baudRate = B9600;

    _at_port_fd = open(port, O_RDWR | O_NOCTTY);
    if (_at_port_fd < 0)
    {
        perror("SerialPort Open Error\n");
        return -1;
    }

    tcgetattr(_at_port_fd, &_oldtio);
    bzero(&_newtio, sizeof(_newtio));

    _newtio.c_cflag = baudRate | CS8 | CLOCAL | CREAD;
    _newtio.c_iflag = IGNPAR | ICRNL;
    _newtio.c_oflag = 0;
    // Canonical Mode
    _newtio.c_lflag = ICANON;
    _newtio.c_cc[VEOF] = 4;

    tcflush(_at_port_fd, TCIFLUSH);
    tcsetattr(_at_port_fd, TCSANOW, &_newtio);
    // Serial Port Open Done
    return 0;
}

int at_close(void)
{
    tcsetattr(_at_port_fd, TCSANOW, &_oldtio);
    close(_at_port_fd);
    return 0;
}

static int at_write(char *str)
{
    return write(_at_port_fd, str, strlen(str));
}

static int at_read(char *str, size_t len)
{
    memset(str, 0, len);
    return read(_at_port_fd, str, len);
}

static int at_getok(void)
{
    char buf[256];
    while (1)
    {
        at_read(buf, sizeof(buf));
        if (strncmp(buf, "OK", 2) == 0)
        {
            return 0;
        }
        else if (strncmp(buf, "ERROR", 5) == 0)
        {
            return -1;
        }
    }
}

int at_gpsinit()
{
    at_write("AT!GPSEND=0\n");
    if (at_getok() != 0)
    {
        printf("err AT!GPSEND\n");
        return -1;
    }

    at_write("AT!GPSTRACK=1,255,1000,1000,1\n");
    if (at_getok() != 0)
    {
        printf("err AT!GPSTRACK\n");
        return -1;
    }

    at_write("AT!GPSSTATUS?\n");
    if (at_getok() != 0)
    {
        printf("err AT!GPSSTATUS\n");
        return -1;
    }
    return 0;
}

int at_gpslocget(double *lat, double *lon, int *fix)
{
    char buf[256];
    int d, m;
    double s;
    char dir;
    *fix = 0;
    at_write("AT!GPSLOC?\n\0");
    while (1)
    {
        at_read(buf, sizeof(buf));
        if (strncmp(buf, "Lat:", 4) == 0)
        {
            sscanf(buf, "Lat: %d Deg %d Min %lf Sec %c", &d, &m, &s, &dir);
            *lat = ((dir == 'N') ? 1.0 : -1.0) * (1.0 * d + m / 60.0 + s / 3600.0);
        }
        else if (strncmp(buf, "Lon:", 4) == 0)
        {
            sscanf(buf, "Lon: %d Deg %d Min %lf Sec %c", &d, &m, &s, &dir);
            *lon = ((dir == 'E') ? 1.0 : -1.0) * (1.0 * d + m / 60.0 + s / 3600.0);
        }
        else if (strncmp(buf, "2D Fix", 6) == 0)
        {
            *fix = 2;
        }
        else if (strncmp(buf, "3D Fix", 6) == 0)
        {
            *fix = 3;
        }
        else if (strncmp(buf, "Unknown", 7) == 0)
        {
            return -1;
        }
        else if (strncmp(buf, "Not Available", 13) == 0)
        {
            return -1;
        }
        else if (strncmp(buf, "OK", 2) == 0)
        {
            return 0;
        }
        else if (strncmp(buf, "ERROR", 5) == 0)
        {
            return -1;
        }
    }
}