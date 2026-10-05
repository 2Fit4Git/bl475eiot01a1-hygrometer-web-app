#include "web_server.h"

#include <stdio.h>
#include <string.h>

#include "wifi.h"

#define HTTP_SOCKET 0U
#define HTTP_PORT 80U
#define HTTP_WAIT_MS 200U
#define HTTP_REQUEST_SIZE 512U
#define HTTP_BODY_SIZE 1280U
#define HTTP_HEADER_SIZE 160U

static const char html_page[] =
    "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>STM32 Hygrometer</title><style>"
    "body{font:1.1rem system-ui;max-width:34rem;margin:3rem auto;padding:0 "
    "1rem;"
    "background:#f3f6f8;color:#17212b}.card{background:white;padding:1.5rem;"
    "border-radius:1rem;box-shadow:0 3px 16px #1232}h1{margin-top:0}"
    ".value{font-size:2.2rem;font-weight:700}.muted{color:#536575}</style></"
    "head>"
    "<body><main class=\"card\"><h1>On-board hygrometer</h1>"
    "<p>Temperature</p><div class=\"value\"><span id=\"t\">--</span> "
    "&deg;C</div>"
    "<p>Relative humidity</p><div class=\"value\"><span id=\"h\">--</span> "
    "%</div>"
    "<p class=\"muted\" id=\"status\">Connecting to "
    "sensor...</p></main><script>"
    "async function update(){try{const r=await "
    "fetch('/api/readings',{cache:'no-store'});"
    "if(!r.ok)throw Error('HTTP '+r.status);const d=await r.json();"
    "document.getElementById('t').textContent=d.temperature_c.toFixed(2);"
    "document.getElementById('h').textContent=d.humidity_percent.toFixed(1);"
    "document.getElementById('status').textContent='Updated '+new "
    "Date().toLocaleTimeString()"
    "}catch(e){document.getElementById('status').textContent='Reading "
    "unavailable'}}"
    "update();setInterval(update,1000);</script></body></html>";

static int send_response(uint16_t socket, unsigned int status_code,
                         const char *reason, const char *content_type,
                         const char *body, size_t body_length) {
  char header[HTTP_HEADER_SIZE];
  int header_length;
  uint16_t sent_length;

  header_length =
      snprintf(header, sizeof(header),
               "HTTP/1.1 %u %s\r\n"
               "Content-Type: %s\r\n"
               "Content-Length: %u\r\n"
               "Connection: close\r\n"
               "Cache-Control: no-store\r\n\r\n",
               status_code, reason, content_type, (unsigned int)body_length);
  if ((header_length < 0) || ((size_t)header_length >= sizeof(header))) {
    return -1;
  }
  if (WIFI_SendData(socket, (const uint8_t *)header, (uint16_t)header_length,
                    &sent_length, 1000U) != WIFI_STATUS_OK ||
      sent_length != (uint16_t)header_length) {
    return -1;
  }
  if (WIFI_SendData(socket, (const uint8_t *)body, (uint16_t)body_length,
                    &sent_length, 1000U) != WIFI_STATUS_OK ||
      sent_length != (uint16_t)body_length) {
    return -1;
  }

  return 0;
}

int WebServer_Start(void) {
  return (WIFI_StartServer(HTTP_SOCKET, WIFI_TCP_PROTOCOL, 1U, "hygrometer",
                           HTTP_PORT) == WIFI_STATUS_OK)
             ? 0
             : -1;
}

int WebServer_Poll(const HTS221_Sample *sample) {
  uint8_t remote_ip[4];
  uint16_t remote_port;
  uint8_t request[HTTP_REQUEST_SIZE + 1U];
  uint16_t received_length;
  uint16_t socket = HTTP_SOCKET;
  WIFI_Status_t status;
  int result = 0;

  if (sample == NULL) {
    return -1;
  }

  status = WIFI_WaitServerConnection(HTTP_SOCKET, HTTP_WAIT_MS, remote_ip,
                                     sizeof(remote_ip), &remote_port);
  if (status == WIFI_STATUS_TIMEOUT) {
    return 0;
  }
  if (status != WIFI_STATUS_OK) {
    return -1;
  }

  status = WIFI_ReceiveData(socket, request, HTTP_REQUEST_SIZE,
                            &received_length, 1000U);
  if (status == WIFI_STATUS_OK && received_length > 0U) {
    char body[HTTP_BODY_SIZE];
    int body_length;

    request[received_length] = '\0';
    if (strncmp((const char *)request, "GET /api/readings ", 18U) == 0) {
      body_length = snprintf(body, sizeof(body),
                             "{\"temperature_c\":%.2f,"
                             "\"humidity_percent\":%.1f}\n",
                             (double)sample->temperature_c,
                             (double)sample->humidity_percent);
      if ((body_length < 0) || ((size_t)body_length >= sizeof(body)) ||
          send_response(socket, 200U, "OK", "application/json; charset=utf-8",
                        body, (size_t)body_length) != 0) {
        result = -1;
      }
    } else if (strncmp((const char *)request, "GET / ", 6U) == 0 ||
               strncmp((const char *)request, "GET /? ", 7U) == 0) {
      if (send_response(socket, 200U, "OK", "text/html; charset=utf-8",
                        html_page, sizeof(html_page) - 1U) != 0) {
        result = -1;
      }
    } else {
      static const char not_found[] = "Not found\n";
      if (send_response(socket, 404U, "Not Found", "text/plain; charset=utf-8",
                        not_found, sizeof(not_found) - 1U) != 0) {
        result = -1;
      }
    }
  } else if (status != WIFI_STATUS_OK) {
    result = -1;
  }

  if (WIFI_CloseServerConnection(HTTP_SOCKET) != WIFI_STATUS_OK) {
    result = -1;
  }
  return result;
}
