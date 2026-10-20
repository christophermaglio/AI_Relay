#include <WiFi.h>
#include <WebServer.h>
#include <SuperDMZ.h>

// ===== CONFIGURATION =====
const char* WIFI_SSID = "***Insert your SSID here***";
const char* WIFI_PASSWORD = "***Insert your Password here***";
const char* SUPERDMZ_TOKEN ="***Insert you SuperDMZ Token here";

// ===== SERVER =====
WebServer server(80);
SuperDMZ tunnel;

String latestMessage = "No messages posted yet.";

// ===== WEB PAGE =====
const char webpage[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>PUBLIC MESSAGE BOARD</title>
<style>
body {
  font-family: Arial, sans-serif;
  max-width: 600px;
  margin: 40px auto;
  padding: 15px;
  background: #f5f5f5;
}
textarea {
  width: 100%;
  box-sizing: border-box;
  height: 100px;
  padding: 12px;
  font-size: 16px;
}
button {
  margin-top: 10px;
  padding: 12px 24px;
  font-size: 16px;
  cursor: pointer;
}
#latest {
  background: white;
  padding: 15px;
  min-height: 50px;
  overflow-wrap: anywhere;
  white-space: pre-wrap;
}
</style>
</head>
<body>

<h2>PUBLIC MESSAGE BOARD</h2>
<pre>Max Characters = 7400</pre>

<textarea id="message" maxlength="7400"
  placeholder="Write a public message..."></textarea>

<br>
<button id="postButton" onclick="postMessage()">Post</button>

<h3>LATEST MESSAGE</h3>
<div id="latest">Loading...</div>

<script>
async function loadMessage() {
  try {
    const response = await fetch("/message");

    if (!response.ok) {
      throw new Error("HTTP " + response.status);
    }

    const result = await response.json();

    document.getElementById("latest").textContent =
      result.message;
  } catch (error) {
    console.error("Load failed:", error);
    document.getElementById("latest").textContent =
      "Unable to load message.";
  }
}

async function postMessage() {
  const box = document.getElementById("message");
  const button = document.getElementById("postButton");
  const message = box.value;

  if (!message.trim()) {
    alert("Please enter a message.");
    return;
  }

  button.disabled = true;
  button.textContent = "Posting...";

  try {
    // GET workaround: SuperDMZ's nginx edge rejects POST (405),
    // so the message goes as a URL-encoded query parameter.
    const response = await fetch(
      "/post?message=" + encodeURIComponent(message)
    );

    const result = await response.text();

    if (!response.ok) {
      throw new Error(
        "HTTP " + response.status + ": " + result
      );
    }

    box.value = "";
    await loadMessage();

  } catch (error) {
    console.error("POST error:", error);
    alert("POST failed: " + error.message);
  } finally {
    button.disabled = false;
    button.textContent = "Post";
  }
}

loadMessage();
</script>

</body>
</html>
)HTML";

// ===== JSON ESCAPING =====
String jsonEscape(const String& input) {
  String output;
  output.reserve(input.length() + 16);

  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];

    switch (c) {
      case '"': output += "\\\""; break;
      case '\\': output += "\\\\"; break;
      case '\n': output += "\\n"; break;
      case '\r': output += "\\r"; break;
      case '\t': output += "\\t"; break;
      default:
        if ((uint8_t)c >= 0x20) {
          output += c;
        }
        break;
    }
  }

  return output;
}

// ===== HOME PAGE =====
void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", webpage);
}

// ===== GET LATEST MESSAGE =====
void handleMessage() {
  String json = "{\"message\":\"";
  json += jsonEscape(latestMessage);
  json += "\"}";

  server.send(200, "application/json; charset=utf-8", json);
}

// ===== POST NEW MESSAGE (via GET query param — SuperDMZ nginx blocks POST) =====
void handlePost() {
  if (!server.hasArg("message")) {
    server.send(400, "text/plain",
                "No message received in request");
    return;
  }

  String message = server.arg("message");
  message.trim();

  if (message.length() == 0) {
    server.send(400, "text/plain",
                "Message cannot be empty");
    return;
  }

  if (message.length() > 7400) {
    server.send(413, "text/plain",
                "Message too long; maximum is 500 characters");
    return;
  }

  // Replace the previous message in RAM.
  latestMessage = message;

  server.send(200, "text/plain", "Message posted successfully");
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wi-Fi connected. LAN IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/message", HTTP_GET, handleMessage);
  server.on("/post", HTTP_GET, handlePost);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Route not found");
  });

  server.begin();
  Serial.println("Web server started on port 80");

  tunnel.onStatus([](bool online, const char* publicUrl) {
    Serial.print("SuperDMZ status: ");
    Serial.println(online ? "ONLINE" : "OFFLINE");

    if (online) {
      Serial.print("Public URL: ");
      Serial.println(publicUrl);
    }
  });

  tunnel.onLog([](const char* line) {
    Serial.println(line);
  });

  bool started = tunnel.begin(SUPERDMZ_TOKEN, 80);

  Serial.print("SuperDMZ begin: ");
  Serial.println(started ? "OK" : "FAILED");
}

// ===== MAIN LOOP =====
void loop() {
  server.handleClient();
  tunnel.loop();
}
