#ifndef MG_TLS
#define MG_TLS 2
#endif

#define MICRODISCORD_IMPLEMENTATION
#include "microdiscord.h"
#include "backends/mongoose_backend.c"

/**
 * Exemplo de Bot com struct Message.
 * 
 * Compilacao:
 * gcc -o example example.c backends/mongoose.c -DMG_TLS=2 -lssl -lcrypto -lpthread
 */

int main(int argc, char *argv[]) {
  extern int mg_log_level;
  mg_log_level = 0;

  if (argc < 4) {
    fprintf(stderr, "Uso: %s <token> <guild_id> <channel_id>\n", argv[0]);
    return 1;
  }

  Backend *mg = microdiscord_mongoose_backend_new();
  MicroDiscord bot;
  microdiscord_init(&bot, mg, argv[1], argv[2], argv[3]);

  printf("--- MicroDiscord Bot Iniciado ---\n");

  while (1) {
    Message msg;
    printf("Aguardando...\n");
    
    if (microdiscord_recv_message(&bot, &msg) == SUCCESS) {
      printf("[%s] %s: %s\n", msg.id, msg.author, msg.content);
      
      if (strcasecmp(msg.content, "ping") == 0) {
        microdiscord_send_message(&bot, "Pong! :ping_pong:");
      }
    }
  }

  free(mg);
  return 0;
}
