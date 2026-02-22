#ifndef PISCORD_BACKEND_LIBCURL_H
#define PISCORD_BACKEND_LIBCURL_H

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

struct ResponseBuffer {
    char *data;
    size_t size;
};

static size_t libcurl_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct ResponseBuffer *mem = (struct ResponseBuffer *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0;

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

static int libcurl_http_request(Piscord *self, char *url, int method, HttpHeader *headers, int headers_len, char *body, char *response, int len) {
    CURL *curl;
    CURLcode res;
    long http_code = 0;
    struct ResponseBuffer chunk = { .data = malloc(1), .size = 0 };

    curl = curl_easy_init();
    if (!curl) return 0;

    struct curl_slist *curl_headers = NULL;
    for (int i = 0; i < headers_len; i++) {
        char header_str[PISCORD_BUFFER_SIZE];
        snprintf(header_str, sizeof(header_str), "%s: %s", headers[i].name, headers[i].value);
        curl_headers = curl_slist_append(curl_headers, header_str);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    if (method == PISCORD_HTTP_POST) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        }
    }

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (chunk.data) {
            strncpy(response, chunk.data, len - 1);
            response[len - 1] = '\0';
            
            if (http_code < 200 || http_code >= 300) {
                fprintf(stderr, "HTTP Error %ld: %s\n", http_code, chunk.data);
            }
        }
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(curl_headers);
    curl_easy_cleanup(curl);
    free(chunk.data);

    return (int)http_code >= 200 && (int)http_code < 300;
}

#endif
