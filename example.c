#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PISCORD_IMPLEMENTATION
#include "piscord.h"
#include "backend/libcurl_http.h"
#include "backend/cjson_json.h"

void piscord_backend_curl_cjson_init(Piscord *self) {
    self->http_request = libcurl_http_request;
    self->json_encode = cjson_encode;
    self->json_decode_array = cjson_decode_array;
}

int main(int argc, char *argv[]) {
    struct Piscord discord;
    
    char *token = getenv("DISCORD_TOKEN");
    char *guild_id = getenv("DISCORD_GUILD_ID");
    char *channel_id = getenv("DISCORD_CHANNEL_ID");

    if (!token || !guild_id || !channel_id) {
        fprintf(stderr, "Please set DISCORD_TOKEN, DISCORD_GUILD_ID, and DISCORD_CHANNEL_ID environment variables.\n");
        return 1;
    }

    piscord_init(&discord, token, guild_id, channel_id);
    piscord_backend_curl_cjson_init(&discord);

    printf("Bot is running. Send 'ping' in the channel to see a response.\n");
    PiscordMessage msg_buffer[5];

    while (1) {
        int count = piscord_recv_message(&discord, msg_buffer, 5);
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                printf("Received message from %s: %s\n", msg_buffer[i].author, msg_buffer[i].content);
                
                if (strcmp(msg_buffer[i].content, "ping") == 0) {
                    printf("Responding with pong to %s...\n", msg_buffer[i].author);
                    if (!piscord_send_message(&discord, "pong")) {
                        fprintf(stderr, "Failed to send message!\n");
                    }
                }
            }
        }
        sleep(1);
    }

    return 0;
}
