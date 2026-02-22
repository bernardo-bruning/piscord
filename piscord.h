/**
 * microdiscord.h - v0.1.0 - public domain - Bernardo de Olveira Bruning, February 2026
 */

#ifndef PISCORD_H
#define PISCORD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef PISCORD_BUFFER_SIZE
  #define PISCORD_BUFFER_SIZE 2048
#endif

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

typedef int (*PiscordHttpRequestFn)(Piscord *self, char *url, int method, HttpHeader *headers, int headers_len, char *body, char *response, int len);
typedef int (*PiscordJsonEncodeFn)(Piscord *self, char *buf, size_t len, struct JsonField *fields, int num);
typedef int (*PiscordJsonDecodeArrayFn)(Piscord *self, char *buf, int len, int num_objects, int num_fields, struct JsonField *fields_array);

struct Piscord {
  char *token, *guild_id, *channel_id, *url;
  char last_message_id[64];
  void *user_data;
  
  PiscordHttpRequestFn http_request;
  PiscordJsonEncodeFn json_encode;
  PiscordJsonDecodeArrayFn json_decode_array;
};

void piscord_init(struct Piscord *self, char *token, char *guild_id, char *channel_id);
int piscord_send_message(struct Piscord *self, char *message);
int piscord_recv_message(struct Piscord *self, PiscordMessage *messages, int num_messages);

#endif // PISCORD_H

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
    {"User-Agent", "Piscord (https://github.com/bernardo-bruning/microdiscord, 0.1.0)"}
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
    {"User-Agent", "Piscord (https://github.com/bernardo-bruning/microdiscord, 0.1.0)"}
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
    snprintf(url, sizeof(url), "%s/channels/%s/messages?limit=1", self->url, self->channel_id, num_messages);
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
#endif // PISCORD_IMPLEMENTATION
