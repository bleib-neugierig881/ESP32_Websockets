/*
    This sketch shows the Ethernet event usage
    THE WEBSOCKET PORTION HAS NOT YET BEEN TESTED.

*/

#include <Arduino.h>
#include <ETH.h>
#include <SPI.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"


#ifndef ETH_PHY_CS   
#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 1
#define ETH_PHY_CS   14
#define ETH_PHY_IRQ  10
#define ETH_PHY_RST  9
#endif

// SPI pins
#define ETH_SPI_SCK  13
#define ETH_SPI_MISO 12
#define ETH_SPI_MOSI 11

static bool eth_connected = false;

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


void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-eth0");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: Serial.println("ETH Connected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:    Serial.printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif)); Serial.println(ETH);
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("ETH Lost IP");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default: break;
  }
}

// void testClient(const char *host, uint16_t port) {
//   Serial.print("\nconnecting to ");
//   Serial.println(host);

//   NetworkClient client;
//   if (!client.connect(host, port)) {
//     Serial.println("connection failed");
//     return;
//   }
//   client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
//   while (client.connected() && !client.available());
//   while (client.available()) {
//     Serial.write(client.read());
//   }

//   Serial.println("closing connection\n");
//   client.stop();
// }

IPAddress stIP(192,168,0,4);
IPAddress stGateway(192,168,0,1);
IPAddress stSubnet(255,255,255,0);

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);
  Network.onEvent(onEvent);

  SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);
  ETH.config(stIP,stGateway,stSubnet);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", (const uint8_t *)htmlContent, htmlContentLength);
  });
#if USE_TWO_ETH_PORTS
  ETH1.begin(ETH1_PHY_TYPE, ETH1_PHY_ADDR, ETH1_PHY_CS, ETH1_PHY_IRQ, ETH1_PHY_RST, SPI);
#endif
}

void loop() {
  if (eth_connected) {

    //testClient("google.com", 80);
  }
  delay(10000);
}
