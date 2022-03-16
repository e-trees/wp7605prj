#ifndef _CURL_MODULE_H
#define _CURL_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

int curl_req(char *url);
char *curl_urlenc(char *str);
#endif