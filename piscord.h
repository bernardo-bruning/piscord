/**
 * piscord.h - v0.1.0 - public domain - Bernardo de Oliveira Bruning, February 2026
 *
 * piscord is a minimalist, single-header C library for interacting with the Discord API.
 * It is designed to be highly portable and dependency-agnostic by using a callback-based
 * backend system. You provide the HTTP and JSON implementations, and piscord
 * handles the Discord-specific logic.
 *
 * USAGE:
 *   #define PISCORD_IMPLEMENTATION
 *   #include "piscord.h"
 *
 *   // 1. Initialize the bot
 *   Piscord bot;
 *   piscord_init(&bot, "YOUR_TOKEN", "GUILD_ID", "CHANNEL_ID");
 *
 *   // 2. Register your backends (e.g., using libcurl and cJSON)
 *   bot.http_request = my_curl_request;
 *   bot.json_encode = my_cjson_encode;
 *   bot.json_decode_array = my_cjson_decode_array;
 *
 *   // 3. Use the API
 *   piscord_send_message(&bot, "Hello world!");
 *
 * BACKEND SYSTEM:
 *   This library does NOT include networking or JSON parsing code by default.
 *   You must assign function pointers to the `Piscord` struct for:
 *     - http_request: Perform GET/POST HTTPS requests.
 *     - json_encode: Serialize a list of `JsonField` to a JSON string.
 *     - json_decode_array: Parse a JSON array of objects into `JsonField` structures.
 *
 *   This allows you to use any library (libcurl, mbedtls, cJSON, Jansson, etc.)
 *   or even platform-specific APIs without modifying piscord.h.
 *
 * LICENSE:
 *   This software is in the public domain. Where that dedication is not recognized, 
 *   you are granted a perpetual, irrevocable license to copy, distribute, 
 *   and modify this file as you see fit.
 */

#ifndef PISCORD_H
#define PISCORD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef PISCORD_BUFFER_SIZE
  #define PISCORD_BUFFER_SIZE 2048
#endif

/* --- Constants --- */

#define PISCORD_HTTP_GET 0
#define PISCORD_HTTP_POST 1

const int PISCORD_SUCCESS = 1;
const int PISCORD_FAILURE = 0;
const int PISCORD_ERR_HTTP_ERROR = -1;
const int PISCORD_ERR_JSON_ENCODE = -2;

#define PISCORD_JSON_STR_TYPE 0
#define PISCORD_JSON_INT_TYPE 1
#define PISCORD_JSON_BOOL_TYPE 2
#define PISCORD_JSON_ARRAY_TYPE 3

/* --- Types --- */

typedef struct HttpHeader {
  char *name, *value;
} HttpHeader;

typedef struct JsonField {
  char *name;
  int type;
  void *target;
  int max;
} JsonField;

typedef struct PiscordMessage {
  char content[PISCORD_BUFFER_SIZE];
  char author[64];
  char id[64];
} PiscordMessage;

typedef struct Piscord Piscord;

/**
 * Backend callback types.
 */
typedef int (*PiscordHttpRequestFn)(Piscord *self, char *url, int method, HttpHeader *headers, int headers_len, char *body, char *response, int len);
typedef int (*PiscordJsonEncodeFn)(Piscord *self, char *buf, size_t len, struct JsonField *fields, int num);
typedef int (*PiscordJsonDecodeArrayFn)(Piscord *self, char *buf, int len, int num_objects, int num_fields, struct JsonField *fields_array);

struct Piscord {
  char *token, *guild_id, *channel_id, *url;
  char last_message_id[64];
  void *user_data; /* Reserved for client-side context */
  
  /* Callbacks - MUST be set by the user before calling API functions */
  PiscordHttpRequestFn http_request;
  PiscordJsonEncodeFn json_encode;
  PiscordJsonDecodeArrayFn json_decode_array;
};

/* --- Public API --- */

/**
 * Initializes the Piscord context with Discord credentials.
 */
void piscord_init(struct Piscord *self, char *token, char *guild_id, char *channel_id);

/**
 * Sends a plain text message to the configured channel.
 * Returns PISCORD_SUCCESS (1) on success.
 */
int piscord_send_message(struct Piscord *self, char *message);

/**
 * Fetches new messages from the configured channel.
 * Only returns messages sent AFTER the last one received by this instance.
 * Returns the number of messages received, or PISCORD_FAILURE (0) if none.
 */
int piscord_recv_message(struct Piscord *self, PiscordMessage *messages, int num_messages);

#endif /* PISCORD_H */

/* --- Implementation --- */

#ifdef PISCORD_IMPLEMENTATION

void piscord_init(Piscord *self, char *token, char *guild_id, char *channel_id) {
  self->url = "https://discord.com/api/v10";
  self->token = token;
  self->guild_id = guild_id;
  self->channel_id = channel_id;
  self->last_message_id[0] = '\0';
  self->user_data = NULL;
  self->http_request = NULL;
  self->json_encode = NULL;
  self->json_decode_array = NULL;
}

int piscord_send_message(struct Piscord *self, char *message) {
  char response[PISCORD_BUFFER_SIZE], 
        message_request[PISCORD_BUFFER_SIZE], 
        url[PISCORD_BUFFER_SIZE],
        token[PISCORD_BUFFER_SIZE];
  int err;

  snprintf(token, sizeof(token), "Bot %s", self->token);
  snprintf(url, sizeof(url), "%s/channels/%s/messages", self->url, self->channel_id);

  HttpHeader headers[] = {
    {"Authorization", token},
    {"Content-Type", "application/json"},
    {"User-Agent", "Piscord (https://github.com/bernardo-bruning/piscord, 0.1.0)"}
  };
  JsonField fields[] = {
    {"content", PISCORD_JSON_STR_TYPE, message, PISCORD_BUFFER_SIZE}
  };

  if (!self->json_encode) return PISCORD_ERR_JSON_ENCODE;
  err = self->json_encode(self, message_request, PISCORD_BUFFER_SIZE, fields, 1);
  if (!err) return PISCORD_ERR_JSON_ENCODE;

  if (!self->http_request) return PISCORD_ERR_HTTP_ERROR;
  if (!self->http_request(self, url, PISCORD_HTTP_POST, headers, 3, message_request, response, PISCORD_BUFFER_SIZE)) {
    return PISCORD_ERR_HTTP_ERROR;
  }

  return PISCORD_SUCCESS;
}

int piscord_recv_message(struct Piscord *self, PiscordMessage *messages, int num_messages) {
  char response[PISCORD_BUFFER_SIZE * 4], 
        url[PISCORD_BUFFER_SIZE],
        token[PISCORD_BUFFER_SIZE];
  HttpHeader headers[] = {
    {"Authorization", token},
    {"Accept", "application/json"},
    {"User-Agent", "Piscord (https://github.com/bernardo-bruning/piscord, 0.1.0)"}
  };
  
  JsonField fields[num_messages * 3];
  for (int i = 0; i < num_messages; i++) {
    JsonField *f = &fields[i * 3];
    f[0] = (JsonField){ "content",         PISCORD_JSON_STR_TYPE, messages[i].content, PISCORD_BUFFER_SIZE };
    f[1] = (JsonField){ "author.username", PISCORD_JSON_STR_TYPE, messages[i].author,  64 };
    f[2] = (JsonField){ "id",              PISCORD_JSON_STR_TYPE, messages[i].id,      64 };
  }

  snprintf(token, sizeof(token), "Bot %s", self->token);
  
  if (self->last_message_id[0] == '\0') {
    snprintf(url, sizeof(url), "%s/channels/%s/messages?limit=%d", self->url, self->channel_id, num_messages);
  } else {
    snprintf(url, sizeof(url), "%s/channels/%s/messages?limit=%d&after=%s", self->url, self->channel_id, num_messages, self->last_message_id);
  }

  if (!self->http_request) return PISCORD_ERR_HTTP_ERROR;
  if (!self->http_request(self, url, PISCORD_HTTP_GET, headers, 3, NULL, response, sizeof(response))) {
    return PISCORD_ERR_HTTP_ERROR;
  }

  if (!self->json_decode_array) return PISCORD_FAILURE;
  int count = self->json_decode_array(self, response, sizeof(response), num_messages, 3, fields);

  if (count < 1) {
    return PISCORD_FAILURE;
  }

  strcpy(self->last_message_id, messages[0].id);

  return count;
}
#endif /* PISCORD_IMPLEMENTATION */
