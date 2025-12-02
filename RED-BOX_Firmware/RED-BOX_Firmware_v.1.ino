#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>       
#include <Adafruit_Sensor.h>

Adafruit_BMP085 bmp;               

const char* ssid = "RED-BOX";
const char* password = "RED-BOX-admin";

WebServer server(80);

bool logging = false;
String currentFile = "";
File logFile;
unsigned long startTime = 0;
float seaLevelPressure = 1013.25; 

float baselineAlt = 0;
float maxAlt = 0;
bool apogeeDetected = false;

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  Wire.begin(6, 7); 
  if (!bmp.begin()) {
    Serial.println("BMP180 not found!");
    while (1);
  }

  float altSum = 0;
  for (int i = 0; i < 50; i++) {
    altSum += bmp.readAltitude(seaLevelPressure);
    delay(20);
  }
  baselineAlt = altSum / 50.0;
  Serial.printf("Baseline altitude: %.2f m\n", baselineAlt);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  setupWebServer();
  server.begin();
}

void loop() {
  server.handleClient();

  if (logging && logFile) {
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 10) { 
      lastLog = millis();

      float pressure = bmp.readPressure() / 100.0F; 
      float temp = bmp.readTemperature();
      float alt = bmp.readAltitude(seaLevelPressure) - baselineAlt;
      float timestamp = (millis() - startTime) / 1000.0;

      if (alt > maxAlt) maxAlt = alt;
      if (alt < maxAlt - 5 && !apogeeDetected && maxAlt > 50) {
        apogeeDetected = true;
        logFile.println(String(timestamp, 3) + ",APOGEE," + String(maxAlt, 2));
      }

      String line = String(timestamp, 3) + "," +
                    String(pressure, 2) + "," +
                    String(temp, 2) + "," +
                    String(alt, 2);

      logFile.println(line);
      logFile.flush();
    }
  }
}

void startLogging() {
  if (logging) return;
  apogeeDetected = false;
  maxAlt = 0;

  char filename[20];
  int num = 0;
  do {
    sprintf(filename, "/flight%03d.csv", num++);
  } while (SPIFFS.exists(filename));
  
  currentFile = String(filename);
  logFile = SPIFFS.open(currentFile, "w");
  if (!logFile) return;

  logFile.println("time_s,pressure_hPa,temp_C,altitude_m,event");
  startTime = millis();
  logging = true;
}

void stopLogging() {
  if (!logging) return;
  if (logFile) {
    logFile.println(String((millis() - startTime) / 1000.0, 3) + ",LANDED,0");
    logFile.close();
  }
  logging = false;
}

void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String html = R"=====(
<!DOCTYPE html>
<html><head><title>Rocket Logger</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body {font-family: Arial; text-align: center; margin:40px;}
  button {padding:15px 30px; font-size:18px; margin:10px;}
  .file {margin:10px; padding:10px; border:1px solid #ccc; display:inline-block;}
</style>
</head>
<body>
<h1>RED-BOX</h1>
<div id="status">Status: <span id="logstat">Stopped</span></div>
<h2>Altitude: <span id="alt">0.0</span> m | Apogee: <span id="apo">0.0</span> m</h2>
<button onclick="fetch('/start')">START LOGGING</button>
<button onclick="fetch('/stop')">STOP LOGGING</button>
<h2>Files</h2>
<div id="files"></div>
<script>
setInterval(() => {
  fetch('/status').then(r=>r.text()).then(t=> {
    let d = t.split(',');
    document.getElementById('alt').innerText = d[0];
    document.getElementById('apo').innerText = d[1];
    document.getElementById('logstat').innerText = d[2];
  });
  fetch('/list').then(r=>r.text()).then(t=> {
    document.getElementById('files').innerHTML = t;
  });
}, 1000);

</script>
</body></html>
)=====";
    server.send(200, "text/html", html);
  });

  server.on("/start", []() { startLogging(); server.send(200); });
  server.on("/stop", []() { stopLogging(); server.send(200); });

  server.on("/status", []() {
    float alt = bmp.readAltitude(seaLevelPressure) - baselineAlt;
    if (alt > maxAlt) maxAlt = alt;
    String state = logging ? "Logging" : "Stopped";
    server.send(200, "text/plain", 
      String(alt, 1) + "," + String(maxAlt, 1) + "," + state);
  });

  server.on("/list", []() {
    String out = "";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
      if (String(file.name()).endsWith(".csv")) {
        out += "<div class='file'><a href='" + String(file.name()) + "'>" + 
               String(file.name()) + "</a> (" + file.size() + " bytes) " +
               "<a href='/delete?file=" + String(file.name()) + "'>[Delete]</a></div><br>";
      }
      file = root.openNextFile();
    }
    server.send(200, "text/html", out);
  });

  server.on("/delete", []() {
    String file = server.arg("file");
    if (SPIFFS.exists(file)) SPIFFS.remove(file);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  
  server.onNotFound([]() {
    if (SPIFFS.exists(server.uri())) {
      File f = SPIFFS.open(server.uri(), "r");
      server.streamFile(f, "text/csv");
      f.close();
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });
}
