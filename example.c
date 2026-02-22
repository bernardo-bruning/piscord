#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PISCORD_IMPLEMENTATION
#include "piscord.h"
#include "backend/libcurl_http.h"
#include "backend/cjson_json.h"

void on_message(Piscord *self, PiscordMessage *msg) {
    printf("Received message from %s: %s\n", msg->author, msg->content);
    
    if (strcmp(msg->content, "ping") == 0) {
        printf("Responding with pong to %s...\n", msg->author);
        if (!piscord_send_message(self, "pong")) {
            fprintf(stderr, "Failed to send message!\n");
        }
    }
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

    piscord_init(&discord, token, guild_id, channel_id, libcurl_http_request, cjson_encode, cjson_decode_array);
    discord.on_message = on_message;

    printf("Bot is running. Send 'ping' in the channel to see a response.\n");

    while (1) {
        piscord_poll(&discord);
        sleep(1);
    }

    return 0;
}
