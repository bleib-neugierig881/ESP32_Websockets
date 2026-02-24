# ESP32-S3-ETH Websocket Dev

Pulling in all kinds of code. Ultimately want to produce a websocket server that can push live controller data to a client page.
Will post useful resources I find along the way here.

## Design Resources

ESP32-S3 Wiki from Espressif:<https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/api-conventions.html>

ESP32-S3-ETH ref page. Look at schematic for the SPI connections to the W5500 ethernet controller (_lower left corner under **POE**_).

## Code References

Built using Async TCP lib from ESP32Async, referenced their Websocket example to get started on websockets.

Used Ethernet library example from ESP32S3 Dev Module Board package to get ethernet functioning.

Waveshare's wiki had a demo code folder but when I downloaded/extracted it the ".ino" files for ethernet examples did not appear to exist.

## Troubleshooting

Post #7 in this forum was super helpful for remedying a dumb redundant library naming issue: [Latest ESP Async Webserver Library Breaks Working Sketches]

[latest esp async webserver library breaks working sketches]: https://forum.arduino.cc/t/latest-esp-async-webserver-library-breaks-working-sketches/1366271/7
