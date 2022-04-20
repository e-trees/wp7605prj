#include <stdio.h>
#include "bledev_mgr.h"
#include "blescan.h"
#include "curl_module.h"
#include <pthread.h>
#include <sys/wait.h>

#define READ (0)
#define WRITE (1)

char *filename = NULL;
char *url = NULL;

int popen2(int *fd_r, int *fd_w)
{
    int pipe_c2p[2];
    int pipe_p2c[2];
    int pid;

    if (pipe(pipe_c2p) < 0)
    {
        perror("popen2");
        return -1;
    }

    if (pipe(pipe_p2c) < 0)
    {
        perror("popen2 pgen2");
        close(pipe_c2p[READ]);
        close(pipe_c2p[WRITE]);
        return -1;
    }

    if ((pid = fork()) < 0)
    {
        perror("popen2 fork");

        close(pipe_c2p[READ]);
        close(pipe_c2p[WRITE]);

        close(pipe_p2c[READ]);
        close(pipe_p2c[WRITE]);

        return -1;
    }

    if (pid == 0)
    {
        close(pipe_p2c[WRITE]);
        close(pipe_c2p[READ]);

        dup2(pipe_p2c[READ], 0);
        dup2(pipe_c2p[WRITE], 1);

        close(pipe_p2c[READ]);
        close(pipe_c2p[WRITE]);

        if (execl("./blescan", "./blescan", NULL) < 0)
        {
            perror("popen2 exec");
            close(pipe_p2c[READ]);
            close(pipe_c2p[WRITE]);
            return -1;
        }
    }

    close(pipe_p2c[READ]);
    close(pipe_c2p[WRITE]);

    *fd_r = pipe_c2p[READ];
    *fd_w = pipe_p2c[WRITE];

    return pid;
}

void doCurlReq(void *arg)
{
    char *buf = (char *)arg;
    curl_postjson(url, buf);
    free(buf);
}

int main(int argc, char **argv)
{
    char buf[4096];
    char *jsonbuf;
    int size, i, cnt;
    int blescan_pid;
    FILE *logfd = NULL;
    char *p;
    int rssi;
    pthread_t handle;
    bledev_info_t *tmp, *prev;

    int interval = 5;
    int dumppreview = 0;
    // parse parameter
    for (i = 1; i < argc; i++)
    {
        p = argv[i];
        if (p[0] == '-')
        {
            p++;
            if (p[0] == 'u')
            {
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    url = argv[++i];
                }
            }
            else if (p[0] == 't')
            {
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    interval = atoi(argv[++i]);
                }
            }
            else if (p[0] == 'f')
            {
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    filename = argv[++i];
                }
            }
            else if (p[0] == 'p')
            {
                dumppreview = 1;
            }
            else if (p[0] == 'h')
            {
                printf("-u : Post Request URL\n");
                printf("-t : Dump Interval\n");
                printf("-f : Save File Name\n");
                printf("-p : Dump Preview\n");
                printf("-h : Help\n");
            }
        }
    }

    printf("interval[%d] URL[%s] logfile[%s]\n", interval, url, filename);

    if (filename)
    {
        logfd = fopen(filename, "a+");
        if (logfd == NULL)
        {
            perror("logfile open");
            return -1;
        }
    }

    int fd_r = fileno(stdin);
    int fd_w = fileno(stdout);
    time_t t = time(NULL);

    bledev_init();
    blescan_pid = popen2(&fd_r, &fd_w);
    if(blescan_pid < 0)
    {
        return -1;
    }

    while (kill(blescan_pid, 0) == 0)
    {
        size = read(fd_r, buf, 4095);
        if (size == -1)
        {
            perror("error");
            return 1;
        }
        if (size > 0)
        {
            p = strtok(buf, "\n");
            while (p != NULL)
            {
                if (strlen(p) == 20)
                {
                    p[17] = '\0';
                    rssi = (signed char)strtol(&p[18], 0, 16);
                    bledev_add(p, rssi);
                }
                p = strtok(NULL, "\n");
            }
        }
        if (time(NULL) - t >= interval)
        {
            prev = bledev_listget();
            tmp = prev;
            cnt = 0;
            while (prev)
            {
                // addr,rssi,time,lat,lon
                if (dumppreview)
                {
                    printf("%s,%d,%ld,%f,%f\n", prev->addr, prev->rssi, prev->time, prev->lat, prev->lon);
                }
                if (logfd != NULL)
                {
                    fprintf(logfd, "%s,%d,%ld,%f,%f\n", prev->addr, prev->rssi, prev->time, prev->lat, prev->lon);
                    fflush(logfd);
                }
                prev = prev->next;
                cnt++;
            }
            printf("Dump Item %d\n", cnt);
            if (url)
            {
                prev = tmp;
                size = bledev_listsize() * 100 + 100;
                fflush(stdout);
                jsonbuf = malloc(sizeof(char) * size);
                memset(jsonbuf, 0, size);
                p = jsonbuf;
                p += sprintf(p, "{\"time\":%ld,\"data\":[", time(NULL));
                while (prev)
                {
                    p += sprintf(p, "{\"addr\":\"%s\",\"rssi\":%d,\"time\":%ld,\"lat\":%f,\"lon\":%f}%s", prev->addr, prev->rssi, prev->time, prev->lat, prev->lon, (prev->next) ? "," : "");
                    prev = prev->next;
                }
                p += sprintf(p, "]}");
                fflush(stdout);

                pthread_create(&handle, NULL, (void *)doCurlReq, jsonbuf);
                pthread_detach(handle);
            }
            bledev_listrelease();
            t = time(NULL);
        }
        waitpid(blescan_pid, NULL, WNOHANG);
    }
    printf("end");
    return 0;
}