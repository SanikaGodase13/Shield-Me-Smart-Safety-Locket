#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

// Replace with your network credentials
const char* WIFI_SSID = "Tanvi";
const char* WIFI_PASS = "12345678";

WebServer server(80);

static auto hiRes = esp32cam::Resolution::find(800, 600);  // High resolution for streaming

void handleStream() {
  WiFiClient client = server.client();
  
  // HTTP response header for MJPEG streaming
  String response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);

  while (true) {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      Serial.println("CAPTURE FAIL");
      break;
    }
    
    // Prepare each JPEG frame as a part of the MJPEG stream
    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", frame->size());
    frame->writeTo(client);
    client.print("\r\n");

    Serial.printf("STREAMED FRAME: %dx%d, %d bytes\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));

    if (!client.connected()) {
      Serial.println("Client disconnected");
      break;
    }

    delay(100);  // Adjust delay for the desired frame rate (e.g., ~10 FPS with 100ms delay)
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Configure and initialize the camera
  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(hiRes);
  cfg.setBufferCount(2);
  cfg.setJpeg(80);  // Set JPEG quality (higher = better quality, more bandwidth)

  bool ok = Camera.begin(cfg);
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  if (!ok) {
    while (true) {
      delay(1000);  // Halt if the camera initialization fails
    }
  }

  // Connect to Wi-Fi
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("Stream URL: http://");
  Serial.println(WiFi.localIP());
  Serial.println("/stream");

  // Define the MJPEG streaming route
  server.on("/stream", handleStream);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  // Handle incoming client requests
}
