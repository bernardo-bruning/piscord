#include "mongoose_backend.c"
#include "mongoose.c"
#include <assert.h>

int main() {
    char *res = NULL;
    Backend *b = microdiscord_mongoose_backend_new();
    const char *echo_url = "http://httpbin.org/post";
    const char *payload = "hello_microdiscord";

    printf("Testando backend Mongoose contra %s...\n", echo_url);
    
    int status = b->post(b, echo_url, NULL, payload, &res);

    assert(status == SUCCESS);
    assert(res != NULL);
    assert(strstr(res, payload) != NULL);

    printf("Mongoose Backend: OK\n");

    b->free_response(b, res);
    free(b);
    return 0;
}
