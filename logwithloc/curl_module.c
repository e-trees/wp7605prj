#include "curl_module.h"

struct Buffer
{
    char *data;
    int data_size;
};

static size_t buffer_writer(char *ptr, size_t size, size_t nmemb, void *stream)
{
    struct Buffer *buf = (struct Buffer *)stream;
    int block = size * nmemb;
    if (!buf)
    {
        return block;
    }

    if (!buf->data)
    {
        buf->data = (char *)malloc(block);
    }
    else
    {
        buf->data = (char *)realloc(buf->data, buf->data_size + block);
    }

    if (buf->data)
    {
        memcpy(buf->data + buf->data_size, ptr, block);
        buf->data_size += block;
    }

    return block;
}

char *curl_urlenc(char *str)
{
    CURL *curl;
    curl = curl_easy_init();
    char *enc = curl_easy_escape(curl, str, strlen(str));
    curl_easy_cleanup(curl);
    return enc;
}

int curl_req(char *url)
{

    CURL *curl;
    struct Buffer *buf;

    buf = (struct Buffer *)malloc(sizeof(struct Buffer));
    buf->data = NULL;
    buf->data_size = 0;

    curl = curl_easy_init();
    printf("curl url [%s]\n", url);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buffer_writer);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (buf->data)
    {
        printf("curl get result [%s]\n", buf->data);
        free(buf->data);
    }

    free(buf);

    return EXIT_SUCCESS;
}