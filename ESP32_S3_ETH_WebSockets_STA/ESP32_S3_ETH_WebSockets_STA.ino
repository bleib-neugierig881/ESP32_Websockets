/*
Unique Board Settings for ESP32 S3 ETH
Choose ESP32 S3 Dev Module
USB CDC On Boot: Enabled
CPU Freq: 240MHz
Core Debug Level: Debug
USB DFU On Boot: Disabled
Erase All Flash..: Disabled
Events Run on: Core 1
Flash Mode: QIO 80MHz
Flash Size 16MB (128Mb)
JTAG: Disabled
Arduino Runs on: Core 1
USB Firmware... "Disabled"
Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
PSRAM: OPI PSRAM
Upload Mode: UART0/Hardware CDC
Upload Speed: 921600
USB mode: Hardware CDC and JTAG
*/
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2016-2026 Hristo Gochkov, Mathieu Carbou, Emil Muratov, Will Miles

//
// WebSocket example
//

#include <Arduino.h>
#if defined(ESP32) || defined(LIBRETINY)
#include "AsyncTCP.h"
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
#include <RPAsyncTCP.h>
#include <WiFi.h>
#endif
#include "driver/temperature_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif
  temperature_sensor_handle_t temp_handle = NULL;
  temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(20, 50);
#ifdef __cplusplus
}
#endif



#include "ESPAsyncWebServer.h"

static const char *htmlContent PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
  <title>WebSocket</title>
</head>
<body>
  <h1>WebSocket Example</h1>
  <p>Open your browser console!</p>
  <input type="text" id="message" placeholder="Type a message">
  <button onclick='sendMessage()'>Send</button>
  <script>
    var ws = new WebSocket('ws://10.0.0.62/ws');
    ws.onopen = function() {
      console.log("WebSocket connected");
    };
    ws.onmessage = function(event) {
      console.log("WebSocket message: " + event.data);
    };
    ws.onclose = function() {
      console.log("WebSocket closed");
    };
    ws.onerror = function(error) {
      console.log("WebSocket error: " + error);
    };
    function sendMessage() {
      var message = document.getElementById("message").value;
      ws.send(message);
      console.log("WebSocket sent: " + message);
    }
  </script>
</body>
</html>
  )";
static const size_t htmlContentLength = strlen_P(htmlContent);

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
const char *ssid = "YOURNET";
const char *password = "YOURPASS";

void setup() {
  Serial.begin(115200);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_handle));

#if ASYNCWEBSERVER_WIFI_SUPPORTED
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to network...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
#endif

  // serves root html page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", (const uint8_t *)htmlContent, htmlContentLength);
  });

  //
  // Run in terminal 1: websocat ws://192.168.4.1/ws => should stream data
  // Run in terminal 2: websocat ws://192.168.4.1/ws => should stream data
  // Run in terminal 3: websocat ws://192.168.4.1/ws => should fail:
  //
  // To send a message to the WebSocket server:
  //
  // echo "Hello!" | websocat ws://192.168.4.1/ws
  //
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    (void)len;

    if (type == WS_EVT_CONNECT) {
      ws.textAll("new client connected");
      Serial.println("ws connect");
      client->setCloseClientOnQueueFull(false);
      client->ping();

    } else if (type == WS_EVT_DISCONNECT) {
      ws.textAll("client disconnected");
      Serial.println("ws disconnect");

    } else if (type == WS_EVT_ERROR) {
      Serial.println("ws error");

    } else if (type == WS_EVT_PONG) {
      Serial.println("ws pong");

    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      Serial.printf("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final, info->opcode);
      String msg = "";
      if (info->final && info->index == 0 && info->len == len) {
        if (info->opcode == WS_TEXT) {
          data[len] = 0;
          Serial.printf("ws text: %s\n", (char *)data);
          client->ping();
        }
      }
    }
  });

  // shows how to prevent a third WS client to connect
  server.addHandler(&ws).addMiddleware([](AsyncWebServerRequest *request, ArMiddlewareNext next) {
    // ws.count() is the current count of WS clients: this one is trying to upgrade its HTTP connection
    if (ws.count() > 1) {
      // if we have 2 clients or more, prevent the next one to connect
      request->send(503, "text/plain", "Server is busy");
    } else {
      // process next middleware and at the end the handler
      next();
    }
  });

  server.addHandler(&ws);

  server.begin();
}

static uint32_t lastWS = 0;
static uint32_t deltaWS = 500;

static uint32_t lastHeap = 0;
float tsens_out;

void loop() {
  ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
  ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));

  uint32_t now = millis();

  if (now - lastWS >= deltaWS) {
    ws.printfAll("Chip Temp %3f degC",tsens_out);
    lastWS = millis();
  }
  ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));

  if (now - lastHeap >= 2000) {
    Serial.printf("Connected clients: %u / %u total\n", ws.count(), ws.getClients().size());

    // this can be called to also set a soft limit on the number of connected clients
    ws.cleanupClients(2);  // no more than 2 clients

#ifdef ESP32
    Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
#endif
    lastHeap = now;
  }
}