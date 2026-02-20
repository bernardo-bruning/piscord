#ifndef MICRODISCORD_H
#define MICRODISCORD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define SUCCESS 1
#define ERR_BACKEND_NULL 1
#define ERR_HTTP_ERROR 2

typedef struct Backend Backend;
typedef struct Backend {
  int (*get)(Backend *self, const char *url, const char *headers, char **response);
  int (*post)(Backend *self, const char *url, const char *headers, const char *body, char **response);
  void (*sleep)(Backend *self, int ms);
  void (*free_response)(Backend *self, char *response);
  int (*json_scanf)(const char *json, const char *path, const char *fmt, ...);
  int (*json_printf)(char *buf, size_t len, const char *fmt, ...);
  void* data;
} Backend;

typedef struct Message {
  char id[64];
  char author[64];
  char content[2048];
} Message;

typedef struct MicroDiscord {
  char *token;
  char *guild_id;
  char *channel_id;
  const char *base_url;
  char last_id[64];
  Backend *backend;
} MicroDiscord;

int microdiscord_init(MicroDiscord *self, Backend *backend, char *token, char *guild_id, char *channel_id);
int microdiscord_send_message(MicroDiscord *self, char *message);
int microdiscord_recv_message(MicroDiscord *self, Message *msg);

#ifdef MICRODISCORD_IMPLEMENTATION

int microdiscord_init(MicroDiscord *self, Backend *backend, char *token, char *guild_id, char *channel_id) {
  if (!backend) return ERR_BACKEND_NULL;
  self->token = token;
  self->guild_id = guild_id;
  self->channel_id = channel_id;
  self->base_url = "https://discord.com/api/v10";
  self->backend = backend;
  self->last_id[0] = '\0';

  char *res = NULL, headers[256], url[512];
  snprintf(headers, sizeof(headers), "Authorization: Bot %s\r\n", self->token);
  snprintf(url, sizeof(url), "%s/channels/%s/messages?limit=1", self->base_url, self->channel_id);
  
  if (self->backend->get(self->backend, url, headers, &res) != SUCCESS || !res) return SUCCESS;

  self->backend->json_scanf(res, "$[0].id", "%s", self->last_id);
  self->backend->free_response(self->backend, res);
  return SUCCESS;
}

int microdiscord_send_message(MicroDiscord *self, char *message) {
  char url[512], body[2048], headers[256], *res = NULL;
  snprintf(url, sizeof(url), "%s/channels/%s/messages", self->base_url, self->channel_id);
  self->backend->json_printf(body, sizeof(body), "{%Q: %Q}", "content", message);
  snprintf(headers, sizeof(headers), "Authorization: Bot %s\r\nContent-Type: application/json\r\n", self->token);

  if (self->backend->post(self->backend, url, headers, body, &res) != SUCCESS) return ERR_HTTP_ERROR;
  if (!res) return SUCCESS;

  self->backend->json_scanf(res, "$.id", "%s", self->last_id);
  self->backend->free_response(self->backend, res);
  return SUCCESS;
}

int microdiscord_recv_message(MicroDiscord *self, Message *msg) {
  char url[512], headers[256];
  snprintf(headers, sizeof(headers), "Authorization: Bot %s\r\n", self->token);

  while (1) {
    snprintf(url, sizeof(url), "%s/channels/%s/messages?limit=1%s%s", 
             self->base_url, self->channel_id, self->last_id[0] ? "&after=" : "", self->last_id);
    
    char *res = NULL, new_id[64] = {0};
    int status = self->backend->get(self->backend, url, headers, &res);
    
    if (status != SUCCESS || !res) {
      if (res) self->backend->free_response(self->backend, res);
      self->backend->sleep(self->backend, 10000);
      continue;
    }

    if (self->backend->json_scanf(res, "$[0].id", "%s", new_id) != 1 || strcmp(new_id, self->last_id) == 0) {
      self->backend->free_response(self->backend, res);
      self->backend->sleep(self->backend, 5000);
      continue;
    }

    // Preenche a struct Message com os dados extraidos
    strcpy(self->last_id, new_id);
    strcpy(msg->id, new_id);
    self->backend->json_scanf(res, "$[0].content", "%s", msg->content);
    self->backend->json_scanf(res, "$[0].author.username", "%s", msg->author);

    self->backend->free_response(self->backend, res);
    return SUCCESS; 
  }
}

#endif // MICRODISCORD_IMPLEMENTATION
#endif // MICRODISCORD_H
