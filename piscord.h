/**
 * microdiscord.h - v0.1.0 - public domain - Bernardo de Olveira Bruning, February 2026
 */

#ifndef PISCORD_H
#define PISCORD_H

#ifndef PISCORD_CONTEXT
  #define PISCORD_CONTEXT void *
#endif

// #ifndef PISCORD_HTTP_REQUEST
//   #error "You must define PISCORD_HTTP_REQUEST(ctx, url, method, headers, body, response, len) before including microdiscord.h"
// #endif
//
// #ifndef PISCORD_JSON_ENCODE
//   #error "You must define PISCORD_JSON_ENCODE(ctx, buf, len, fields, num_fields) before including microdiscord.h"
// #endif
//
// #ifndef PISCORD_JSON_DECODE
//   #error "You must define PISCORD_JSON_DECODE(ctx, json, buf, len) before including microdiscord.h"
// #endif

#ifndef PISCORD_SPRINTF
  #include <stdio.h>
  #define PISCORD_SPRINTF(ctx, buf, len, fmt, ...) snprintf(buf, len, fmt, __VA_ARGS__)
#endif

#ifndef PISCORD_BUFFER_SIZE
  #define PISCORD_BUFFER_SIZE 2048
#endif

const int PISCORD_HTTP_GET = 0;
const int PISCORD_HTTP_POST = 1;

const int HTTP_OK = 200;

const int PISCORD_SUCCESS = 0;
const int PISCORD_ERR_HTTP_ERROR = 1;
const int PISCORD_ERR_JSON_ENCODE = 2;

typedef struct HttpHeader {
  char *name, *value;
} HttpHeader;

typedef struct JsonField {
  char *name;
  char *type;
  void *target;
  int max;
} JsonField;

typedef struct Piscord {
  char *token, *guild_id, *channel_id, *url;
} Piscord;

void piscord_init(struct Piscord *self, char *token, char *guild_id, char *channel_id);
int piscord_send_message(struct Piscord *self, PISCORD_CONTEXT ctx, char *message);
int piscord_recv_message(struct Piscord *self, PISCORD_CONTEXT ctx, char *message);


#endif // PISCORD_H

#ifdef PISCORD_IMPLEMENTATION

void piscord_init(Piscord *self, char *token, char *guild_id, char *channel_id) {
  self->url = "https://discord.com/api/v10";
  self->token = token;
  self->guild_id = guild_id;
  self->channel_id = channel_id;
}

int piscord_send_message(struct Piscord *self, PISCORD_CONTEXT ctx, char *message) {
  char response[PISCORD_BUFFER_SIZE], 
        message_request[PISCORD_BUFFER_SIZE], 
        url[PISCORD_BUFFER_SIZE],
        token[PISCORD_BUFFER_SIZE];
  int status, err;

  PISCORD_SPRINTF(ctx, token, sizeof(token), "Bot %s", self->token);
  PISCORD_SPRINTF(ctx, url, sizeof(url), "%s/channels/%s/messages", self->url, self->channel_id);

  HttpHeader headers[] = {
    {"Authorization", token},
    {"Content-Type", "application/json"}
  };
  JsonField fields[] = {
    {"content", "string", message, PISCORD_BUFFER_SIZE}
  };

  err = PISCORD_JSON_ENCODE(ctx, message_request, PISCORD_BUFFER_SIZE, fields, 1);
  if (!err) return PISCORD_ERR_JSON_ENCODE;

  status = PISCORD_HTTP_REQUEST(ctx, url, PISCORD_HTTP_POST, headers, message_request, response, sizeof(response));
  if (status != HTTP_OK) return PISCORD_ERR_HTTP_ERROR;

  return PISCORD_SUCCESS;
}

int piscord_recv_message(struct Piscord *self, PISCORD_CONTEXT ctx, char *message) {
  // TODO: implement
  return PISCORD_SUCCESS;
}
#endif // PISCORD_IMPLEMENTATION
