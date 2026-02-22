# microdiscord

A minimalist, single-header C library for interacting with the Discord API.

`microdiscord` follows the **STB-style library** philosophy: it's a single header file that provides the core logic while remaining completely dependency-agnostic. You bring your own networking (HTTP) and JSON parsing backends.

## Features

- **Single Header**: Easy to integrate into any project.
- **Dependency-Agnostic**: Use `libcurl`, `mbedtls`, `cJSON`, or any other library of your choice.
- **Minimal Footprint**: Designed for embedded systems or small utility tools.
- **Modular Backends**: Easily swap HTTP or JSON implementations without touching the core logic.

## Quick Start

1. Include `piscord.h` in your project.
2. Define `PISCORD_IMPLEMENTATION` in one C file before including the header.
3. Provide your backend callbacks.

```c
#define PISCORD_IMPLEMENTATION
#include "piscord.h"
#include "backend/libcurl_http.h"
#include "backend/cjson_json.h"

int main() {
    Piscord bot;
    piscord_init(&bot, "TOKEN", "GUILD_ID", "CHANNEL_ID");
    
    // Set up backends
    bot.http_request = libcurl_http_request;
    bot.json_encode = cjson_encode;
    bot.json_decode_array = cjson_decode_array;

    piscord_send_message(&bot, "Hello from microdiscord!");
    return 0;
}
```

## Compilation

The project uses `pkg-config` to manage dependencies for the example backends (`libcurl` and `cjson`).

```bash
make
export DISCORD_TOKEN="your_token"
export DISCORD_GUILD_ID="your_guild_id"
export DISCORD_CHANNEL_ID="your_channel_id"
./piscord
```

## License

This project is in the public domain. See `piscord.h` for more details.
