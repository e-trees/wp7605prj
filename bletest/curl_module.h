#ifndef _CURL_MODULE_H
#define _CURL_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define CURL_TIMEOUT 20L

int curl_req(char *url);
char *curl_urlenc(char *str);
int curl_postjson(char *url, char *jsondata);
#endif