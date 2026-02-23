#define PISCORD_IMPLEMENTATION
#include "../piscord.h"
#include "../includes/tap-h/tap.h"

/* Mock Backends */
static int mock_http_request(Piscord *self, char *url, int method, HttpHeader *headers, int headers_len, char *body, char *response, int len) {
    piscord_internal_strcpy(response, "[{\"content\":\"test\", \"author\":{\"username\":\"user\"}, \"id\":\"123\"}]");
    return 1;
}

static int mock_json_encode(Piscord *self, char *buf, int len, struct JsonField *fields, int num) {
    return 1;
}

static int mock_json_decode_array_empty(Piscord *self, char *buf, int len, int num_objects, int num_fields, struct JsonField *fields_array) {
    return 0;
}

static int mock_json_decode_array_single(Piscord *self, char *buf, int len, int num_objects, int num_fields, struct JsonField *fields_array) {
    /* fields_array layout is [content, author, id] per message */
    piscord_internal_strcpy((char*)fields_array[0].target, "hello");
    piscord_internal_strcpy((char*)fields_array[1].target, "author");
    piscord_internal_strcpy((char*)fields_array[2].target, "999");
    return 1;
}

static int message_callback_called = 0;
static void on_message(Piscord *self, PiscordMessage *msg) {
    message_callback_called++;
}

int main() {
    plan(7);

    Piscord bot;
    
    /* Test 1: Init */
    piscord_init(&bot, "token", "guild", "channel", 
                 mock_http_request, mock_json_encode, mock_json_decode_array_empty);
    
    ok(bot.token != NULL, "Bot token is initialized");
    ok(bot.http_request == mock_http_request, "HTTP backend is injected");
    
    /* Test 2: Empty Poll */
    bot.on_message = on_message;
    int count = piscord_poll(&bot);
    ok(count == 0, "Poll returns 0 messages for empty response");
    ok(message_callback_called == 0, "OnMessage callback not called for empty response");

    /* Test 3: Successful Poll */
    message_callback_called = 0;
    bot.json_decode_array = mock_json_decode_array_single;
    count = piscord_poll(&bot);
    
    ok(count == 1, "Poll returns 1 message");
    ok(message_callback_called == 1, "OnMessage callback called once");
    
    /* Verify last_message_id update */
    int id_matches = 1;
    const char *expected_id = "999";
    for(int i=0; i<3; i++) if(bot.last_message_id.data[i] != expected_id[i]) id_matches = 0;
    
    ok(id_matches, "last_message_id was updated to 999");

    return done_testing();
}
