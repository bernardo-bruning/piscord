#include <stdio.h>
#include <string.h>

#define PISCORD_IMPLEMENTATION
#define PISCORD_HTTP_REQUEST(ctx, url, method, headers, body, response, len) (printf("HTTP REQUEST: %s\n", url), 200)
#define PISCORD_JSON_ENCODE(ctx, buf, len, fields, num) (printf("JSON ENCODE\n"), 1)
#define PISCORD_JSON_DECODE(ctx, json, buf, len) (printf("JSON DECODE\n"), 0)
#define PISCORD_SPRINTF(ctx, buf, len, fmt, ...) snprintf(buf, len, fmt, __VA_ARGS__)

#include "piscord.h"

int my_json_encode(PISCORD_CONTEXT *ctx, char *buf, size_t len, JsonField *fields, int num) {
  for (int i = 0; i < num; i++) {
    snprintf(buf, len, "%s: %s", fields[i].name, fields[i].target);
  }
  printf("JSON: %s\n", buf);
  return PISCORD_SUCCESS;
}

int main(int argc, char *argv[]) {
  struct Piscord discord;
  piscord_init(&discord, "YOUR_TOKEN", "GUILD_ID", "CHANNEL_ID");
  
  printf("Sending message...\n");
  int result = piscord_send_message(&discord, NULL, "Hello from microdiscord!");
  
  if (result == PISCORD_SUCCESS) {
    printf("Message sent successfully!\n");
  } else {
    printf("Failed to send message: %d\n", result);
  }

  return 0;
}
