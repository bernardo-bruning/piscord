#include "../microdiscord.h"
#include "mongoose.h"
#include <unistd.h>
#include <stdarg.h>

struct request_ctx {
  char **res;
  bool done;
  int status;
};

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
  struct request_ctx *ctx = (struct request_ctx *) c->fn_data;
  if (ev != MG_EV_HTTP_MSG) {
    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE) ctx->done = true;
    return;
  }

  struct mg_http_message *hm = (struct mg_http_message *) ev_data;
  ctx->status = hm->message.len > 12 ? atoi(hm->message.buf + 9) : 0;

  if (ctx->res && hm->body.len > 0) {
    *ctx->res = (char *) malloc(hm->body.len + 1);
    if (*ctx->res) {
      memcpy(*ctx->res, hm->body.buf, hm->body.len);
      (*ctx->res)[hm->body.len] = '\0';
    }
  }

  ctx->done = true;
  c->is_closing = 1;
}

static int mg_backend_post(Backend *self, const char *url, const char *headers, const char *body, char **response) {
  struct mg_mgr mgr;
  struct request_ctx ctx = { .res = response, .done = false, .status = 0 };
  mg_mgr_init(&mgr);

  struct mg_connection *c = mg_http_connect(&mgr, url, ev_handler, (void *)&ctx);
  if (!c) { mg_mgr_free(&mgr); return ERR_HTTP_ERROR; }

  if (strncmp(url, "https://", 8) == 0) {
    struct mg_tls_opts opts = {0};
    mg_tls_init(c, &opts);
  }

  int len = body ? (int)strlen(body) : 0;
  mg_printf(c, "POST %s HTTP/1.1\r\nHost: %.*s\r\n%sContent-Length: %d\r\n\r\n",
            url, (int)mg_url_host(url).len, mg_url_host(url).buf, headers ? headers : "", len);
  if (len > 0) mg_send(c, body, len);

  while (!ctx.done) mg_mgr_poll(&mgr, 50);
  
  if (ctx.status < 200 || ctx.status >= 300) {
    fprintf(stderr, "HTTP Status: %d\n", ctx.status);
    if (response && *response) fprintf(stderr, "Discord Response: %s\n", *response);
  }

  mg_mgr_free(&mgr);
  return (ctx.status >= 200 && ctx.status < 300) ? SUCCESS : ERR_HTTP_ERROR;
}

static int mg_backend_get(Backend *self, const char *url, const char *headers, char **response) {
  struct mg_mgr mgr;
  struct request_ctx ctx = { .res = response, .done = false, .status = 0 };
  mg_mgr_init(&mgr);

  struct mg_connection *c = mg_http_connect(&mgr, url, ev_handler, (void *)&ctx);
  if (!c) { mg_mgr_free(&mgr); return ERR_HTTP_ERROR; }

  if (strncmp(url, "https://", 8) == 0) {
    struct mg_tls_opts opts = {0};
    mg_tls_init(c, &opts);
  }

  mg_printf(c, "GET %s HTTP/1.1\r\nHost: %.*s\r\n%s\r\n", 
            url, (int)mg_url_host(url).len, mg_url_host(url).buf, headers ? headers : "");

  while (!ctx.done) mg_mgr_poll(&mgr, 50);
  mg_mgr_free(&mgr);
  return (ctx.status >= 200 && ctx.status < 300) ? SUCCESS : ERR_HTTP_ERROR;
}

static void mg_backend_free_response(Backend *self, char *response) {
  free(response);
}

static void mg_backend_sleep(Backend *self, int ms) {
  usleep(ms * 1000);
}

static int mg_backend_json_scanf(const char *json, const char *path, const char *fmt, ...) {
  if (!json) return 0;
  struct mg_str s = mg_str(json);
  va_list ap;
  va_start(ap, fmt);
  int res = 0;

  if (strcmp(fmt, "%s") == 0) {
    char *ptr = va_arg(ap, char *), *val = mg_json_get_str(s, path);
    if (val) { strcpy(ptr, val); free(val); res = 1; }
  } else if (strcmp(fmt, "%d") == 0) {
    int *ptr = va_arg(ap, int *);
    long val = mg_json_get_long(s, path, -32768);
    if (val != -32768) { *ptr = (int)val; res = 1; }
  }
  va_end(ap);
  return res;
}

static int mg_backend_json_printf(char *buf, size_t len, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int res = (int) mg_vsnprintf(buf, len, fmt, &ap); // Fix: Mongoose vsnprintf para %Q
  va_end(ap);
  return res;
}

Backend* microdiscord_mongoose_backend_new() {
  Backend *b = (Backend *) malloc(sizeof(Backend));
  b->get = mg_backend_get; b->post = mg_backend_post; b->sleep = mg_backend_sleep;
  b->json_scanf = mg_backend_json_scanf; b->json_printf = mg_backend_json_printf;
  b->free_response = mg_backend_free_response; b->data = NULL;
  return b;
}
