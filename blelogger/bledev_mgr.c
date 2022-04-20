#include "bledev_mgr.h"

bledev_info_t _BLEDEV_ROOTPTR = {0, "", 0, 0, 0, NULL};
pthread_mutex_t _BLEDEV_ROOT_mutex = PTHREAD_MUTEX_INITIALIZER;

int bledev_init(void)
{
    pid_t pid;
    int status;
    int ret;
    char *argv[] = {"/bin/sh", "-c", "/usr/bin/hciconfig hci0 down;/usr/bin/hciconfig hci0 up", NULL};
    char *envp[] = {NULL};
    at_open("/dev/ttyAT");
    at_gpsinit();

    // ble dongle setup
    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(2);
    }
    else if (pid == 0)
    {
        execve(argv[0], argv, envp);
        perror("execve");
        exit(3);
    }
    else
    {
        if ((ret = wait(&status)) < 0)
        {
            perror("wait");
            exit(4);
        }
        return 0;
    }
}

int bledev_add(char addr[18], int rssi)
{
    int r, fix, strr;
    bledev_info_t *prev = &_BLEDEV_ROOTPTR;
    bledev_info_t *tmp;
    r = pthread_mutex_lock(&_BLEDEV_ROOT_mutex);
    if (r != 0)
    {
        perror("cannot lock");
    }
    while (prev->next != NULL)
    {
        strr = strcmp(addr, (prev->next)->addr);
        if (strr == 0)
        {
            r = pthread_mutex_unlock(&_BLEDEV_ROOT_mutex);
            if (r != 0)
            {
                perror("cannot unlock");
            }
            return -1;
        }
        else if (strr < 0)
        {
            break;
        }
        prev = prev->next;
    }
    tmp = malloc(sizeof(bledev_info_t));
    if (tmp == NULL)
    {
        perror("no malloc\n");
        return -1;
    }
    tmp->next = NULL;
    strncpy(tmp->addr, addr, 18);
    tmp->rssi = rssi;
    tmp->time = time(NULL);
    at_gpslocget(&tmp->lat, &tmp->lon, &fix);
    if (fix < 2)
    {
        tmp->lat = 0;
        tmp->lon = 0;
    }
    tmp->next = prev->next;
    prev->next = tmp;
    r = pthread_mutex_unlock(&_BLEDEV_ROOT_mutex);
    if (r != 0)
    {
        perror("cannot unlock");
    }
    return 0;
}

bledev_info_t *bledev_listget(void)
{
    int r;
    r = pthread_mutex_lock(&_BLEDEV_ROOT_mutex);
    if (r != 0)
    {
        perror("cannot lock");
    }
    return _BLEDEV_ROOTPTR.next;
}

int bledev_listrelease(void)
{
    int r;
    bledev_info_t *tmp, *prev = _BLEDEV_ROOTPTR.next;
    while (prev != NULL)
    {
        tmp = prev;
        prev = prev->next;
        free(tmp);
    }
    _BLEDEV_ROOTPTR.next = NULL;
    r = pthread_mutex_unlock(&_BLEDEV_ROOT_mutex);
    if (r != 0)
    {
        perror("cannot lock");
        return -1;
    }
    return 0;
}

int bledev_listsize(void)
{
    int cnt=0;

    bledev_info_t *prev = _BLEDEV_ROOTPTR.next;
    while (prev != NULL)
    {
        cnt++;
        prev = prev->next;
    }
    return cnt;
}